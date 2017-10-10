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
#include <string/revision.h>
#include <cr_i18n.h>
#include "device_utilities.h"
#include "capabilities.h"
#include "device_fw.h"

enum major_status_code
{
	NO_POST_CODE = 0x00,
	INIT_MJ_FAILURE_CHECKPOINT = 0XA1, // Unrecoverable FW error
	CPU_EXCEPTION = 0xE1, // CPU Exception
	INIT_MJ_INIT_COMPLETE = 0XF0, // FW initialization complete
};

void generate_event_for_bad_driver(NVM_UINT32 *p_results);
int check_dimm_manageability(const NVM_UID device_uid,
		struct device_discovery *p_discovery,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results);
int check_dimm_health(const NVM_UID device_uid, const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results);
void check_dimm_power_limitation(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results);
int check_ddrt_io_init_done(const NVM_UID device_uid,const NVM_NFIT_DEVICE_HANDLE device_handle,
	NVM_UINT32 *p_results);
int check_dimm_bsr(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results);
int check_dimm_viral_state(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results);
int check_dimm_fw_update_status(const NVM_UID device_uid,
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
			if ((rc = lookup_dev_uid(device_uid, &discovery)) == NVM_SUCCESS)
			{
				rc = check_dimm_manageability(device_uid, &discovery,
									p_diagnostic, p_results);

				if (IS_DEVICE_MANAGEABLE(&discovery))
				{
					int tmp_rc = 0;
					NVM_NFIT_DEVICE_HANDLE device_handle =
					discovery.device_handle;

					tmp_rc = check_dimm_bsr(device_uid, device_handle,
						p_diagnostic, p_results);
					KEEP_ERROR(rc, tmp_rc);
					tmp_rc = check_dimm_health(device_uid,
						device_handle, p_diagnostic, p_results);
					KEEP_ERROR(rc, tmp_rc);

					check_dimm_power_limitation(device_uid,
						device_handle, p_diagnostic, p_results);

					tmp_rc = check_dimm_viral_state(device_uid,
						device_handle, p_diagnostic, p_results);
					KEEP_ERROR(rc, tmp_rc);

					tmp_rc = check_dimm_fw_update_status(device_uid,
						device_handle, p_diagnostic, p_results);
					KEEP_ERROR(rc, tmp_rc);

					tmp_rc = check_ddrt_io_init_done(device_uid,
						device_handle,p_results);
					KEEP_ERROR(rc, tmp_rc);
				}
			} // DIMM does not exist
		}
	}

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

void generate_event_for_non_manageable_dimm_with_hex_value(
		const int event_code,
		const NVM_UID uid,
		const NVM_UINT16 hex_value)
{
	COMMON_LOG_ENTRY();

	char hex_value_str[NVM_EVENT_ARG_LEN];
	s_snprintf(hex_value_str, sizeof (hex_value_str),
			"0x%X", hex_value);

	store_event_by_parts(
			EVENT_TYPE_DIAG_QUICK,
			EVENT_SEVERITY_WARN,
			event_code,
			uid,
			0, // Action required
			uid, hex_value_str, NULL,
			DIAGNOSTIC_RESULT_ABORTED);

	COMMON_LOG_EXIT();
}

NVM_BOOL is_device_fw_api_revision_supported(NVM_VERSION fw_api_version)
{
	COMMON_LOG_ENTRY();

	NVM_UINT16 major = 0;
	NVM_UINT16 minor = 0;
	parse_fw_revision(&major, &minor, fw_api_version, NVM_VERSION_LEN);

	NVM_BOOL is_supported = is_fw_api_version_supported(major, minor);

	COMMON_LOG_EXIT_RETURN_I(is_supported);
	return is_supported;
}

int check_dimm_manageability(const NVM_UID device_uid,
		struct device_discovery *p_discovery,
		const struct diagnostic *p_diagnostic, NVM_UINT32* p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!is_subsystem_vendor_id_supported(p_discovery->subsystem_vendor_id))
	{
		rc = NVM_ERR_NOTMANAGEABLE;
		generate_event_for_non_manageable_dimm_with_hex_value(
				EVENT_CODE_DIAG_QUICK_NOT_MANAGEABLE_VENDOR_ID,
				device_uid,
				p_discovery->subsystem_vendor_id);
		(*p_results)++;
	}
	else if (!is_subsystem_device_id_supported(p_discovery->subsystem_device_id))
	{
		rc = NVM_ERR_NOTMANAGEABLE;
		generate_event_for_non_manageable_dimm_with_hex_value(
				EVENT_CODE_DIAG_QUICK_NOT_MANAGEABLE_DEVICE_ID,
				device_uid,
				p_discovery->subsystem_device_id);
		(*p_results)++;
	}
	else if (!is_device_fw_api_revision_supported(p_discovery->fw_api_version))
	{
		rc = NVM_ERR_NOTMANAGEABLE;
		store_event_by_parts(
				EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_QUICK_NOT_MANAGEABLE_FW_API,
				device_uid,
				0, // Action required
				device_uid, p_discovery->fw_api_version, NULL,
				DIAGNOSTIC_RESULT_ABORTED);
		(*p_results)++;
	}

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
					expected_temp_str, DIAGNOSTIC_RESULT_WARNING);
			(*p_results)++;
		}

		// check controller temperature to alarm threshold
		NVM_UINT64 controller_temp_threshold = thresholds.controller_temperature;
		if (p_dimm_smart->validation_flags.parts.controller_temperature_field &&
				!diag_check(p_diagnostic, DIAG_THRESHOLD_QUICK_CONTROLLER_TEMP,
						p_dimm_smart->controller_temperature,
						&controller_temp_threshold, EQUALITY_LESSTHAN))
		{
			NVM_REAL32 actual =
				fw_convert_fw_celsius_to_float(p_dimm_smart->controller_temperature);
			NVM_REAL32 controller_threshold =
				fw_convert_fw_celsius_to_float(controller_temp_threshold);

			char actual_temp_str[10];
			s_snprintf(actual_temp_str, 10, "%.4f", actual);
			char expected_temp_str[10];
			s_snprintf(expected_temp_str, 10, "%.4f", controller_threshold);
			store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
					EVENT_SEVERITY_WARN,
					EVENT_CODE_DIAG_QUICK_BAD_CORE_TEMP,
					device_uid,
					0,
					device_uid,
					actual_temp_str,
					expected_temp_str, DIAGNOSTIC_RESULT_WARNING);
			(*p_results)++;
		}

		// check spare capacity to alarm threshold
		NVM_UINT64 spare_threshold = thresholds.spare;
		if (p_dimm_smart->validation_flags.parts.spare_block_field &&
				!diag_check(p_diagnostic, DIAG_THRESHOLD_QUICK_AVAIL_SPARE, p_dimm_smart->spare,
				&spare_threshold, EQUALITY_GREATERTHANEQUAL))
		{
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
					DIAGNOSTIC_RESULT_WARNING);
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
					DIAGNOSTIC_RESULT_WARNING);
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
				DIAGNOSTIC_RESULT_WARNING);
		(*p_results)++;
	}
}

void check_media_disabled_status(const struct diagnostic *p_diagnostic,
		const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	if (BSR_MEDIA_DISABLED(bsr))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_DIAG_QUICK_MEDIA_DISABLED,
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

void check_media_ready_status(const struct diagnostic *p_diagnostic,
		const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	NVM_UINT16 code = EVENT_CODE_DIAG_QUICK_UNKNOWN;
	if (!BSR_MEDIA_READY_STATUS(bsr))
	{
		code = EVENT_CODE_DIAG_QUICK_MEDIA_NOT_READY;
	}
	else if (BSR_MEDIA_ERROR(bsr))
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

void check_fw_boot_status(const struct diagnostic *p_diagnostic,
		const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	NVM_UINT8 major_status_code = BSR_MAJOR_CHECKPOINT(bsr);
	NVM_UINT8 minor_status_code = BSR_MINOR_CHECKPOINT(bsr);
	NVM_EVENT_ARG checkpoint_str;
	s_snprintf(checkpoint_str, NVM_EVENT_ARG_LEN, "0x%x:0x%x",
			major_status_code, minor_status_code);

	NVM_UINT16 code = EVENT_CODE_DIAG_QUICK_UNKNOWN;
	if (major_status_code == NO_POST_CODE)
	{
		code = EVENT_CODE_DIAG_QUICK_NO_POST_CODE;
		checkpoint_str[0] = 0;
	}
	else if (major_status_code == CPU_EXCEPTION)
	{
		code = EVENT_CODE_DIAG_QUICK_INTERNAL_CPU_ERROR;
	}
	else if (major_status_code == INIT_MJ_FAILURE_CHECKPOINT)
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

void check_ait_dram_not_ready(const unsigned long long bsr,
		const NVM_UID device_uid,
		NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	if (!BSR_H_AIT_DRAM_READY(bsr))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_DIAG_QUICK_AIT_DRAM_NOT_READY,
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

int check_ddrt_io_init_done(const NVM_UID device_uid,const NVM_NFIT_DEVICE_HANDLE device_handle,
	NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	struct device_discovery discovery;
	NVM_BOOL test_passed = 0;
	struct pt_payload_ddrt_init_info p_ddrt_init_info;
	unsigned long long bsr = 0;
	int rc = NVM_SUCCESS;
	nvm_get_device_discovery(device_uid, &discovery);
	if (atof(discovery.fw_api_version) >= 1.6)
	{
		if (NVM_SUCCESS == (rc = fw_get_ddrt_io_init(device_handle.handle,&p_ddrt_init_info)))
		{
			if(p_ddrt_init_info.ddrt_io_info!=DDRT_TRAINING_COMPLETE)
			{
				test_passed = 0;
			}
			else
			{
				test_passed = 1;
			}
		}
	}
	else
	{
	    rc = fw_get_bsr(device_handle, &bsr);
		test_passed = (BSR_DDRT_IO_INIT_STATUS(bsr) == BSR_DDRT_NOT_READY) ? 0 : 1;
	}

	if (!test_passed)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
			EVENT_SEVERITY_CRITICAL,
			EVENT_CODE_DIAG_QUICK_DDRT_IO_INIT_FAILED,
			device_uid,
			1,
			device_uid,
			NULL,
			NULL,
			DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
    return rc;
}

int check_dimm_bsr(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	unsigned long long bsr = 0;
	rc = fw_get_bsr(device_handle, &bsr);
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
		check_media_disabled_status(p_diagnostic, bsr, device_uid, p_results);
		check_media_ready_status(p_diagnostic, bsr, device_uid, p_results);
		check_fw_boot_status(p_diagnostic, bsr, device_uid, p_results);
		check_fw_assert(p_diagnostic, bsr, device_uid, p_results);
		check_fw_stalled(p_diagnostic, bsr, device_uid, p_results);
		check_ait_dram_not_ready(bsr, device_uid, p_results);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void check_dimm_ait_dram_status(struct pt_payload_smart_health *p_dimm_smart,
		const NVM_UID device_uid, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	if (p_dimm_smart->validation_flags.parts.ait_dram_status_field &&
		p_dimm_smart->ait_dram_status == AIT_DRAM_DISABLED)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_DIAG_QUICK_AIT_DRAM_DISABLED,
				device_uid,
				1,
				device_uid,
				NULL,
				NULL, DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}
	COMMON_LOG_EXIT();
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
		check_dimm_ait_dram_status(&dimm_smart, device_uid, p_results);

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
		char socket_number[8];
		s_snprintf(socket_number, sizeof (socket_number), "%hu", socket);

		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_QUICK_BAD_POWER_LIMITATION,
				NULL,
				0,
				socket_number,
				NULL, NULL,
				DIAGNOSTIC_RESULT_WARNING);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

NVM_UINT64 get_error_threshold_from_config_db(const char *threshold_sql_key)
{
	COMMON_LOG_ENTRY();

	int error_threshold_config = 0;
	get_config_value_int(threshold_sql_key, &error_threshold_config);

	COMMON_LOG_EXIT_RETURN_I(error_threshold_config);
	return (NVM_UINT64)error_threshold_config;
}

NVM_BOOL errors_exceed_threshold(const struct diagnostic *p_diagnostic,
		const NVM_UINT32 threshold_exclude_flag,
		const NVM_UINT64 media_errors, const char *threshold_sql_key)
{
	NVM_UINT64 error_threshold = get_error_threshold_from_config_db(threshold_sql_key);

	return !diag_check(p_diagnostic, threshold_exclude_flag,
			media_errors, &error_threshold, EQUALITY_LESSTHANEQUAL);
}

int check_dimm_fw_update_status(const NVM_UID device_uid,
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_fw_image_info fw_image_info;
	rc = fw_get_fw_image_info(device_handle.handle, &fw_image_info);
	if ((rc == NVM_SUCCESS) &&
			(fw_image_info.last_fw_update_status == LAST_FW_UPDATE_LOAD_FAILED))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_QUICK,
				EVENT_SEVERITY_CRITICAL,
				EVENT_CODE_DIAG_QUICK_FW_LOAD_FAILED,
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
