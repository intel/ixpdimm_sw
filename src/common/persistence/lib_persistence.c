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
 * This file contains the implementation of common utility methods for
 * interfacing with the configuration database.
 */

#include <stdio.h>
#include <stdlib.h>

#include <common_types.h>
#include <string/s_str.h>
#include <os/os_adapter.h>
#include <file_ops/file_ops_adapter.h>

#include "lib_persistence.h"
#include "logging.h"
#include "config_settings.h"

/*!
 * Determines if we enable support data by default
 */
#ifdef BUILD_ESX
const char *max_snapshots = "0";
const char *log_destination = "1"; // log to syslog by default
#else
const char *max_snapshots = MAX_SUPPORT_SNAPSHOTS_DEFAULT;
const char *log_destination = "0"; // log to DB by default
#endif

// GLOBAL database pointer for this process
PersistentStore *p_store;

// helper functions
void add_config_value_to_pstore(const PersistentStore *p_ps, const char *key, const char *value);
void apply_bound_to_config_value(const char *key, int *value);
void apply_bound(int *value, int lower, int upper);

/*
 * get the path to the config database file
 */
int get_lib_store_path(COMMON_PATH path)
{
#ifdef __ESX__
	/*
	 * ESX cannot have the config DB part of the install process, so it must created. Ideally
	 * it will be created in the install directory so that any of the ESX tools (esxcli,
	 * vm-support, etc) will be able to access it.
	 */
	get_install_dir(path);
	s_strcat(path, COMMON_PATH_LEN, CONFIG_FILE);

	if (!file_exists(path, COMMON_PATH_LEN))
	{
		// try to create it in the install directory
		if (create_default_config(path) != COMMON_SUCCESS)
		{
			// if can't create db in install path then ESX is going to have a hard time, but
			// maybe this is the build system so try in the local dir anyway
			s_strcpy(path, "./", COMMON_PATH_LEN);
			s_strcat(path, COMMON_PATH_LEN, CONFIG_FILE);
			if (!file_exists(path, COMMON_PATH_LEN))
			{
				// don't need to check the result of this. There's a check later if the DB exists.
				create_default_config(path);
			}
		}
	}
#else
	// try the local directory first
	// end users won't have the config file in their local dir,
	// but developers will
	s_strcpy(path, "./", COMMON_PATH_LEN);
	s_strcat(path, COMMON_PATH_LEN, CONFIG_FILE);
	if (!file_exists(path, COMMON_PATH_LEN))
	{
		// try to find the config file in the install directory
		get_install_dir(path);
		s_strcat(path, COMMON_PATH_LEN, CONFIG_FILE);
	}
#endif
	return file_exists(path, COMMON_PATH_LEN) ? COMMON_SUCCESS : COMMON_ERR_UNKNOWN;
}

int create_default_config(const char *path)
{
	int rc = COMMON_ERR_UNKNOWN;
	if (!path)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		// create the database (delete if already exists)
		PersistentStore *p_ps = create_PersistentStore(path, 1);
		if (p_ps != NULL)
		{
			rc = set_default_config_settings(p_ps);
			free_PersistentStore(&p_ps);
		}
	}
	return rc;
}


/*
 * Set up the connection to the product configuration and support database.
 */
int open_lib_store(const char *path)
{
	int rc = COMMON_SUCCESS;
	if (p_store == NULL)
	{
		if (!path)
		{
			rc = COMMON_ERR_INVALIDPARAMETER;
		}
		else
		{
			p_store = open_PersistentStore(path);
			if (p_store == NULL)
			{
				rc = COMMON_ERR_FAILED;
			}
			else
			{
				rc = log_init();
			}
		}
	}
	return rc;
}


/*
 * Close the configuration database and flush the log to the database.
 */
int close_lib_store()
{
	int rc = COMMON_SUCCESS;
	log_close();
	if (free_PersistentStore(&p_store) != DB_SUCCESS)
	{
		rc = COMMON_ERR_UNKNOWN;
	}

	return rc;
}

/*
 * Return a pointer to the configuration database
 */
PersistentStore *get_lib_store()
{
	return p_store;
}

/*
 * Return a pointer to the configuration database
 */
PersistentStore *open_default_lib_store()
{
	// if not already open, open it
	if (p_store == NULL)
	{
		COMMON_PATH path;
		if (get_lib_store_path(path) == COMMON_SUCCESS)
		{
			open_lib_store(path);
		}
	}
	return p_store;
}



/*
 * Retrieve a configuration setting from the database
 */
int get_config_value_int(const char *key, int *value)
{
	int rc = COMMON_ERR_UNKNOWN;
	if (!key || !value)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		char value_str[CONFIG_VALUE_LEN];
		if ((rc = get_config_value(key, value_str)) == COMMON_SUCCESS)
		{
			*value = strtol(value_str, NULL, 0);
			rc = COMMON_SUCCESS;
		}
	}
	return rc;
}

/*
 * Retrieve a configuration setting from the database
 */
int get_config_value(const char *key, char *value)
{
	int rc = COMMON_ERR_UNKNOWN;
	if (!key || !value)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		if (p_store)
		{
			struct db_config config;
			if ((rc = db_get_config_by_key(p_store, key, &config)) == DB_SUCCESS)
			{
				s_strcpy(value, config.value, CONFIG_VALUE_LEN);
				rc = COMMON_SUCCESS;
			}
		}
	}
	return rc;
}

/*
 * Return a bounded config value as an int
 */
int get_bounded_config_value(const char *key, char *value)
{
	int intValue;
	int rc = get_bounded_config_value_int(key, &intValue);

	s_snprintf(value, CONFIG_VALUE_LEN, "%d", intValue);

	return rc;
}

NVM_BOOL is_valid_value(const char *key, int value)
{
	NVM_BOOL isValid = 1;

	int boundedValue = value;

	apply_bound_to_config_value(key, &boundedValue);

	if (value != boundedValue)
	{
		isValid = 0;
	}
	return isValid;
}

int get_bounded_config_value_int(const char *key, int *value)
{
	int rc = get_config_value_int(key, value);

	if (rc == COMMON_SUCCESS)
	{
		apply_bound_to_config_value(key, value);
	}
	return rc;
}

void apply_bound_to_config_value(const char *key, int *value)
{
	if ((s_strncmp(key, SQL_KEY_EVENT_POLLING_INTERVAL_MINUTES,
			s_strnlen(key, CONFIG_SETTINGS_KEY_MAX_LEN)) == 0) &&
			(*value < EVENT_POLLING_INTERVAL_MINUTES_BOUND))
	{
		apply_bound(value, EVENT_POLLING_INTERVAL_MINUTES_BOUND, INT_MAX);
	}
	else if ((s_strncmp(key, SQL_KEY_SUPPORT_SNAPSHOT_MAX,
			s_strnlen(key, CONFIG_SETTINGS_KEY_MAX_LEN)) == 0))
	{
		apply_bound(value, 0, MAX_SUPPORT_SNAPSHOTS_BOUND);
	}
	else if ((s_strncmp(key, SQL_KEY_LOG_MAX,
			s_strnlen(key, CONFIG_SETTINGS_KEY_MAX_LEN)) == 0))
	{
		apply_bound(value, 0, LOG_MAX_BOUND);
	}
	else if ((s_strncmp(key, SQL_KEY_PERFORMANCE_MONITOR_INTERVAL_MINUTES,
			s_strnlen(key, CONFIG_SETTINGS_KEY_MAX_LEN)) == 0) &&
			(*value < PERFORMANCE_MONITOR_INTERVAL_MINUTES_BOUND))
	{
		apply_bound(value, PERFORMANCE_MONITOR_INTERVAL_MINUTES_BOUND, INT_MAX);
	}
	else if ((s_strncmp(key, SQL_KEY_PERFORMANCE_LOG_MAX,
			s_strnlen(key, CONFIG_SETTINGS_KEY_MAX_LEN)) == 0) &&
			(*value > PERFORMANCE_LOG_MAX_BOUND))
	{
		apply_bound(value, 0, PERFORMANCE_LOG_MAX_BOUND);
	}
	else if ((s_strncmp(key, SQL_KEY_PERFORMANCE_LOG_TRIM_PERCENT,
			s_strnlen(key, CONFIG_SETTINGS_KEY_MAX_LEN)) == 0) &&
			(*value < PERFORMANCE_LOG_TRIM_PERCENT_BOUND))
	{
		apply_bound(value, PERFORMANCE_LOG_TRIM_PERCENT_BOUND, MAX_TRIM_PERCENT);
	}
	else if ((s_strncmp(key, SQL_KEY_EVENT_LOG_TRIM_PERCENT,
			s_strnlen(key, CONFIG_SETTINGS_KEY_MAX_LEN)) == 0) &&
			(*value < EVENT_LOG_TRIM_PERCENT_BOUND))
	{
		apply_bound(value, EVENT_LOG_TRIM_PERCENT_BOUND, MAX_TRIM_PERCENT);
	}
	else if ((s_strncmp(key, SQL_KEY_EVENT_MONITOR_INTERVAL_MINUTES,
			s_strnlen(key, CONFIG_SETTINGS_KEY_MAX_LEN)) == 0) &&
			(*value < EVENT_MONITOR_INTERVAL_MINUTES_BOUND))
	{
		apply_bound(value, EVENT_MONITOR_INTERVAL_MINUTES_BOUND, INT_MAX);
	}
	else if ((s_strncmp(key, SQL_KEY_EVENT_LOG_MAX,
			s_strnlen(key, CONFIG_SETTINGS_KEY_MAX_LEN)) == 0))
	{
		apply_bound(value, 0, EVENT_LOG_MAX_BOUND);
	}
}

void apply_bound(int *value, int lower, int upper)
{
	if (*value < lower)
	{
		*value = lower;
	}

	if (*value > upper)
	{
		*value = upper;
	}
}
/*
 * Add a new config setting
 */
int add_config_value(const char *key, const char *value)
{
	int rc = COMMON_ERR_UNKNOWN;
	if (!key || !value)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		if (p_store)
		{
			// remove it first so it doesn't error on dup key, ignore errors
			rm_config_value(key);

			// add it
			struct db_config config;
			s_strcpy(config.key, key, CONFIG_KEY_LEN);
			s_strcpy(config.value, value, CONFIG_VALUE_LEN);
			if (db_add_config(p_store, &config) == DB_SUCCESS)
			{
				rc = COMMON_SUCCESS;
			}
		}
	}
	return rc;
}

/*
 * Remove a config setting.
 */
int rm_config_value(const char *key)
{
	int rc = COMMON_ERR_UNKNOWN;
	if (!key)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		if (p_store)
		{
			if (db_delete_config_by_key(p_store, key) == DB_SUCCESS)
			{
				rc = COMMON_SUCCESS;
			}
		}
	}
	return rc;
}

/*
 * Add a new config setting to the given persistent store
 */
void add_config_value_to_pstore(const PersistentStore *p_ps, const char *key, const char *value)
{
	if (p_ps)
	{
		struct db_config config;
		s_strcpy(config.key, key, CONFIG_KEY_LEN);
		s_strcpy(config.value, value, CONFIG_VALUE_LEN);
		db_add_config(p_ps, &config);
	}
}

/*
 * Set the configuration settings to their default values
 */
int set_default_config_settings(PersistentStore *p_ps)
{
	int rc = COMMON_ERR_UNKNOWN;
	if (p_ps != NULL)
	{
		// Add default configuration settings
		add_config_value_to_pstore(p_ps, SQL_KEY_LOG_LEVEL, "0");
		add_config_value_to_pstore(p_ps, SQL_KEY_EVENT_POLLING_INTERVAL_MINUTES, "1");
		add_config_value_to_pstore(p_ps, SQL_KEY_ENCRYPT_GATHER_SUPPORT, "1");
		add_config_value_to_pstore(p_ps, SQL_KEY_GATHER_SUPPORT_FILTER,
			"15"); // GSF_HOST_DATA | GSF_NAMESPACE_DATA | GSF_SERIAL_NUMS | GSF_SYSTEM_LOG
		add_config_value_to_pstore(p_ps, SQL_KEY_SUPPORT_SNAPSHOT_MAX,
				max_snapshots);
		// TODO: These values need to be confirmed with real HW
		add_config_value_to_pstore(p_ps, SQL_KEY_VALID_MANUFACTURER, "0x0089");
		add_config_value_to_pstore(p_ps, SQL_KEY_VALID_MODEL_NUM, "MN: 0123456789");
		add_config_value_to_pstore(p_ps, SQL_KEY_VALID_VENDOR_ID, "0x8086");
		add_config_value_to_pstore(p_ps, SQL_KEY_UNCORRECTABLE_THRESHOLD, "10");
		add_config_value_to_pstore(p_ps, SQL_KEY_CORRECTED_THRESHOLD, "10");
		add_config_value_to_pstore(p_ps, SQL_KEY_ERASURE_CODED_CORRECTED_THRESHOLD, "10");
		add_config_value_to_pstore(p_ps,
			SQL_KEY_PERCENT_USED_THRESHOLD, "90"); // TODO: value TBD
		add_config_value_to_pstore(p_ps, SQL_KEY_MAX_HEALTH_STATUS, "0"); // 0 means normal
		add_config_value_to_pstore(p_ps, SQL_KEY_LOG_DESTINATION, log_destination);
		add_config_value_to_pstore(p_ps, SQL_KEY_LOG_MAX, "10000");
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_TEMPERATURE_THRESHOLD, "81.5");
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_SPARE_BLOCK_THRESHOLD, "50");
		add_config_value_to_pstore(p_ps, SQL_KEY_FW_LOG_LEVEL, "1");
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_TIME_DRIFT, "120"); // 2 minutes
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_TDP_POW_MIN, "10");
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_TDP_POW_MAX, "18");
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_PEAK_POW_BUDGET_MIN, "100");
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_PEAK_POW_BUDGET_MAX, "20000");
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_AVG_POW_BUDGET_MIN, "100");
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_AVG_POW_BUDGET_MAX, "18000");
		add_config_value_to_pstore(p_ps, SQL_KEY_DEFAULT_DIE_SPARING_AGGRESSIVENESS, "128");

		// monitor configs
		add_config_value_to_pstore(p_ps, SQL_KEY_PERFORMANCE_MONITOR_ENABLED, "1");
		// 180 = every 3 hours
		add_config_value_to_pstore(p_ps, SQL_KEY_PERFORMANCE_MONITOR_INTERVAL_MINUTES, "180");
		add_config_value_to_pstore(p_ps, SQL_KEY_PERFORMANCE_LOG_MAX, "10000");
		add_config_value_to_pstore(p_ps, SQL_KEY_PERFORMANCE_LOG_TRIM_PERCENT, "30");

		add_config_value_to_pstore(p_ps, SQL_KEY_EVENT_MONITOR_ENABLED, "1");
		add_config_value_to_pstore(p_ps, SQL_KEY_EVENT_MONITOR_INTERVAL_MINUTES, "1");
		add_config_value_to_pstore(p_ps, SQL_KEY_EVENT_LOG_MAX, "10000");
		add_config_value_to_pstore(p_ps, SQL_KEY_EVENT_LOG_TRIM_PERCENT, "10");
		add_config_value_to_pstore(p_ps, SQL_KEY_TOPOLOGY_STATE_VALID, "0");

		// CLI default device identifier output - HANDLE (or uid)
		add_config_value_to_pstore(p_ps, SQL_KEY_CLI_DIMM_ID, "HANDLE");
		add_config_value_to_pstore(p_ps, SQL_KEY_CLI_SIZE, "AUTO");
		rc = COMMON_SUCCESS;
	}
	return rc;
}
