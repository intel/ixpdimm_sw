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
 * This file contains the implementation of the quick health diagnostic
 * for the native API.
 */

#include "nvm_management.h"
#include "diagnostic.h"
#include "device_adapter.h"
#include "utility.h"
#include <persistence/config_settings.h>
#include <persistence/logging.h>
#include <persistence/lib_persistence.h>
#include <persistence/event.h>
#include <string/s_str.h>
#include <cr_i18n.h>
#include "device_utilities.h"
#include "capabilities.h"

// firmware checkpoint codes and boot status register
#define	MEDIA_READY_STATUS(bits)	((bits >> 16) & 0b11) // bits 17:16
#define	DDRT_IO_INIT_STATUS(bits)	((bits >> 18) & 0b11) // bits 19:18
#define	MAILBOX_INTERFACE_READY_STATUS(bits)	((bits >> 20) & 0b01) // bit 20
#define	MINOR_CHECKPOINT(bits)	((bits >> 8) & 0xff) // bits 15:8
#define	MAJOR_CHECKPOINT(bits)	((bits) & 0xff) // bits 7:0
#define	BSR_H_ASSERTION(bits)	((bits >> 32) & 0b1) // bit 32
#define	BSR_H_MI_STALLED(bits)	((bits >> 33) & 0b1) // bit 33
#define	STATUS_NOT_READY	0b00
#define	STATUS_ERROR	0b10
enum major_status_code
{
	NO_POST_CODE = 0x00,
	INIT_MJ_FAILURE_CHECKPOINT = 0XA1, // Unrecoverable FW error
	INIT_MJ_INIT_COMPLETE = 0XF0, // FW initialization complete
};

void generate_event_for_bad_driver(NVM_UINT32 *p_results);
int check_dimm_manageability(const NVM_UID device_uid,
		struct device_discovery *p_discovery,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results);
int check_dimm_identification(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results);
int check_dimm_health(const NVM_UID device_uid, const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results);
int check_dimm_media_errors(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results);
void check_dimm_power_limitation(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results);
int check_dimm_bsr(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results);
int check_dimm_viral_state(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results);

/*
 * Run the quick health check diagnostic algorithm
 */
int diag_quick_health_check(const NVM_UID device_uid,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	*p_results = 0;

	if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		// clear previous results for requested UID and specifically
		// system-wide - leave results for other UIDs alone
		diag_clear_results(EVENT_TYPE_DIAG_QUICK, 1, device_uid);
		diag_clear_results(EVENT_TYPE_DIAG_QUICK, 1, "");

		if (!is_supported_driver_available())
		{
			rc = NVM_ERR_BADDRIVER;
			generate_event_for_bad_driver(p_results);
		}
		else if ((rc = IS_NVM_FEATURE_SUPPORTED(quick_diagnostic)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR("The quick health diagnostic is not supported.");
		}
		else
		{
			struct device_discovery discovery;
			if ((rc = lookup_dev_uid(device_uid, &discovery)) == NVM_SUCCESS &&
					(rc = check_dimm_manageability(device_uid, &discovery,
							p_diagnostic, p_results)) == NVM_SUCCESS)
			{
				NVM_NFIT_DEVICE_HANDLE device_handle = discovery.device_handle;

				int tmp_rc = check_dimm_bsr(device_uid, device_handle, p_diagnostic, p_results);
				KEEP_ERROR(rc, tmp_rc);

				if (((rc = check_dimm_identification(device_uid, device_handle,
						p_diagnostic, p_results)) == NVM_SUCCESS) &&
						((*p_results) == 0)) // abort test if DIMM is unrecognized
				{
					tmp_rc = check_dimm_health(device_uid, device_handle, p_diagnostic, p_results);
					KEEP_ERROR(rc, tmp_rc);

					check_dimm_power_limitation(device_uid, device_handle, p_diagnostic,
						p_results);

					tmp_rc = check_dimm_media_errors(device_uid, device_handle, p_diagnostic,
							p_results);
					KEEP_ERROR(rc, tmp_rc);

					int tmp_rc =
						check_dimm_viral_state(device_uid, device_handle, p_diagnostic, p_results);
					KEEP_ERROR(rc, tmp_rc);

					if ((rc == NVM_SUCCESS) && (*p_results == 0)) // No errors/warnings
					{
						// store success event
						store_event_by_parts(
							EVENT_TYPE_DIAG_QUICK,
							EVENT_SEVERITY_INFO,
							EVENT_CODE_DIAG_QUICK_SUCCESS,
							device_uid,
							0,
							NULL,
							NULL,
							NULL,
							DIAGNOSTIC_RESULT_OK);
						(*p_results)++;
					}
				} // end unrecognized dimm
			} // DIMM does not exist or not manageable
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void generate_event_for_bad_driver(NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	store_event_by_parts(
			EVENT_TYPE_DIAG_QUICK,
			EVENT_SEVERITY_CRITICAL,
			EVENT_CODE_DIAG_QUICK_BAD_DRIVER,
			NULL,
			1, // Action required
			NULL, NULL, NULL,
			DIAGNOSTIC_RESULT_FAILED);
	(*p_results)++;

	COMMON_LOG_EXIT();
}

int check_dimm_manageability(const NVM_UID device_uid,
		struct device_discovery *p_discovery,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_discovery->manageability == MANAGEMENT_INVALIDCONFIG)
	{
		rc = NVM_ERR_NOTMANAGEABLE;

		store_event_by_parts(
				EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_QUICK_NOT_MANAGEABLE,
				device_uid,
				0, // Action required
				device_uid, NULL, NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void check_valid_dimm_manufacturer(const struct diagnostic *p_diagnostic,
		const struct pt_payload_identify_dimm *p_id_dimm, const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	int config_manufacturer = 0;
	get_config_value_int(SQL_KEY_VALID_MANUFACTURER, &config_manufacturer);
	NVM_UINT64 valid_manufacturer = config_manufacturer;
	NVM_UINT64 actual_manufacturer = 0;
	actual_manufacturer = MANUFACTURER_TO_UINT(p_id_dimm->mf);
	if (!diag_check(p_diagnostic, DIAG_THRESHOLD_QUICK_VALID_MANUFACTURER, actual_manufacturer,
			&valid_manufacturer, EQUALITY_EQUAL))
	{
		char actual_manufacturer_str[10];
		s_snprintf(actual_manufacturer_str, 10, "0x%02llx", actual_manufacturer);
		char valid_manufacturer_str[10];
		s_snprintf(valid_manufacturer_str, 10, "0x%02llx", valid_manufacturer);
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_QUICK_INVALID_MANUFACTURER,
				device_uid,
				0,
				device_uid,
				actual_manufacturer_str,
				valid_manufacturer_str, DIAGNOSTIC_RESULT_ABORTED);
		(*p_results)++;
	}
}

void check_valid_dimm_model_number(const struct diagnostic *p_diagnostic,
		const struct pt_payload_identify_dimm *p_id_dimm, const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	char valid_model[CONFIG_VALUE_LEN] = "";
	get_config_value(SQL_KEY_VALID_MODEL_NUM, valid_model);
	if (!diag_check_str(p_diagnostic, DIAG_THRESHOLD_QUICK_VALID_MODEL_NUMBER, p_id_dimm->mn,
			valid_model))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_QUICK_INVALID_MODEL_NUMBER,
				device_uid,
				0,
				device_uid,
				p_id_dimm->mn,
				valid_model, DIAGNOSTIC_RESULT_ABORTED);
		(*p_results)++;
	}
}

void check_valid_dimm_vendor_id(const struct diagnostic *p_diagnostic,
		const struct pt_payload_identify_dimm *p_id_dimm,
		const NVM_UID device_uid, NVM_UINT32 *p_results)
{
	int vendor_config = 0;
	get_config_value_int(SQL_KEY_VALID_VENDOR_ID, &vendor_config);
	NVM_UINT64 valid_vendor = vendor_config;
	if (!diag_check(p_diagnostic, DIAG_THRESHOLD_QUICK_VALID_VENDOR_ID,
			(NVM_UINT64) p_id_dimm->vendor_id, &valid_vendor, EQUALITY_EQUAL))
	{
		char actual_vendor_str[10];
		s_snprintf(actual_vendor_str, 10, "0x%04x", p_id_dimm->vendor_id);
		char expected_vendor_str[10];
		s_snprintf(expected_vendor_str, 10, "0x%04x", valid_vendor);
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK, EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_QUICK_INVALID_VENDORID, device_uid,
				0, device_uid, actual_vendor_str,
				expected_vendor_str, DIAGNOSTIC_RESULT_ABORTED);
		(*p_results)++;
	}
}

int check_dimm_identification(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// setup ioctl payloads and commands
	struct pt_payload_identify_dimm id_dimm;
	memset(&id_dimm, 0, sizeof (struct pt_payload_identify_dimm));
	struct fw_cmd id_cmd;
	memset(&id_cmd, 0, sizeof (struct fw_cmd));
	id_cmd.device_handle = device_handle.handle;
	id_cmd.opcode = PT_IDENTIFY_DIMM;
	id_cmd.sub_opcode = 0;
	id_cmd.output_payload_size = sizeof (id_dimm);
	id_cmd.output_payload = &id_dimm;

	if ((rc = ioctl_passthrough_cmd(&id_cmd)) == NVM_SUCCESS)
	{
		check_valid_dimm_manufacturer(p_diagnostic, &id_dimm, device_uid, p_results);
		check_valid_dimm_model_number(p_diagnostic, &id_dimm, device_uid, p_results);
		check_valid_dimm_vendor_id(p_diagnostic, &id_dimm, device_uid, p_results);
	}
	s_memset(&id_dimm, sizeof (id_dimm));

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int check_dimm_alarm_thresholds(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		struct pt_payload_smart_health *p_dimm_smart,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// check alarm thresholds
	struct pt_payload_alarm_thresholds thresholds;
	if (NVM_SUCCESS
			== (rc = fw_get_alarm_thresholds(device_handle.handle, &thresholds)))
	{
		// check media temperature to alarm threshold
		NVM_UINT64 media_temp_threshold = thresholds.media_temperature;
		if (p_dimm_smart->validation_flags.parts.media_temperature_field &&
				!diag_check(p_diagnostic, DIAG_THRESHOLD_QUICK_MEDIA_TEMP,
						p_dimm_smart->media_temperature,
						&media_temp_threshold, EQUALITY_LESSTHAN))
		{
			NVM_REAL32 actual = fw_convert_fw_celsius_to_float(p_dimm_smart->media_temperature);
			NVM_REAL32 media_threshold = fw_convert_fw_celsius_to_float(media_temp_threshold);

			// log error
			char actual_temp_str[10];
			s_snprintf(actual_temp_str, 10, "%.4f", actual);
			char expected_temp_str[10];
			s_snprintf(expected_temp_str, 10, "%.4f", media_threshold);
			store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
					EVENT_SEVERITY_WARN,
					EVENT_CODE_DIAG_QUICK_BAD_MEDIA_TEMP,
					device_uid,
					0,
					device_uid,
					actual_temp_str,
					expected_temp_str, DIAGNOSTIC_RESULT_FAILED);
			(*p_results)++;
		}

		// check controller temperature to alarm threshold
		NVM_UINT64 controller_temp_threshold = thresholds.controller_temperature;
		if (p_dimm_smart->validation_flags.parts.sizeof_vendor_data_field &&
				!diag_check(p_diagnostic, DIAG_THRESHOLD_QUICK_CONTROLLER_TEMP,
						p_dimm_smart->controller_temperature,
						&controller_temp_threshold, EQUALITY_LESSTHAN))
		{
			NVM_REAL32 actual =
				fw_convert_fw_celsius_to_float(p_dimm_smart->controller_temperature);
			NVM_REAL32 controller_threshold =
				fw_convert_fw_celsius_to_float(controller_temp_threshold);

			// log error
			char actual_temp_str[10];
			s_snprintf(actual_temp_str, 10, "%.4f", actual);
			char expected_temp_str[10];
			s_snprintf(expected_temp_str, 10, "%.4f", controller_threshold);
			store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
					EVENT_SEVERITY_WARN,
					EVENT_CODE_DIAG_QUICK_BAD_CONTROLLER_TEMP,
					device_uid,
					0,
					device_uid,
					actual_temp_str,
					expected_temp_str, DIAGNOSTIC_RESULT_FAILED);
			(*p_results)++;
		}

		// check spare capacity to alarm threshold
		NVM_UINT64 spare_threshold = thresholds.spare;
		if (p_dimm_smart->validation_flags.parts.spare_block_field &&
				!diag_check(p_diagnostic, DIAG_THRESHOLD_QUICK_AVAIL_SPARE, p_dimm_smart->spare,
				&spare_threshold, EQUALITY_GREATERTHANEQUAL))
		{
			// log error
			char actual_spare_str[10];
			s_snprintf(actual_spare_str, 10, "%u", p_dimm_smart->spare);
			char expected_spare_str[10];
			s_snprintf(expected_spare_str, 10, "%u", spare_threshold);
			store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
					EVENT_SEVERITY_WARN,
					EVENT_CODE_DIAG_QUICK_BAD_SPARE,
					device_uid,
					0,
					device_uid,
					actual_spare_str,
					expected_spare_str,
					DIAGNOSTIC_RESULT_FAILED);
			(*p_results)++;
		}

		// check percent used to threshold
		int percent_used_threshold_config = 0;
		get_config_value_int(SQL_KEY_PERCENT_USED_THRESHOLD, &percent_used_threshold_config);
		NVM_UINT64 percent_used_threshold = percent_used_threshold_config;
		if (p_dimm_smart->validation_flags.parts.percentage_used_field &&
				!diag_check(p_diagnostic, DIAG_THRESHOLD_QUICK_PERC_USED,
				p_dimm_smart->percentage_used, &percent_used_threshold, EQUALITY_LESSTHAN))
		{
			// log error
			char actual_percent_str[10];
			s_snprintf(actual_percent_str, 10, "%u", p_dimm_smart->percentage_used);
			char expected_percent_str[10];
			s_snprintf(expected_percent_str, 10, "%u", percent_used_threshold);
			store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
					EVENT_SEVERITY_WARN,
					EVENT_CODE_DIAG_QUICK_BAD_PERCENT_USED,
					device_uid,
					0,
					device_uid,
					actual_percent_str,
					expected_percent_str,
					DIAGNOSTIC_RESULT_FAILED);
			(*p_results)++;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void dimm_smart_health_status_to_string(const enum smart_health_status health_status,
		char *str, const size_t str_size)
{
	enum device_health dev_health = smart_health_status_to_device_health(health_status);
	const char *health_str = TR(get_string_for_device_health_status(dev_health));
	s_strcpy(str, health_str, str_size);
}

void check_dimm_smart_health_status(const struct diagnostic *p_diagnostic,
		struct pt_payload_smart_health *p_dimm_smart, const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	int max_health_status_config = 0;
	get_config_value_int(SQL_KEY_MAX_HEALTH_STATUS, &max_health_status_config);
	NVM_UINT64 max_health_status = max_health_status_config;
	if (p_dimm_smart->validation_flags.parts.health_status_field &&
			!diag_check(p_diagnostic, DIAG_THRESHOLD_QUICK_HEALTH, p_dimm_smart->health_status,
			&max_health_status, EQUALITY_LESSTHANEQUAL))
	{
		NVM_EVENT_ARG actual_health_str;
		dimm_smart_health_status_to_string(p_dimm_smart->health_status,
				actual_health_str, sizeof (actual_health_str));
		NVM_EVENT_ARG expected_health_str;
		dimm_smart_health_status_to_string(max_health_status_config,
				expected_health_str, sizeof (expected_health_str));

		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_QUICK_BAD_HEALTH,
				device_uid,
				0,
				device_uid,
				actual_health_str,
				expected_health_str,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}
}

int get_bsr(const NVM_NFIT_DEVICE_HANDLE device_handle, unsigned long long *p_bsr)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle.handle;
	cmd.opcode = BIOS_EMULATED_COMMAND;
	cmd.sub_opcode = SUBOP_GET_BOOT_STATUS;
	cmd.output_payload_size = sizeof (unsigned long long);
	cmd.output_payload = p_bsr;
	rc = ioctl_passthrough_cmd(&cmd);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void check_media_ready_status(const struct diagnostic *p_diagnostic,
		const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	NVM_UINT16 code = EVENT_CODE_DIAG_QUICK_UNKNOWN;
	if (MEDIA_READY_STATUS(bsr) == STATUS_NOT_READY)
	{
		code = EVENT_CODE_DIAG_QUICK_MEDIA_NOT_READY;
	}
	else if (MEDIA_READY_STATUS(bsr) == STATUS_ERROR)
	{
		code = EVENT_CODE_DIAG_QUICK_MEDIA_READY_ERROR;
	}

	if (code != EVENT_CODE_DIAG_QUICK_UNKNOWN)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				code,
				device_uid,
				1,
				device_uid,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

void check_ddrt_init_complete_status(const struct diagnostic *p_diagnostic,
		const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	NVM_UINT16 code = EVENT_CODE_DIAG_QUICK_UNKNOWN;
	if (DDRT_IO_INIT_STATUS(bsr) == STATUS_NOT_READY)
	{
		code = EVENT_CODE_DIAG_QUICK_DDRT_IO_INIT_NOT_READY;
	}
	else if (DDRT_IO_INIT_STATUS(bsr) == STATUS_ERROR)
	{
		code = EVENT_CODE_DIAG_QUICK_DDRT_IO_INIT_ERROR;
	}

	if (code != EVENT_CODE_DIAG_QUICK_UNKNOWN)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				code,
				device_uid,
				1,
				device_uid,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

void check_mailbox_ready_status(const struct diagnostic *p_diagnostic,
		const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	if (MAILBOX_INTERFACE_READY_STATUS(bsr) == STATUS_NOT_READY)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_DIAG_QUICK_MAILBOX_INTERFACE_NOT_READY,
				device_uid,
				1,
				device_uid,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

void check_fw_boot_status(const struct diagnostic *p_diagnostic,
		const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	NVM_UINT8 major_status_code = MAJOR_CHECKPOINT(bsr);
	NVM_UINT8 minor_status_code = MINOR_CHECKPOINT(bsr);
	NVM_EVENT_ARG checkpoint_str;
	s_snprintf(checkpoint_str, NVM_EVENT_ARG_LEN, "ox%x:ox%x",
			major_status_code, minor_status_code);

	NVM_UINT16 code = EVENT_CODE_DIAG_QUICK_UNKNOWN;
	if (major_status_code == NO_POST_CODE)
	{
		code = EVENT_CODE_DIAG_QUICK_NO_POST_CODE;
		checkpoint_str[0] = 0;
	}
	else if ((major_status_code == INIT_MJ_FAILURE_CHECKPOINT) ||
			(major_status_code != INIT_MJ_INIT_COMPLETE))
	{
		code = EVENT_CODE_DIAG_QUICK_FW_INITIALIZATION_INCOMPLETE;
	}

	if (code != EVENT_CODE_DIAG_QUICK_UNKNOWN)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				code,
				device_uid,
				1,
				device_uid,
				checkpoint_str,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

void check_fw_assert(const struct diagnostic *p_diagnostic,
		const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	if (BSR_H_ASSERTION(bsr))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_DIAG_QUICK_FW_HIT_ASSERT,
				device_uid,
				1,
				device_uid,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

void check_fw_stalled(const struct diagnostic *p_diagnostic,
		const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	if (BSR_H_MI_STALLED(bsr))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_DIAG_QUICK_FW_STALLED,
				device_uid,
				1,
				device_uid,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

int check_dimm_viral_state(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_get_config_data_policy config_data;
	rc = fw_get_config_data_policy(device_handle.handle, &config_data);
	if ((rc == NVM_SUCCESS) &&
			(config_data.viral_status))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_DIAG_QUICK_VIRAL_STATE,
				device_uid,
				1,
				device_uid,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int check_dimm_bsr(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	unsigned long long bsr = 0;
	rc = get_bsr(device_handle, &bsr);
	if (rc != NVM_SUCCESS)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_DIAG_QUICK_UNREADABLE_BSR,
				device_uid,
				1,
				device_uid,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}
	else
	{
		check_media_ready_status(p_diagnostic, bsr, device_uid, p_results);
		check_ddrt_init_complete_status(p_diagnostic, bsr, device_uid, p_results);
		check_mailbox_ready_status(p_diagnostic, bsr, device_uid, p_results);
		check_fw_boot_status(p_diagnostic, bsr, device_uid, p_results);
		check_fw_assert(p_diagnostic, bsr, device_uid, p_results);
		check_fw_stalled(p_diagnostic, bsr, device_uid, p_results);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int check_dimm_health(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_smart_health dimm_smart;
	if (NVM_SUCCESS == (rc =
			fw_get_smart_health(device_handle.handle, &dimm_smart)))
	{
		check_dimm_smart_health_status(p_diagnostic, &dimm_smart, device_uid, p_results);

		rc = check_dimm_alarm_thresholds(device_uid, device_handle, &dimm_smart,
				p_diagnostic, p_results);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void check_dimm_power_limitation(const NVM_UID device_uid,
	const NVM_NFIT_DEVICE_HANDLE device_handle,
	const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	NVM_UINT16 socket = device_handle.parts.socket_id;
	if (get_dimm_power_limited(socket) == 1)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK, EVENT_SEVERITY_WARN,
			EVENT_CODE_DIAG_QUICK_BAD_POWER_LIMITATION, NULL, 0, NULL,
			NULL, NULL, DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

void check_given_errors(const struct diagnostic *p_diagnostic,
	NVM_UINT32 *p_results, const NVM_UID device_uid,
	NVM_UINT64 media_errors, const char *p_sql_key,
	const NVM_UINT32 threshold_key, const NVM_UINT16 event_key)
{
	COMMON_LOG_ENTRY();

	int error_threshold_config = 0;
	get_config_value_int(p_sql_key, &error_threshold_config);
	NVM_UINT64 error_threshold = error_threshold_config;

	if (!diag_check(p_diagnostic, threshold_key,
		media_errors, &error_threshold, EQUALITY_LESSTHANEQUAL))
	{
		char actual_errors_str[10];
		s_snprintf(actual_errors_str, 10, "%u", media_errors);
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK, EVENT_SEVERITY_WARN,
			event_key, device_uid, 0, device_uid,
			actual_errors_str, NULL, DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

int check_dimm_media_errors(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_memory_info_page2 memory_info_page2;
	if (NVM_SUCCESS == (rc = fw_get_memory_info_page(device_handle.handle, 2,
		&memory_info_page2, sizeof (memory_info_page2))))
	{
		check_given_errors(p_diagnostic, p_results, device_uid, memory_info_page2.media_errors_uc,
			SQL_KEY_UNCORRECTABLE_THRESHOLD,
			DIAG_THRESHOLD_QUICK_UNCORRECT_ERRORS,
			EVENT_CODE_DIAG_QUICK_BAD_UNCORRECTABLE_MEDIA_ERRORS);

		NVM_UINT64 media_errors_ce = 0;
		NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(memory_info_page2.media_errors_ce, media_errors_ce);
		check_given_errors(p_diagnostic, p_results, device_uid, media_errors_ce,
			SQL_KEY_CORRECTED_THRESHOLD,
			DIAG_THRESHOLD_QUICK_CORRECTED_ERRORS,
			EVENT_CODE_DIAG_QUICK_BAD_CORRECTED_MEDIA_ERRORS);

		NVM_UINT64 media_errors_ecc = 0;
		NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(memory_info_page2.media_errors_ecc, media_errors_ecc);
		check_given_errors(p_diagnostic, p_results, device_uid, media_errors_ecc,
			SQL_KEY_ERASURE_CODED_CORRECTED_THRESHOLD,
			DIAG_THRESHOLD_QUICK_ERASURE_CODED_CORRECTED_ERRORS,
			EVENT_CODE_DIAG_QUICK_BAD_ERASURE_CODED_CORRECTED_MEDIA_ERRORS);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
