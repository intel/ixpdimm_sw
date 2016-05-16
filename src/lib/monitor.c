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
 * Implementation of the Native API monitoring functions.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "device_adapter.h"
#include "nvm_management.h"
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include "monitor.h"
#include <persistence/logging.h>
#include <string/s_str.h>
#include <uid/uid.h>
#include <cr_i18n.h>
#include <os/os_adapter.h>
#include <persistence/schema.h>
#include "device_utilities.h"
#include "capabilities.h"
#include "nvm_context.h"
#include "system.h"
#include <time/time_utilities.h>

/*
 * Local defines
 */
// upper limits on valid threshold values - defined in FIS
#define	LIMIT_SPARE_THRESHOLD_VALUE	100u

/*
 * global (to this file) variables
 */
#ifdef __WINDOWS__
#include <windows.h>
	extern HANDLE g_eventmonitor_lock;
#else
	extern pthread_mutex_t g_eventmonitor_lock;
#endif

// Have we added a hook to get called back when the db changes?
static int g_is_polling = 0;
// event callback list
static struct event_notify_callback g_event_callback_list[MAX_EVENT_SUBSCRIBERS];
static int g_current_event_id = 0; // When polling this stores the most recent event id
static NVM_UINT32 g_poll_interval_sec = 60; // Default poll interval. Overridden in config database

/*
 * Helper functions
 */
static void *poll_events(void *arg); // run in a seperate thread for polling the event table
static int timeToQuit(unsigned long timeoutSeconds); // helps polling to know when to stop
static int get_nvm_event_id(); // get most recent id from event table


int get_smart_log_sensors(const NVM_UID device_uid,
		const NVM_UINT32 dev_handle, struct sensor *p_sensors)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	/*
	 * If unable to get threshold, there is still a lot of sensor info that can be set from
	 * the smart health log ... if it's successful. So if alarm thresholds fails, just set
	 * it to 0 and continue.
	 */
	struct pt_payload_alarm_thresholds thresholds;
	memset(&thresholds, 0, sizeof (thresholds));
	int threshold_rc = fw_get_alarm_thresholds(dev_handle, &thresholds);
	KEEP_ERROR(rc, threshold_rc);

	struct pt_payload_smart_health dimm_smart;
	memset(&dimm_smart, 0, sizeof (dimm_smart));
	int smart_health_rc = fw_get_smart_health(dev_handle, &dimm_smart);
	KEEP_ERROR(rc, smart_health_rc);

	if (smart_health_rc == NVM_SUCCESS)
	{
		struct sensor *p_sensor = NULL;
		// Media Temperature
		{
			p_sensor = &p_sensors[SENSOR_MEDIA_TEMPERATURE];
			if (dimm_smart.validation_flags.parts.media_temperature_field)
			{
				p_sensor->reading = nvm_encode_temperature(
					fw_convert_fw_celsius_to_float(dimm_smart.media_temperature));
			}
			p_sensor->upper_critical_settable = 1;
			p_sensor->upper_critical_support  = 1;
			p_sensor->settings.enabled = (NVM_BOOL)((thresholds.enable &
					THRESHOLD_ENABLED_MEDIA_TEMP) ? 1 : 0);
			p_sensor->settings.upper_critical_threshold =
					nvm_encode_temperature(
						fw_convert_fw_celsius_to_float(thresholds.media_temperature));
			if (dimm_smart.validation_flags.parts.alarm_trips_field &&
					((dimm_smart.alarm_trips & MEDIA_TEMPERATURE_TRIP_BIT) != 0))
			{
				p_sensor->current_state = SENSOR_CRITICAL;
			}
			else if (!dimm_smart.validation_flags.parts.alarm_trips_field ||
					!dimm_smart.validation_flags.parts.media_temperature_field)
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
			}
			else
			{
				p_sensor->current_state = SENSOR_NORMAL;
			}
		}

		// Spare Capacity
		{
			p_sensor = &p_sensors[SENSOR_SPARECAPACITY];
			if (dimm_smart.validation_flags.parts.spare_block_field)
			{
				p_sensor->reading = (NVM_UINT64)dimm_smart.spare;
			}
			p_sensor->lower_critical_settable = 1;
			p_sensor->lower_critical_support  = 1;
			p_sensor->settings.enabled = (NVM_BOOL)((thresholds.enable &
					THRESHOLD_ENABLED_SPARE) ? 1 : 0);
			p_sensor->settings.lower_critical_threshold = thresholds.spare;
			if (dimm_smart.validation_flags.parts.alarm_trips_field &&
					((dimm_smart.alarm_trips & SPARE_BLOCKS_TRIP_BIT) != 0))
			{
				p_sensor->current_state = SENSOR_CRITICAL;
			}
			else if (!dimm_smart.validation_flags.parts.alarm_trips_field ||
					!dimm_smart.validation_flags.parts.spare_block_field)
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
			}
			else
			{
				p_sensor->current_state = SENSOR_NORMAL;
			}
		}

		// Wear Level
		{
			int percent_used_threshold;
			get_config_value_int(SQL_KEY_PERCENT_USED_THRESHOLD, &percent_used_threshold);
			p_sensor = &p_sensors[SENSOR_WEARLEVEL];
			if (dimm_smart.validation_flags.parts.percentage_used_field)
			{
				p_sensor->reading = (NVM_UINT64)dimm_smart.percentage_used;
			}
			if (!dimm_smart.validation_flags.parts.percentage_used_field)
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
			}
			else if (p_sensor->reading >= percent_used_threshold)
			{
				p_sensor->current_state = SENSOR_CRITICAL;
			}
			else
			{
				p_sensor->current_state = SENSOR_NORMAL;
			}
		}

		// Power Cycles
		{
			p_sensor = &p_sensors[SENSOR_POWERCYCLES];
			if (dimm_smart.validation_flags.parts.sizeof_vendor_data_field)
			{
				p_sensor->reading = dimm_smart.vendor_data.power_cycles;
				p_sensor->current_state = SENSOR_NORMAL;
			}
		}

		// Power On Time
		{
			p_sensor = &p_sensors[SENSOR_POWERONTIME];
			if (dimm_smart.validation_flags.parts.sizeof_vendor_data_field)
			{
				p_sensor->reading = dimm_smart.vendor_data.power_on_seconds;
				p_sensor->current_state = SENSOR_NORMAL;
			}
		}

		// Up Time
		{
			p_sensor = &p_sensors[SENSOR_UPTIME];
			if (dimm_smart.validation_flags.parts.sizeof_vendor_data_field)
			{
				p_sensor->reading = dimm_smart.vendor_data.uptime;
				p_sensor->current_state = SENSOR_NORMAL;
			}
		}

		// Unsafe Shutdowns
		{
			p_sensor = &p_sensors[SENSOR_UNSAFESHUTDOWNS];
			if (dimm_smart.validation_flags.parts.sizeof_vendor_data_field)
			{
				p_sensor->reading = dimm_smart.vendor_data.unsafe_shutdowns;
				p_sensor->current_state = SENSOR_NORMAL;
			}
		}

		// Controller Temperature
		{
			p_sensor = &p_sensors[SENSOR_CONTROLLER_TEMPERATURE];
			if (dimm_smart.validation_flags.parts.sizeof_vendor_data_field)
			{
				p_sensor->reading = nvm_encode_temperature(
					fw_convert_fw_celsius_to_float(dimm_smart.controller_temperature));
			}
			p_sensor->upper_critical_settable = 1;
			p_sensor->upper_critical_support  = 1;
			p_sensor->settings.enabled = (NVM_BOOL)((thresholds.enable &
					THRESHOLD_ENABLED_CONTROLLER_TEMP) ? 1 : 0);
			p_sensor->settings.upper_critical_threshold =
					nvm_encode_temperature(
						fw_convert_fw_celsius_to_float(thresholds.controller_temperature));
			if (dimm_smart.validation_flags.parts.alarm_trips_field &&
					((dimm_smart.alarm_trips & CONTROLLER_TEMP_TRIP_BIT) != 0))
			{
				p_sensor->current_state = SENSOR_CRITICAL;
			}
			else if (!dimm_smart.validation_flags.parts.alarm_trips_field ||
					!dimm_smart.validation_flags.parts.sizeof_vendor_data_field)
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
			}
			else
			{
				p_sensor->current_state = SENSOR_NORMAL;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_media_page2_sensors(const NVM_UID device_uid,
		const NVM_UINT32 dev_handle, struct sensor *p_sensors)
{
	COMMON_LOG_ENTRY();
	struct pt_payload_memory_info_page2 meminfo2;
	memset(&meminfo2, 0, sizeof (meminfo2));
	int rc = fw_get_memory_info_page(dev_handle, 2, &meminfo2, sizeof (meminfo2));
	if (rc == NVM_SUCCESS)
	{
		// Media errors - Uncorrectable
		p_sensors[SENSOR_MEDIAERRORS_UNCORRECTABLE].reading = meminfo2.media_errors_uc;
		p_sensors[SENSOR_MEDIAERRORS_UNCORRECTABLE].current_state = SENSOR_NORMAL;

		// Media errors - Corrected
		NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(meminfo2.media_errors_ce,
				p_sensors[SENSOR_MEDIAERRORS_CORRECTED].reading);
		p_sensors[SENSOR_MEDIAERRORS_CORRECTED].current_state = SENSOR_NORMAL;

		// Media errors - erasure coded
		NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(meminfo2.media_errors_ecc,
				p_sensors[SENSOR_MEDIAERRORS_ERASURECODED].reading)
		p_sensors[SENSOR_MEDIAERRORS_ERASURECODED].current_state = SENSOR_NORMAL;

		// Write Count - Maximum
		p_sensors[SENSOR_WRITECOUNT_MAXIMUM].reading = meminfo2.write_count_max;
		p_sensors[SENSOR_WRITECOUNT_MAXIMUM].current_state = SENSOR_NORMAL;

		// Write Count - Average
		p_sensors[SENSOR_WRITECOUNT_AVERAGE].reading = meminfo2.write_count_average;
		p_sensors[SENSOR_WRITECOUNT_AVERAGE].current_state = SENSOR_NORMAL;

		// Media Errors - Host
		p_sensors[SENSOR_MEDIAERRORS_HOST].reading = meminfo2.uncorrectable_host;
		p_sensors[SENSOR_MEDIAERRORS_HOST].current_state = SENSOR_NORMAL;

		// Media Errors - Non Host
		p_sensors[SENSOR_MEDIAERRORS_NONHOST].reading = meminfo2.uncorrectable_non_host;
		p_sensors[SENSOR_MEDIAERRORS_NONHOST].current_state = SENSOR_NORMAL;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_fw_error_log_sensors(const NVM_UID device_uid,
		const NVM_UINT32 dev_handle, struct sensor sensors[NVM_MAX_DEVICE_SENSORS])
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;
	unsigned int error_count = 0;

	// Count from the different error logs. If an error is found, subsequent calls won't be
	// executed. The sensor will only be filled out if all counts were a success.
	IF_SUCCESS_EXEC_AND_SUM(fw_get_fw_error_log_count(dev_handle,
			DEV_FW_ERR_LOG_LOW, DEV_FW_ERR_LOG_MEDIA), rc, error_count);
	IF_SUCCESS_EXEC_AND_SUM(fw_get_fw_error_log_count(dev_handle,
			DEV_FW_ERR_LOG_HIGH, DEV_FW_ERR_LOG_MEDIA), rc, error_count);
	IF_SUCCESS_EXEC_AND_SUM(fw_get_fw_error_log_count(dev_handle,
			DEV_FW_ERR_LOG_LOW, DEV_FW_ERR_LOG_THERMAL), rc, error_count);
	IF_SUCCESS_EXEC_AND_SUM(fw_get_fw_error_log_count(dev_handle,
			DEV_FW_ERR_LOG_HIGH, DEV_FW_ERR_LOG_THERMAL), rc, error_count);

	if (rc == NVM_SUCCESS)
	{
		sensors[SENSOR_FWERRORLOGCOUNT].reading = error_count;
		sensors[SENSOR_FWERRORLOGCOUNT].current_state = SENSOR_NORMAL;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_power_limited_sensor(const NVM_UID device_uid,
		const NVM_UINT32 dev_handle, struct sensor sensors[NVM_MAX_DEVICE_SENSORS])
{
	COMMON_LOG_ENTRY();
	struct device_discovery device;
	memset(&device, 0, sizeof (device));
	int rc = nvm_get_device_discovery(device_uid, &device);
	if (rc == NVM_SUCCESS)
	{
		int power_reading = get_dimm_power_limited(device.socket_id);
		if (power_reading >= 0)
		{
			sensors[SENSOR_POWERLIMITED].reading = (NVM_UINT64)power_reading;
			sensors[SENSOR_POWERLIMITED].current_state = SENSOR_NORMAL;
		}
		else
		{
			rc = power_reading;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to populate sensor information with default attributes
 * that don't require current values.
 */
void initialize_sensors(const NVM_UID device_uid,
		struct sensor sensors[NVM_MAX_DEVICE_SENSORS])
{
	for (int i = 0; i < NVM_MAX_DEVICE_SENSORS; i++)
	{
		memmove(sensors[i].device_uid, device_uid, NVM_MAX_UID_LEN);
		sensors[i].type = i;
	}
	sensors[SENSOR_MEDIA_TEMPERATURE].units = UNIT_CELSIUS;
	sensors[SENSOR_SPARECAPACITY].units = UNIT_PERCENT;
	sensors[SENSOR_WEARLEVEL].units = UNIT_PERCENT;
	sensors[SENSOR_POWERCYCLES].units = UNIT_CYCLES;
	sensors[SENSOR_POWERONTIME].units = UNIT_SECONDS;
	sensors[SENSOR_UPTIME].units = UNIT_SECONDS;
	sensors[SENSOR_UNSAFESHUTDOWNS].units = UNIT_COUNT;
	sensors[SENSOR_MEDIAERRORS_UNCORRECTABLE].units = UNIT_COUNT;
	sensors[SENSOR_MEDIAERRORS_CORRECTED].units = UNIT_COUNT;
	sensors[SENSOR_MEDIAERRORS_ERASURECODED].units = UNIT_COUNT;
	sensors[SENSOR_WRITECOUNT_MAXIMUM].units = UNIT_COUNT;
	sensors[SENSOR_WRITECOUNT_AVERAGE].units = UNIT_COUNT;
	sensors[SENSOR_MEDIAERRORS_HOST].units = UNIT_COUNT;
	sensors[SENSOR_MEDIAERRORS_NONHOST].units = UNIT_COUNT;
	sensors[SENSOR_FWERRORLOGCOUNT].units = UNIT_COUNT;
	sensors[SENSOR_POWERLIMITED].units = UNIT_COUNT;
	sensors[SENSOR_CONTROLLER_TEMPERATURE].units = UNIT_CELSIUS;
}

int get_all_sensors(const NVM_UID device_uid,
		const NVM_UINT32 dev_handle, struct sensor sensors[NVM_MAX_DEVICE_SENSORS])
{
	COMMON_LOG_ENTRY();

	initialize_sensors(device_uid, sensors);
	int rc = get_smart_log_sensors(device_uid, dev_handle, sensors);
	KEEP_ERROR(rc, get_media_page2_sensors(device_uid, dev_handle, sensors));
	KEEP_ERROR(rc, get_fw_error_log_sensors(device_uid, dev_handle, sensors));
	KEEP_ERROR(rc, get_power_limited_sensor(device_uid, dev_handle, sensors));

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * This function populates sensor information for the specified device.
 * Sensors are used to monitor a particular aspect of a device by settings
 * thresholds against a current value.  Sensor information is returned
 * as part of the device_details structure.
 * The number of sensors for a given device is defined as NVM_MAX_DEVICE_SENSORS.
 */
int nvm_get_sensors(const NVM_UID device_uid, struct sensor *p_sensors,
		const NVM_UINT16 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct device_discovery discovery;
	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(get_sensors)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving "NVM_DIMM_NAME" sensors is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_sensors == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_status is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		struct sensor sensors[NVM_MAX_DEVICE_SENSORS];
		memset(sensors, 0, sizeof (sensors));
		const NVM_UINT32 dev_handle = discovery.device_handle.handle;
		get_all_sensors(device_uid, dev_handle, sensors);

		// even if the array is too small, we still want to fill in as many as possible.
		// Just return the error code
		if (count < NVM_MAX_DEVICE_SENSORS)
		{
			rc = NVM_ERR_ARRAYTOOSMALL;
		}

		// fill in the user provided sensor array
		memset(p_sensors, 0, sizeof (struct sensor) * count);
		for (int i = 0; i < count; i++)
		{
			memmove(&p_sensors[i], &sensors[i], sizeof (struct sensor));
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * This function queries and passes back information about a specific sensor.
 */
int nvm_get_sensor(const NVM_UID device_uid, const enum sensor_type type,
		struct sensor *p_sensor)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;
	struct sensor sensors[NVM_MAX_DEVICE_SENSORS];

	if (NULL == p_sensor)
	{
		COMMON_LOG_ERROR("sensor return data pointer is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((type < SENSOR_MEDIA_TEMPERATURE) || (type >= NVM_MAX_DEVICE_SENSORS))
	{
		COMMON_LOG_ERROR("request for an out of range sensor type");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = nvm_get_sensors(device_uid, sensors, NVM_MAX_DEVICE_SENSORS))
			== NVM_SUCCESS)
	{
		memmove(p_sensor, &sensors[type], sizeof (struct sensor));
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * These strings correlate to enum sensor_type
 */
static const char *SENSOR_STRINGS[NVM_MAX_DEVICE_SENSORS] =
{
	// SENSOR_MEDIA_TEMPERATURE
	N_TR("Media Temperature"),
	// SENSOR_SPARECAPACITY
	N_TR("Spare Capacity"),
	// SENSOR_WEARLEVEL
	N_TR("Wear Level"),
	// SENSOR_POWERCYCLES
	N_TR("Power Cycles"),
	// SENSOR_POWERONTIME
	N_TR("Power On Time"),
	// SENSOR_UPTIME
	N_TR("Uptime"),
	// SENSOR_UNSAFESHUTDOWNS
	N_TR("Unsafe Shutdowns"),
	// SENSOR_FWERRORLOGCOUNT
	N_TR("FW Error Log Count"),
	// SENSOR_POWERLIMITED
	N_TR("Power Limited"),
	// SENSOR_MEDIAERRORS_UNCORRECTABLE
	N_TR("Uncorrectable Media Errors"),
	// SENSOR_MEDIAERRORS_CORRECTED
	N_TR("Corrected Media Errors"),
	// SENSOR_MEDIAERRORS_ERASURECODED
	N_TR("Erasure Coded Media Errors"),
	// SENSOR_WRITECOUNT_MAXIMUM
	N_TR("Maximum Write Count"),
	// SENSOR_WRITECOUNT_AVERAGE
	N_TR("Average Write Count"),
	// SENSOR_MEDIAERRORS_HOST
	N_TR("Host Media Errors"),
	// SENSOR_MEDIAERRORS_NONHOST
	N_TR("Non-host Media Errors"),
	// SENSOR_CONTROLLER_TEMPERATURE
	N_TR("Controller Temperature"),
};

void sensor_type_to_string(const enum sensor_type type, char *p_dst, size_t dst_size)
{
	if (type < NVM_MAX_DEVICE_SENSORS)
	{
		s_strcpy(p_dst, SENSOR_STRINGS[type], dst_size);
	}
	else
	{
		s_strcpy(p_dst, N_TR("Unknown"), dst_size);
	}
}

/*
 * This function modifies the settings for the sensor specified.
 */
int nvm_set_sensor_settings(const NVM_UID device_uid, const enum sensor_type type,
		const struct sensor_settings *p_settings)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;
	struct pt_payload_alarm_thresholds thresholds;
	memset(&thresholds, 0, sizeof (struct pt_payload_alarm_thresholds));

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_sensors)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying "NVM_DIMM_NAME" sensors is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((type != SENSOR_CONTROLLER_TEMPERATURE) &&
			(type != SENSOR_MEDIA_TEMPERATURE) &&
			(type != SENSOR_SPARECAPACITY))
	{
		COMMON_LOG_ERROR("Invalid parameter, type is invalid");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_settings == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_sensor_settings is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1))
			== NVM_SUCCESS)
	{
		if ((rc = fw_get_alarm_thresholds(discovery.device_handle.handle, &thresholds))
				== NVM_SUCCESS)
		{
			if ((int)type == SENSOR_MEDIA_TEMPERATURE)
			{
				thresholds.media_temperature =
					fw_convert_float_to_fw_celsius(
						nvm_decode_temperature(p_settings->upper_critical_threshold));

				if (p_settings->enabled)
				{
					thresholds.enable |= THRESHOLD_ENABLED_MEDIA_TEMP;
				}
				else
				{
					thresholds.enable &= ~THRESHOLD_ENABLED_MEDIA_TEMP;
				}
			}
			else if ((int)type == SENSOR_CONTROLLER_TEMPERATURE)
			{
				thresholds.controller_temperature =
					fw_convert_float_to_fw_celsius(
						nvm_decode_temperature(p_settings->upper_critical_threshold));

				if (p_settings->enabled)
				{
					thresholds.enable |= THRESHOLD_ENABLED_CONTROLLER_TEMP;
				}
				else
				{
					thresholds.enable &= ~THRESHOLD_ENABLED_CONTROLLER_TEMP;
				}
			}
			else
			{
				NVM_UINT64 threshold = p_settings->lower_critical_threshold;
				if ((threshold > 0) && (threshold < LIMIT_SPARE_THRESHOLD_VALUE))
				{
					thresholds.spare = (unsigned char)threshold;
					if (p_settings->enabled)
					{
						thresholds.enable |= THRESHOLD_ENABLED_SPARE;
					}
					else
					{
						thresholds.enable &= ~THRESHOLD_ENABLED_SPARE;
					}
				}
				else
				{
					rc = NVM_ERR_BADTHRESHOLD;
				}
			}

			if (rc == NVM_SUCCESS) // no errors
			{
				if ((rc = fw_set_alarm_thresholds(discovery.device_handle.handle, &thresholds))
						== NVM_SUCCESS)
				{
					// Log an event indicating we successfully changed the settings
					NVM_EVENT_ARG type_arg;
					sensor_type_to_string(type, type_arg, NVM_EVENT_ARG_LEN);
					NVM_EVENT_ARG uid_arg;
					uid_to_event_arg(device_uid, uid_arg);
					log_mgmt_event(EVENT_SEVERITY_INFO,
							EVENT_CODE_MGMT_SENSOR_SETTINGS_CHANGE,
							device_uid,
							0, // no action required
							type_arg, uid_arg, NULL);
				}

				// clear any device context - device has changed
				invalidate_devices();
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * Convert a uid to an event argument (string)
 */
void uid_to_event_arg(const NVM_UID uid, NVM_EVENT_ARG arg)
{
	if (uid && arg)
	{
		NVM_UID uid_str;
		uid_copy(uid, uid_str);
		s_strcpy(arg, uid_str, NVM_EVENT_ARG_LEN);
	}
}

/*
 * Log a management software event to the config DB and to syslog if applicable
 */
int log_mgmt_event(const enum event_severity severity, const NVM_UINT16 code,
		const NVM_UID device_uid, const NVM_BOOL action_required, const NVM_EVENT_ARG arg1,
		const NVM_EVENT_ARG arg2, const NVM_EVENT_ARG arg3)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// store the event in the db and syslog
	rc = store_event_by_parts(EVENT_TYPE_MGMT, severity, code, device_uid,
			action_required, arg1, arg2, arg3, DIAGNOSTIC_RESULT_UNKNOWN);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * Retrieve the number of events in the native API library event database.
 */
int nvm_get_event_count(const struct event_filter *p_filter)
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		rc = process_events_matching_filter(p_filter, NULL, 0, 0);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve a list of stored events from the native API library database and
 * optionally filter the results.
 */
int nvm_get_events(const struct event_filter *p_filter,
		struct event *p_events, const NVM_UINT16 count)
{
	COMMON_LOG_ENTRY_PARAMS("count: %d", count);
	int rc = 0;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_events == NULL)
	{
		rc = NVM_ERR_INVALIDPARAMETER;
		COMMON_LOG_ERROR("p_events is NULL");
	}
	else if (count == 0)
	{
		rc = NVM_ERR_INVALIDPARAMETER;
		COMMON_LOG_ERROR("count is 0");
	}
	else
	{
		memset(p_events, 0, count * sizeof (struct event));
		rc = process_events_matching_filter(p_filter, p_events, count, 0);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int nvm_purge_events(const struct event_filter *p_filter)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		rc = process_events_matching_filter(p_filter, NULL, 0, 1);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Be sure we are listening for database updates
 */
int init_event_notify()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;


	if (mutex_lock(&g_eventmonitor_lock))
	{
		// if we're not already polling for new events
		if (!g_is_polling)
		{
			// first time registering, so zero out callback pointers
			for (int i = 0; i < MAX_EVENT_SUBSCRIBERS; i++)
			{
				g_event_callback_list[i].p_event_callback = 0;
			}

			g_current_event_id = get_nvm_event_id();
			int poll_interval = 0;
			if (NVM_SUCCESS == get_bounded_config_value_int(
					SQL_KEY_EVENT_POLLING_INTERVAL_MINUTES, &poll_interval))
			{
				g_poll_interval_sec = (NVM_UINT32)poll_interval * SECONDSPERMINUTE;
			}

			g_is_polling = 1;

			mutex_unlock(&g_eventmonitor_lock);

			// start polling
			NVM_UINT64 thread_id;
			create_thread(&thread_id, poll_events, NULL);
		}
		else
		{
			mutex_unlock(&g_eventmonitor_lock);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Stop polling on event table
 */
int remove_event_notify()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (mutex_lock(&g_eventmonitor_lock))
	{
		// if we're listening for database changes, stop
		if (g_is_polling)
		{
			// trigger we are done polling
			g_is_polling = 0;
		}
		mutex_unlock(&g_eventmonitor_lock);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Register a callback for events
 */
int nvm_add_event_notify(const enum event_type type,
		void (*p_event_callback) (struct event *p_event))
{
	COMMON_LOG_ENTRY_PARAMS("type: %d", type);
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_event_callback == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_event_callback is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		// be sure we have the polling setup and running to get callback when a new event is created
		init_event_notify();
		int notify_added = 0;
		int i = 0;

		if (mutex_lock(&g_eventmonitor_lock))
		{
			// if there's an open callback on the list, fill it
			for (i = 0; (i < MAX_EVENT_SUBSCRIBERS) && (!notify_added); i++)
			{
				if (NULL == g_event_callback_list[i].p_event_callback)
				{
					g_event_callback_list[i].p_event_callback = p_event_callback;
					g_event_callback_list[i].type = type;
					notify_added = 1;
				}
			}
			mutex_unlock(&g_eventmonitor_lock);
		}

		// if no place to add one, return error
		if (!notify_added)
		{
			rc = NVM_ERR_EXCEEDSMAXSUBSCRIBERS;
			COMMON_LOG_ERROR("Max subscribers exceeded.");
		}

		// if successful, return the callback ID - just using the index so should be unique
		if (NVM_SUCCESS == rc)
		{
			rc = i - 1;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Remove the specified callback for device events.  Callback_id used is just the index
 * of the event_callback_list.
 */
int nvm_remove_event_notify(const int callback_id)
{
	COMMON_LOG_ENTRY_PARAMS("callback_id: %d", callback_id);
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	if (callback_id < 0)
	{
		COMMON_LOG_ERROR("Invalid callback_id < 0");
		rc = NVM_ERR_BADCALLBACK;
	}
	else if (callback_id >= MAX_EVENT_SUBSCRIBERS)
	{
		COMMON_LOG_ERROR("Invalid callback_id > MAX_EVENT_SUBSCRIBERS");
		rc = NVM_ERR_BADCALLBACK;
	}
	else
	{
		int still_listening = 0;
		int i = 0;
		if (mutex_lock(&g_eventmonitor_lock))
		{
			g_event_callback_list[callback_id].p_event_callback = NULL;

			// check to see if there are any others still listening for callbacks
			for (i = 0; i < MAX_EVENT_SUBSCRIBERS; i++)
			{
				if (NULL != g_event_callback_list[i].p_event_callback)
				{
					still_listening = 1;
					break;
				}
			}
			mutex_unlock(&g_eventmonitor_lock);
		}
		// if not, remove the device notify
		if (0 == still_listening)
		{
			remove_event_notify();
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 *  Acknowledges the specified event
 */
int nvm_acknowledge_event(NVM_UINT32 event_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		PersistentStore *p_store = get_lib_store();
		if (!p_store)
		{
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			struct db_event event;
			rc = db_get_event_by_id(p_store, event_id, &event);
			if (rc == DB_SUCCESS)
			{
				if (!event.action_required)
				{
					// error to acknowledge an event that does not have action_required set to true
					rc = NVM_ERR_NOTSUPPORTED;
				}
				else
				{
					event.action_required = 0;
					rc = db_update_event_by_id(p_store, event_id, &event);
				}
			}
			else
			{
				COMMON_LOG_ERROR_F("Invalid event id %d", event_id);
				rc = NVM_ERR_INVALIDPARAMETER;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the most recent event id.
 */
static int get_nvm_event_id()
{
	COMMON_LOG_ENTRY();
	int event_id = 0;
	PersistentStore *p_store = get_lib_store();
	if (p_store)
	{
		run_scalar_sql(p_store, "select max(id) from event", &event_id);
	}
	COMMON_LOG_EXIT_RETURN_I(event_id);
	return event_id;
}

/*
 * hold until it's time to stop polling or the wait time expires
 */
static int timeToQuit(unsigned long timeoutSeconds)
{
	COMMON_LOG_ENTRY();
	int time_to_quit = 0;
	time_t beg = time(NULL);
	time_t cur;
	unsigned long elapsedSeconds = 0;
	do
	{
		if (mutex_lock(&g_eventmonitor_lock))
		{
			time_to_quit = !g_is_polling;
			mutex_unlock(&g_eventmonitor_lock);
		}
		if (time_to_quit)
		{
			break;
		}
		// sleep
		cur = time(NULL);
		elapsedSeconds = (unsigned long)difftime(cur, beg);
		nvm_sleep(1000); // sleep for 1 second between checks
	}
	while (elapsedSeconds < timeoutSeconds);

	COMMON_LOG_EXIT_RETURN_I(time_to_quit);
	return time_to_quit;
}

/*
 * Look at the event table for any new records. For each new record notify any registered callbacks.
 */
static void *poll_events(void *arg)
{
	COMMON_LOG_ENTRY();
	struct event_filter filter; // used to get all events
	memset(&filter, 0, sizeof (filter));

	while (!timeToQuit(g_poll_interval_sec))
	{
		if (mutex_lock(&g_eventmonitor_lock))
		{
			// ensure nothing comes in while we are working
			db_begin_transaction(get_lib_store());
			int next_event_id = get_nvm_event_id();
			if (g_current_event_id < next_event_id)
			{
				int event_count = process_events_matching_filter(&filter, NULL, 0, 0);
				if (event_count < 0)
				{
					COMMON_LOG_ERROR_F("Error polling events while getting event count. Error: %d",
							event_count);
				}
				else
				{
					struct event events[event_count];
					event_count = process_events_matching_filter(&filter, events, event_count, 0);
					if (event_count < 0)
					{
						COMMON_LOG_ERROR_F("Error polling events while getting events. Error: %d",
								event_count);
					}
					else
					{
						// can't necessarily guarantee order of events,
						// so loop through each and only
						// execute callbacks if the event id is new.
						for (int e = 0; e < event_count; e++)
						{
							if (events[e].event_id > g_current_event_id)
							{
								for (int i = 0; (i < MAX_EVENT_SUBSCRIBERS); i++)
								{
									if (NULL != g_event_callback_list[i].p_event_callback)
									{
										// check if subscriber is interested in this event
										if (g_event_callback_list[i].type == events[e].type ||
												g_event_callback_list[i].type == EVENT_TYPE_ALL)
										{
											g_event_callback_list[i].p_event_callback(&events[e]);
										}
									}
								}
							}
						}
						// save new current event id so events aren't repeatedly sent to callbacks
						g_current_event_id = next_event_id;
					}
				}
			}
			else if (next_event_id < g_current_event_id) // Should never happen
			{
				COMMON_LOG_WARN("Most current event ID is less than the last event ID. Polling "
						"May have missed events to send to listeners.");
				g_current_event_id = next_event_id;
			}
			db_end_transaction(get_lib_store());

			mutex_unlock(&g_eventmonitor_lock);
		}
	}

	COMMON_LOG_EXIT();
	return NULL; // no return value expected
}
