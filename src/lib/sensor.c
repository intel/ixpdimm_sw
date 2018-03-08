/*
 * Copyright (c) 2016, Intel Corporation
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

#include <persistence/logging.h>
#include <cr_i18n.h>
#include <string/s_str.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include "nvm_types.h"
#include "nvm_management.h"
#include "system.h"
#include "device_fw.h"
#include "capabilities.h"
#include "device_utilities.h"
#include "monitor.h"
#include "nvm_context.h"

/*
 * Implementation of the Native API sensor functions.
 */

// upper limits on valid threshold values - defined in FIS
#define	LIMIT_SPARE_THRESHOLD_VALUE    100u

struct sensor_thresholds
{
	NVM_REAL32 controller_temp_shutdown;
	NVM_REAL32 media_temp_shutdown;
	NVM_REAL32 throttling_start;
	NVM_REAL32 throttling_stop;
	NVM_REAL32 alarm_media_temp_threshold;
	NVM_REAL32 alarm_controller_temp_threshold;
	NVM_UINT8 alarm_spare_block_threshold;
	NVM_BOOL is_media_alarm_threshold_enabled;
	NVM_BOOL is_controller_alarm_threshold_enabled;
	NVM_BOOL is_spare_alarm_threshold_enabled;
};

int get_sensor_thresholds(int device_handle, struct sensor_thresholds *p_thresholds)
{
	memset(p_thresholds, 0, sizeof (*p_thresholds));
	struct pt_payload_device_characteristics characteristics;
	int rc = fw_get_id_dimm_device_characteristics(device_handle, &characteristics);

	if (rc == NVM_SUCCESS)
	{
		p_thresholds->media_temp_shutdown =
			fw_convert_fw_celsius_to_float(characteristics.media_temp_shutdown_threshold);
		p_thresholds->controller_temp_shutdown =
			fw_convert_fw_celsius_to_float(characteristics.controller_temp_shutdown_threshold);
		p_thresholds->throttling_start =
			fw_convert_fw_celsius_to_float(characteristics.throttling_start_threshold);
		p_thresholds->throttling_stop =
			fw_convert_fw_celsius_to_float(characteristics.throttling_stop_threshold);
	}

	struct pt_payload_alarm_thresholds alarm_thresholds;
	memset(&alarm_thresholds, 0, sizeof (alarm_thresholds));
	int threshold_rc = fw_get_alarm_thresholds(device_handle, &alarm_thresholds);
	KEEP_ERROR(rc, threshold_rc);
	if (threshold_rc == NVM_SUCCESS)
	{
		p_thresholds->alarm_media_temp_threshold =
			fw_convert_fw_celsius_to_float(alarm_thresholds.media_temperature);
		p_thresholds->alarm_controller_temp_threshold =
			fw_convert_fw_celsius_to_float(alarm_thresholds.controller_temperature);
		p_thresholds->alarm_spare_block_threshold = alarm_thresholds.spare;
		p_thresholds->is_media_alarm_threshold_enabled = (NVM_BOOL)
			(alarm_thresholds.enable & THRESHOLD_ENABLED_MEDIA_TEMP ? 1 : 0);
		p_thresholds->is_controller_alarm_threshold_enabled = (NVM_BOOL)
			(alarm_thresholds.enable & THRESHOLD_ENABLED_CONTROLLER_TEMP ? 1 : 0);
		p_thresholds->is_spare_alarm_threshold_enabled = (NVM_BOOL)
			(alarm_thresholds.enable & THRESHOLD_ENABLED_SPARE ? 1 : 0);
	}

	return rc;
}

int get_smart_log_sensors(const NVM_UINT32 dev_handle,
      struct sensor *p_sensors, NVM_BOOL include_thresholds)
{
	COMMON_LOG_ENTRY();

	struct pt_payload_smart_health dimm_smart;
	memset(&dimm_smart, 0, sizeof (dimm_smart));
	int rc = fw_get_smart_health(dev_handle, &dimm_smart);

	if (rc == NVM_SUCCESS)
	{
		/*
		 * If unable to get threshold, there is still a lot of sensor info that can be set from
		 * the smart health log ... if it's successful. So if thresholds fail, just continue.
		 */
		struct sensor_thresholds thresholds;
		if (include_thresholds)
		{
			int threshold_rc = get_sensor_thresholds(dev_handle, &thresholds);
			KEEP_ERROR(rc, threshold_rc);
		}

		struct sensor *p_sensor = NULL;
		// Media Temperature
		{
			p_sensor = &p_sensors[SENSOR_MEDIA_TEMPERATURE];
			NVM_REAL32 media_temp_celsius = 0;
			if (dimm_smart.validation_flags.parts.media_temperature_field)
			{
				media_temp_celsius = fw_convert_fw_celsius_to_float(dimm_smart.media_temperature);
				p_sensor->reading = nvm_encode_temperature(media_temp_celsius);
			}
			else
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
			}
			p_sensor->upper_noncritical_settable = 1;
			p_sensor->upper_noncritical_support = 1;
			p_sensor->upper_critical_support = 1;
			p_sensor->upper_fatal_support = 1;
			p_sensor->settings.enabled = thresholds.is_media_alarm_threshold_enabled;
			p_sensor->settings.upper_noncritical_threshold =
				nvm_encode_temperature(thresholds.alarm_media_temp_threshold);
			p_sensor->settings.upper_critical_threshold =
				nvm_encode_temperature(thresholds.throttling_start);
			p_sensor->settings.lower_critical_threshold =
				nvm_encode_temperature(thresholds.throttling_stop);
			p_sensor->settings.upper_fatal_threshold =
				nvm_encode_temperature(thresholds.media_temp_shutdown);

			if (media_temp_celsius > thresholds.media_temp_shutdown)
			{
				p_sensor->current_state = SENSOR_FATAL;
			}
			else if (media_temp_celsius > thresholds.throttling_start)
			{
				p_sensor->current_state = SENSOR_CRITICAL;
			}
			else if (dimm_smart.validation_flags.parts.alarm_trips_field &&
				((dimm_smart.alarm_trips & MEDIA_TEMPERATURE_TRIP_BIT) != 0))
			{
				p_sensor->current_state = SENSOR_NONCRITICAL;
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
				p_sensor->reading = (NVM_UINT64) dimm_smart.spare;
			}
			else
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
			}
			p_sensor->lower_noncritical_settable = 1;
			p_sensor->lower_noncritical_support = 1;
			p_sensor->settings.enabled = thresholds.is_spare_alarm_threshold_enabled;
			p_sensor->settings.lower_noncritical_threshold = thresholds.alarm_spare_block_threshold;
			if (dimm_smart.validation_flags.parts.alarm_trips_field &&
				((dimm_smart.alarm_trips & SPARE_BLOCKS_TRIP_BIT) != 0))
			{
				p_sensor->current_state = SENSOR_NONCRITICAL;
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
				p_sensor->reading = (NVM_UINT64) dimm_smart.percentage_used;
			}
			else
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
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
			else
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
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
			else
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
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
			else
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
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
			else
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
			}
		}

		// Controller Temperature
		{
			NVM_REAL32 controller_temp_celsius = 0;
			p_sensor = &p_sensors[SENSOR_CONTROLLER_TEMPERATURE];
			if (dimm_smart.validation_flags.parts.sizeof_vendor_data_field)
			{
				controller_temp_celsius =
					fw_convert_fw_celsius_to_float(dimm_smart.controller_temperature);
				p_sensor->reading = nvm_encode_temperature(controller_temp_celsius);
			}
			else
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
			}
			p_sensor->upper_noncritical_settable = 1;
			p_sensor->upper_noncritical_support = 1;
			p_sensor->upper_critical_support = 1;
			p_sensor->upper_fatal_support = 1;
			p_sensor->settings.enabled = thresholds.is_controller_alarm_threshold_enabled;
			p_sensor->settings.upper_noncritical_threshold =
				nvm_encode_temperature(thresholds.alarm_controller_temp_threshold);
			p_sensor->settings.upper_fatal_threshold =
				nvm_encode_temperature(thresholds.controller_temp_shutdown);

			if (controller_temp_celsius > thresholds.controller_temp_shutdown)
			{
				p_sensor->current_state = SENSOR_FATAL;
			}
			else if (dimm_smart.validation_flags.parts.alarm_trips_field &&
				((dimm_smart.alarm_trips & CONTROLLER_TEMP_TRIP_BIT) != 0))
			{
				p_sensor->current_state = SENSOR_NONCRITICAL;
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

		// Health
		{
			p_sensor = &p_sensors[SENSOR_HEALTH];
			if (dimm_smart.validation_flags.parts.health_status_field)
			{
				p_sensor->reading = dimm_smart.health_status;

				switch (dimm_smart.health_status)
				{
				case SMART_NORMAL:
					p_sensor->current_state = SENSOR_NORMAL;
					break;
				case SMART_CRITICAL:
					p_sensor->current_state = SENSOR_CRITICAL;
					break;
				case SMART_NON_CRITICAL:
					p_sensor->current_state = SENSOR_NONCRITICAL;
					break;
				case SMART_FATAL:
					p_sensor->current_state = SENSOR_FATAL;
					break;
				default:
					p_sensor->current_state = SENSOR_UNKNOWN;
					break;
				}
			}
			else
			{
				p_sensor->current_state = SENSOR_UNKNOWN;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_fw_error_log_sensors(const NVM_UINT32 dev_handle,
      struct sensor sensors[NVM_MAX_DEVICE_SENSORS])
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

/*
 * Helper function to populate sensor information with default attributes
 * that don't require current values.
 */
void initialize_sensors(struct sensor *p_sensors, const NVM_UINT16 count)
{
	// Clear out all data first
	memset(p_sensors, 0, sizeof(struct sensor)*count);

	for (int i = 0; i < count; i++)
	{
		p_sensors[i].type = i;
		p_sensors[i].current_state = SENSOR_NOT_INITIALIZED;
	}
	p_sensors[SENSOR_MEDIA_TEMPERATURE].units = UNIT_CELSIUS;
	p_sensors[SENSOR_SPARECAPACITY].units = UNIT_PERCENT;
	p_sensors[SENSOR_WEARLEVEL].units = UNIT_PERCENT;
	p_sensors[SENSOR_POWERCYCLES].units = UNIT_CYCLES;
	p_sensors[SENSOR_POWERONTIME].units = UNIT_SECONDS;
	p_sensors[SENSOR_UPTIME].units = UNIT_SECONDS;
	p_sensors[SENSOR_UNSAFESHUTDOWNS].units = UNIT_COUNT;
	p_sensors[SENSOR_FWERRORLOGCOUNT].units = UNIT_COUNT;
	p_sensors[SENSOR_CONTROLLER_TEMPERATURE].units = UNIT_CELSIUS;
	p_sensors[SENSOR_HEALTH].units = UNIT_COUNT;
}


int get_sensors_by_category(struct device_discovery *p_discovery,
		struct sensor *p_sensors, const NVM_UINT16 count,
	NVM_SENSOR_CATEGORY_BITMASK categories, NVM_SENSOR_CATEGORY_BITMASK thresholds)
{
	int rc = NVM_SUCCESS;
	initialize_sensors(p_sensors, count);
	if (categories & SENSOR_CAT_SMART_HEALTH)
	{
		NVM_BOOL get_thresh = thresholds & SENSOR_CAT_SMART_HEALTH;
		KEEP_ERROR(rc, get_smart_log_sensors(p_discovery->device_handle.handle,
											p_sensors, get_thresh));
	}

	if (categories & SENSOR_CAT_FW_ERROR)
	{
		KEEP_ERROR(rc, get_fw_error_log_sensors(p_discovery->device_handle.handle,
												p_sensors));
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * This function populates sensor information for the specified device.
 * Sensors are used to monitor a particular aspect of a device by settings
 * thresholds against a current value. Sensor information is returned
 * as part of the device_details structure.
 * The number of sensors for a given device is defined as NVM_MAX_DEVICE_SENSORS.
 *
 * Populate a provided sensor array with a bitmask specified category of sensor
 * data from a particular dimm, specified by dev_handle.
 *
 * Returns NVM_ERR_ARRAYTOOSMALL if the input array length (indicated by count)
 * is smaller than NVM_MAX_DEVICE_SENSORS, even if the number of requested
 * sensors is smaller than NVM_MAX_DEVICE_SENSORS.
 */
int nvm_get_sensors_by_category(const NVM_UINT32 dev_handle,
		struct sensor *p_sensors, const NVM_UINT16 count,
	NVM_SENSOR_CATEGORY_BITMASK categories, NVM_SENSOR_CATEGORY_BITMASK thresholds)
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
	else if (p_sensors == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_sensors is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (count < NVM_MAX_DEVICE_SENSORS)
	{
		rc = NVM_ERR_ARRAYTOOSMALL;
	}
	else if ((rc = lookup_device_nfit_by_handle(dev_handle, &discovery)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Invalid parameter, device handle is invalid");
		// Return already set rc
	}
	// Note: The firmware api version is not populated at this point because
	// of the lookup_device_nfit call above, so it won't be checked by the
	// IS_DEVICE_MANAGEABLE macro. However, there aren't any changes planned
	// for the sensor api so we save on a few SMM calls.
	else if (!IS_DEVICE_MANAGEABLE(&discovery))
	{
		rc = NVM_ERR_NOTMANAGEABLE;
	}
	else
	{
		rc = get_sensors_by_category(&discovery, p_sensors, count,
				categories, thresholds);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * This function populates sensor information for the specified device.
 * Sensors are used to monitor a particular aspect of a device by settings
 * thresholds against a current value.  Sensor information is returned
 * as part of the device_details structure.
 * The number of sensors for a given device is defined as NVM_MAX_DEVICE_SENSORS.
 *
 * Returns NVM_ERR_ARRAYTOOSMALL if the input array length (indicated by count)
 * is smaller than NVM_MAX_DEVICE_SENSORS.
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
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_sensors == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_sensors is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (count < NVM_MAX_DEVICE_SENSORS)
	{
		rc = NVM_ERR_ARRAYTOOSMALL;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		// Get all categories and thresholds
		NVM_SENSOR_CATEGORY_BITMASK categories = SENSOR_CAT_ALL;
		NVM_SENSOR_CATEGORY_BITMASK thresholds = SENSOR_CAT_ALL;
		rc = get_sensors_by_category(&discovery, p_sensors, count,
				categories, thresholds);
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
	else if ((rc = nvm_get_sensors(device_uid, sensors, NVM_MAX_DEVICE_SENSORS)) == NVM_SUCCESS)
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
		// SENSOR_CONTROLLER_TEMPERATURE
		N_TR("Controller Temperature"),
		// SENSOR_HEALTH
		N_TR("Health"),
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
		COMMON_LOG_ERROR("Modifying "
			NVM_DIMM_NAME
			" sensors is not supported.");
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
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		if ((rc = fw_get_alarm_thresholds(discovery.device_handle.handle, &thresholds))
			== NVM_SUCCESS)
		{
			if ((int)type == SENSOR_MEDIA_TEMPERATURE)
			{
				thresholds.media_temperature =
					fw_convert_float_to_fw_celsius(
						nvm_decode_temperature(p_settings->upper_noncritical_threshold));

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
						nvm_decode_temperature(p_settings->upper_noncritical_threshold));

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
				NVM_UINT64 threshold = p_settings->lower_noncritical_threshold;
				if ((threshold > 0) && (threshold < LIMIT_SPARE_THRESHOLD_VALUE))
				{
					thresholds.spare = (unsigned char) threshold;
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
