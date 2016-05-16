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
 * This file defines the configuration settings key names.
 */

#ifndef	_CONFIG_SETTINGS_H_
#define	_CONFIG_SETTINGS_H_

#ifdef __cplusplus
extern "C"
{
#endif

//! Default maximum number of support snapshots to keep
#define MAX_SUPPORT_SNAPSHOTS_DEFAULT "100"

//! Maximum number of support snapshots to keep
#define MAX_SUPPORT_SNAPSHOTS_BOUND 100

//! Minimum allowed interval for event polling
#define EVENT_POLLING_INTERVAL_MINUTES_BOUND 1

//! Minimum allowed interval for performance monitor
#define PERFORMANCE_MONITOR_INTERVAL_MINUTES_BOUND 1

//! Maximum number of performance log entries
#define PERFORMANCE_LOG_MAX_BOUND 10000

//! Minimum performance log trim percent
#define PERFORMANCE_LOG_TRIM_PERCENT_BOUND 10

//! Maximum number of log entries
#define LOG_MAX_BOUND 10000

//! Minimum allowed interval for event monitor
#define EVENT_MONITOR_INTERVAL_MINUTES_BOUND 1

//! Minimum event log trim percent
#define EVENT_LOG_TRIM_PERCENT_BOUND 10

//! Maximum event log entries
#define EVENT_LOG_MAX_BOUND 10000

//! The maximum length of a config setting SQL key
#define CONFIG_SETTINGS_KEY_MAX_LEN 256

//! The maximum trim percentage
#define MAX_TRIM_PERCENT 100

/*
 * SQL 'config' table KEY names
 */

//! SQL Key name for encrypted gather support
#define	SQL_KEY_ENCRYPT_GATHER_SUPPORT		"ENCRYPT_GATHER_SUPPORT"

//! SQL Key name for the gather support filter
#define	SQL_KEY_GATHER_SUPPORT_FILTER		"GATHER_SUPPORT_FILTER"

//! SQL Key name for the maximum number of support snapshots to keep
#define	SQL_KEY_SUPPORT_SNAPSHOT_MAX	"SUPPORT_SNAPSHOT_MAX"

//! SQL Key name used to globally store system wide logging level
#define	SQL_KEY_LOG_LEVEL	"LOG_LEVEL"

//! SQL Key name for the default simulator file location
#define	SQL_KEY_DEFAULT_SIMULATOR	"DEFAULT_SIMULATOR"

//! SQL Key name for the valid manufacturer value
#define	SQL_KEY_VALID_MANUFACTURER "VALID_MANUFACTURER"

//! SQL Key name for the valid model number value
#define	SQL_KEY_VALID_MODEL_NUM "VALID_MODEL_NUM"

//! SQL Key name for the valid vendor id value
#define	SQL_KEY_VALID_VENDOR_ID "VALID_VENDOR_ID"

//! SQL Key name for the ecc uncorrectable errors threshold
#define	SQL_KEY_UNCORRECTABLE_THRESHOLD "UNCORRECTABLE_ERRORS_THRESHOLD"

//! SQL Key name for the ecc corrected errors threshold
#define	SQL_KEY_CORRECTED_THRESHOLD "CORRECTED_ERRORS_THRESHOLD"

//! SQL Key name for the erasure coded corrected errors threshold
#define	SQL_KEY_ERASURE_CODED_CORRECTED_THRESHOLD "ERASURE_CODED_CORRECTED_ERRORS_THRESHOLD"

//! SQL Key name for the percent used threshold
#define	SQL_KEY_PERCENT_USED_THRESHOLD "PERCENT_USED_THRESHOLD"

//! SQL Key name for the percent used threshold
#define	SQL_KEY_MAX_HEALTH_STATUS "MAX_HEALTH_STATUS"

//! SQL Key name for where to write the logs
#define	SQL_KEY_LOG_DESTINATION "LOG_DESTINATION"

//! SQL Key name for the maximum number of logs to keep
#define	SQL_KEY_LOG_MAX "LOG_MAX"

//! SQL Key name for the default temperature threshold in Celsius
#define	SQL_KEY_DEFAULT_TEMPERATURE_THRESHOLD "DEFAULT_TEMPERATURE_THRESHOLD"

//! SQL Key name for the default spare block threshold
#define	SQL_KEY_DEFAULT_SPARE_BLOCK_THRESHOLD "DEFAULT_SPARE_BLOCK_THRESHOLD"

//! SQL Key name for the default firmware log level
#define	SQL_KEY_FW_LOG_LEVEL "FW_LOG_LEVEL"

//! SQL Key name for the default firmware time drift
#define	SQL_KEY_DEFAULT_TIME_DRIFT "FW_TIME_DRIFT"

//! SQL Key name for the current minimum TDP DIMM power in W
#define	SQL_KEY_DEFAULT_TDP_POW_MIN "FW_TDP_POW_MIN"

//! SQL Key name for the current maximum TDP DIMM power in W
#define	SQL_KEY_DEFAULT_TDP_POW_MAX "FW_TDP_POW_MAX"

//! SQL Key name for the valid minimum power budget in mW for inst. power
#define	SQL_KEY_DEFAULT_PEAK_POW_BUDGET_MIN "FW_PEAK_POW_BUDGET_MIN"

//! SQL Key name for the valid maximum power budget in mW for inst. power
#define	SQL_KEY_DEFAULT_PEAK_POW_BUDGET_MAX "FW_PEAK_POW_BUDGET_MAX"

//! SQL Key name for the valid minimum power budget in mW for avg. power
#define	SQL_KEY_DEFAULT_AVG_POW_BUDGET_MIN "FW_AVG_POW_BUDGET_MIN"

//! SQL Key name for the valid maximum power budget in mW for avg. power
#define	SQL_KEY_DEFAULT_AVG_POW_BUDGET_MAX "FW_AVG_POW_BUDGET_MAX"

//! SQL Key name for the default die sparing policy aggressiveness
#define	SQL_KEY_DEFAULT_DIE_SPARING_AGGRESSIVENESS "FW_DIE_SPARING_AGGRESSIVENESS"

//! SQL Key name for whether the topology state has been initialized
#define	SQL_KEY_TOPOLOGY_STATE_VALID "TOPOLOGY_STATE_VALID"

//! SQL Key name for minimum severity an event must have to log it to syslog
#define	SQL_KEY_EVENT_SYSLOG_MIN_SEVERITY	"EVENT_SYSLOG_MIN_SEVERITY"

//! SQL Key name for the event polling interval
#define	SQL_KEY_EVENT_POLLING_INTERVAL_MINUTES "EVENT_POLLING_INTERVAL_MINUTES"

//! SQL Key name for default CLI device identifier
#define	SQL_KEY_CLI_DIMM_ID	"CLI_DEFAULT_DIMM_ID"

//! SQL Key name for default CLI device identifier
#define	SQL_KEY_CLI_SIZE	"CLI_DEFAULT_SIZE"

// EVENT MONITOR KEYS
//! SQL Key name for event monitor enabled
#define	SQL_KEY_EVENT_MONITOR_ENABLED "EVENT_MONITOR_ENABLED"

//! SQL Key name for event monitor interval
#define	SQL_KEY_EVENT_MONITOR_INTERVAL_MINUTES "EVENT_MONITOR_INTERVAL_MINUTES"

//! SQL Key name for maximum number of events to store.
#define	SQL_KEY_EVENT_LOG_MAX	"EVENT_LOG_MAX"

//! SQL Key name for the % of logs to be trimmed from the event log
#define	SQL_KEY_EVENT_LOG_TRIM_PERCENT "EVENT_LOG_TRIM_PERCENT"


// PERFORMANCE MONITOR KEYS
//! SQL Key name for performance monitor enabled
#define	SQL_KEY_PERFORMANCE_MONITOR_ENABLED "PERFORMANCE_MONITOR_ENABLED"

//! SQL Key name for performance monitor interval
#define	SQL_KEY_PERFORMANCE_MONITOR_INTERVAL_MINUTES "PERFORMANCE_MONITOR_INTERVAL_MINUTES"

//! SQL Key name for max performance logs stored
#define	SQL_KEY_PERFORMANCE_LOG_MAX "PERFORMANCE_LOG_MAX"

//! SQL Key name for the % of performance logs to be trimmed if max number of rows is exceeded
#define	SQL_KEY_PERFORMANCE_LOG_TRIM_PERCENT "PERFORMANCE_LOG_TRIM_PERCENT"

#ifdef __cplusplus
}
#endif

#endif /* _CONFIG_SETTINGS_H_ */
