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
 * This file contains the implementation of the firmware diagnostic
 * for the native API.
 */

#include "nvm_management.h"
#include "diagnostic.h"
#include "device_fw.h"
#include "device_adapter.h"
#include <persistence/logging.h>
#include <persistence/event.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include <string/s_str.h>
#include <string/revision.h>
#include <uid/uid.h>
#include "device_utilities.h"
#include "capabilities.h"

/*
 * Used in firmware consistency and settings check diagnostic to display
 * debug level enum as strings
 */
static const char *fw_log_level_strings[] =
{ "Disabled", "Error", "Warning", "Info", "Debug", "Unknown" };

void find_optimal_fw_revision(char *optimal_fw_revision, char *model_number,
		const struct device_discovery *p_dimms, int dev_count);
int get_fw_system_time(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct pt_payload_system_time *payload);

extern int get_fw_power_mgmt_policy(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct pt_payload_power_mgmt_policy *payload);
extern int get_fw_die_spare_policy(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct pt_get_die_spare_policy *payload);

/*
 * Run the firmware consistency and settings check diagnostic algorithm
 */
int diag_firmware_check(const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// clear previous results
	diag_clear_results(EVENT_TYPE_DIAG_FW_CONSISTENCY, 0, NULL);
	*p_results = 0;

	if ((rc = IS_NVM_FEATURE_SUPPORTED(fw_consistency_diagnostic)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("The firmware consistency check diagnostic is not supported.");
	}
	else
	{
		int dev_count = nvm_get_device_count();
		if (dev_count == 0)
		{
			store_event_by_parts(EVENT_TYPE_DIAG_FW_CONSISTENCY,
					EVENT_SEVERITY_WARN, EVENT_CODE_DIAG_FW_NO_DIMMS, NULL, 0, NULL,
					NULL, NULL, DIAGNOSTIC_RESULT_ABORTED);
			(*p_results)++;
		}
		else if (dev_count > 0)
		{
			// get device_discovery information of all dimms
			struct device_discovery dimms[dev_count];
			dev_count = nvm_get_devices(dimms, dev_count);

			if (dev_count > 0)
			{
				if (!(p_diagnostic->excludes & DIAG_THRESHOLD_FW_CONSISTENT))
				{
					// construct an array of model numbers
					char model_list[NVM_MAX_TOPO_SIZE][NVM_MODEL_LEN] = {};
					int model_count = 0;
					NVM_BOOL model_exists = 0;
					for (int dev_index = 0; dev_index < dev_count; dev_index++)
					{
						for (int model_index = 0; model_index < model_count; model_index++)
						{
							if (strncmp(model_list[model_index], dimms[dev_index].model_number,
									NVM_MODEL_LEN) == 0)
							{
								model_exists = 1;
								break;
							}
							model_exists = 0;
						}
						if (model_exists == 0)
						{
							s_strncpy(model_list[model_count], NVM_MODEL_LEN,
									dimms[dev_index].model_number, NVM_MODEL_LEN);
							model_count++;
						}
					}

					// get optimal FW revision per model number
					NVM_VERSION optimal_fw_rev[model_count];
					for (int current_model = 0; current_model < model_count; current_model++)
					{
						find_optimal_fw_revision(optimal_fw_rev[current_model],
								model_list[current_model], dimms, dev_count);
					}

					// compare firmware revisions of dimms having the same model number
					char inconsistent_uids_event_str[NVM_EVENT_ARG_LEN] = {0};
					NVM_BOOL inconsistency_flag = 0;
					for (int model_num = 0; model_num < model_count; model_num++)
					{
						for (int dev_num = 0; dev_num < dev_count; dev_num++)
						{
							if (strncmp(model_list[model_num], dimms[dev_num].model_number,
									NVM_MODEL_LEN) == 0)
							{
								if (strncmp(optimal_fw_rev[model_num],
										dimms[dev_num].fw_revision, NVM_VERSION_LEN) != 0)
								{
									NVM_UID uid_str;
									uid_copy(dimms[dev_num].uid, uid_str);
									s_strcat(uid_str, (NVM_MAX_UID_LEN + 2), ", ");
									s_strcat(inconsistent_uids_event_str,
											NVM_EVENT_ARG_LEN, uid_str);
									inconsistency_flag = 1;
								}
							}
						}
						// log an event per model number if fw version is inconsistent
						if (inconsistency_flag)
						{
							store_event_by_parts(EVENT_TYPE_DIAG_FW_CONSISTENCY,
									EVENT_SEVERITY_WARN,
									EVENT_CODE_DIAG_FW_INCONSISTENT, NULL, 0,
									inconsistent_uids_event_str,
									model_list[model_num], optimal_fw_rev[model_num],
									DIAGNOSTIC_RESULT_FAILED);
							(*p_results)++;
						}
						inconsistency_flag = 0;
					}
				}

				// get default temperature and spare capacity thresholds
				char max_threshold_str[CONFIG_VALUE_LEN];
				get_config_value(SQL_KEY_DEFAULT_TEMPERATURE_THRESHOLD, max_threshold_str);
				float max_temp_threshold_config = strtof(max_threshold_str, NULL);

				char expected_temp_threshold_str[NVM_EVENT_ARG_LEN];
				s_snprintf(expected_temp_threshold_str, NVM_EVENT_ARG_LEN, "%.4f",
						max_temp_threshold_config);

				int min_spare_block_threshold_config = 0;
				get_config_value_int(SQL_KEY_DEFAULT_SPARE_BLOCK_THRESHOLD,
						&min_spare_block_threshold_config);
				char expected_spare_block_threshold_str[NVM_EVENT_ARG_LEN];
				s_snprintf(expected_spare_block_threshold_str, NVM_EVENT_ARG_LEN, "%u",
						min_spare_block_threshold_config);
				NVM_UINT64 min_spare_block_threshold = min_spare_block_threshold_config;

				// get default FW debug log level
				int default_log_level = 0;
				get_config_value_int(SQL_KEY_FW_LOG_LEVEL, &default_log_level);
				char default_log_level_str[NVM_EVENT_ARG_LEN];
				if (default_log_level <= FW_LOG_LEVEL_UNKNOWN)
				{
					s_snprintf(default_log_level_str, NVM_EVENT_ARG_LEN, "%s",
							fw_log_level_strings[default_log_level]);
				}
				else
				{
					s_strcpy(default_log_level_str,
							fw_log_level_strings[FW_LOG_LEVEL_UNKNOWN],
							NVM_EVENT_ARG_LEN);
				}

				// get default reasonable time drift
				int default_time_drift = 0;
				get_config_value_int(SQL_KEY_DEFAULT_TIME_DRIFT, &default_time_drift);
				char default_time_drift_str[10];
				s_snprintf(default_time_drift_str, 10, "%u", default_time_drift);

				// get default peak power budget, avg power budget min/max's
				int default_peak_power_budget_min_config = 0;
				int default_peak_power_budget_max_config = 0;
				get_config_value_int(SQL_KEY_DEFAULT_PEAK_POW_BUDGET_MIN,
						&default_peak_power_budget_min_config);
				get_config_value_int(SQL_KEY_DEFAULT_PEAK_POW_BUDGET_MAX,
						&default_peak_power_budget_max_config);
				char expected_peak_power_budget_range_str[NVM_EVENT_ARG_LEN];
				s_snprintf(expected_peak_power_budget_range_str, NVM_EVENT_ARG_LEN, "[%d - %d] mW.",
						default_peak_power_budget_min_config, default_peak_power_budget_max_config);
				NVM_UINT64 default_peak_power_budget_min = default_peak_power_budget_min_config;
				NVM_UINT64 default_peak_power_budget_max = default_peak_power_budget_max_config;

				int default_avg_power_budget_min_config = 0;
				int default_avg_power_budget_max_config = 0;
				get_config_value_int(SQL_KEY_DEFAULT_AVG_POW_BUDGET_MIN,
						&default_avg_power_budget_min_config);
				get_config_value_int(SQL_KEY_DEFAULT_AVG_POW_BUDGET_MAX,
						&default_avg_power_budget_max_config);
				char expected_avg_power_budget_range_str[NVM_EVENT_ARG_LEN];
				s_snprintf(expected_avg_power_budget_range_str, NVM_EVENT_ARG_LEN, "[%d - %d] mW.",
						default_avg_power_budget_min_config, default_avg_power_budget_max_config);
				NVM_UINT64 default_avg_power_budget_min = default_avg_power_budget_min_config;
				NVM_UINT64 default_avg_power_budget_max = default_avg_power_budget_max_config;

				// get default die sparing policy aggressiveness
				int default_die_sparing_level_config = 0;
				get_config_value_int(SQL_KEY_DEFAULT_DIE_SPARING_AGGRESSIVENESS,
						&default_die_sparing_level_config);
				char expected_die_sparing_aggressiveness_str[NVM_EVENT_ARG_LEN];
				s_snprintf(expected_die_sparing_aggressiveness_str, NVM_EVENT_ARG_LEN, "%d",
						default_die_sparing_level_config);
				NVM_UINT64 default_die_sparing_level = default_die_sparing_level_config;

				struct sensor sensors[NVM_MAX_DEVICE_SENSORS];
				NVM_UID device_uid_str;
				struct device_discovery discovery;
				for (int current_dev = 0; current_dev < dev_count; current_dev++)
				{
					if ((rc = exists_and_manageable(dimms[current_dev].uid,
							&discovery, 1)) == NVM_SUCCESS)
					{
						NVM_UID uid_str;
						uid_copy(dimms[current_dev].uid, uid_str);
						// verify if threshold values of temperature and spare capacity are
						// in accordance with best practices
						nvm_get_sensors(dimms[current_dev].uid, sensors, NVM_MAX_DEVICE_SENSORS);
						uid_copy(dimms[current_dev].uid, device_uid_str);
						if (!diag_check_real(p_diagnostic,
							DIAG_THRESHOLD_FW_MEDIA_TEMP,
							nvm_decode_temperature(
							sensors[SENSOR_MEDIA_TEMPERATURE].settings.upper_noncritical_threshold),
							&max_temp_threshold_config, EQUALITY_LESSTHANEQUAL))
						{
							NVM_UINT64 actual_temp_threshold =
								sensors[SENSOR_MEDIA_TEMPERATURE].
									settings.upper_noncritical_threshold;
							char actual_temp_threshold_str[10];
							s_snprintf(actual_temp_threshold_str, 10, "%.4f",
									nvm_decode_temperature(actual_temp_threshold));
							store_event_by_parts(EVENT_TYPE_DIAG_FW_CONSISTENCY,
									EVENT_SEVERITY_WARN,
								EVENT_CODE_DIAG_FW_BAD_TEMP_MEDIA_THRESHOLD,
								dimms[current_dev].uid, 0, uid_str, actual_temp_threshold_str,
								expected_temp_threshold_str,
								DIAGNOSTIC_RESULT_FAILED);
						(*p_results)++;
					}

					NVM_UINT64 actual_temp_threshold =
						sensors[SENSOR_CONTROLLER_TEMPERATURE].settings.upper_noncritical_threshold;
					if (!diag_check_real(p_diagnostic,
							DIAG_THRESHOLD_FW_CORE_TEMP,
							nvm_decode_temperature(actual_temp_threshold),
							&max_temp_threshold_config, EQUALITY_LESSTHANEQUAL))
					{
						char actual_temp_threshold_str[10];
						s_snprintf(actual_temp_threshold_str, 10, "%.4f",
								nvm_decode_temperature(actual_temp_threshold));
						store_event_by_parts(EVENT_TYPE_DIAG_FW_CONSISTENCY,
								EVENT_SEVERITY_WARN,
								EVENT_CODE_DIAG_FW_BAD_TEMP_CONTROLLER_THRESHOLD,
									dimms[current_dev].uid, 0, uid_str, actual_temp_threshold_str,
									expected_temp_threshold_str,
									DIAGNOSTIC_RESULT_FAILED);
							(*p_results)++;
						}

						if (!diag_check(p_diagnostic,
								DIAG_THRESHOLD_FW_SPARE,
								sensors[SENSOR_SPARECAPACITY].settings.lower_noncritical_threshold,
								&min_spare_block_threshold, EQUALITY_GREATERTHANEQUAL))
						{
							char actual_spare_block_threshold_str[10];
							s_snprintf(actual_spare_block_threshold_str, 10, "%u",
								sensors[SENSOR_SPARECAPACITY].settings.lower_noncritical_threshold);
							store_event_by_parts(EVENT_TYPE_DIAG_FW_CONSISTENCY,
									EVENT_SEVERITY_WARN,
									EVENT_CODE_DIAG_FW_BAD_SPARE_BLOCK,
									dimms[current_dev].uid, 0, uid_str,
									expected_spare_block_threshold_str,
									actual_spare_block_threshold_str,
									DIAGNOSTIC_RESULT_FAILED);
							(*p_results)++;
						}

						// verify FW debug log level is set in accordance with best practices
						enum fw_log_level log_level;
						if (NVM_SUCCESS == nvm_get_fw_log_level(dimms[current_dev].uid,
								&log_level))
						{
							if ((!(p_diagnostic->excludes & DIAG_THRESHOLD_FW_DEBUGLOG)) &&
										(log_level != default_log_level))
							{
								char current_log_level_str[NVM_EVENT_ARG_LEN];
								s_snprintf(current_log_level_str, NVM_EVENT_ARG_LEN, "%s",
										fw_log_level_strings[log_level]);
								store_event_by_parts(EVENT_TYPE_DIAG_FW_CONSISTENCY,
										EVENT_SEVERITY_WARN,
										EVENT_CODE_DIAG_FW_BAD_FW_LOG_LEVEL,
										dimms[current_dev].uid, 0, uid_str, current_log_level_str,
										default_log_level_str,
										DIAGNOSTIC_RESULT_FAILED);
								(*p_results)++;
							}
						}

						if (!(p_diagnostic->excludes & DIAG_THRESHOLD_FW_TIME))
						{
							// verify host time and NVM DIMM time are within reasonable window.
							struct pt_payload_system_time time_payload;
							memset(&time_payload, 0, sizeof (time_payload));
							time_t now = time(0); // returns system time in seconds
							if ((rc = get_fw_system_time(dimms[current_dev].device_handle,
									&time_payload)) == NVM_SUCCESS)
							{
								time_t current_time_drift = now - time_payload.time;
								char current_time_drift_str[NVM_EVENT_ARG_LEN];
								char time_drift_lag_str[NVM_EVENT_ARG_LEN];
								if ((current_time_drift > 0) &&
										(current_time_drift > default_time_drift))
								{
									s_snprintf(current_time_drift_str,
											NVM_EVENT_ARG_LEN, "%llu",
											(unsigned long long) current_time_drift);
									s_snprintf(time_drift_lag_str, NVM_EVENT_ARG_LEN,
											"%s", "<");
									store_event_by_parts(
											EVENT_TYPE_DIAG_FW_CONSISTENCY,
											EVENT_SEVERITY_WARN,
											EVENT_CODE_DIAG_FW_SYSTEM_TIME_DRIFT,
											dimms[current_dev].uid, 0, uid_str,
											time_drift_lag_str, current_time_drift_str,
											DIAGNOSTIC_RESULT_FAILED);
									(*p_results)++;
								}
								else if ((current_time_drift < 0) &&
										(abs(current_time_drift) > default_time_drift))
								{
									s_snprintf(current_time_drift_str,
											NVM_EVENT_ARG_LEN, "%llu", abs(
													current_time_drift));
									s_snprintf(time_drift_lag_str, NVM_EVENT_ARG_LEN,
											"%s", ">");
									store_event_by_parts(
											EVENT_TYPE_DIAG_FW_CONSISTENCY,
											EVENT_SEVERITY_WARN,
											EVENT_CODE_DIAG_FW_SYSTEM_TIME_DRIFT,
											dimms[current_dev].uid, 0, uid_str,
											time_drift_lag_str, current_time_drift_str,
											DIAGNOSTIC_RESULT_FAILED);
									(*p_results)++;
								}
							}
						}

						if (!(p_diagnostic->excludes & DIAG_THRESHOLD_FW_POW_MGMT_POLICY))
						{
							// verify power management policies meet best practices
							struct pt_payload_power_mgmt_policy power_payload;
							memset(&power_payload, 0, sizeof (power_payload));
							get_fw_power_mgmt_policy(dimms[current_dev].device_handle,
									&power_payload);

							char field_str[NVM_EVENT_ARG_LEN];
							if (power_payload.enabled)
							{

								if ((diag_check(p_diagnostic,
										DIAG_THRESHOLD_FW_AVG_POW_BUDGET_MIN,
										power_payload.average_power_budget,
										&default_avg_power_budget_min,
										EQUALITY_LESSTHAN)) ||
										(diag_check(p_diagnostic,
										DIAG_THRESHOLD_FW_AVG_POW_BUDGET_MAX,
										power_payload.average_power_budget,
										&default_avg_power_budget_max,
										EQUALITY_GREATHERTHAN)))
								{
									s_snprintf(field_str, NVM_EVENT_ARG_LEN, "%s: %hu",
											"average power budget",
											power_payload.average_power_budget);
									store_event_by_parts(
											EVENT_TYPE_DIAG_FW_CONSISTENCY,
											EVENT_SEVERITY_WARN,
											EVENT_CODE_DIAG_FW_BAD_POWER_MGMT_POLICY,
											dimms[current_dev].uid, 0, uid_str,
											field_str,
											expected_avg_power_budget_range_str,
											DIAGNOSTIC_RESULT_FAILED);
									(*p_results)++;
								}

								if ((diag_check(p_diagnostic,
										DIAG_THRESHOLD_FW_PEAK_POW_BUDGET_MIN,
										power_payload.peak_power_budget,
										&default_peak_power_budget_min,
										EQUALITY_LESSTHAN)) ||
										(diag_check(p_diagnostic,
										DIAG_THRESHOLD_FW_PEAK_POW_BUDGET_MAX,
										power_payload.peak_power_budget,
										&default_peak_power_budget_max,
										EQUALITY_GREATHERTHAN)))
								{
									s_snprintf(field_str, NVM_EVENT_ARG_LEN, "%s: %hu",
											"peak power budget",
											power_payload.peak_power_budget);
									store_event_by_parts(
											EVENT_TYPE_DIAG_FW_CONSISTENCY,
											EVENT_SEVERITY_WARN,
											EVENT_CODE_DIAG_FW_BAD_POWER_MGMT_POLICY,
											dimms[current_dev].uid, 0, uid_str,
											field_str,
											expected_peak_power_budget_range_str,
											DIAGNOSTIC_RESULT_FAILED);
									(*p_results)++;
								}
							}
							else
							{
								s_snprintf(field_str, NVM_EVENT_ARG_LEN, "%s: %hhu",
										"power management policy enable",
										power_payload.enabled);
								char expected_power_mgmt_enabled_str[NVM_EVENT_ARG_LEN];
								s_snprintf(expected_power_mgmt_enabled_str,
										NVM_EVENT_ARG_LEN, "%u", 1);
								store_event_by_parts(EVENT_TYPE_DIAG_FW_CONSISTENCY,
										EVENT_SEVERITY_WARN,
										EVENT_CODE_DIAG_FW_BAD_POWER_MGMT_POLICY,
										dimms[current_dev].uid, 0, uid_str, field_str,
										expected_power_mgmt_enabled_str,
										DIAGNOSTIC_RESULT_FAILED);
								(*p_results)++;
							}
						}

						if (!(p_diagnostic->excludes & DIAG_THRESHOLD_FW_DIE_SPARING_POLICY))
						{
							// verify die sparing policies are in accordance with best practices
							struct pt_get_die_spare_policy spare_payload;
							memset(&spare_payload, 0, sizeof (spare_payload));
							get_fw_die_spare_policy(discovery.device_handle,
									&spare_payload);
							char field_str[NVM_EVENT_ARG_LEN];
							if (spare_payload.enable)
							{
								if (!diag_check(p_diagnostic,
										DIAG_THRESHOLD_FW_DIE_SPARING_LEVEL,
										spare_payload.aggressiveness,
										&default_die_sparing_level, EQUALITY_EQUAL))
								{
									s_snprintf(field_str, NVM_EVENT_ARG_LEN,
											"%s: %hhu", "die sparing aggressiveness",
											spare_payload.aggressiveness);
									store_event_by_parts(
											EVENT_TYPE_DIAG_FW_CONSISTENCY,
											EVENT_SEVERITY_WARN,
											EVENT_CODE_DIAG_FW_BAD_DIE_SPARING_POLICY,
											dimms[current_dev].uid, 0, uid_str, field_str,
											expected_die_sparing_aggressiveness_str,
											DIAGNOSTIC_RESULT_FAILED);
									(*p_results)++;
								}
							}
							else
							{
								s_snprintf(field_str, NVM_EVENT_ARG_LEN, "%s: %hhu",
										"die sparing policy enable",
										spare_payload.enable);
								char expected_die_sparing_enabled_str[NVM_EVENT_ARG_LEN];
								s_snprintf(expected_die_sparing_enabled_str,
										NVM_EVENT_ARG_LEN, "%u", 1);
								store_event_by_parts(EVENT_TYPE_DIAG_FW_CONSISTENCY,
										EVENT_SEVERITY_WARN,
										EVENT_CODE_DIAG_FW_BAD_DIE_SPARING_POLICY,
										dimms[current_dev].uid, 0, uid_str, field_str,
										expected_die_sparing_enabled_str,
										DIAGNOSTIC_RESULT_FAILED);
								(*p_results)++;
							}
						}
					}
				}

				if ((rc == NVM_SUCCESS) && (*p_results == 0)) // No errors/warnings
				{
					store_event_by_parts(EVENT_TYPE_DIAG_FW_CONSISTENCY,
							EVENT_SEVERITY_INFO, EVENT_CODE_DIAG_FW_SUCCESS, NULL, 0,
							NULL, NULL, NULL, DIAGNOSTIC_RESULT_OK);
					(*p_results)++;
				}
			}// nvm_get_devices failed
			else
			{
				rc = dev_count;
			}
		} // nvm_get_device_count failed
		else
		{
			rc = dev_count;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to find the optimal firmware version
 */
void find_optimal_fw_revision(char *optimal_fw_revision, char *model_number,
		const struct device_discovery *p_dimms, int dev_count)
{
	s_strncpy(optimal_fw_revision, NVM_VERSION_LEN,
							"00.00.00.0000", NVM_VERSION_LEN);
	for (int i = 0; i < dev_count; i++)
	{
		if (strncmp(model_number, p_dimms[i].model_number, NVM_MODEL_LEN) == 0)
		{
			// parse the version string into parts
			NVM_UINT16 opt_major, opt_minor, opt_hotfix, opt_build;
			NVM_UINT16 major, minor, hotfix, build;
			parse_main_revision(&opt_major, &opt_minor, &opt_hotfix,
					&opt_build, optimal_fw_revision, NVM_VERSION_LEN);
			parse_main_revision(&major, &minor, &hotfix, &build,
					p_dimms[i].fw_revision, NVM_VERSION_LEN);

			if (opt_major < major)
			{
				s_strncpy(optimal_fw_revision, NVM_VERSION_LEN,
						p_dimms[i].fw_revision, NVM_VERSION_LEN);
			}
			else if (opt_major == major)
			{
				if (opt_minor < minor)
				{
					s_strncpy(optimal_fw_revision, NVM_VERSION_LEN,
							p_dimms[i].fw_revision, NVM_VERSION_LEN);
				}
				else if (opt_minor == minor)
				{
					if (opt_hotfix < hotfix)
					{
						s_strncpy(optimal_fw_revision, NVM_VERSION_LEN,
								p_dimms[i].fw_revision, NVM_VERSION_LEN);
					}
					else if (opt_hotfix == hotfix)
					{
						if (opt_build < build)
						{
							s_strncpy(optimal_fw_revision, NVM_VERSION_LEN,
									p_dimms[i].fw_revision, NVM_VERSION_LEN);
						}
					}
				}
			}
		}
	}
}

/*
 * Helper function to get firmware system time
 */
int get_fw_system_time(NVM_NFIT_DEVICE_HANDLE dimm_handle, struct pt_payload_system_time *payload)
{

	int rc = NVM_SUCCESS;
	struct fw_cmd fw_cmd;
	memset(&fw_cmd, 0, sizeof (fw_cmd));
	fw_cmd.device_handle = dimm_handle.handle;
	fw_cmd.opcode = PT_GET_ADMIN_FEATURES;
	fw_cmd.sub_opcode = SUBOP_SYSTEM_TIME;
	fw_cmd.output_payload_size = sizeof (*payload);
	fw_cmd.output_payload = payload;
	rc = ioctl_passthrough_cmd(&fw_cmd);
	return rc;
}
