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
		NVM_UINT8 memory, NVM_BOOL set_poison);
int inject_temperature_error(NVM_UINT32 device_handle, NVM_UINT64 temperature,
	NVM_BOOL enable_injection);
int inject_software_trigger(struct device_discovery *p_discovery, enum error_type type,
	NVM_BOOL enable_trigger);
int inject_dirty_shutdown_trigger(struct device_discovery * p_discovery, NVM_BOOL enable_trigger);

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
		input.triggers_to_modify = 0x1;
		input.die_sparing_trigger = enable_trigger;
		break;
	case ERROR_TYPE_MEDIA_FATAL_ERROR:
		input.triggers_to_modify = 0x1 << FATAL_ERROR_TRIGGER;
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
 * Helper function to inject spare capacity error
 */
int inject_spare_capacity_error(struct device_discovery *p_discovery,
		NVM_UINT16 spareCapacity, NVM_BOOL enable_trigger)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// Set up input payload
	struct pt_payload_sw_triggers input;
	memset(&input, 0, sizeof (input));
	input.triggers_to_modify = 0x1 << SPARE_BLOCK_PERCENTAGE_TRIGGER;
	unsigned char spare_block_percentage = (unsigned char)spareCapacity;
	input.spare_block_percentage_trigger = (spare_block_percentage << 1) | enable_trigger;

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
 * Helper function to inject dirty shutdown trigger
 */
int inject_dirty_shutdown_trigger(struct device_discovery *p_discovery, NVM_BOOL enable_trigger)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// Set up input payload
	struct pt_payload_sw_triggers input;
	memset(&input, 0, sizeof (input));
	input.triggers_to_modify = 0x1 << UNSAFE_SHUTDOWN_TRIGGER;
	input.unsafe_shutdown_trigger = enable_trigger;

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
		COMMON_LOG_ERROR_F("Failed to trigger dirty shutdown trip on dimm %u",	cmd.device_handle);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int verify_dimm_lock_state(enum lock_state  lock_state)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	if (lock_state == LOCK_STATE_LOCKED)
	{
		rc = NVM_ERR_BADSECURITYSTATE;
		COMMON_LOG_ERROR("Setting poison is not possible when dimm is locked.");
	}

	return rc;
}

/*
 * Helper function to allow setting/clearing poison on a particular DPA
 */
int inject_poison_error(struct device_discovery *p_discovery, NVM_UINT64 dpa, NVM_UINT8 memory,
		NVM_BOOL set_poison)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	if (set_poison)
	{
		rc = verify_dimm_lock_state(p_discovery->lock_state);
	}

	if (rc == NVM_SUCCESS)
	{
		// Set up input
		struct pt_payload_poison_err poison_err_input;
		memset(&poison_err_input, 0, sizeof (poison_err_input));
		poison_err_input.enable = set_poison;
		poison_err_input.dpa_address = dpa;
		poison_err_input.memory = memory;

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
			if (rc == NVM_ERR_INVALIDPARAMETER)
			{
				rc = NVM_ERR_INVALIDMEMORYTYPE;
			}
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
		switch (p_error->type)
		{
		case ERROR_TYPE_TEMPERATURE:
			rc = inject_temperature_error(discovery.device_handle.handle,
					p_error->temperature, 1);
			break;
		case ERROR_TYPE_POISON:
			rc = inject_poison_error(&discovery, p_error->dpa, p_error->memory_type, 1);
			break;
		case ERROR_TYPE_DIRTY_SHUTDOWN:
			rc = inject_dirty_shutdown_trigger(&discovery, 1);
			break;
		case ERROR_TYPE_SPARE_CAPACITY:
			rc = inject_spare_capacity_error(&discovery, p_error->spareCapacity, 1);
			break;
		case ERROR_TYPE_DIE_SPARING:
		case ERROR_TYPE_MEDIA_FATAL_ERROR:
			rc = inject_software_trigger(&discovery, p_error->type, 1);
			break;
		default:
			break;
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
		switch (p_error->type)
		{
		case ERROR_TYPE_TEMPERATURE:
			rc = inject_temperature_error(discovery.device_handle.handle,
					0, 0);
			break;
		case ERROR_TYPE_POISON:
			rc = inject_poison_error(&discovery, p_error->dpa, p_error->memory_type, 0);
			break;
		case ERROR_TYPE_DIE_SPARING:
			rc = inject_software_trigger(&discovery, p_error->type, 0);
			break;
		case ERROR_TYPE_DIRTY_SHUTDOWN:
			rc = inject_dirty_shutdown_trigger(&discovery, 0);
			break;
		case ERROR_TYPE_SPARE_CAPACITY:
			rc = inject_spare_capacity_error(&discovery, p_error->spareCapacity, 0);
			break;
		case ERROR_TYPE_MEDIA_FATAL_ERROR:
			rc = NVM_ERR_NOTSUPPORTED;
			break;
		default:
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
