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
 * The file describes the entry points of the Native Management API.
 * It is intended to be used by clients of the Native Management API
 * in order to perform management actions.
 *
 * To compile applications using the Native Management API, include this header file
 * and link with the -lixpdimm-api option.
 */

#ifndef	_NVM_MANAGEMENT_H_
#define	_NVM_MANAGEMENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "nvm_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define	NVM_VERSION_MAJOR   __VERSION_MAJOR__ // Major version number
#define	NVM_VERSION_MINOR   __VERSION_MINOR__ // Minor version number
#define	NVM_VERSION_HOTFIX   __VERSION_HOTFIX__ // Hot fix version number
#define	NVM_VERSION_BUILDNUM	__VERSION_BUILDNUM__ // Build version number

/*
 * Convert an array of 8 unsigned chars into an unsigned 64 bit value
 * @remarks While it doesn't seem right to be casting 8 bit chars to unsigned long
 * long, this is an issue with gcc - see http:// gcc.gnu.org/bugzilla/show_bug.cgi?id=47821.
 */
#define	NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(arr, val) \
	val = ((unsigned long long)(arr[7] & 0xFF) << 56) + \
	((unsigned long long)(arr[6] & 0xFF) << 48) + \
	((unsigned long long)(arr[5] & 0xFF) << 40) + \
	((unsigned long long)(arr[4] & 0xFF) << 32) + \
	((unsigned long long)(arr[3] & 0xFF) << 24) + \
	((unsigned long long)(arr[2] & 0xFF) << 16) + \
	((unsigned long long)(arr[1] & 0xFF) << 8) + \
	(unsigned long long)(arr[0] & 0xFF);

/*
 * Convert an unsigned 64 bit integer to an array of 8 unsigned chars
 */
#define	NVM_64_BIT_VALUE_TO_8_BYTE_ARRAY(val, arr) \
	arr[7] = (unsigned char)((val >> 56) & 0xFF); \
	arr[6] = (unsigned char)((val >> 48) & 0xFF); \
	arr[5] = (unsigned char)((val >> 40) & 0xFF); \
	arr[4] = (unsigned char)((val >> 32) & 0xFF); \
	arr[3] = (unsigned char)((val >> 24) & 0xFF); \
	arr[2] = (unsigned char)((val >> 16) & 0xFF); \
	arr[1] = (unsigned char)((val >> 8) & 0xFF); \
	arr[0] = (unsigned char)(val & 0xFF);

/*
 * Encode the temperature as a NVM_UINT32
 */
static inline NVM_UINT32 nvm_encode_temperature(NVM_REAL32 value)
{
	NVM_UINT32 result;
	result = (NVM_UINT32)(value * 10000);
	return result;
}

/*
 * Decode a NVM_UINT32 as a temperature
 */
static inline NVM_REAL32 nvm_decode_temperature(NVM_UINT32 value)
{
	NVM_REAL32 result;
	result = (NVM_REAL32)(value / 10000.0);
	return result;
}

/*
 * ****************************************************************************
 * ENUMS
 * ****************************************************************************
 */

/*
 * The operating system type.
 */
enum os_type
{
	OS_TYPE_UNKNOWN = 0, // The OS type can not be determined
	OS_TYPE_WINDOWS = 1, // Windows
	OS_TYPE_LINUX = 2, // Linux
	OS_TYPE_ESX = 3 // ESX
};

/*
 * Compatibility of the device, FW and configuration with the management software.
 */
enum manageability_state
{
	MANAGEMENT_UNKNOWN = 0,  // Device is not recognized or manageability cannot be determined.
	MANAGEMENT_VALIDCONFIG = 1, // Device is fully manageable.
	MANAGEMENT_INVALIDCONFIG = 2 // Device is recognized but cannot be managed.
};

/*
 * Security and Sanitize state of the AEP DIMM.
 */
enum lock_state
{
	LOCK_STATE_UNKNOWN = 0, // Device lock state can not be determined.
	LOCK_STATE_DISABLED = 1, // Security is not enabled on the device.
	LOCK_STATE_UNLOCKED = 2, // Security is enabled and unlocked and un-frozen.
	LOCK_STATE_LOCKED = 3, // Security is enabled and locked and un-frozen.
	LOCK_STATE_FROZEN = 4, // Security is enabled, unlocked and frozen.
	LOCK_STATE_PASSPHRASE_LIMIT = 5, // The passphrase limit has been reached, reset required.
	LOCK_STATE_NOT_SUPPORTED = 6 // Security is not supported
};

/*
 * The device type.
 */
enum memory_type
{
	MEMORY_TYPE_UNKNOWN	= 0, // The type of DIMM cannot be determined.
	MEMORY_TYPE_DDR4 = 1, // DDR4.
	MEMORY_TYPE_NVMDIMM = 2 // NGNVM.
};

/*
 * The device format factor.
 */
enum device_form_factor
{
	DEVICE_FORM_FACTOR_UNKNOWN = 0,	// The form factor cannot be determined.
	DEVICE_FORM_FACTOR_DIMM	= 8, // DIMM.
	DEVICE_FORM_FACTOR_SODIMM = 12, // SODIMM.
};

/*
 * Overall health summary of the device
 */
enum device_health
{
	DEVICE_HEALTH_UNKNOWN = 0, // The HEALTH can not be determined.
	DEVICE_HEALTH_NORMAL = 5, // Normal.
	DEVICE_HEALTH_NONCRITICAL = 15, // Maintenance required.
	DEVICE_HEALTH_CRITICAL = 25, // Features or performance degraded due to failure.
	DEVICE_HEALTH_FATAL = 30, // Data loss has occurred or is imminent.
};

/*
 * The address range scrub (ARS) operation status for the AEP DIMM
 */
enum device_ars_status
{
	DEVICE_ARS_STATUS_UNKNOWN,
	DEVICE_ARS_STATUS_NOTSTARTED,
	DEVICE_ARS_STATUS_INPROGRESS,
	DEVICE_ARS_STATUS_COMPLETE
};

/*
 * The type of sensor.
 * @internal
 * These enums are also used as indexes in the device.sensors array.  It is important to
 * keep them in order and with valid values (0 - 12)
 * @endinternal
 */
enum sensor_type
{
	SENSOR_MEDIA_TEMPERATURE = 0, // Device media temperature in degrees Celsius.
	SENSOR_SPARECAPACITY = 1, // Amount of spare capacity remaining as a percentage.
	SENSOR_WEARLEVEL = 2, // An estimate of the device life used as a percentage.
	SENSOR_POWERCYCLES = 3, // Number of power cycles over the lifetime of the device.
	SENSOR_POWERONTIME = 4, // Total power-on time over the lifetime of the device.
	SENSOR_UPTIME = 5, // Total power-on time since the last power cycle of the device.
	SENSOR_UNSAFESHUTDOWNS = 6, // Device shutdowns without notification.
	SENSOR_FWERRORLOGCOUNT = 7, // The total number of firmware error log entries.
	SENSOR_POWERLIMITED = 8, // Whether or not the AEP DIMM is power limited.
	SENSOR_MEDIAERRORS_UNCORRECTABLE = 9, // Number of ECC uncorrectable errors.
	SENSOR_MEDIAERRORS_CORRECTED = 10, // Number of ECC corrected errors.
	SENSOR_MEDIAERRORS_ERASURECODED = 11, // Number of erasure code corrected errors.
	SENSOR_WRITECOUNT_MAXIMUM = 12, // Max num of data writes to a single block across the device.
	SENSOR_WRITECOUNT_AVERAGE = 13, // Avg num of data writes to all blocks across the device.
	SENSOR_MEDIAERRORS_HOST = 14, // Num of ECC errors encountered by hosts requests only.
	SENSOR_MEDIAERRORS_NONHOST = 15, // Num of ECC errors encountered by non-host requests only.
	SENSOR_CONTROLLER_TEMPERATURE = 16, // Device media temperature in degrees Celsius.
};

/*
 * The units of measurement for a sensor.
 */
enum sensor_units
{
	UNIT_COUNT = 1, // In numbers of something (0,1,2 ... n).
	UNIT_CELSIUS = 2, // In units of Celsius degrees.
	UNIT_SECONDS = 21, // In seconds of time.
	UNIT_MINUTES = 22, // In minutes of time.
	UNIT_HOURS = 23, // In hours of time.
	UNIT_CYCLES = 39, // Cycles
	UNIT_PERCENT = 65 // In units of percentage.
};

/*
 * The current status of a sensor
 */
enum sensor_status
{
	SENSOR_UNKNOWN = 0, // Sensor status cannot be determined.
	SENSOR_NORMAL = 1, // Current value of the sensor is in the normal range.
	SENSOR_NONCRITICAL = 2, // Current value of the sensor is in non critical range.
	SENSOR_CRITICAL = 3, // Current value of the sensor is in the critical error range.
	SENSOR_FATAL = 4 // Current value of the sensor is in the fatal error range.
};

/*
 * 	The type of the event that occurred.  Can be used to filter subscriptions.
 */
enum event_type
{
	EVENT_TYPE_ALL = 0,	// Subscribe or filter on all event types
	EVENT_TYPE_CONFIG = 1, // Device configuration status
	EVENT_TYPE_HEALTH = 2, // Device health event.
	EVENT_TYPE_MGMT	= 3, // Management software generated event.
	EVENT_TYPE_DIAG = 4, // Subscribe or filter on all diagnostic event types
	EVENT_TYPE_DIAG_QUICK = 5, // Quick diagnostic test event.
	EVENT_TYPE_DIAG_PLATFORM_CONFIG = 6, // Platform config diagnostic test event.
	EVENT_TYPE_DIAG_PM_META = 7, // PM metadata diagnostic test event.
	EVENT_TYPE_DIAG_SECURITY = 8, // Security diagnostic test event.
	EVENT_TYPE_DIAG_FW_CONSISTENCY = 9 // FW consistency diagnostic test event.
};

/*
 * Perceived severity of the event
 */
enum event_severity
{
	EVENT_SEVERITY_INFO = 2, // Informational event.
	EVENT_SEVERITY_WARN = 3, // Warning or degraded.
	EVENT_SEVERITY_CRITICAL = 6, // Critical.
	EVENT_SEVERITY_FATAL = 7 // Fatal or nonrecoverable.
};

enum diagnostic_result
{
	DIAGNOSTIC_RESULT_UNKNOWN = 0,
	DIAGNOSTIC_RESULT_OK = 2,
	DIAGNOSTIC_RESULT_WARNING = 3,
	DIAGNOSTIC_RESULT_FAILED = 5,
	DIAGNOSTIC_RESULT_ABORTED = 6
};

/*
 * Logging level used with the library logging functions.
 */
enum log_level
{
	LOG_LEVEL_ERROR = 0, // Error message
	LOG_LEVEL_WARN = 1, // Warning message
	LOG_LEVEL_INFO = 2, // Informational message
	LOG_LEVEL_DEBUG = 3 // Debug message
};

/*
 * Logging level used with the firmware logging functions.
 */
enum fw_log_level
{
	FW_LOG_LEVEL_DISABLED = 0, // Logging Disabled
	FW_LOG_LEVEL_ERROR = 1, // Error message
	FW_LOG_LEVEL_WARN = 2, // Warning message
	FW_LOG_LEVEL_INFO = 3, // Informational message
	FW_LOG_LEVEL_DEBUG = 4, // Debug message
	FW_LOG_LEVEL_UNKNOWN = 5 // Unknown fw log level setting
};

/*
 * Injected error type
 */
enum error_type
{
	ERROR_TYPE_POISON = 1, // Inject a poison error.
	ERROR_TYPE_TEMPERATURE = 2, // Inject a media temperature error.
	ERROR_TYPE_DIE_SPARING = 3, // Trigger or revert an artificial die sparing.
	ERROR_TYPE_SPARE_ALARM = 4, // Trigger or clear a spare capacity threshold alarm.
	ERROR_TYPE_MEDIA_FATAL_ERROR = 5, // Inject or clear a fake media fatal error.
};

enum interleave_size
{
	INTERLEAVE_SIZE_64B  = 0,
	INTERLEAVE_SIZE_128B = 1,
	INTERLEAVE_SIZE_256B = 2,
	INTERLEAVE_SIZE_4KB  = 3,
	INTERLEAVE_SIZE_1GB  = 4
};

enum interleave_ways
{
	INTERLEAVE_WAYS_0 =  0,
	INTERLEAVE_WAYS_1  = 1,
	INTERLEAVE_WAYS_2  = 2,
	INTERLEAVE_WAYS_3  = 3,
	INTERLEAVE_WAYS_4  = 4,
	INTERLEAVE_WAYS_6  = 6,
	INTERLEAVE_WAYS_8  = 8,
	INTERLEAVE_WAYS_12 = 12,
	INTERLEAVE_WAYS_16 = 16,
	INTERLEAVE_WAYS_24 = 24
};

/*
 * Diagnostic test type
 */
enum diagnostic_test
{
	DIAG_TYPE_QUICK = 0, // verifies manageable DIMM host mailbox is accessible and basic health
	DIAG_TYPE_PLATFORM_CONFIG = 1, // verifies BIOS config matches installed HW
	DIAG_TYPE_PM_META = 2, // verifies consistent and correct PM metadata
	DIAG_TYPE_SECURITY = 3, // verifies all manageable DIMMS have consistent security state
	DIAG_TYPE_FW_CONSISTENCY = 4 // verifies all DIMMS have consistent FW and attributes
};

/*
 * Diagnostic threshold type.
 */
enum diagnostic_threshold_type
{
	DIAG_THRESHOLD_QUICK_HEALTH = 1,
	DIAG_THRESHOLD_QUICK_MEDIA_TEMP = 1 << 1,
	DIAG_THRESHOLD_QUICK_CONTROLLER_TEMP = 1 << 2,
	DIAG_THRESHOLD_QUICK_AVAIL_SPARE = 1 << 3,
	DIAG_THRESHOLD_QUICK_PERC_USED = 1 << 4,
	DIAG_THRESHOLD_QUICK_SPARE_DIE = 1 << 5,
	DIAG_THRESHOLD_QUICK_UNCORRECT_ERRORS = 1 << 6,
	DIAG_THRESHOLD_QUICK_CORRECTED_ERRORS = 1 << 7,
	DIAG_THRESHOLD_QUICK_ERASURE_CODED_CORRECTED_ERRORS = 1 << 8,
	DIAG_THRESHOLD_QUICK_VALID_VENDOR_ID = 1 << 9,
	DIAG_THRESHOLD_QUICK_VALID_MANUFACTURER = 1 << 10,
	DIAG_THRESHOLD_QUICK_VALID_MODEL_NUMBER = 1 << 11,
	DIAG_THRESHOLD_QUICK_VIRAL = 1 << 12,
	DIAG_THRESHOLD_SECURITY_CONSISTENT = 1 << 13,
	DIAG_THRESHOLD_SECURITY_ALL_DISABLED = 1 << 14,
	DIAG_THRESHOLD_SECURITY_ALL_NOTSUPPORTED = 1 << 15,
	DIAG_THRESHOLD_FW_CONSISTENT = 1 << 16,
	DIAG_THRESHOLD_FW_MEDIA_TEMP = 1 << 17,
	DIAG_THRESHOLD_FW_CORE_TEMP = 1 << 18,
	DIAG_THRESHOLD_FW_SPARE = 1 << 19,
	DIAG_THRESHOLD_FW_POW_MGMT_POLICY = 1 << 20,
	DIAG_THRESHOLD_FW_PEAK_POW_BUDGET_MIN = 1 << 21,
	DIAG_THRESHOLD_FW_PEAK_POW_BUDGET_MAX = 1 << 22,
	DIAG_THRESHOLD_FW_AVG_POW_BUDGET_MIN = 1 << 23,
	DIAG_THRESHOLD_FW_AVG_POW_BUDGET_MAX = 1 << 24,
	DIAG_THRESHOLD_FW_DIE_SPARING_POLICY = 1 << 25,
	DIAG_THRESHOLD_FW_DIE_SPARING_LEVEL = 1 << 26,
	DIAG_THRESHOLD_FW_TIME = 1 << 27,
	DIAG_THRESHOLD_FW_DEBUGLOG = 1 << 28,
	DIAG_THRESHOLD_PCONFIG_NFIT = 1 << 29,
	DIAG_THRESHOLD_PCONFIG_PCAT = 1 << 30,
	DIAG_THRESHOLD_PCONFIG_PCD = 1llu << 31,
	DIAG_THRESHOLD_PCONFIG_CURRENT_PCD = 1llu << 32,
	DIAG_THRESHOLD_PCONFIG_UNCONFIGURED = 1llu << 33,
	DIAG_THRESHOLD_PCONFIG_BROKEN_ISET = 1llu << 34,
	DIAG_THRESHOLD_PCONFIG_MAPPED_CAPACITY = 1llu << 35,
	DIAG_THRESHOLD_PCONFIG_BEST_PRACTICES = 1llu << 36
};

// The volatile memory mode currently selected by the BIOS.
enum volatile_mode
{
	VOLATILE_MODE_1LM = 0, // 1LM Mode
	VOLATILE_MODE_MEMORY = 1, // Memory Mode
	VOLATILE_MODE_AUTO = 2, // Memory Mode if DDR4 + AEP DIMM present, 1LM otherwise
	VOLATILE_MODE_UNKNOWN = 3, // The current volatile memory mode cannot be determined.
};

// The App Direct mode currently selected by the BIOS.
enum app_direct_mode
{
	APP_DIRECT_MODE_DISABLED = 0, // App Direct mode disabled.
	APP_DIRECT_MODE_ENABLED = 1, // App Direct mode enabled.
	APP_DIRECT_MODE_UNKNOWN = 2, // The current App Direct mode cannot be determined.
};

// Interface format code as reported by NFIT
enum nvm_format {
	FORMAT_NONE = 0,
	FORMAT_BLOCK_STANDARD = 0x201,
	FORMAT_BYTE_STANDARD = 0x301
};

/*
 * Status of last AEP DIMM shutdown
 */
enum shutdown_status
{
	SHUTDOWN_STATUS_UNKNOWN = 0, // The last shutdown status cannot be determined.
	SHUTDOWN_STATUS_PM_ADR = 1 << 0, // Async DIMM Refresh command received
	SHUTDOWN_STATUS_PM_S3 = 1 << 1, // PM S3 received
	SHUTDOWN_STATUS_PM_S5 = 1 << 2, // PM S5 received
	SHUTDOWN_STATUS_DDRT_POWER_FAIL = 1 << 3, // DDRT power fail command received
	SHUTDOWN_STATUS_PMIC_12V_POWER_FAIL = 1 << 4,
	SHUTDOWN_STATUS_WARM_RESET = 1 << 5, // PM warm reset received
	SHUTDOWN_STATUS_FORCED_THERMAL = 1 << 6, // Thermal shutdown received
	SHUTDOWN_STATUS_CLEAN = 1 << 7 // Denotes a proper clean shutdown
};

/*
 * Status of the device current configuration
 */
enum config_status
{
	CONFIG_STATUS_NOT_CONFIGURED = 0, // The device is not configured.
	CONFIG_STATUS_VALID = 1, // The device has a valid configuration.
	CONFIG_STATUS_ERR_CORRUPT = 2, // The device configuration is corrupt.
	CONFIG_STATUS_ERR_BROKEN_INTERLEAVE = 3, // The interleave set is broken.
	CONFIG_STATUS_ERR_REVERTED = 4, // The configuration failed and was reverted.
	CONFIG_STATUS_ERR_NOT_SUPPORTED = 5, // The configuration is not supported by the BIOS.
};

/*
 * Status of current configuration goal
 */
enum config_goal_status
{
	CONFIG_GOAL_STATUS_UNKNOWN = 0, // The configuration goal status cannot be determined.
	CONFIG_GOAL_STATUS_NEW = 1, // The configuration goal has not yet been applied.
	CONFIG_GOAL_STATUS_SUCCESS = 2, // The configuration goal was applied successfully.
	CONFIG_GOAL_STATUS_ERR_BADREQUEST = 3, // The configuration goal was invalid.
	CONFIG_GOAL_STATUS_ERR_INSUFFICIENTRESOURCES = 4, // Not enough resources to apply the goal.
	CONFIG_GOAL_STATUS_ERR_FW = 5, // Failed to apply the goal due to a FW error.
	CONFIG_GOAL_STATUS_ERR_UNKNOWN = 6, // Failed to apply the goal for an unknown reason.
};

/*
 *  * Status of NVM jobs
 */
enum nvm_job_status
{
	NVM_JOB_STATUS_UNKNOWN 		= 0,
	NVM_JOB_STATUS_NOT_STARTED 	= 1,
	NVM_JOB_STATUS_RUNNING 		= 2,
	NVM_JOB_STATUS_COMPLETE 	= 3
};

/*
 * Type of job
 */
enum nvm_job_type
{
	NVM_JOB_TYPE_SANITIZE 	= 0,
	NVM_JOB_TYPE_ARS 		= 1
};

/*
 * Security related definition of interleave set or namespace.
 */
enum encryption_status
{
	NVM_ENCRYPTION_OFF = 0,
	NVM_ENCRYPTION_ON = 1,
	NVM_ENCRYPTION_IGNORE = 2
};

/*
 * Erase capable definition of interleave set or namespace.
 */
enum erase_capable_status
{
	NVM_ERASE_CAPABLE_FALSE = 0,
	NVM_ERASE_CAPABLE_TRUE = 1,
	NVM_ERASE_CAPABLE_IGNORE = 2
};

/*
 * firmware type
 */
enum device_fw_type
{
	DEVICE_FW_TYPE_UNKNOWN = 0, // fw image type cannot be determined
	DEVICE_FW_TYPE_PRODUCTION = 1,
	DEVICE_FW_TYPE_DFX = 2,
	DEVICE_FW_TYPE_DEBUG = 3
};

/*
 * ****************************************************************************
 * STRUCTURES
 * ****************************************************************************
 */

/*
 * The host server that the native API library is running on.
 */
struct host
{
	char name[NVM_COMPUTERNAME_LEN]; // The host computer name.
	enum os_type os_type; // OS type.
	char os_name[NVM_OSNAME_LEN]; // OS name string.
	char os_version[NVM_OSVERSION_LEN]; // OS version string.
	NVM_BOOL mixed_sku; // One or more AEP DIMMs have different SKUs.
	NVM_BOOL sku_violation; // Configuration of AEP DIMMs are unsupported due to a license issue.
};

/*
 * Software versions (one per server).
 */
struct sw_inventory
{
	NVM_VERSION mgmt_sw_revision; // Host software version.
	NVM_VERSION vendor_driver_revision; // Vendor specific NVDIMM driver version.
	NVM_BOOL vendor_driver_compatible; // Is vendor driver compatible with MGMT SW?
};

/*
 * Structure that describes a memory device in the system.
 * This data is harvested from the SMBIOS table Type 17 structures.
 */
struct memory_topology
{
	NVM_UINT16 physical_id; // Memory device's physical identifier (SMBIOS handle)
	enum memory_type memory_type; // Type of memory device
	enum device_form_factor form_factor; // Form factor of the memory device
	NVM_UINT64 raw_capacity; // Raw capacity of the device in bytes
	NVM_UINT64 data_width; // Width in bits used to store user data
	NVM_UINT64 total_width; // Width in bits for data and error correction/data redundancy
	NVM_UINT64 speed; // Speed in MHz
	char part_number[NVM_PART_NUM_LEN]; // Part number assigned by the vendor
	char device_locator[NVM_DEVICE_LOCATOR_LEN]; // Physically-labeled socket of device location
	char bank_label[NVM_BANK_LABEL_LEN]; // Physically-labeled bank of device location
};

/*
 * Structure that describes the security capabilities of a device
 */
struct device_security_capabilities
{
	NVM_BOOL passphrase_capable; // AEP DIMM supports the nvm_(set|remove)_passphrase command
	NVM_BOOL unlock_device_capable;  // AEP DIMM supports the nvm_unlock_device command
	NVM_BOOL erase_crypto_capable;  // AEP DIMM supports nvm_erase command with the CRYPTO
};

/*
 * Structure that describes the capabilities supported by an AEP DIMM
 */
struct device_capabilities
{
	NVM_BOOL die_sparing_capable; // AEP DIMM supports die sparing
	NVM_BOOL memory_mode_capable; // AEP DIMM supports memory mode
	NVM_BOOL storage_mode_capable; // AEP DIMM supports storage mode
	NVM_BOOL app_direct_mode_capable; // AEP DIMM supports app direct mode
};

/*
 * The device_discovery structure describes an enterprise-level view of a device with
 * enough information to allow callers to uniquely identify a device and determine its status.
 * The UID in this structure is used for all other device management calls to uniquely
 * identify a device.  It is intended that this structure will not change over time to
 * allow the native API library to communicate with older and newer revisions of devices.
 * @internal
 * Keep this structure to data from the Identify DIMM command and calculated data.
 * @endinternal
 */
struct device_discovery
{
	// Calculated by MGMT
	NVM_UID uid; // Unique identifier of the device.
	enum manageability_state manageability; // Compatibility of device, FW and mgmt.

	// ACPI
	NVM_NFIT_DEVICE_HANDLE device_handle; // The unique device handle of the memory module
	NVM_UINT16 physical_id; // The unique physical ID of the memory module
	NVM_UINT16 vendor_id;	// The vendor identifier.
	NVM_UINT16 device_id; // The device identifier.
	NVM_UINT16 revision_id; // The revision identifier.
	NVM_UINT16 channel_pos; // The memory module's position in the memory channel
	NVM_UINT16 channel_id; // The memory channel number
	NVM_UINT16 memory_controller_id; // The ID of the associated memory controller
	NVM_UINT16 socket_id; // The processor socket identifier.
	NVM_UINT16 node_controller_id; // The node controller ID.

	// SMBIOS
	enum memory_type memory_type; //	The type of memory used by the DIMM.

	// Identify Intel DIMM Gen 1
	NVM_MANUFACTURER manufacturer; // The manufacturer ID code determined by JEDEC JEP-106
	NVM_SERIAL_NUMBER serial_number;	// Serial number assigned by the vendor.
	NVM_UINT16 subsystem_vendor_id;		// vendor identifier of the AEP DIMM non-volatile
										// memory subsystem controller
	NVM_UINT16 subsystem_device_id;		// device identifier of the AEP DIMM non-volatile
										// memory subsystem controller
	NVM_UINT16 subsystem_revision_id; 	// revision identifier of the AEP DIMM non-volatile
										// memory subsystem controller
	NVM_BOOL manufacturing_info_valid;	// manufacturing location and date validity
	NVM_UINT8 manufacturing_location;	// AEP DIMM manufacturing location assigned by vendor
										// only valid if manufacturing_info_valid=1
	NVM_UINT16 manufacturing_date;		// Date the AEP DIMM was manufactured, assigned by vendor
										// only valid if manufacturing_info_valid=1

	char model_number[NVM_MODEL_LEN]; 	// Model number assigned by the vendor.
	NVM_VERSION fw_revision; // The current active firmware revision.
	NVM_VERSION fw_api_version; // API version of the currently running FW
	NVM_UINT64 capacity; // Raw capacity in bytes.
	NVM_UINT16 interface_format_codes[NVM_MAX_IFCS_PER_DIMM];
	struct device_security_capabilities security_capabilities;
	// Get Security State
	enum lock_state lock_state;  // Indicates if the DIMM is in a locked security state
	struct device_capabilities device_capabilities; // Capabilities supported by the device
	NVM_UINT32 dimm_sku;
};

/*
 * The status of a particular device
 */

struct device_status
{
	enum device_health health; // Overall device health.
	NVM_BOOL is_new; // Unincorporated with the rest of the devices.
	NVM_BOOL is_missing; // If the device is missing.
	NVM_UINT8 die_spares_used; // Number of spare devices on the AEP DIMM that are consumed.
	NVM_UINT8 last_shutdown_status; // State of last AEP DIMM shutdown.
	enum config_status config_status; // Status of last configuration request.
	NVM_UINT64 last_shutdown_time; // Time of the last shutdown - seconds since 1 January 1970
	NVM_BOOL mixed_sku; // One or more AEP DIMMs have different SKUs.
	NVM_BOOL sku_violation; // The AEP DIMM configuration is unsupported due to a license issue.
	NVM_BOOL viral_state; // Current viral status of AEP DIMM.
	enum device_ars_status ars_status; // Address range scrub operation status for the AEP DIMM
	NVM_UINT32 new_error_count; // Count of new fw errors from the AEP DIMM
	NVM_UINT64 newest_error_log_timestamp; // Timestamp of the newest log entry in the fw error log
};

/*
 * A snapshot of the performance metrics for a specific device.
 * @remarks All data is cumulative over the life the device.
 */
struct device_performance
{
	time_t time; // The time the performance snapshot was gathered.
	// These next fields are 16 bytes in the fw spec, but it would take 100 years
	// of over 31 million reads/writes per second to reach the limit, so we
	// are just using 8 bytes here.
	NVM_UINT64 bytes_read; // Total bytes of data read from the DIMM.
	NVM_UINT64 host_reads; // Lifetime number of read requests the DIMM serviced.
	NVM_UINT64 bytes_written; // Total bytes of data written to the DIMM.
	NVM_UINT64 host_writes; // Lifetime number of write requests the DIMM serviced.
	NVM_UINT64 block_reads; // Lifetime number of BW read requests the DIMM has services.
	NVM_UINT64 block_writes; // Lifetime number of BW write requests the DIMM has services.
};

/*
 * The threshold settings for a particular sensor
 */
struct sensor_settings
{
	NVM_BOOL enabled; // If firmware notifications are enabled when sensor value is critical.
	NVM_UINT64 upper_critical_threshold; // The upper critical threshold.
	NVM_UINT64 lower_critical_threshold; // The lower critical threshold.
	NVM_UINT64 upper_fatal_threshold; // The upper fatal threshold.
	NVM_UINT64 lower_fatal_threshold; // The lower fatal threshold.
	NVM_UINT64 upper_noncritical_threshold; // The upper noncritical threshold.
	NVM_UINT64 lower_noncritical_threshold; // The lower noncritical threshold.
};

/*
 * The current state and settings of a particular sensor
 */
struct sensor
{
	NVM_UID device_uid; // The unique identifier of the device this sensor applies to.
	enum sensor_type type; // The type of sensor.
	enum sensor_units units; // The units of measurement for the sensor.
	enum sensor_status current_state; // The current state of the sensor.
	NVM_UINT64 reading; // The current value of the sensor.
	struct sensor_settings settings; // The settings for the sensor.
	NVM_BOOL lower_critical_settable; // If the lower_critical_threshold value is modifiable.
	NVM_BOOL upper_critical_settable; // If the upper_critical_threshold value is modifiable.
	NVM_BOOL lower_critical_support; // If the lower_critical_threshold value is supported.
	NVM_BOOL upper_critical_support; // If the upper_critical_threshold value is supported.
	NVM_BOOL lower_fatal_settable; // If the lower_fatal_threshold value is modifiable.
	NVM_BOOL upper_fatal_settable; // If the upper_fatal_threshold value is modifiable.
	NVM_BOOL lower_fatal_support; // If the lower_fatal_threshold value is supported.
	NVM_BOOL upper_fatal_support; // If the upper_fatal_threshold value is supported.
	NVM_BOOL lower_noncritical_settable; // If the lower_noncritical_threshold value is modifiable.
	NVM_BOOL upper_noncritical_settable; // If the upper_noncritical_threshold value is modifiable.
	NVM_BOOL lower_noncritical_support; // If the lower_noncritical_threshold value is supported.
	NVM_BOOL upper_noncritical_support; // If the upper_noncritical_threshold value is supported.
};

/*
 * Device partition capacities (in bytes) used for a single device or aggregated across the server.
 */
struct device_capacities
{
	NVM_UINT64 capacity; // The total AEP DIMM capacity in bytes.
	NVM_UINT64 memory_capacity; // The total AEP DIMM capacity in bytes for memory mode.
	NVM_UINT64 app_direct_capacity; // The total AEP DIMM capacity in bytes for app direct mode.
	NVM_UINT64 storage_capacity; // AEP DIMM capacity allocated that can be used as storage.
	NVM_UINT64 unconfigured_capacity; // Unconfigured AEP DIMM capacity. Can be used as storage.
	NVM_UINT64 inaccessible_capacity; // AEP DIMM capacity not licensed for this AEP DIMM SKU.
	NVM_UINT64 reserved_capacity; // AEP DIMM capacity reserved for metadata.
};

/*
 * Modifiable settings of a device.
 */
struct device_settings
{
	NVM_BOOL first_fast_refresh; // Enable/disable acceleration of first refresh cycle.
	NVM_BOOL viral_policy; // Enable/disable viral policies.
};

/*
 * Detailed information about a device.
 */
struct device_details
{
	struct device_discovery discovery;	// Basic device identifying information.
	struct device_status status; // Device health and status.
	NVM_UINT8 padding[2]; // struct alignment
	struct device_performance performance; // A snapshot of the performance metrics.
	struct sensor sensors[NVM_MAX_DEVICE_SENSORS]; // Device sensors.
	struct device_capacities capacities; // Partition information

	// from SMBIOS Type 17 Table
	enum device_form_factor form_factor; // The type of DIMM.
	NVM_UINT64 data_width;	// The width in bits used to store user data.
	NVM_UINT64 total_width;	// The width in bits for data and ECC and/or redundancy.
	NVM_UINT64 speed; // The speed in nanoseconds.
	char part_number[NVM_PART_NUM_LEN]; // The part number
	char device_locator[NVM_DEVICE_LOCATOR_LEN]; // The socket or board position label
	char bank_label[NVM_BANK_LABEL_LEN]; // The bank label

	NVM_BOOL power_management_enabled; // Enable or disable power management.
	NVM_UINT8 power_limit; // dimm power limit in watts (10-18W).
	NVM_UINT16 peak_power_budget; // instantaneous power budget in mW (100-20000 mW).
	NVM_UINT16 avg_power_budget; // average power budget in mW (100-18000 mW).
	NVM_BOOL die_sparing_enabled; // Enable or disable die sparing.
	NVM_UINT8 die_sparing_level; // How aggressive to be in die sparing (0-255).
	struct device_settings settings; // Modifiable features of the device.
};

/*
 * Detailed information about firmware image log information of a device.
 */
struct device_fw_info
{
	/*
	 * BCD-formatted revision of the active firmware in the format MM.mm.hh.bbbb
	 * MM = 2-digit major version
	 * mm = 2-digit minor version
	 * hh = 2-digit hot fix version
	 * bbbb = 4-digit build version
	 */
	NVM_VERSION active_fw_revision;
	enum device_fw_type active_fw_type; // active FW type.
	char active_fw_commit_id[NVM_COMMIT_ID_LEN]; // commit id of active FW for debug/troubleshooting

	// build configuration of active FW for debug/troubleshooting
	char active_fw_build_configuration[NVM_BUILD_CONFIGURATION_LEN];

	NVM_BOOL staged_fw_pending; // set if new FW is staged for execution on the next reboot.
	NVM_VERSION staged_fw_revision; //  BCD formatted revision of the staged FW.
	enum device_fw_type staged_fw_type; // staged FW type.
};

/*
 * Details about a specific interleave format supported by memory
 */
struct interleave_format
{
	NVM_BOOL recommended; // is this format a recommended format
	enum interleave_size channel; // channel interleave of this format
	enum interleave_size imc; // memory controller interleave of this format
	enum interleave_ways ways; // number of ways for this format
};

/*
 * Supported capabilities of a specific memory mode
 */
struct memory_capabilities
{
	NVM_BOOL supported; // is the memory mode supported by the BIOS
	NVM_UINT16 interleave_alignment_size; // interleave alignment size in 2^n bytes.
	NVM_UINT16 interleave_formats_count; // Number of interleave formats supported by BIOS
	struct interleave_format interleave_formats[NVM_INTERLEAVE_FORMATS]; // interleave formats
};

/*
 * Supported features and capabilities BIOS supports
 */
struct platform_capabilities
{
	NVM_BOOL bios_config_support; // available BIOS support for CR config changes
	NVM_BOOL bios_runtime_support; // runtime interface used to validate management configuration
	NVM_BOOL memory_mirror_supported; // indicates if DIMM mirror is supported
	NVM_BOOL storage_mode_supported; // is storage mode supported
	NVM_BOOL memory_spare_supported; // pm spare is supported
	NVM_BOOL memory_migration_supported; // pm memory migration is supported
	struct memory_capabilities one_lm_mode; // capabilities for 1LM mode
	struct memory_capabilities memory_mode; // capabilities for Memory mode
	struct memory_capabilities app_direct_mode; // capabilities for App Direct mode
	enum volatile_mode current_volatile_mode; // The volatile memory mode selected by the BIOS.
	enum app_direct_mode current_app_direct_mode; // The App Direct mode selected by the BIOS.
};

/*
 * AEP DIMM software-supported features
 */
struct nvm_features
{
	NVM_BOOL get_platform_capabilities; // get platform supported capabilities
	NVM_BOOL get_devices; // retrieve the list of AEP DIMMs installed on the server
	NVM_BOOL get_device_smbios; // retrieve the SMBIOS information for AEP DIMMs
	NVM_BOOL get_device_health; // retrieve the health status for AEP DIMMs
	NVM_BOOL get_device_settings; // retrieve AEP DIMM settings
	NVM_BOOL modify_device_settings; // modify AEP DIMM settings
	NVM_BOOL get_device_security; // retrieve AEP DIMM security state
	NVM_BOOL modify_device_security; // modify AEP DIMM security settings
	NVM_BOOL get_device_performance; // retrieve AEP DIMM performance metrics
	NVM_BOOL get_device_firmware; // retrieve AEP DIMM firmware version
	NVM_BOOL update_device_firmware; // update the firmware version on AEP DIMMs
	NVM_BOOL get_sensors; // get health sensors on AEP DIMMS
	NVM_BOOL modify_sensors; // modify the AEP DIMM health sensor settings
	NVM_BOOL get_device_capacity; // retrieve how AEP DIMM capacity is mapped by BIOS
	NVM_BOOL modify_device_capacity; // modify how the AEP DIMM capacity is provisioned
	NVM_BOOL get_pools; // retrieve pools of AEP DIMM capacity
	NVM_BOOL get_namespaces; // retrieve the list of namespaces allocated from pools
	NVM_BOOL get_namespace_details; // retrieve detailed info about each namespace
	NVM_BOOL create_namespace; // create a new namespace
	NVM_BOOL rename_namespace; // rename an existing namespace
	NVM_BOOL grow_namespace; // increase the capacity of a namespace
	NVM_BOOL shrink_namespace; // decrease the capacity of a namespace
	NVM_BOOL enable_namespace; // enable a namespace
	NVM_BOOL disable_namespace; // disable a namespace
	NVM_BOOL delete_namespace; // delete a namespace
	NVM_BOOL get_address_scrub_data; // retrieve address range scrub data
	NVM_BOOL start_address_scrub; // initiate an address range scrub
	NVM_BOOL quick_diagnostic; // quick health diagnostic
	NVM_BOOL platform_config_diagnostic; // platform configuration diagnostic
	NVM_BOOL pm_metadata_diagnostic; // persistent memory metadata diagnostic
	NVM_BOOL security_diagnostic; // security diagnostic
	NVM_BOOL fw_consistency_diagnostic; // firmware consistency diagnostic
	NVM_BOOL memory_mode; // access AEP DIMM capacity as memory
	NVM_BOOL app_direct_mode; // access AEP DIMM persistent memory in App Direct Mode
	NVM_BOOL storage_mode; // access AEP DIMM persistent memory in Storage Mode
	NVM_BOOL error_injection; // error injection on AEP DIMMs
};

/*
 * Supported features and capabilities the driver/software supports
 */
struct sw_capabilities
{
	NVM_UINT32 block_size_count; // number of supported block sizes in the block sizes array
	NVM_UINT32 block_sizes[NVM_MAX_BLOCK_SIZES]; // the driver-validated list of block sizes
	NVM_UINT64 min_namespace_size; // smallest namespace supported by the driver, in bytes
	NVM_BOOL namespace_memory_page_allocation_capable;
};

/*
 * Aggregation of AEP DIMM SKU capabilities across all manageable AEP DIMMs in the system.
 */
struct dimm_sku_capabilities
{
	NVM_BOOL mixed_sku; // One or more AEP DIMMs have different SKUs.
	NVM_BOOL memory_sku; // One or more AEP DIMMs support memory mode.
	NVM_BOOL app_direct_sku; // One or more AEP DIMMs support app direct mode.
	NVM_BOOL storage_sku; // One or more AEP DIMMs support storage mode.
};

/*
 * Combined AEP DIMM capabilities
 */
struct nvm_capabilities
{
	struct nvm_features nvm_features; // supported features of the AEP DIMM software
	struct sw_capabilities sw_capabilities; // driver supported capabilities
	struct platform_capabilities platform_capabilities; // platform-supported capabilities
	struct dimm_sku_capabilities sku_capabilities; // aggregated AEP DIMM SKU capabilities
};

/*
 * Quality of service attributes for an app direct interleave set
 */
struct app_direct_attributes
{
	struct interleave_format interleave; // The interleave format
	NVM_BOOL mirrored; // Mirrored by the iMC
	NVM_UID dimms[NVM_MAX_DEVICES_PER_POOL]; // Unique ID's of underlying AEP DIMMs.
};

/*
 * Interleave set information
 */
struct interleave_set
{
	NVM_UINT32 set_index; // unique identifier from the PCD
	NVM_UINT32 driver_id; // unique identifier from the driver
	NVM_UINT64 size; // size in bytes
	NVM_UINT64 available_size; // free size in bytes
	struct interleave_format settings;
	NVM_UINT8 socket_id;
	NVM_UINT8 dimm_count;
	NVM_UID dimms[NVM_MAX_DEVICES_PER_SOCKET];
	NVM_BOOL mirrored;
	enum interleave_set_health health;
	enum encryption_status encryption; // on if lockstates of all dimms is enabled
	NVM_BOOL erase_capable; // true if all dimms in the set support erase
};

/*
 * Information about a pool of AEP DIMM capacity
 */
struct pool
{
	NVM_UID pool_uid; // Unique identifier of the pool.
	enum pool_type type; // The type of pool.
	NVM_UINT64 capacity; // Size of the pool in bytes.
	NVM_UINT64 free_capacity; // Available size of the pool in bytes.
	// The processor socket identifier.
	NVM_INT16 socket_id;
	NVM_UINT16 dimm_count; // The number of dimms in this pool.
	NVM_UINT16 ilset_count; // The number of interleave sets in this pool.
	// Raw capacity of each dimm in the pool in bytes.
	NVM_UINT64 raw_capacities[NVM_MAX_DEVICES_PER_POOL];
	// Memory mode capacity of each dimm in the pool in bytes.
	NVM_UINT64 memory_capacities[NVM_MAX_DEVICES_PER_POOL];
	// Storage mode capacity of each dimm in the pool in bytes.
	NVM_UINT64 storage_capacities[NVM_MAX_DEVICES_PER_POOL];
	NVM_UID dimms[NVM_MAX_DEVICES_PER_POOL]; // Unique ID's of underlying AEP DIMMs.
	// The interleave sets in this pool
	struct interleave_set ilsets[NVM_MAX_DEVICES_PER_POOL * 2];
	enum pool_health health; // Rolled up health of the underlying AEP DIMMs.
	// possible to create a namespace wholly contained on AEP DIMMs that have encryption enabled.
	NVM_BOOL encryption_enabled;
	// possible to create a namespace wholly contained on AEP DIMMs that support encryption.
	NVM_BOOL encryption_capable;
	// true if its possible to create namespace that would be wholly contained on erasable dimms
	NVM_BOOL erase_capable;
};

/*
 * Describes the configuration goal for a particular AEP DIMM.
 */
struct config_goal
{
	NVM_UINT64 memory_size; // Gibibytes of memory mode capacity on the DIMM.
	NVM_UINT16 app_direct_count; // The number of app direct interleave sets (0-2).
	NVM_UINT64 app_direct_1_size; // Gibibytes of AD 1 interleave set on the DIMM.
	NVM_UINT16 app_direct_1_set_id; // Unique index identifying interleave set for AD 1.
	struct app_direct_attributes app_direct_1_settings; // AD 1 QoS attributes.
	NVM_UINT64 app_direct_2_size; // Gibibytes of AD 2 interleave set on the DIMM.
	NVM_UINT16 app_direct_2_set_id; // Unique index identifying interleave set for AD 2.
	struct app_direct_attributes app_direct_2_settings; // AD 2 QoS attributes.
	enum config_goal_status status; // Status for the config goal. Ignored for input.
};

/*
 * Basic discovery information about a namespace.
 */
struct namespace_discovery
{
	NVM_UID namespace_uid; // Unique identifier of the namespace.
	char friendly_name[NVM_NAMESPACE_NAME_LEN]; // User supplied friendly name.
};

/*
 * Structure that describes the security features of a namespace
 */
struct namespace_security_features
{
	// encryption status of the NVDIMM or interleave set
	enum encryption_status encryption;
	// true if the parent NVDIMM or interleave set is erase capable
	enum erase_capable_status erase_capable;
};

/*
 * Detailed information about a namespace.
 */
struct namespace_details
{
	struct namespace_discovery discovery; // Basic discovery information.
	NVM_UID pool_uid; // The pool the namespace is created from.
	NVM_UINT32 block_size; // Block size in bytes.
	NVM_UINT64 block_count; // Number of blocks.
	enum namespace_type type; // The type of namespace
	enum namespace_health health; // Rolled-up health of the underlying AEP DIMMs.
	enum namespace_enable_state enabled; // If namespace is exposed to the OS.
	NVM_BOOL btt; // optimized for speed
	struct namespace_security_features security_features; // Security features
	struct interleave_format interleave_format;
	NVM_BOOL mirrored;
	union
	{
		NVM_UID device_uid; // Used when creating a storage Namespace
		NVM_UINT32 interleave_setid; // Used when creating an app direct Namespace
	} creation_id; // the identifier used by the driver when creating a Namespace
	enum namespace_memory_page_allocation memory_page_allocation;
};

/*
 * Caller specified settings for creating a new namespace.
 */
struct namespace_create_settings
{
	char friendly_name[NVM_NAMESPACE_NAME_LEN]; // User supplied friendly name.
	NVM_UINT16 block_size; // Block size in bytes.
	NVM_UINT64 block_count; // The number of blocks.
	enum namespace_type type; // The type of namespace.
	enum namespace_enable_state enabled; // If the namespace is exposed to OS after creation.
	NVM_BOOL btt; // optimized for speed
	struct namespace_security_features security_features; // Security features
	enum namespace_memory_page_allocation memory_page_allocation;
};

/*
 * Namespace size ranges. All sizes are in bytes.
 */
struct possible_namespace_ranges
{
    NVM_UINT64 largest_possible_app_direct_ns; // largest app direct namespace size possible
    NVM_UINT64 smallest_possible_app_direct_ns; // smallest app direct namespace size possible
    NVM_UINT64 app_direct_increment; // Valid increment between smallest & largest app direct size
    NVM_UINT64 largest_possible_storage_ns; // largest storage namespace size possible
    NVM_UINT64 smallest_possible_storage_ns; // smallest storage namespace size possible
    NVM_UINT64 storage_increment; // Valid increment between smallest & largest storage size
};

/*
 * The details of a specific device event that can be subscribed to
 * using #nvm_add_event_notify.
 */
struct event
{
	NVM_UINT32 event_id; // Unique ID of the event.
	enum event_type type; // The type of the event that occurred.
	enum event_severity severity; // The severity of the event.
	NVM_UINT16 code; // A numerical code for the specific event that occurred.
	NVM_BOOL action_required; // A flag indicating that the event needs a corrective action.
	NVM_UID uid; // The unique ID of the item that had the event.
	time_t time; // The time the event occurred.
	NVM_EVENT_MSG message; // A detailed description of the event type that occurred in English.
	NVM_EVENT_ARG args[NVM_MAX_EVENT_ARGS]; // The message arguments.
	enum diagnostic_result diag_result; // The diagnostic completion state (only for diag events).
};

/*
 * Limits the events returned by the #nvm_get_events method to
 * those that meet the conditions specified.
 */
struct event_filter
{
	/*
	 * A bit mask specifying the values in this structure used to limit the results.
	 * Any combination of the following or 0 to return all events.
	 * #NVM_FILTER_ON_TYPE
	 * #NVM_FITLER_ON_SEVERITY
	 * #NVM_FILTER_ON_CODE
	 * #NVM_FILTER_ON_UID
	 * #NVM_FILTER_ON_AFTER
	 * #NVM_FILTER_ON_BEFORE
	 * #NVM_FILTER_ON_EVENT
	 * #NVM_FILTER_ON_AR
	 */
	NVM_UINT8 filter_mask;

	/*
	 * The type of events to retrieve. Only used if
	 * #NVM_FILTER_ON_TYPE is set in the #filter_mask.
	 */
	enum event_type type;

	/*
	 * The type of events to retrieve. Only used if
	 * #NVM_FILTER_ON_SEVERITY is set in the #filter_mask.
	 */
	enum event_severity severity;

	/*
	 * The specific event code to retrieve. Only used if
	 * #NVM_FILTER_ON_CODE is set in the #filter_mask.
	 */
	NVM_UINT16 code;

	/*
	 * The identifier to retrieve events for.
	 * Only used if #NVM_FILTER_ON_UID is set in the #filter_mask.
	 */
	NVM_UID uid; // filter on specific item

	/*
	 * The time after which to retrieve events.
	 * Only used if #NVM_FILTER_ON_AFTER is set in the #filter_mask.
	 */
	time_t after; // filter on events after specified time

	/*
	 * The time before which to retrieve events.
	 * Only used if #NVM_FILTER_ON_BEFORE is set in the #filter_mask.
	 */
	time_t before; // filter on events before specified time

	/*
	 * Event ID number (row ID)
	 * Only used if #NVM_FILTER_ON_EVENT is set in the #filter mask.
	 */
	int event_id; // filter of specified event

	/*
	 * Only this action_required events are to be retrieved.
	 */
	NVM_BOOL action_required;
};

/*
 * An entry in the native API trace log.
 */
struct log
{
	NVM_PATH file_name; // The file that generated the log.
	int line_number; // The line number that generated the log.
	enum log_level level; // The log level.
	char message[NVM_LOG_MESSAGE_LEN]; // The log message
	time_t time; // The time
};

/*
 * An injected device error.
 */
struct device_error
{
	enum error_type type; // The type of error to inject.
	NVM_UINT64 dpa; // only valid if injecting poison error
	NVM_UINT64 temperature; // only valid if injecting temperature error
};

/*
 * A structure to hold a diagnostic threshold.
 * Primarily for allowing caller to override default thresholds.
 */
struct diagnostic_threshold
{
	enum diagnostic_threshold_type type; // A diagnostic threshold indicator
	NVM_UINT64 threshold; // numeric threshold
	char threshold_str[NVM_THRESHOLD_STR_LEN]; // text value used as a "threshold"
};

/*
 * A diagnostic test.
 */
struct diagnostic
{
	enum diagnostic_test test; // The type of diagnostic test to run
	NVM_UINT64 excludes; // Bitmask - zero or more diagnostic_threshold_type enums
	struct diagnostic_threshold *p_overrides; // override default thresholds that trigger failure
	NVM_UINT32 overrides_len; // size of p_overrides array
};

/*
 * Describes the identity of a system's physical processor in a NUMA context
 */
struct socket
{
	NVM_UINT16 id; // Zero-indexed NUMA node number
	NVM_UINT8 type; // Physical processor type number (via CPUID)
	NVM_UINT8 model; // Physical processor model number (via CPUID)
	NVM_UINT8 brand; // Physical processor brand index (via CPUID)
	NVM_UINT8 family; // Physical processor family number (via CPUID)
	NVM_UINT8 stepping; // Physical processor stepping number (via CPUID)
	char manufacturer[NVM_SOCKET_MANUFACTURER_LEN]; // Physical processor manufacturer (via CPUID)
	NVM_UINT16 logical_processor_count; // Logical processor count on node (incl. Hyperthreading)
};

struct job
{
	NVM_UID uid;
	NVM_UINT8 percent_complete;
	enum nvm_job_status status;
	enum nvm_job_type type;
	NVM_UID affected_element;
	void *result;
};

/*
 * ****************************************************************************
 * ENTRY POINT METHODS
 * ****************************************************************************
 */

/*
 * system.c
 */

/*
 * Retrieve just the host server name that the native API is running on.
 * @param[in, out] host_name
 * 		A caller supplied buffer to hold the host server name
 * @param[in] host_name_len
 * 		The length of the host_name buffer. Should be = #NVM_COMPUTERNAME_LEN.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_get_host_name(char *host_name, const NVM_SIZE host_name_len);

/*
 * Retrieve basic information about the host server the native API library is running on.
 * @param[in,out] p_host
 * 		A pointer to a #host structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_get_host(struct host *p_host);

/*
 * Retrieve a list of installed software versions related to AEP DIMM management.
 * @param[in,out] p_inventory
 * 		A pointer to a #sw_inventory structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @remarks If a version cannot be retrieved, the version is returned as all zeros.
 * @remarks AEP DIMM firmware revisions are not included.
 * @return Returns one of the following return_codes:
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_get_sw_inventory(struct sw_inventory *p_inventory);

/*
 * Retrieves the number of physical processors (NUMA nodes) in the system.
 * @pre
 * 		The OS must support its respective NUMA implementation.
 * @remarks
 * 		This method should be called before #nvm_get_socket or #nvm_get_sockets
 * @remarks
 * 		This method should never return a value less than 1.
 * @return
 * 		Returns the number of nodes on success or one of the following @link #return_code
 * 		return_codes: @endlink @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_get_socket_count();

/*
 * Retrieves #socket information about each processor socket in the system.
 * @pre
 * 		The OS must support its respective NUMA implementation.
 * @remarks
 * 		This method should never return a value less than 1.
 * @param[in,out] p_sockets
 * 		An array of #socket structures allocated by the caller.
 * @param[in] count
 * 		The size of the array
 * @return
 * 		Returns the number of nodes on success or one of the following @link #return_code
 * 		return_codes: @endlink @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_ARRAYTOOSMALL @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_get_sockets(struct socket *p_sockets, const NVM_UINT16 count);

/*
 * Retrieves #socket information about a given processor socket.
 * @pre
 * 		The OS must support its respective NUMA implementation.
 * @param[in] socket_id
 * 		The NUMA node identifier
 * @param[in,out] p_socket
 * 		A pointer to a #socket structure allocated by the caller.
 * @return
 * 		Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_BADSOCKET @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_get_socket(const NVM_UINT16 socket_id, struct socket *p_socket);


/*
 * device.c
 */

/*
 * Returns the number of memory devices installed in the system. This count includes
 * both AEP DIMMs and other memory devices, such as DRAM.
 * @pre The caller must have administrative privileges.
 * @remarks This method should be called before #nvm_get_device_topology.
 * @return Returns the number of devices in the topology on success
 * or one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_memory_topology_count();

/*
 * Retrieves basic topology information about all memory devices installed in the
 * system, including both AEP DIMMs and other memory devices, such as DRAM.
 * @pre The caller must have administrative privileges.
 * @return Returns the number of devices in the topology on success
 * or one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_ARRAYTOOSMALL @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_memory_topology(struct memory_topology *p_devices,
		const NVM_UINT8 count);

/*
 * Returns the number of devices installed in the system whether they are
 * fully compatible with the current native API library version or not.
 * @pre The caller must have administrative privileges.
 * @remarks This method should be called before #nvm_get_devices.
 * @remarks The number of devices can be 0.
 * @return Returns the number of devices on success
 * or one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOMEMORY @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_device_count();

/*
 * Retrieves #device_discovery information
 * about each device in the system whether they are fully compatible
 * with the current native API library version or not.
 * @param[in,out] p_devices
 * 		An array of #device_discovery structures allocated by the caller.
 * @param[in] count
 * 		The size of the array.
 * @pre The caller must have administrative privileges.
 * @remarks To allocate the array of #device_discovery structures,
 * call #nvm_get_device_count before calling this method.
 * @return Returns the number of devices on success
 * or one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_ARRAYTOOSMALL @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_devices(struct device_discovery *p_devices, const NVM_UINT8 count);

/*
 * Retrieve #device_discovery information about the device specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in,out] p_discovery
 * 		A pointer to a #device_discovery structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_device_discovery(const NVM_UID device_uid,
		struct device_discovery *p_discovery);

/*
 * Retrieve the #device_status of the device specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in,out] p_status
 * 		A pointer to a #device_status structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @pre The device is manageable.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 *		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_device_status(const NVM_UID device_uid,
		struct device_status *p_status);

/*
 * Retrieve #device_settings information about the device specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[out] p_settings
 * 		A pointer to a #device_settings structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @pre The device is manageable.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 *		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_device_settings(const NVM_UID device_uid,
		struct device_settings *p_settings);

/*
 * Set one or more configurable properties on the specified device.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] p_settings
 * 		A pointer to an #device_settings structure containing the modified settings.
 * @pre The caller must have administrative privileges.
 * @pre The device is manageable.
 * @remarks Retrieve the current #device_settings using #nvm_get_device_details and change
 * the specific settings as desired.
 * @remarks A given property change may require similar changes to related devices to
 * represent a consistent correct configuration.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 *		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_modify_device_settings(const NVM_UID device_uid,
		const struct device_settings *p_settings);

/*
 * Retrieve #device_details information about the device specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in,out] p_details
 * 		A pointer to a #device_details structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @pre The device is manageable.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 *		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_device_details(const NVM_UID device_uid,
		struct device_details *p_details);

/*
 * Retrieve a current snapshot of the performance metrics for the device specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in,out] p_performance
 * 		A pointer to a #device_performance structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @pre The device is manageable.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 *		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_device_performance(const NVM_UID device_uid,
		struct device_performance *p_performance);

/*
 * Retrieve the firmware image log information from the device specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in, out] p_fw_info
 * 		A pointer to a #device_fw_info structure allocated by the caller.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_device_fw_image_info(const NVM_UID device_uid,
		struct device_fw_info *p_fw_info);

/*
 * Push a new FW image to the device specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] path
 * 		Absolute file path to the new firmware image.
 * @param[in] path_len
 * 		String length of path, should be < #NVM_PATH_LEN.
 * @param[in] activate
 * 		If activate is 1 the firmware will be activated on-the-fly. If 0 a reboot is required.
 * 		It's critical to note that someone outside management software is responsible for quiescing
 * 		I/O activity and otherwise making sure the AEP DIMM and its associated capacity can be
 * 		offline for a period of time while the new FW image boots.
 * @param[in] force
 * 		If attempting to downgrade the minor version, force must be true.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @remarks A FW update may require similar changes to related devices to
 * represent a consistent correct configuration.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_BADFILE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADFIRMWARE @n
 * 		#NVM_ERR_REQUIRESFORCE @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_update_device_fw(const NVM_UID device_uid,
		const NVM_PATH path, const NVM_SIZE path_len, const NVM_BOOL activate,
		const NVM_BOOL force);

/*
 * Examine the FW image to determine if it is valid for the device specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] path
 * 		Absolute file path to the new firmware image.
 * @param[in] path_len
 * 		String length of path, should be < #NVM_PATH_LEN.
 * @param image_version
 * 		Firmware image version returned after examination
 * @param image_version_len
 * 		Buffer size for the image version
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @remarks A FW update may require similar changes to related devices to
 * represent a consistent correct configuration.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_BADFIRMWARE @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_BADFILE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_REQUIRESFORCE @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_examine_device_fw(const NVM_UID device_uid,
		const NVM_PATH path, const NVM_SIZE path_len,
		NVM_VERSION image_version, const NVM_SIZE image_version_len);

/*
 * Retrieve the supported capabilities for all devices in aggregate.
 * @param[in,out] p_capabilties
 * 		A pointer to an #nvm_capabilities structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_BADPCAT @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_nvm_capabilities(struct nvm_capabilities *p_capabilties);

/*
 * Retrieve the aggregate capacities across all manageable AEP DIMMs in the system.
 * @param[in,out] p_capacities
 * 		A pointer to an #device_capacities structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_nvm_capacities(struct device_capacities *p_capacities);

/*
 * security.c
 */

/*
 * If data at rest security is not enabled, this method enables it and
 * sets the passphrase. If data at rest security was previously enabled, this method changes
 * the passphrase to the new passphrase specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] old_passphrase
 * 		The current passphrase or NULL if security is disabled.
 * @param[in] old_passphrase_len
 * 		String length of old_passphrase,
 * 		should be <= #NVM_PASSPHRASE_LEN or 0 if security is disabled.
 * @param[in] new_passphrase
 * 		The new passphrase.
 * @param[in] new_passphrase_len
 * 		String length of new_passphrase, should be <= #NVM_PASSPHRASE_LEN.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @pre Device security is not frozen.
 * @pre The device passphrase limit has not been reached.
 * @post The device will be unlocked and frozen.
 * @post The device will be locked on the next reset.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_SECURITYFROZEN @n
 * 		#NVM_ERR_LIMITPASSPHRASE @n
 * 		#NVM_ERR_BADPASSPHRASE @n
 * 		#NVM_ERR_INVALIDPASSPHRASE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_set_passphrase(const NVM_UID device_uid,
		const NVM_PASSPHRASE old_passphrase, const NVM_SIZE old_passphrase_len,
		const NVM_PASSPHRASE new_passphrase, const NVM_SIZE new_passphrase_len);

/*
 * Disables data at rest security and removes the passphrase.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] passphrase
 * 		The current passphrase.
 * @param[in] passphrase_len
 * 		String length of passphrase, should be <= #NVM_PASSPHRASE_LEN.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @pre Device security is enabled and the passphrase has been set using #nvm_set_passphrase.
 * @pre Device security is not frozen.
 * @pre The device passphrase limit has not been reached.
 * @post The device will be unlocked if it is currently locked.
 * @post Device security will be disabled.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_SECURITYFROZEN @n
 * 		#NVM_ERR_SECURITYDISABLED @n
 * 		#NVM_ERR_LIMITPASSPHRASE @n
 * 		#NVM_ERR_BADPASSPHRASE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_remove_passphrase(const NVM_UID device_uid,
		const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len);

/*
 * Unlocks the device with the passphrase specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] passphrase
 * 		The current passphrase.
 * @param[in] passphrase_len
 * 		String length of passphrase, should be <= #NVM_PASSPHRASE_LEN.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @pre Device security is enabled and the passphrase has been set using #nvm_set_passphrase.
 * @pre Device security is not frozen.
 * @pre The device passphrase limit has not been reached.
 * @post The device will be unlocked and frozen.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_SECURITYFROZEN @n
 * 		#NVM_ERR_SECURITYDISABLED @n
 * 		#NVM_ERR_LIMITPASSPHRASE @n
 * 		#NVM_ERR_BADPASSPHRASE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 *		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 *		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 *		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_unlock_device(const NVM_UID device_uid,
		const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len);

/*
 * Prevent security lock state changes to the dimm until the next reboot
 * @param[in] device_uid
 * 		The device identifier.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @pre The device supports unlocking a device.
 * @pre Current dimm security state is unlocked
 * @post dimm security state will be frozen
 * @post Device security will be changed.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 *		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_SECURITYFROZEN @n
 * 		#NVM_ERR_BADPASSPHRASE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_freezelock_device(const NVM_UID device_uid);

/*
 * Erases data on the device specified by zeroing the device encryption key.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] passphrase
 * 		The current passphrase.
 * @param[in] passphrase_len
 * 		String length of passphrase, should be <= #NVM_PASSPHRASE_LEN.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @pre The device supports overwriting a device.
 * @pre Device security is disabled or sanitize antifreeze.
 * @post All user data is inaccessible.
 * @post Device security will be changed.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 *		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_SECURITYFROZEN @n
 * 		#NVM_ERR_BADPASSPHRASE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_erase_device(const NVM_UID device_uid,
		const NVM_PASSPHRASE passphrase, const NVM_SIZE passphrase_len);

/*
 * monitor.c
 */

/*
 * Retrieve all the health sensors for the specified AEP DIMM.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in,out] p_sensors
 * 		An array of #sensor structures allocated by the caller.
 * @param[in] count
 * 		The size of the array.  Should be #NVM_MAX_DEVICE_SENSORS.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @remarks Sensors are used to monitor a particular aspect of a device by
 * settings thresholds against a current value.
 * @remarks The number of sensors for a device is defined as #NVM_MAX_DEVICE_SENSORS.
 * @remarks Sensor information is returned as part of the #device_details structure.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_ARRAYTOOSMALL @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 *		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_sensors(const NVM_UID device_uid, struct sensor *p_sensors,
		const NVM_UINT16 count);

/*
 * Retrieve a specific health sensor from the specified AEP DIMM.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] s_type
 * 		The specific #sensor_type to retrieve.
 * @param[in,out] p_sensor
 * 		A pointer to a #sensor structure allocated by the caller.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_sensor(const NVM_UID device_uid, const enum sensor_type type,
		struct sensor *p_sensor);

/*
 * Change the critical threshold on the specified health sensor for the specified
 * AEP DIMM.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] s_type
 * 		The specific #sensor_type to modify.
 * @param[in] p_sensor_settings
 * 		The modified settings.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 *		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_set_sensor_settings(const NVM_UID device_uid,
		const enum sensor_type type, const struct sensor_settings *p_settings);

/*
 * Adds a function to be called when an event that matches the specified type.
 * An event structure is populated and passed
 * to the callback function describing the event that occurred.
 * @param[in] type
 * 		The type of event to notify on.  Use #EVENT_TYPE_ALL to subscribe
 * 		to all events for the device.
 * @param[in] p_event_callback
 * 		A pointer to the callback function.
 * @pre The caller has administrative privileges.
 * @remarks Callers must remove the callback using #nvm_remove_event_notify
 * to avoid memory leaks.
 * @return Returns a callback identifier if the callback was added or one of the
 * following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_add_event_notify(const enum event_type type,
		void (*p_event_callback) (struct event *p_event));

/*
 * Remove a callback subscription that was previously added using #nvm_add_event_notify.
 * @param[in] callback_id
 * 		The identifier for a specific callback, returned from #nvm_add_event_notify.
 * @pre The caller has administrative privileges.
 * @pre The callback was previously added using #nvm_add_event_notify
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADCALLBACK @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_remove_event_notify(const int callback_id);

/*
 * Retrieve the number of events in the native API library event database.
 * @param[in] p_filter
 * 		A pointer to an event_filter structure allocated by the caller to
 * 		optionally filter the event count. NULL will return the count of
 * 		all event log entries.
 * @pre The caller must have administrative privileges.
 * @return Returns the number of events on success or
 * one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_get_event_count(const struct event_filter *p_filter);

/*
 * Retrieve a list of stored events from the native API library database and
 * optionally filter the results.
 * @param[in] p_filter
 * 		A pointer to an event_filter structure to optionally
 * 		limit the results.  NULL to return all the events.
 * @param[in,out] p_events
 * 		An array of #event structures allocated by the caller.
 * @param[in] count
 * 		The size of the array.
 * @pre The caller must have administrative privileges.
 * @remarks The native API library stores a maximum of 10,000 events in the table,
 * rolling the table once the maximum is reached. However, the maximum number of events
 * is configurable by modifying the EVENT_LOG_MAX_ROWS value in the configuration database.
 * @return Returns the number of events on success or
 * one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_get_events(const struct event_filter *p_filter,
		struct event *p_events, const NVM_UINT16 count);

/*
 * Purge stored events from the native API database.
 * @param[in] p_filter
 * 		A pointer to an event_filter structure to optionally
 * 		purge only specific events. NULL to purge all the events.
 * @pre The caller must have administrative privileges.
 * @return Returns the number of events removed or
 * one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_purge_events(const struct event_filter *p_filter);

/*
 * Acknowledge an event from the native API database.
 * (i.e. setting action required field from true to false)
 * @param[in] event_id
 * 		The event id of the event to be acknowledged.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_acknowledge_event(NVM_UINT32 event_id);

/*
 * namespace.c
 */

/*
 * Retrieve the number of configured pools of AEP DIMM capacity in the host server.
 * @pre The caller has administrative privileges.
 * @remarks This method should be called before #nvm_get_pools.
 * @return Returns the number of pools on success or
 * one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only) @n
 */
extern NVM_API int nvm_get_pool_count();

/*
 * Retrieve a list of the configured pools of AEP DIMM capacity in host server.
 * @param[in,out] p_pools
 * 		An array of #pool structures allocated by the caller.
 * @param[in] count
 * 		The size of the array.
 * @pre The caller has administrative privileges.
 * @remarks To allocate the array of #pool structures,
 * call #nvm_get_pool_count before calling this method.
 * @return Returns the number of pools on success
 * or one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_INVALIDPERMISSIONS
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_ARRAYTOOSMALL @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only) @n
 */
extern NVM_API int nvm_get_pools(struct pool *p_pools, const NVM_UINT8 count);

/*
 * Retrieve a specific pool of AEP DIMM capacity.
 * @param[in] pool_uid
 * 		The identifier of the pool to retrieve
 * @param[in,out] p_pool
 * 		A pointer to a #pool structure allocated by the caller.
 * @pre The caller has administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADPOOL @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only) @n
 */
extern NVM_API int nvm_get_pool(const NVM_UID pool_uid, struct pool *p_pool);

/*
 * Takes a pool UUID and returns the largest and smallest app direct and storage namespaces
 * that can be created on that pool.
 * @pre The caller has administrative privileges.
 * @param[in] pool_id
 * 		UUID of the pool getting ranges for
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * @param[in,out] p_range
 * 		Structure that will contain the ranges
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only) @n
 */
extern NVM_API int nvm_get_available_persistent_size_range(const NVM_UID pool_uid,
		struct possible_namespace_ranges *p_range);

/*
 * Modify how the AEP DIMM capacity is provisioned by the BIOS on the next reboot.
 * @param[in] device_uid
 * 		The AEP DIMM identifier.
 * @param[in] p_goal
 * 		A pointer to a #config_goal structure allocated and populated by the caller.
 * @pre The caller has administrative privileges.
 * @pre The specified AEP DIMM is manageable by the host software.
 * @pre Any existing namespaces created from capacity on the
 * 		AEP DIMM must be deleted first.
 * @remarks This operation stores the specified configuration goal on the AEP DIMM
 * 		for the BIOS to read on the next reboot.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 *		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_NAMESPACESEXIST @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_BADSIZE @n
 * 		#NVM_ERR_BADDEVICECONFIG @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only) @n
 */
extern NVM_API int nvm_create_config_goal(const NVM_UID device_uid,
		struct config_goal *p_goal);

/*
 * Retrieve the configuration goal from the specified AEP DIMM.
 * @param device_uid
 * 		The AEP DIMM identifier.
 * @param p_goal
 * 		A pointer to a #config_goal structure allocated by the caller.
 * @pre The caller has administrative privileges.
 * @pre The specified AEP DIMM is manageable by the host software.
 * @remarks A configuration goal is stored on the AEP DIMM until the
 * 		BIOS successfully processes it on reboot.
 * 		Use @link nvm_delete_config_goal @endlink to erase a
 * 		configuration goal from an AEP DIMM.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 *		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_NOTFOUND @n
 * 		#NVM_ERR_BADDEVICECONFIG @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only) @n
 */
extern NVM_API int nvm_get_config_goal(const NVM_UID device_uid, struct config_goal *p_goal);

/*
 * Erase the pool configuration goal from the specified AEP DIMM.
 * @param device_uid
 * 		The AEP DIMM identifier.
 * @pre The caller has administrative privileges.
 * @pre The specified AEP DIMM is manageable by the host software.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 *		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only) @n
 */
extern NVM_API int nvm_delete_config_goal(const NVM_UID device_uid);

/*
 * Store the configuration settings of how the AEP DIMM capacity
 * is currently provisioned to a file in order to duplicate the
 * configuration elsewhere.
 * @param device_uid
 * 		The AEP DIMM identifier.
 * @param file
 * 		The absolute file path in which to store the configuration data.
 * @param file_len
 * 		String length of file, should be < #NVM_PATH_LEN.
 * @param append
 * 		Whether to append the AEP DIMM configuration to an existing file
 * 		or create/replace the file.
 * @pre The caller has administrative privileges.
 * @pre The specified AEP DIMM is manageable by the host software.
 * @pre The specified AEP DIMM is currently configured.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 *		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_BADFILE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_BADDEVICECONFIG
 * 		#NVM_ERR_NOTFOUND
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only) @n
 */
extern NVM_API int nvm_dump_config(const NVM_UID device_uid,
		const NVM_PATH file, const NVM_SIZE file_len,
		const NVM_BOOL append);

/*
 * Modify how the AEP DIMM capacity is provisioned by the BIOS on the
 * next reboot by applying the configuration goal previously stored in the
 * specified file with @link nvm_dump_config @endlink.
 * @param device_uid
 * 		The AEP DIMM identifier.
 * @param file
 * 		The absolute file path containing the pool configuration goal to load.
 * @param file_len
 * 		String length of file, should be < #NVM_PATH_LEN.
 * @pre The caller has administrative privileges.
 * @pre The specified AEP DIMM is manageable by the host software.
 * @pre Any existing namespaces created from capacity on the
 * 		AEP DIMM must be deleted first.
 * @pre If the configuration goal contains any app direct memory,
 * 		all AEP DIMMs that are part of the interleave set must be included in the file.
 * @pre The specified AEP DIMM must be >= the total capacity of the AEP DIMM
 * 		specified in the file.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 *		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_BADFILE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_NAMESPACESEXIST @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_BADSIZE @n
 * 		#NVM_ERR_BADDEVICECONFIG @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only) @n
 */
extern NVM_API int nvm_load_config(const NVM_UID device_uid,
		const NVM_PATH file, const NVM_SIZE file_len);

/*
 * Retrieve the number of namespaces allocated from pools of AEP DIMM
 * capacity in the host server.
 * @pre The caller must have administrative privileges.
 * @remarks This method should be called before #nvm_get_namespaces.
 * @return Returns the number of namespaces on success or
 * one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_namespace_count();

/*
 * Determine if any existing namespaces of the specified type
 * utilize capacity from the specified device.
 * @param device_uid
 * 		The AEP DIMM identifier.
 * @param[in] type
 * 		The type of namespace, use UNKNOWN to check for all namespaces.
 * @pre The caller must have administrative privileges.
 * @return Returns 1 if namespaces exist or
 * one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS = No namespaces @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_device_namespace_count(const NVM_UID uid,
		const enum namespace_type type);

/*
 * Retrieve discovery information about each namespace allocated from pools
 * of AEP DIMM capacity in the host server.
 * @param[in,out] p_namespaces
 * 		An array of #namespace_discovery structures allocated by the caller.
 * @param[in] count
 * 		The size of the array.
 * @pre The caller must have administrative privileges.
 * @remarks To allocate the array of #namespace_discovery structures,
 * call #nvm_get_namespace_count before calling this method.
 * @return Returns the number of namespaces on success
 * or one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_ARRAYTOOSMALL @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_namespaces(struct namespace_discovery *p_namespaces,
		const NVM_UINT8 count);

/*
 * Retrieve detailed information about the specified namespace.
 * @param[in] namespace_uid
 * 		The namespace identifier.
 * @param[in,out] p_namespace
 * 		A pointer to an #namespace_details structure allocated by the caller.
 * @pre The caller must have administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADNAMESPACE @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_namespace_details(const NVM_UID namespace_uid,
		struct namespace_details *p_namespace);

/*
 * Given a struct namespace_create_settings, adjust the block count so that the namespace
 * size meets alignment requirements for a namespace creation request.
 * @param[in] pool_uid
 * 		The pool identifier to create the namespace from.
 * @param[in,out] p_settings
 * 		The creation settings for a namespace
 * @param[in] *p_format
 * 		An optionally supplied pointer to an interleave_format struct allocated by the caller to
 * 		specify desired interleave set format for the namespace
 * 	@pre The caller has administrative privileges
 * 	@remarks  the namespace size is adjusted for alignment only, no other size requirements
 * 	are considered
 * 	@return Returns one of the  following @link #return_code return_codes: @endlink @n
 *		#NVM_SUCCESS @n
 *		#NVM_ERR_BADSIZE @n
 *		#NVM_ERR_NOTSUPPORTED @n
 *		#NVM_ERR_NOMEMORY @n
 *		#NVM_ERR_INVALIDPARAMETER @n
 *		#NVM_ERR_INVALIDPERMISSIONS @n
 *		#NVM_ERR_DRIVERFAILED @n
 *		#NVM_ERR_DATATRANSFERERROR @n
 *		#NVM_ERR_BADSECURITYGOAL @n
 *		#NVM_ERR_BADNAMESPACESETTINGS @n
 *		#NVM_ERR_BADDEVICE @n
 *		#NVM_ERR_DEVICEERROR @n
 *		#NVM_ERR_DEVICEBUSY @n
 *		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 *		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_adjust_create_namespace_block_count(const NVM_UID pool_uid,
		struct namespace_create_settings *p_settings, const struct interleave_format *p_format);
/*
 * Given namespace_uid and a block count, adjust the block count so that the namespace
 * size meets alignment requirements for a namespace modification request.
 * @param[in] namespace_uid
 * 		The namespace that is to be modified.
 * @param[in,out] block_count
 * 		The requested block count for the namespace
 * 	@pre The caller has administrative privileges
 * 	@remarks  the namespace size is adjusted for alignment only, no other size requirements
 * 	are considered
 * 	@return Returns one of the  following @link #return_code return_codes: @endlink @n
 *		#NVM_SUCCESS @n
 *		#NVM_ERR_BADSIZE @n
 *		#NVM_ERR_NOTSUPPORTED @n
 *		#NVM_ERR_NOMEMORY @n
 *		#NVM_ERR_INVALIDPARAMETER @n
 *		#NVM_ERR_INVALIDPERMISSIONS @n
 *		#NVM_ERR_BADNAMESPACE @n
 *		#NVM_ERR_DRIVERFAILED @n
 *		#NVM_ERR_DATATRANSFERERROR @n
 *		#NVM_ERR_DEVICEERROR @n
 *		#NVM_ERR_DEVICEBUSY @n
 *		#NVM_ERR_BADDEVICE @n
 *		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 *		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_adjust_modify_namespace_block_count(
		const NVM_UID namespace_uid, NVM_UINT64 *p_block_count);

/*
 * Create a new namespace on the specified persistent memory pool of AEP DIMM capacity.
 * @param[in,out] p_namespace_uid
 * 		The namespace identifier of the newly created namespace.
 * @param[in] pool_uid
 * 		The pool identifier to create the namespace from.
 * @param[in] p_settings
 * 		A pointer to an #namespace_create_settings structure describing the new
 * 		namespace settings.
 * @param[in] p_format
 * 		An optionally supplied pointer to an #interleave_format structure describing
 * 		the interleave set format for the namespace.
 * @pre The caller has administrative privileges.
 * @remarks block_size * block_size must result in a size that is less than
 * or equal to the available capacity of the pool.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_BADPOOL @n
 * 		#NVM_ERR_BADBLOCKSIZE @n
 * 		#NVM_ERR_BADSIZE @n
 * 		#NVM_ERR_BADNAMESPACETYPE @n
 * 		#NVM_ERR_BADNAMESPACEENABLESTATE @n
 * 		#NVM_ERR_BADSECURITYGOAL @n
 * 		#NVM_ERR_BADNAMESPACESETTINGS @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_create_namespace(NVM_UID *p_namespace_uid, const NVM_UID pool_uid,
		struct namespace_create_settings *p_settings,
		const struct interleave_format *p_format, const NVM_BOOL allow_adjustment);

/*
 * Change the friendly_name setting on the specified namespace.
 * @param[in] namespace_uid
 * 		The namespace identifier.
 * @param[in] name
 * 		A c-style string that contains the friendly name of the namespace.
 * @pre The caller has administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_BADNAMESPACE @n
 * 		#NVM_ERR_BADSIZE @n*
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_modify_namespace_name(const NVM_UID namespace_uid,
		const NVM_NAMESPACE_NAME name);

/*
 * Change the block_count setting on the specified namespace.
 * @param[in] namespace_uid
 * 		The namespace identifier.
 * @param[in] block_count
 * 		The number of blocks in the new size of the namespace.
 * @pre The caller has administrative privileges.
 * @remarks block_size * block_size must result in a size that is less than
 * or equal to the available capacity of the pool.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_BADNAMESPACE @n
 * 		#NVM_ERR_BADSIZE @n*
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_modify_namespace_block_count(const NVM_UID namespace_uid,
		NVM_UINT64 block_count, NVM_BOOL allow_adjustment);

/*
 * Change the enabled setting on the specified namespace.
 * @param[in] namespace_uid
 * 		The namespace identifier.
 * @param[in] enabled
 * 		NAMESPACE_ENABLE_STATE_ENABLED or NAMESPACE_ENABLE_STATE_DISABLED
 * @pre The caller has administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_BADNAMESPACE @n
 * 		#NVM_ERR_BADSIZE @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_modify_namespace_enabled(const NVM_UID namespace_uid,
		const enum namespace_enable_state enabled);
/*
 * Delete an existing namespace.
 * @param[in] namespace_uid
 * 		The namespace identifier.
 * @pre The caller has administrative privileges.
 * @remarks Resources allocated to the deleted namespace are returned to the
 * pool from which they originated.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_BADNAMESPACE @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_delete_namespace(const NVM_UID namespace_uid);

/*
 * support.c
 */

/*
 * Retrieve the native API library major version number.
 * @remarks Applications and the native API Library are not compatible if they were
 * 		written against different major versions of the native API definition.
 * 		For this reason, it is recommended that every application that uses the
 * 		native API Library to perform the following check:
 * 		if (#nvm_get_major_version() != #NVM_VERSION_MAJOR)
 * @return The major version number of the library.
 */
extern NVM_API int nvm_get_major_version();

/*
 * Retrieve the native API library minor version number.
 * @remarks Unless otherwise stated, every data structure, function, and description
 * 		described in this document has existed with those exact semantics since version 1.0
 * 		of the library.  In cases where functions have been added,
 * 		the appropriate section in this document will describe the version that introduced
 * 		the new feature.  Applications wishing to check for features that were added
 *		may do so by comparing the return value from #nvm_get_minor_version() against the
 * 		minor number in this specification associated with the introduction of the new feature.
 * @return The minor version number of the library.
 */
extern NVM_API int nvm_get_minor_version();

/*
 * Retrieve the native API library hot fix version number.
 * @return The hot fix version number of the library.
 */
extern NVM_API int nvm_get_hotfix_number();

/*
 * Retrieve the native API library build version number.
 * @return The build version number of the library.
 */
extern NVM_API int nvm_get_build_number();

/*
 * Retrieve native API library version as a string in the format MM.mm.hh.bbbb,
 * where MM is the major version, mm is the minor version, hh is the hotfix number
 * and bbbb is the build number.
 * @param[in,out] version_str
 * 		A buffer for the version string allocated by the caller.
 * @param[in] str_len
 * 		Size of the version_str buffer.  Should be #NVM_VERSION_LEN.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 */
extern NVM_API int nvm_get_version(NVM_VERSION version_str, const NVM_SIZE str_len);

/*
 * Given an numeric #return_code, retrieve a textual description of the return code in English.
 * @param[in] code
 * 		The #return_code to retrieve a description of.
 * @param[in,out] description
 * 		A buffer for the the textual description allocated by the caller.
 * @param[in] description_len
 * 		The size of the description buffer. Should be #NVM_ERROR_LEN.
 * @pre No limiting conditions apply to this function.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_BADERRORCODE @n
 */
extern NVM_API int nvm_get_error(const enum return_code code, NVM_ERROR_DESCRIPTION description,
		const NVM_SIZE description_len);

/*
 * Collect support data into a single file to document the context of a problem
 * for offline analysis by support or development personnel.
 * @param[in] support_file
 * 		Absolute file path where the support file will be stored.
 * @param[in] support_file_len
 * 		String length of the file path, should be < #NVM_PATH_LEN.
 * @pre The caller must have administrative privileges.
 * @post A support file exists at the path specified for debug by
 * support or development personnel.
 * @remarks The support file contains a current snapshot of the system as well as any
 * historical snapshots generated by #nvm_save_state, debug logs, events logs, current
 * and historical performance data and basic #host server information.
 * @remarks This operation will be attempt to gather as much information as possible about
 * the state of the system.  Therefore, it will ignore errors during the information
 * gathering process and only generate errors for invalid input parameters
 * or if the support file is not able to be generated.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_BADFILE @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_gather_support(const NVM_PATH support_file, const NVM_SIZE support_file_len);

/*
 * Capture a snapshot of the current state of the system in the configuration database
 * with the current date/time and optionally a user supplied name and description.
 * @param[in] name
 * 		Optional name to describe the snapshot.
 * @param[in] name_len
 * 		String length of the name. Ignored if name is NULL.
 * @pre The caller must have administrative privileges.
 * @remarks This operation will be attempt to gather as much information as possible about
 * the state of the system. Therefore, it will ignore errors during the information gathering
 * process and return the first error (if any) that it comes across.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_save_state(const char *name, const NVM_SIZE name_len);

/*
 * Clear any snapshots generated with #nvm_save_state from the configuration database.
 * @pre The caller must have administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_purge_state_data();

/*
 * Load a simulator database file.  A simulator may be used to emulate
 * a server with 0 or more devices for debugging purposes.
 * @param[in] simulator
 * 		Absolute path to a simulator file.
 * @param[in] simulator_len
 * 		String length of the simulator file path, should be < #NVM_PATH_LEN.
 * @pre Only available for simulated builds of the Native API library, all other
 * builds will return #NVM_ERR_NOTSUPPORTED.
 * @remarks If another file is currently open, it will remove it before
 * loading the current file.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADFILE @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_add_simulator(const NVM_PATH simulator, const NVM_SIZE simulator_len);

/*
 * Remove a simulator database file previously added using #nvm_add_simulator.
 * @pre Only available for simulated builds of the Native API library, all other
 * builds will return #NVM_ERR_NOTSUPPORTED.
 * @pre A simulator file was previously loaded using #nvm_add_simulator.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_NOSIMULATOR @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_remove_simulator();

/*
 * Retrieve the current level of debug logging performed on the specified AEP DIMM.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[out,in] p_log_level
 * 		A buffer for the log_level, allocated by the caller.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_fw_log_level(const NVM_UID device_uid, enum fw_log_level *p_log_level);

/*
 * Set the current level of debug logging performed on the specified AEP DIMM.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] log_level
 * 		The firmware log level.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_set_fw_log_level(const NVM_UID device_uid,
		const enum fw_log_level log_level);

/*
 * Inject an error into the device specified for debugging purposes.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] p_error
 * 		A pointer to a #device_error structure containing the injected
 * 		error information allocated by the caller.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @pre This interface is only supported by the underlying AEP DIMM firmware when it's in a
 * debug state.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_inject_device_error(const NVM_UID device_uid,
		const struct device_error *p_error);

/*
 * Clear an injected error into the device specified for debugging purposes.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] p_error
 * 		A pointer to a #device_error structure containing the injected
 * 		error information allocated by the caller.
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @pre This interface is only supported by the underlying AEP DIMM firmware when it's in a
 * debug state.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_clear_injected_device_error(const NVM_UID device_uid,
		const struct device_error *p_error);

/*
 * Run a diagnostic test on the device specified.
 * @param[in] device_uid
 * 		The device identifier.
 * @param[in] p_diagnostic
 * 		A pointer to a #diagnostic structure containing the
 * 		diagnostic to run allocated by the caller.
 * @param[in,out] p_results
 * 		The number of diagnostic failures. To see full results use #nvm_get_events
 * @pre The caller has administrative privileges.
 * @pre The device is manageable.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_NOTMANAGEABLE @n
 * 		#NVM_ERR_DRIVERFAILED
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_run_diagnostic(const NVM_UID device_uid,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results);

/*
 * logging.c
 */

/*
 * Determine if the native API debug logging is enabled.
 * @pre The caller must have administrative privileges.
 * @return Returns true (1) if debug logging is enabled and false (0) if not,
 * or one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 */
extern NVM_API int nvm_debug_logging_enabled();

/*
 * Toggle whether the native API library performs debug logging.
 * @param[in] debug @n
 * 		0: Log native API errors only. @n
 * 		1: Log all native API traces. @n
 * @pre The caller must have administrative privileges.
 * @remarks By default, the native API library starts logging errors only.
 * @remarks Debug logging may impact native API library performance depending
 * on the workload of the library.  It's recommended that debug logging is only
 * turned on during troubleshooting or debugging.
 * @remarks Changing the debug log level is persistent.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_toggle_debug_logging(const NVM_BOOL enabled);

/*
 * Clear any debug logs captured by the native API library.
 * @pre The caller must have administrative privileges.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_purge_debug_log();

/*
 * Retrieve the number of debug log entries in the native API library database.
 * @pre The caller must have administrative privileges.
 * @return Returns the number of debug log entries on success or
 * one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 */
extern NVM_API int nvm_get_debug_log_count();

/*
 * Retrieve a list of stored debug log entries from the native API library database
 * @todo Does this need to support filtering?
 * @param[in,out] p_logs
 * 		An array of #log structures allocated by the caller.
 * @param[in] count
 * 		The size of the array.
 * @pre The caller must have administrative privileges.
 * @remarks To allocate the array of #log structures,
 * call #nvm_get_debug_log_count before calling this method.
 * @return Returns the number of debug log entries on success
 * or one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_UNKNOWN @n
 */
extern NVM_API int nvm_get_debug_logs(struct log *p_logs, const NVM_UINT32 count);


/*
 * Get the number of current jobs.
 * @pre The caller must have administrative privileges.
 * @return Return the number of current jobs: @endlink @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_job_count();

/*
 * Retrieves #job information about each device in the system
 * @param[in,out] p_jobs
 * 		An array of #job structures allocated by the caller.
 * @param[in] count
 * 		The size of the array.
 * @pre The caller must have administrative privileges.
 * @remarks To allocate the array of #job structures,
 * call #nvm_get_job_count before calling this method.
 * @return Returns the number of devices on success
 * or one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_ERR_ARRAYTOOSMALL @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 *		#NVM_ERR_DRIVERFAILED @n
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDRIVER @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_get_jobs(struct job *p_jobs, const NVM_UINT32 count);

#if __ADD_MANUFACTURING__
/*
 * A device pass-through command. Refer to the FW specification
 * for specific details about the individual fields
 */
struct device_pt_cmd
{
	NVM_UINT8 opcode; // Command opcode.
	NVM_UINT8 sub_opcode; //  Command sub-opcode.
	NVM_UINT32 input_payload_size; //  Size of the input payload.
	void *input_payload; // A pointer to the input payload buffer.
	NVM_UINT32 output_payload_size; // Size of the output payload.
	void *output_payload; // A pointer to the output payload buffer.
	NVM_UINT32 large_input_payload_size; // Size of the large input payload.
	void *large_input_payload; // A pointer to the large input payload buffer.
	NVM_UINT32 large_output_payload_size; // Size of the large output payload.
	void *large_output_payload; // A pointer to the large output payload buffer.
	int result; // Return code from the pass through command
};

/*
 * Send a firmware command directly to the specified device without
 * checking for valid input.
 * @param device_uid
 * 		The device identifier.
 * @param p_cmd
 * 		A pointer to a @link #device_pt_command @endlink structure defining the command to send.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_INVALIDPERMISSIONS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDEVICE @n
 * 		#NVM_ERR_DRIVERFAILED
 * 		#NVM_ERR_DATATRANSFERERROR @n
 * 		#NVM_ERR_DEVICEERROR @n
 * 		#NVM_ERR_DEVICEBUSY @n
 * 		#NVM_ERR_NOSIMULATOR (Simulated builds only)
 */
extern NVM_API int nvm_send_device_passthrough_cmd(const NVM_UID device_uid,
		struct device_pt_cmd *p_cmd);

#endif

#ifdef __cplusplus
}
#endif

#endif  /* _NVM_MANAGEMENT_H_ */
