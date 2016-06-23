/*
 * Copyright (c) 2015 2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Intel Corporation nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file contains the implementation of native API error injection functions.
 */

#include "nvm_management.h"
#include "device_adapter.h"
#include <persistence/lib_persistence.h>
#include "device_utilities.h"
#include "system.h"
#include "capabilities.h"

int inject_poison_error(struct device_discovery *p_discovery, NVM_UINT64 dpa,
	NVM_BOOL set_poison);
int inject_temperature_error(NVM_UINT32 device_handle, NVM_UINT64 temperature,
	NVM_BOOL enable_injection);
int inject_software_trigger(struct device_discovery *p_discovery, enum error_type type,
	NVM_BOOL enable_trigger);

/*
 * Helper function to enable error injection functionality.
 * All other injection commands will fail until this is turned on.
 */
int enable_error_injection_functionality(NVM_INT32 device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_enable_injection input;
	memset(&input, 0, sizeof (input));
	input.enable = 1;

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_INJECT_ERROR;
	cmd.sub_opcode = SUBOP_ENABLE_INJECTION;
	cmd.input_payload = &input;
	cmd.input_payload_size = sizeof (input);
	rc = ioctl_passthrough_cmd(&cmd);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to enable/disable software trigger
 */
int inject_software_trigger(struct device_discovery *p_discovery,
		enum error_type type, NVM_BOOL enable_trigger)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// Set up input payload
	struct pt_payload_sw_triggers input;
	memset(&input, 0, sizeof (input));
	switch (type)
	{
	case ERROR_TYPE_DIE_SPARING:
		if (!p_discovery->device_capabilities.die_sparing_capable)
		{
			COMMON_LOG_ERROR_F("Die sparing is not supported on dimm %u",
					p_discovery->device_handle.handle);
			rc = NVM_ERR_NOTSUPPORTED;
		}
		input.die_sparing_trigger = enable_trigger;
		break;
	case ERROR_TYPE_SPARE_ALARM:
		input.user_spare_block_alarm_trip_trigger = enable_trigger;
		break;
	case ERROR_TYPE_MEDIA_FATAL_ERROR:
		input.fatal_error_trigger = enable_trigger;
		break;
	default:
		break;
	}

	if (rc == NVM_SUCCESS)
	{
		struct fw_cmd cmd;
		memset(&cmd, 0, sizeof (cmd));
		cmd.device_handle = p_discovery->device_handle.handle;
		cmd.opcode = PT_INJECT_ERROR;
		cmd.sub_opcode = SUBOP_ERROR_SW_TRIGGERS;
		cmd.input_payload = &input;
		cmd.input_payload_size = sizeof (input);
		rc = ioctl_passthrough_cmd(&cmd);
		if (rc != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to trigger software alarm trip on dimm %u",
					cmd.device_handle);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to inject/clear a particular artificial temperature into the part
 */
int inject_temperature_error(NVM_UINT32 device_handle, NVM_UINT64 temperature,
	NVM_BOOL enable_injection)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// Set up input
	struct pt_payload_temp_err temp_err_input;
	memset(&temp_err_input, 0, sizeof (temp_err_input));
	temp_err_input.enable = enable_injection;
	temp_err_input.temperature = fw_convert_float_to_fw_celsius(
		nvm_decode_temperature(temperature));

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_INJECT_ERROR;
	cmd.sub_opcode = SUBOP_ERROR_TEMP;
	cmd.input_payload = &temp_err_input;
	cmd.input_payload_size = sizeof (temp_err_input);
	rc = ioctl_passthrough_cmd(&cmd);
	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to set temperature on dimm %u",
			device_handle);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to validate setting poison on a particular DPA
 * Poison is only possible for addresses in the PM range but not while the PM region is locked
 */
int validate_poison_injection(struct device_discovery *p_discovery, NVM_UINT64 dpa)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// check if dimm is locked
	if (p_discovery->lock_state != LOCK_STATE_LOCKED)
	{
		// get the DPA start address of PMEM region from the dimm partition info
		struct pt_payload_get_dimm_partition_info partition_info;
		memset(&partition_info, 0, sizeof (partition_info));
		struct fw_cmd partition_cmd;
		memset(&partition_cmd, 0, sizeof (partition_cmd));
		partition_cmd.device_handle = p_discovery->device_handle.handle;
		partition_cmd.opcode = PT_GET_ADMIN_FEATURES;
		partition_cmd.sub_opcode = SUBOP_DIMM_PARTITION_INFO;
		partition_cmd.output_payload_size = sizeof (partition_info);
		partition_cmd.output_payload = &partition_info;
		if ((rc = ioctl_passthrough_cmd(&partition_cmd)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed getting dimm %u partition information",
					p_discovery->device_handle.handle);
		}
		else
		{
			// DPA is in PM range
			if ((dpa >= partition_info.start_pmem) &&
					(dpa < partition_info.start_pmem + partition_info.pmem_capacity))
			{
				rc = NVM_SUCCESS;
			}
			else
			{
				rc = NVM_ERR_INVALIDPARAMETER;
				COMMON_LOG_ERROR("Setting poison is only possible in PM range.");
			}
		}
	}
	else
	{
		rc = NVM_ERR_BADSECURITYSTATE;
		COMMON_LOG_ERROR("Setting poison is not possible when dimm is locked.");
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to allow setting/clearing poison on a particular DPA
 */
int inject_poison_error(struct device_discovery *p_discovery, NVM_UINT64 dpa, NVM_BOOL set_poison)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	if (set_poison)
	{
		if ((rc = validate_poison_injection(p_discovery, dpa)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR("Invalid parameter, DPA is invalid");
		}
	}

	if (rc == NVM_SUCCESS)
	{
		// Set up input
		struct pt_payload_poison_err poison_err_input;
		memset(&poison_err_input, 0, sizeof (poison_err_input));
		poison_err_input.enable = set_poison;
		poison_err_input.dpa_address = dpa;

		struct fw_cmd cmd;
		memset(&cmd, 0, sizeof (cmd));
		cmd.device_handle = p_discovery->device_handle.handle;
		cmd.opcode = PT_INJECT_ERROR;
		cmd.sub_opcode = SUBOP_ERROR_POISON;
		cmd.input_payload = &poison_err_input;
		cmd.input_payload_size = sizeof (poison_err_input);
		rc = ioctl_passthrough_cmd(&cmd);

		if (rc != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to set/clear poison on dimm %u",
					p_discovery->device_handle.handle);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Inject an error into the device specified.
 */
int nvm_inject_device_error(const NVM_UID device_uid,
		const struct device_error *p_error)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_error == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_error is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(error_injection)) != NVM_SUCCESS)
	{
			COMMON_LOG_ERROR("Error injection is not supported.");
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		if ((rc = enable_error_injection_functionality(discovery.device_handle.handle)) ==
				NVM_SUCCESS)
		{
			switch (p_error->type)
			{
			case ERROR_TYPE_TEMPERATURE:
				rc = inject_temperature_error(discovery.device_handle.handle,
						p_error->temperature, 1);
				break;
			case ERROR_TYPE_POISON:
				rc = inject_poison_error(&discovery, p_error->dpa, 1);
				break;
			case ERROR_TYPE_DIE_SPARING:
			case ERROR_TYPE_SPARE_ALARM:
			case ERROR_TYPE_MEDIA_FATAL_ERROR:
				rc = inject_software_trigger(&discovery, p_error->type, 1);
				break;
			default:
				break;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Clear an injected error into the device specified.
 */
int nvm_clear_injected_device_error(const NVM_UID device_uid,
		const struct device_error *p_error)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_error == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_error is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(error_injection)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Error injection is not supported.");
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		if ((rc = enable_error_injection_functionality(discovery.device_handle.handle)) ==
				NVM_SUCCESS)
		{
			switch (p_error->type)
			{
			case ERROR_TYPE_TEMPERATURE:
				rc = inject_temperature_error(discovery.device_handle.handle,
						0, 0);
				break;
			case ERROR_TYPE_POISON:
				rc = inject_poison_error(&discovery, p_error->dpa, 0);
				break;
			case ERROR_TYPE_DIE_SPARING:
				rc = inject_software_trigger(&discovery, p_error->type, 0);
				break;
			case ERROR_TYPE_SPARE_ALARM:
			case ERROR_TYPE_MEDIA_FATAL_ERROR:
				rc = NVM_ERR_NOTSUPPORTED;
				break;
			default:
				break;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
