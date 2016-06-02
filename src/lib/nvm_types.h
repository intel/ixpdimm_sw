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
 * This file defines standard types used in the Native Management API.
 */

#ifndef NVM_TYPES_H_
#define	NVM_TYPES_H_

#include <stddef.h>
#include <limits.h>

#define	NVM_PRODUCT_NAME	"Intel DIMM Gen 1 Software\0"
#define	NVM_DIMM_NAME_LONG	"Intel DIMM Gen 1"
#define	NVM_DIMM_NAME	"AEP DIMM"
#define	NVM_SYSLOG_SOURCE	"NVM_MGMT"
#define	NVM_DEFAULT_NAMESPACE_NAME	"NvDimmVol"
#define	NVM_SYSTEM	"Intel DIMM Gen 1"
#define	NVM_INTEL_VENDOR_ID	0x8980

#define	NVM_COMPUTERNAME_LEN 256 // Length of host string
#define	NVM_OSNAME_LEN	256 // Length of host OS string
#define	NVM_OSVERSION_LEN	256 // Length of host OS version number
#define	NVM_PROCESSOR_LEN	256 // Length of host processor string
#define	NVM_VERSION_LEN	25  // Length of version string
#define	NVM_ERROR_LEN	256 // Length of return code description
#define	NVM_MODEL_LEN	21 // Length of device model number
#define	NVM_MAX_UID_LEN	37 // Max Length of Unique ID
#define	NVM_SOCKET_MANUFACTURER_LEN	32 // Socket manufacturer string length
#define	NVM_MANUFACTURER_LEN	2 // Number of bytes in the manufacturer ID
#define	NVM_MANUFACTURERSTR_LEN	256 // Manufacturer string length
#define	NVM_SERIAL_LEN	4 // Number of bytes in the serial number
#define	NVM_SERIALSTR_LEN	9 // Serial number string length
#define	NVM_PASSPHRASE_LEN	32 // Length of security passphrase
#define	NVM_MAX_DEVICE_SENSORS	17 // Maximum number of sensors
#define	NVM_EVENT_MSG_LEN	1024 // Length of event message string
#define	NVM_EVENT_ARG_LEN	1024 // Length of event argument string
#define	NVM_MAX_EVENT_ARGS	3 // Maximum number of event arguments
#define	NVM_FILTER_ON_TYPE	0x01 // Filter on event type
#define	NVM_FILTER_ON_SEVERITY	0x02 // Filter on event severity
#define	NVM_FILTER_ON_CODE	0x04 // Filter on code
#define	NVM_FILTER_ON_UID	0x08 // Filter on device uid
#define	NVM_FILTER_ON_AFTER	0x10 // Filter on time after
#define	NVM_FILTER_ON_BEFORE	0x20 // Filter on time before
#define	NVM_FILTER_ON_EVENT	0x40 // Filter on event ID
#define	NVM_FILTER_ON_AR	0x80 // Filter on action required
#define	NVM_PATH_LEN	PATH_MAX // Max length of file or directory path string (OS specific)
#define	NVM_DEVICE_LOCATOR_LEN	128 // Length of the device locator string
#define	NVM_BANK_LABEL_LEN	128	// Length of the bank label string
#define	NVM_NAMESPACE_NAME_LEN	64 // Length of namespace friendly name string
#define	NVM_NAMESPACE_PURPOSE_LEN	64 // Length of namespace purpose string
#define	NVM_MAX_SOCKETS	4 // Maximum number of sockets per system
#define	NVM_MAX_SOCKET_DIGIT_COUNT	4 // Maximum number of digits in a socket count
#define	NVM_MEMORY_CONTROLLER_CHANNEL_COUNT	3 // expected number of channels per iMC
#define	NVM_MAX_INTERLEAVE_SETS_PER_DIMM	2 // Max number of App Direct interleave sets per DIMM
#define	NVM_MAX_POOLS_PER_NAMESPACE	128 // Maximum number of pools for a namespace
#define	NVM_PART_NUM_LEN	32 // Length of device part number string : TBD, guessing at 32
// TODO -guessing and interleave formats size. HSD-20363 should address this.
#define	NVM_INTERLEAVE_FORMATS	32 // Maximum number of memory interleave formats
#define	NVM_MAX_DEVICES_PER_POOL	128 // Maximum number of DIMMs that can be used in a pool
// This number of devices that can go on a socket may go up on future architectures
// so this is something to keep an eye on. 24 should be good for a while
#define	NVM_MAX_DEVICES_PER_SOCKET	24 // Maximum number of dimms that can be on a socket
#define	NVM_LOG_MESSAGE_LEN	2048 // Length of log message string
#define	NVM_DRIVER_REV_LEN	16 // Driver revision length
#define	NVM_MAX_BLOCK_SIZES_PER_POOL	16
#define	NVM_MAX_BLOCK_SIZES	16 // maximum number of block sizes supported by the driver
#define	NVM_MAX_TOPO_SIZE	96 // Maximum number of DIMMs possible for a given memory topology
#define	NVM_THRESHOLD_STR_LEN	1024 // Max threshold string value len
#define	NVM_VOLATILE_POOL_SOCKET_ID	-1 // Volatile pools are system wide and not tied to a socket
#define	NVM_MAX_CONFIG_LINE_LEN	512 // Maximum line size for config data in a dump file
#define	NVM_DIE_SPARES_MAX	4 // Maximum number of spare dies
#define	NVM_COMMIT_ID_LEN	41
#define	NVM_BUILD_CONFIGURATION_LEN 17
#define	NVM_MAX_IFCS_PER_DIMM	9

/*
 * Macros for controlling what is exported by the library
 */
#ifdef __WINDOWS__ // Windows
#define	NVM_HELPER_DLL_IMPORT __declspec(dllimport)
#define	NVM_HELPER_DLL_EXPORT __declspec(dllexport)
#else // Linux/ESX
#define	NVM_HELPER_DLL_IMPORT __attribute__((visibility("default")))
#define	NVM_HELPER_DLL_EXPORT __attribute__((visibility("default")))
#endif // end Linux/ESX

// NVM_API is used for the public API symbols.
// NVM_LOCAL is used for non-api symbols.
#ifdef	__NVM_DLL__ // defined if compiled as a DLL
#ifdef	__NVM_DLL_EXPORTS__ // defined if we are building the DLL (instead of using it)
#define	NVM_API NVM_HELPER_DLL_EXPORT
#else
#define	NVM_API NVM_HELPER_DLL_IMPORT
#endif // NVM_DLL_EXPORTS
#else // NVM_DLL is not defined, everything is exported
#define	NVM_API
#endif // NVM_DLL

typedef size_t NVM_SIZE; // String length size
typedef char NVM_INT8; // 8 bit signed integer
typedef signed short NVM_INT16; // 16 bit signed integer
typedef signed int  NVM_INT32; // 32 bit signed integer
typedef unsigned char NVM_BOOL; // 8 bit unsigned integer as a boolean
typedef unsigned char NVM_UINT8; // 8 bit unsigned integer
typedef unsigned short NVM_UINT16; // 16 bit unsigned integer
typedef unsigned int NVM_UINT32; // 32 bit unsigned integer
typedef unsigned long long NVM_UINT64; // 64 bit unsigned integer
typedef long long NVM_INT64; // 64 bit integer
typedef float NVM_REAL32; // 32 bit floating point number
typedef char NVM_VERSION[NVM_VERSION_LEN]; // Version number string
typedef char NVM_ERROR_DESCRIPTION[NVM_ERROR_LEN]; // Return code description
typedef char NVM_UID[NVM_MAX_UID_LEN]; // Unique ID
typedef char NVM_PASSPHRASE[NVM_PASSPHRASE_LEN]; // Security passphrase
typedef char NVM_EVENT_MSG[NVM_EVENT_MSG_LEN]; // Event message string
typedef char NVM_EVENT_ARG[NVM_EVENT_ARG_LEN]; // Event argument string
typedef char NVM_PATH[NVM_PATH_LEN]; // TFile or directory path
typedef unsigned char NVM_MANUFACTURER[NVM_MANUFACTURER_LEN]; // Manufacturer identifier
typedef unsigned char NVM_SERIAL_NUMBER[NVM_SERIAL_LEN]; // Serial Number
typedef char NVM_NAMESPACE_NAME[NVM_NAMESPACE_NAME_LEN]; // Namespace name

typedef union
{
	struct device_handle_parts
	{
		NVM_UINT32 mem_channel_dimm_num:4;
		NVM_UINT32 mem_channel_id:4;
		NVM_UINT32 memory_controller_id:4;
		NVM_UINT32 socket_id:4;
		NVM_UINT32 node_controller_id:12;
		NVM_UINT32 rsvd:4;
	} parts;
	NVM_UINT32 handle;
} NVM_NFIT_DEVICE_HANDLE;

/*
 * Return values for the entry point functions in the native API library.
 */
enum return_code
{
	NVM_SUCCESS = 0, // The method succeeded.
	NVM_ERR_UNKNOWN = -1, // An unknown error occurred.
	NVM_ERR_NOMEMORY = -2, // Not enough memory to complete the requested operation.
	NVM_ERR_NOTSUPPORTED = -3, // This method is not supported in the current context.
	NVM_ERR_INVALIDPARAMETER = -4, // One or more input parameters were incorrect.
	NVM_ERR_NOTMANAGEABLE = -5,	// The device is not manageable by the management software.
	NVM_ERR_INVALIDPERMISSIONS = -6, // Not appropriate privileges to complete the operation.
	NVM_ERR_BADERRORCODE = -7, // The return code was not valid.
	NVM_ERR_DATATRANSFERERROR = -8, // There was an error in the data transfer.
	NVM_ERR_DEVICEERROR = -9, // There was an internal error in the device.
	NVM_ERR_DEVICEBUSY = -10, // The device is currently busy processing a long operation command.
	NVM_ERR_BADPASSPHRASE = -11, // The passphrase is not valid.
	NVM_ERR_INVALIDPASSPHRASE = -12, // The new passphrase does not meet the minimum requirements.
	NVM_ERR_SECURITYFROZEN = -13, // No changes can be made to the security state of the device.
	NVM_ERR_LIMITPASSPHRASE = -14, // The maximum passphrase submission limit has been reached.
	NVM_ERR_SECURITYDISABLED = -15, // Data at rest security is not enabled.
	NVM_ERR_BADDEVICE = -16, // The device identifier is not valid.
	NVM_ERR_ARRAYTOOSMALL = -17, // The array is not big enough.
	NVM_ERR_BADCALLBACK = -18, // The callback identifier is not valid.
	NVM_ERR_BADFILE = -19, // The file is not valid.
	NVM_ERR_NOSIMULATOR = -20, // No simulator is loaded.
	NVM_ERR_BADPOOL = -21, // The pool identifier is not valid.
	NVM_ERR_BADNAMESPACE = -22, // The namespace identifier is not valid.
	NVM_ERR_BADBLOCKSIZE = -23, // The specified block size is not valid.
	NVM_ERR_BADSIZE = -24, // The size specified is not valid.
	NVM_ERR_BADFIRMWARE = -25, // The firmware image is not valid for the device.
	NVM_ERR_DRIVERFAILED = -26, // The device driver failed the requested operation
	NVM_ERR_BADSOCKET = -29, // The processor socket identifier is not valid.
	NVM_ERR_BADSECURITYSTATE = -30, // Device security state does not permit the request.
	NVM_ERR_REQUIRESFORCE = -31, // This method requires the force flag to proceed.
	NVM_ERR_NAMESPACESEXIST = -32, // Existing namespaces must be deleted first.
	NVM_ERR_NOTFOUND = -33, // The requested item was not found.
	NVM_ERR_BADDEVICECONFIG = -34, // The configuration data is invalid or unrecognized.
	NVM_ERR_DRIVERNOTALLOWED = -35, // Driver is not allowing this command
	NVM_ERR_BADALIGNMENT = -36, // The specified size does not have the required alignment.
	NVM_ERR_BADTHRESHOLD = -37, // The threshold value is invalid.
	NVM_ERR_EXCEEDSMAXSUBSCRIBERS = -38, // Exceeded maximum number of notify subscribers
	NVM_ERR_BADNAMESPACETYPE = -39, // The specified namespace type is not valid.
	NVM_ERR_BADNAMESPACEENABLESTATE = -40, // The specified namespace enable state is not valid.
	NVM_ERR_BADNAMESPACESETTINGS = -41, // Could not create ns with specified settings.
	NVM_ERR_BADPCAT = -42, // The PCAT table is invalid.
	NVM_ERR_TOOMANYNAMESPACES = -43, // The maximum number of namespaces is already present.
	NVM_ERR_CONFIGNOTSUPPORTED = -44, // The requested configuration is not supported.
	NVM_ERR_SKUVIOLATION = -45, // The method is not supported because of a license violation.
	NVM_ERR_BADDRIVER = -46, // The underlying software is missing or incompatible.
	NVM_ERR_ARSINPROGRESS = -47, // Address range scrub in progress.
	NVM_ERR_BADSECURITYGOAL = -48, // No dimm found with matching security goal to create a NS.
	NVM_ERR_INVALIDPASSPHRASEFILE = -49, // The passphrase file is invalid.
};

/*
 * Type of pool.
 */
enum pool_type
{
	POOL_TYPE_UNKNOWN = 0,
	POOL_TYPE_PERSISTENT = 1, // Pool type is non-mirrored App Direct or Storage.
	POOL_TYPE_VOLATILE = 2, // Volatile.
	POOL_TYPE_PERSISTENT_MIRROR = 3, // Persistent.
};

/*
 * Rolled-up health of the underlying AEP DIMMs from which the pool is created.
 */
enum pool_health
{
	POOL_HEALTH_UNKNOWN	= 0, // Health cannot be determined.
	POOL_HEALTH_NORMAL = 1,	// All underlying AEP DIMMs are available.
	POOL_HEALTH_WARNING = 2, // An underlying AEP DIMM has a critical or non-critical health issue
	POOL_HEALTH_DEGRADED = 3, // One or more AEP DIMMs are missing or failed. Pool is mirrored.
	POOL_HEALTH_FAILED = 4 // One or more AEP DIMMs are missing or failed.
};

/*
 * Health of an individual interleave set.
 */
enum interleave_set_health
{
	INTERLEAVE_HEALTH_UNKNOWN = 0, // Health cannot be determined.
	INTERLEAVE_HEALTH_NORMAL = 1, // Available and underlying AEP DIMMs have good health.
	INTERLEAVE_HEALTH_DEGRADED = 2, // In danger of failure, may have degraded performance.
	INTERLEAVE_HEALTH_FAILED = 3 // Interleave set has failed and is unavailable.
};

/*
 * Type of the namespace
 */
enum namespace_type
{
	NAMESPACE_TYPE_UNKNOWN = 0, // Type cannot be determined
	NAMESPACE_TYPE_STORAGE = 1, // Storage namespace
	NAMESPACE_TYPE_APP_DIRECT = 2 // App Direct namespace
};

/*
 * Namespace health
 */
enum namespace_health
{
	NAMESPACE_HEALTH_UNKNOWN = 0, // Namespace health cannot be determined
	NAMESPACE_HEALTH_NORMAL = 5, // Namespace is OK
	NAMESPACE_HEALTH_NONCRITICAL = 10, // Non-critical health issue
	NAMESPACE_HEALTH_CRITICAL = 25, // Critical health issue
	NAMESPACE_HEALTH_BROKENMIRROR = 65535 // Broken mirror
};

/*
 * If the namespace is exposed to the OS
 */
enum namespace_enable_state
{
	NAMESPACE_ENABLE_STATE_UNKNOWN = 0, // Cannot be determined
	NAMESPACE_ENABLE_STATE_ENABLED = 2, // Exposed to OS
	NAMESPACE_ENABLE_STATE_DISABLED = 3 // Hidden from OS
};

/*
 * Where the capacity is allocated from for the underlying OS structures
 * if access to the AppDirect namespace capacity is supported using
 * legacy memory page protocols such as DMA/RDMA
 */
enum namespace_memory_page_allocation
{
	NAMESPACE_MEMORY_PAGE_ALLOCATION_UNKNOWN = 0, // Cannot be determined
	NAMESPACE_MEMORY_PAGE_ALLOCATION_NONE = 1, // No support to access NS using memoryPageProtocols
	NAMESPACE_MEMORY_PAGE_ALLOCATION_DRAM = 2, // Capacity is allocated from DRAM
	NAMESPACE_MEMORY_PAGE_ALLOCATION_APP_DIRECT = 3 // Capacity is allocated from the NS itself
};

#endif /* NVM_TYPES_H_ */
