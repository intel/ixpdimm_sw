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

/*
 * Types and values as defined in the FIS document.
 */

#ifndef	CR_MGMT_FIS_TYPES_H
#define	CR_MGMT_FIS_TYPES_H

/*
 * ****************************************************************************
 * DEFINES
 * ****************************************************************************
 */
#define	DEV_SN_LEN		4  /* DIMM Serial Number buffer length */
#define	DEV_PASSPHRASE_LEN	32  /* Length of a passphrase buffer */
#define	DEV_BCD_DATE_LEN		4   /* Length of a BDC Formatted Date */
#define	DEV_BCD_TIME_LEN		3   /* Length of a BDC Formatted Time */
#define	DEV_BCD_ETC_TIME_LEN	5   /* Len of a BDC Formatted Time for longop */
#define	DEV_COMM_SPECIFIC_DATA	119 /* Length of the specific long operation data */
#define	DEV_FW_REV_LEN		5   /* Length of the Firmware Revision string */
#define	DEV_FW_COMMIT_ID_LEN	40   /* Length of commit identifier of Firmware including null */
#define	DEV_MFR_LEN		2  /* Length of manufacturer ID buffer */
#define	DEV_MODELNUM_LEN		20  /* Length of DIMM Model Number buffer */
#define	DEV_OS_PARTITION		1   /* get platform config OS partition num */
#define DEV_PLT_CFG_OPTIONS	((0 << 1) + 0) /* get platform config options */
#define	DEV_PLT_CFG_LARGE_PAY	0 /* get/set platform config large payload type */
#define	DEV_PLT_CFG_SMALL_PAY	1 /* get/set platform config small payload type */
#define DEV_PLT_CFG_OPT_DATA	(0 << 1) /* get platform config option data */
#define	DEV_PLT_CFG_OPT_SIZE	(1 << 1) /* get platform config option size */
/* get platform config options */
#define	DEV_PLT_CFG_OPT_SMALL_DATA	(DEV_PLT_CFG_SMALL_PAY | DEV_PLT_CFG_OPT_DATA)
#define	DEV_PLT_CFG_OPT_LARGE_DATA	(DEV_PLT_CFG_LARGE_PAY | DEV_PLT_CFG_OPT_DATA)
#define DEV_DDRT_ALERT_TYPES	128 /* Max number of different DDRT Alerts */
#define DEV_SMALL_PAYLOAD_SIZE	128 /* 128B - Size for a passthrough command small payload */
#define DEV_PLT_CFG_PART_SIZE	(128 << 10) /* Size of one partition in platform config data */
#define DEV_ERR_LOG_SIZE		(1 << 20) /* Size of one FW error log */
#define DEV_FW_LOG_PAGE_SIZE	1024 /* Size of one FW log page */
#define	DEV_FW_BUILD_CONFIGURATION_LEN 16 /* Size of the build configuration including null */
/* Number of bytes that can be written to PCD through small payload path at a time */
#define DEV_PLT_CFG_SMALL_PAYLOAD_WRITE_SIZE 64

#define DEV_FW_ERR_LOG_OVERRUN (1 << 7)
#define DEV_FW_ERR_LOG_LOW (0)
#define DEV_FW_ERR_LOG_HIGH (1)
#define DEV_FW_ERR_LOG_MEDIA (0 << 1)
#define DEV_FW_ERR_LOG_THERMAL (1 << 1)
#define DEV_FW_ERR_LOG_RETRIEVE_ENTRIES (0 << 2)
#define DEV_FW_ERR_LOG_RETRIEVE_INFO_DATA (1 << 2)
#define DEV_FW_ERR_LOG_SMALL_PAYLOAD (0 << 3)
#define DEV_FW_ERR_LOG_LARGE_PAYLOAD (1 << 3)

#define DEV_FW_BSR_MAJOR_CHECKPOINT_BITS (0xFF)
#define DEV_FW_BSR_MAJOR_CHECKPOINT_STATUS (0xA0)
#define DEV_FW_BSR_MAJOR_CHECKPOINT_FAILURE (0xA1)
#define DEV_FW_BSR_MAJOR_CHECKPOINT_COMPLETE (0xF0)

#define DEV_FW_BSR_MINOR_CHECKPOINT_BITS (0xFF << 8)
#define DEV_FW_BSR_MINOR_CHECKPOINT_STATUS_FAILURE (0xFF << 8)
#define DEV_FW_BSR_MINOR_CHECKPOINT_COMPLETE_SUCCESS (0x00)

#define DEV_FW_BSR_MEDIA_READY_BITS (0x03 << 16)
#define DEV_FW_BSR_MEDIA_READY_READY (0x01 << 16)
#define DEV_FW_BSR_MEDIA_READY_ERROR (0x02 << 16)

#define DEV_FW_BSR_DDRT_IO_INIT_BITS (0x03 << 18)
#define DEV_FW_BSR_DDRT_IO_INIT_READY (0x01 << 18)
#define DEV_FW_BSR_DDRT_IO_INIT_ERROR (0x02 << 18)

#define DEV_FW_BSR_MBR_READY (0x01 << 20)

#define DEV_FW_BSR_WTS_NMI (0x01 << 21)

#define DEV_FW_BSR_FRCF_COMPLETE (0x01 << 22)

#define DEV_FW_BSR_WDB_FLUSHED (0x01 << 23)

#define DEV_FW_BSR_ASSERTION ((long long) 0x01 << 32)

#define DEV_FW_BSR_STALLED ((long long) 0x01 << 33)


#define DSM_VENDOR_ERROR_SHIFT (0)
#define DSM_MAILBOX_ERROR_SHIFT (16)
#define DSM_BACKGROUND_OP_STATE_SHIFT (24)

#define DSM_VENDOR_ERROR(status) ((status & 0xFFFF) >> DSM_VENDOR_ERROR_SHIFT)
#define DSM_EXTENDED_ERROR(status) ((status & 0xFF0000) >> DSM_MAILBOX_ERROR_SHIFT)
#define DSM_BACKGROUND_OP_STATE(status) ((status & 0x1000000) >> DSM_BACKGROUND_OP_STATE_SHIFT)

// Bit masks for identify_dimm.dimm_sku
#define ENCRYPTION_ENABLED(dimm_sku) ((dimm_sku >> 17) & 1)

#define	TRANSFER_TYPE_INITIATE	0b00
#define	TRANSFER_TYPE_CONTINUE	0b01
#define	TRANSFER_TYPE_END	0b10
#define TRANSFER_HEADER(type, packet)	((packet << 2) | type)
#define	TRANSFER_SIZE	64
#define	TRANSFER_VIA_LARGE_PAYLOAD	0
#define	TRANSFER_VIA_SMALL_PAYLOAD	1

/*
 * Convert a capacity in 4KB multiples to bytes
 */
#define	MULTIPLES_TO_BYTES(val)	(4096llu * ((NVM_UINT64)(val)))

// Intel DIMM Subsystem Device IDs supported by this software
extern const int NUM_SUPPORTED_DEVICE_IDS;
extern const NVM_UINT16 SUPPORTED_DEVICE_IDS[];

// Specify that the ARS command has reached the limit+1 errors it can report and has
// ended prior to reaching the DPA end address
#define	ARS_STATUS_ENDED_EARLY	0x01

/*
 * ****************************************************************************
 * ENUMS
 * ****************************************************************************
 */

/*
 * Destination of the dsm vendor command
 */
enum pt_cmd_destination_flag {
	CMD_DEST_NVM_DIMM = 0,
	CMD_DEST_BIOS = 1,
};

/*
 * Opcodes of the emulated vendor specific BIOS commands
 */
enum bios_emulated_opcode {
	BIOS_EMULATED_COMMAND = 0xFD,
};

#define BUILD_DSM_OPCODE(opcode, subop_code) (NVM_UINT32)(subop_code << 8 | opcode)

enum bios_emulated_command_subop {
	SUBOP_GET_PAYLOAD_SIZE = 0x00,
	SUBOP_WRITE_LARGE_PAYLOAD_INPUT = 0x01,
	SUBOP_READ_LARGE_PAYLOAD_OUTPUT = 0x02,
	SUBOP_GET_BOOT_STATUS = 0x03,
};

/*
 * DSM Function Index's as defined by NFIT 0.8s10
 */
enum dsm_function_index
{
	DSM5_GET_SMART = 1,
	DSM5_GET_PCD_AREA_SIZE = 2,
	DSM5_GET_PCD_AREA_DATA = 3,
	DSM5_SET_PCD_AREA = 4,
	DSM5_VENDOR_SPECIFIC = 5,
	DSM5_QUERY_ARS_CAPABILITIES = 6,
	DSM5_START_ARS = 7,
	DSM5_QUERY_ARS_STATUS = 8,
	DSM5_GET_SMART_THRESHOLD = 9,
};
/*
 * DSM Function Index's as defined by ACPI 6.0 and DSM 0.7
 */
enum dsm_root_function_index
{
	DSM_QUERY_ARS_CAPABILITIES = 1,
	DSM_START_ARS = 2,
	DSM_QUERY_ARS_STATUS = 3,
};

enum dsm_device_function_index
{
	DSM_GET_SMART = 1,
	DSM_GET_SMART_THRESHOLD = 2,
	DSM_GET_BLOCK_FLAGS = 3,
	DSM_GET_LABEL_AREA_SIZE = 4,
	DSM_GET_LABEL_AREA_DATA = 5,
	DSM_SET_LABEL_AREA = 6,
	DSM_GET_VENDOR_COMMAND_EFFECT_LOG_SIZE = 7,
	DSM_GET_VENDOR_COMMAND_EFFECT_LOG = 8,
	DSM_VENDOR_SPECIFIC = 9,
};

/*
 * Error codes for the vendor specific DSM command
 */
enum dsm_vendor_error {
	DSM_VENDOR_SUCCESS = 0x0000,
	DSM_VENDOR_ERR_NOT_SUPPORTED = 0x0001,
	DSM_VENDOR_ERR_NONEXISTING = 0x0002,
	DSM_VENDOR_INVALID_INPUT = 0x0003,
	DSM_VENDOR_HW_ERR = 0x0004,
	DSM_VENDOR_RETRY_SUGGESTED = 0x0005,
	DSM_VENDOR_UNKNOWN = 0x0006,
	DSM_VENDOR_SPECIFIC_ERR = 0x0007,
};

/*
 * Error codes common for emulated BIOS vendor specific DSM
 */
enum bios_vendor_error {
	BIOS_EMU_SUCCESS = 0x00,
	BIOS_EMU_ERR_COMMAND_NOT_SUPPORTED = 0x01,
};

/*
 * Error codes specific to the READ/WRITE Large Input/Output Payload BIOS DSM
 */
enum bios_large_payload_error {
	BIOS_EMU_PAYLOAD_INVALID_OFFSET = 0x80,
	BIOS_EMU_PAYLOAD_INVALID_NUM_BYTES = 0x81,
};


enum identify_dimm_sku_bits
{
	SKU_MEMORY_MODE_ENABLED = 1 << 0,
	SKU_STORAGE_MODE_ENABLED = 1 << 1,
	SKU_APP_DIRECT_MODE_ENABLED = 1 << 2,
	SKU_DIE_SPARING_ENABLED = 1 << 3
};

/* Security Status */
enum security_status {
	SEC_RSVD		= 1 << 0,
	SEC_ENABLED		= 1 << 1,
	SEC_LOCKED		= 1 << 2,
	SEC_FROZEN		= 1 << 3,
	SEC_COUNT_EXP	= 1 << 4,
	SEC_NOT_SUPPORTED	= 1 << 5,
};

/* Sanitize Status */
enum sanitize_status {
	SAN_IDLE		= 0,
	SAN_INPROGRESS	= 1,
	SAN_COMPLETED	= 2
};

/* Alarm Thresholds Enabled */
enum alarm_thresholds_enabled
{
	THRESHOLD_ENABLED_SPARE = 1 << 0,
	THRESHOLD_ENABLED_MEDIA_TEMP = 1 << 1,
	THRESHOLD_ENABLED_CONTROLLER_TEMP = 1 << 2,

};

/* alarm trip bits */
enum alarm_trip_bits
{
	SPARE_BLOCKS_TRIP_BIT = 1 << 0,
	MEDIA_TEMPERATURE_TRIP_BIT = 1 << 1,
	CONTROLLER_TEMP_TRIP_BIT = 1 << 2
};

enum validation_flags
{
	SMART_VALIDATION_FLAG_HEALTH_STATUS = 1 << 0,
	SMART_VALIDATION_FLAG_SPARE_BLOCK = 1 << 1,
	SMART_VALIDATION_FLAG_PERCENT_USED = 1 << 2,
	SMART_VALIDATION_FLAG_MEDIA_TEMPERATURE = 1 << 3,
	SMART_VALIDATION_FLAG_CONTROLLER_TEMPERATURE = 1 << 4,
	SMART_VALIDATION_FLAG_ALARM_TRIPS = 1 << 9,
	SMART_VALIDATION_FLAG_LSS = 1 << 10,
	SMART_VALIDATION_FLAG_VENDOR_DATA = 1 << 11,
};

/*
 * Last shutdown status for a DIMM
 */
enum last_shutdown_status
{
	LSS_PM_ADR_COMMAND = 1 << 0,
	LSS_PM_S3 = 1 << 1,
	LSS_PM_S5 = 1 << 2,
	LSS_DDRT_POWER_FAIL_COMMAND = 1 << 3,
	LSS_PMIC_12V_POWER_FAIL = 1 << 4,
	LSS_PM_WARM_RESET = 1 << 5,
	LSS_THERMAL_SHUTDOWN = 1 << 6,
	LSS_FW_FLUSH_COMPLETE = 1 << 7 // this denotes a proper clean shutdown
};

/*
 * Values for the health_status field of pt_payload_smart_health struct
 */
enum smart_health_status
{
	SMART_NORMAL = 0,
	SMART_NON_CRITICAL = 1 << 0,
	SMART_CRITICAL = 1 << 1,
	SMART_FATAL = 1 << 2,
};

/*
 * Values for the firmware type field of pt_payload_fw_image_info struct
 */
enum firmware_type
{
	FW_TYPE_PRODUCTION = 0x29,
	FW_TYPE_DFX = 0x30,
	FW_TYPE_DEBUG = 0x32,
};

/*
 * Mail Box error codes
 *
 * These are returned from the device driver when it completes a pass through
 * command with an error state from the firmware.
 */
enum mb_error {
	MB_SUCCESS = 0x00, /* Command was successfully completed */
	/* An input parameter was not valid */
		MB_INVALID_CMD_PARAM = 0x01,
	/* There was an error in the data transfer */
		MB_DATA_XFER_ERR = 0x02,
	/* There was an internal device error */
		MB_INTERNAL_DEV_ERR = 0x03,
	/* The command opcode or subopcode was not recognized */
		MB_UNSUPPORTED_CMD = 0x04,
	/* Device is currently busy processing a long operation */
		MB_DEVICE_BUSY = 0x05,
	/* Passphrase or Security Nonce does is not acceptable */
		MB_INVALID_CREDENTIAL = 0x06,
	/* The Security CHeck on the image has failed verification */
		MB_SECURITY_CHK_FAIL = 0x07,
	/* Operation is valid in the current security state */
		MB_INVALID_SECURITY_STATE = 0x08,
	/* The system time has not been set yet */
		MB_SYSTEM_TIME_NOT_SET = 0x09,
	/* Returned if "get data" is called before "set data" */
		MB_DATA_NOT_SET = 0x0A,
	/* Command has been aborted. A long operation command has aborted. */
		MB_ABORTED = 0x0B,
	/* Execute FW was called prior to uploading new FW image. */
		MB_NO_NEW_FW = 0x0C,
	/* Illegal rollback failure. */
		MB_REVISION_FAILURE = 0x0D,
	/* Error injection is not currently enabled on the device. */
		MB_INJECTION_DISABLED = 0x0E,
	/* During configuration lockdown commands that modify config will return this error */
		MB_CONFIG_LOCKED_COMMAND_INVALID = 0x0F,
	/* Invalid Paramter Alignment */
		MB_INVALID_ALIGNMENT = 0x10,
	/* Command is not legally allowed in this mode of the dimm */
		MB_INCOMPATIBLE_DIMM = 0x11,
	/* FW timed out on HW components returning in a timely manner */
		MB_TIMED_OUT = 0x12
};

/*
 * Defines the Firmware Command Table opcodes accessed via the
 * IOCTL_PASSTHROUGH_CMD
 */
enum passthrough_opcode {
	/* NULL command, sub-op 00h always generates err */
		PT_NULL_COMMAND = 0x00,
	/* Retrieve physical inventory data for a DIMM */
		PT_IDENTIFY_DIMM = 0x01,
	/* Retrieve security information from a DIMM */
		PT_GET_SEC_INFO = 0x02,
	/* Send a security related command to a DIMM */
		PT_SET_SEC_INFO = 0x03,
	/* Retrieve modifiable settings for a DIMM */
		PT_GET_FEATURES = 0x04,
	PT_SET_FEATURES = 0x05, /* Modify settings for DIMM */
		PT_GET_ADMIN_FEATURES = 0x06, /* Get admin features */
		PT_SET_ADMIN_FEATURES = 0x07, /* Change admin features */
	/* Retrieve administrative data, error info, other FW data */
		PT_GET_LOG = 0x08,
	PT_UPDATE_FW = 0x09, /* Move an image to the DIMM */
	/* Debug only CMD to trigger error conditions */
		PT_INJECT_ERROR = 0x0A,
	PT_RESERVED_1 = 0x0B,
	PT_RESERVED_2 = 0x0C,
	PT_RESERVED_3 = 0x0D,
	/* Debug only command to get debug features */
		PT_GET_DBG_FEATURES = 0xE2,
	/* Debug only command to set debug features */
		PT_SET_DBG_FEATURES = 0xE3,
};

/*
 * Defines the Sub-Opcodes for PT_IDENTIFY_DIMM
 */
enum identify_dimm_subop {
	SUBOP_IDENTIFY_DIMM_IDENTIFY = 0x0,
	SUBOP_IDENTIFY_DIMM_CHARACTERISTICS = 0x1

};

/*
 * Defines the Sub-Opcodes for PT_GET_SEC_INFO
 */
enum get_sec_info_subop {
	SUBOP_GET_SEC_STATE = 0x00, /* Returns the DIMM security state */
		SUBOP_GET_SAN_STATE = 0x01 /* Returns the DIMM sanitize status */
};

/*
 * Defines the Sub-Opcodes for PT_SET_SEC_INFO
 */
enum set_sec_info_subop {
	/* Changes the security administrator passphrase */
		SUBOP_SET_PASS = 0xF1,
	/* Disables the current password on a drive*/
		SUBOP_DISABLE_PASS = 0xF2,
	/* Unlocks the persistent region */
		SUBOP_UNLOCK_UNIT = 0xF3,
	/* Second cmd in erase sequence */
		SUBOP_SEC_ERASE_UNIT = 0xF5,
	/* Prevents changes to all security states until reset */
		SUBOP_SEC_FREEZE_LOCK = 0xF6
};

/*
 * Defines the Sub-Opcodes for PT_GET_FEATURES & PT_SET_FEATURES
 */
enum get_set_feat_subop {
	SUBOP_ALARM_THRESHOLDS = 0x01, /* TODO Get Alarm Thresholds. */
		SUBOP_POLICY_POW_MGMT = 0x02, /* TODO Power management & throttling. */
	/* TODO Action policy to extend DIMM life. */
		SUBOP_POLICY_DIE_SPARING = 0x03,
	SUBOP_POLICY_ADDRESS_RANGE_SCRUB = 0x04,
	SUBOP_DDRT_ALERTS = 0x05, /*Alerts to notify user*/
		SUBOP_OPT_CONFIG_DATA_POLICY = 0x06,
};

/*
 * Defines the Sub-Opcodes for PT_GET_ADMIN_FEATURES &
 * PT_SET_ADMIN_FEATURES
 */
enum get_set_admin_feat_subop {
	SUBOP_SYSTEM_TIME = 0x00, /* TODO Get/Set System Time */
	/* Get/Set Platform Config Data (PCD) */
		SUBOP_PLATFORM_DATA_INFO = 0x01,
	/* Set/Get DIMM Partition Config */
		SUBOP_DIMM_PARTITION_INFO = 0x02,
	/* Get/Set log level of FW */
		SUBOP_FW_DBG_LOG_LEVEL = 0x03,
	SUBOP_PERSISTENT_PARTITION = 0x04, /* Get/Set info about persistent partition */
		SUBOP_DDRT_IO_INIT_INFO = 0x06, /* Get/Set DDRT Initialization info */
		SUBOP_FW_LOAD_FLAG = 0x07, /* Get/Set the FW Load Flag value */
};

/*
 * Defines the Sub-Opcodes for PT_GET_DBG_FEATURES & PT_SET_DBG_FEATURES
 */
enum get_set_dbg_feat_subop {
	SUBOP_CSR = 0x00, /* TODO */
		SUBOP_ERR_POLICY = 0x01,
	SUBOP_THERMAL_POLICY = 0x02,
	SUBOP_MEDIA_TRAIN_DATA = 0x03,
};

/*
 * Defines the Sub-Opcodes for PT_INJECT_ERROR
 */
enum inject_error_subop {
	SUBOP_ENABLE_INJECTION = 0x00, /* Allows for errors to be injected */
	SUBOP_ERROR_POISON = 0x01, /* Sets poison bit on a DPA */
	SUBOP_ERROR_TEMP = 0x02, /* Injects a particular temperature to cause temperature error */
	SUBOP_ERROR_SW_TRIGGERS = 0x03, /* SW override triggers to trip various SW alarms */
};

/*
 * Defines the Sub-Opcodes for PT_GET_LOG
 */
enum get_log_subop {
	/* Retrieves various SMART & Health information. */
		SUBOP_SMART_HEALTH = 0x00,
	SUBOP_FW_IMAGE_INFO = 0x01, /* Retrieves firmware image information. */
		SUBOP_FW_DBG_LOG = 0x02, /* TODO Retrieves the binary firmware log. */
		SUBOP_MEM_INFO = 0x03, /* TODO*/
	/* TODO Status of any pending long operations. */
		SUBOP_LONG_OPERATION_STAT = 0x04,
	SUBOP_ERROR_LOG = 0x05, /* Retrieves firmware error log */
};

/*
 * Defines the Sub-Opcodes for PT_UPDATE_FW
 */
enum update_fw_subop {
	SUBOP_UPDATE_FW = 0x00, /* Update the running FW */
	/* Force the execution of a newly updated FW image. */
		SUBOP_EXECUTE_FW = 0x01
};

/*
 * ****************************************************************************
 * STRUCTURES
 * ****************************************************************************
 */

/*
 * The struct defining the passthrough command and payloads to be operated
 * upon by the firmware.
 */
struct fw_cmd {
	unsigned int device_handle; /* The device handle reported by the nfit */
	unsigned char opcode; /* The command opcode. */
	unsigned char sub_opcode; /* The command sub-opcode. */
	unsigned int input_payload_size; /* The size of the input payload */
	void *input_payload; /* A pointer to the input payload buffer */
	unsigned int output_payload_size; /* The size of the output payload */
	void *output_payload; /* A pointer to the output payload buffer */
	unsigned int large_input_payload_size; /* Size large input payload */
	void *large_input_payload; /* A pointer to the large input buffer */
	unsigned int large_output_payload_size;/* Size large output payload */
	void *large_output_payload; /* A pointer to the large output buffer */
};

/*
 * ****************************************************************************
 * Payloads for passthrough fw commands
 * ****************************************************************************
 */

/*
 * Passthrough Payload:
 *		Opcode: 0x01h (Identify DIMM)
 *	Small Output Payload
 * * Updated to FIS 0.81*
 */
struct pt_payload_identify_dimm {
	unsigned short vendor_id;
	unsigned short device_id;
	unsigned short revision_id;
	unsigned short ifc; /* Interface format code */
	unsigned char fwr[DEV_FW_REV_LEN]; /* BCD formated firmware revision */
	unsigned char api_ver; /* BCD formated api version */
	unsigned char fswr; /* Feature SW Required Mask */
	unsigned char rsrvd_a;
	unsigned short nbw; /* Number of block windows */
	unsigned short nwfa; /* Number write flush addresses */
	unsigned long long wfas; /* Start address of the write flush addresses */
	unsigned int obmcr; /* Offset of block mode control region */
	unsigned int rc; /* raw capacity in 4KB multiples*/
	unsigned char mf[DEV_MFR_LEN]; /* ASCII Manufacturer */
	unsigned char sn[DEV_SN_LEN]; /* ASCII Serial Number */
	char mn[DEV_MODELNUM_LEN]; /* ASCII Model Number */
	unsigned int dimm_sku;
	unsigned char resrvd[62]; /* Reserved */
} __attribute__((packed));

/*
 * Passthrough Payload:
 * 		Opcode: 0x01h (Identify DIMM)
 * 		Sub-opcode: 0x01h (Device Characteristics)
 * 	* Updated to FIS 1.2 *
 */
struct pt_payload_device_characteristics {
	unsigned short controller_temp_shutdown_threshold;
	unsigned short media_temp_shutdown_threshold;
	unsigned short throttling_start_threshold;
	unsigned short throttling_stop_threshold;
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x02h (Get Security Info)
 *		Sub-Opcode:	0x00h (Get Security State)
 *	Small Output Payload
 */
struct pt_payload_get_security_state {
	/*
	 * Bit0: reserved
	 * Bit1: Security Enabled (E)
	 * Bit2: Security Locked (L)
	 * Bit3: Security Frozen (F)
	 * Bit4: Security Count Expired (C)
	 * Bit5: Security Disabled
	 * Bit6: reserved
	 * Bit7: reserved
	 */
	unsigned char security_status;

	unsigned char reserved[7]; /* TODO */
} __attribute__((packed));

struct pt_payload_sanitize_dimm_status {
	unsigned char state;
	unsigned char progress;
	unsigned char reserved[126];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x02h (Get Security Info)
 *		Sub-Opcode:	0x01h (Get Sanitize State)
 *	Small Output Payload
 */
struct pt_payload_get_sanitize_state {
	/*
	 * 0x00 = idle
	 * 0x01 = in progress
	 * 0x02 = completed
	 * 0x03-0xff - Reserved
	 */
	unsigned char sanitize_status;
	/*
	 * Percent complete the DIMM has been sanitized so far, 0-100
	 */
	unsigned char sanitize_progress;
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x03h (Set Security Info)
 *		Sub-Opcode:	0xF1h (Set Passphrase)
 *	Small Input Payload
 */
struct pt_payload_set_passphrase {
	/* The current security passphrase */
	char passphrase_current[DEV_PASSPHRASE_LEN];
	unsigned char rsvd1[32];
	/* The new passphrase to be set/changed to */
	char passphrase_new[DEV_PASSPHRASE_LEN];
	unsigned char rsvd2[32];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x03h (Set Security Info)
 *		Sub-Opcode:	0xF2h (Disable Passphrase)
 *		Sub-Opcode:	0xF3h (Unlock Unit)
 *	Small Input Payload
 */
struct pt_payload_passphrase {
	/* The end user passphrase */
	char passphrase_current[DEV_PASSPHRASE_LEN];
	unsigned char rsvd[96];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *             Opcode:         0x03h (Set Security Info)
 *             Sub-Opcode:     0x01h (Overwrite DIMM)
 *     Small Input Payload
 */
struct pt_payload_secure_erase_unit {
	/* the end user passphrase */
	char passphrase_current[DEV_PASSPHRASE_LEN];
	unsigned char invert_pattern_flag;
	char reserved[3];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x04/0x05h (Get/Set Features)
 *		Sub-Opcode:	0x01h (Alarm Thresholds)
 *	Get - Small Output Payload
 *	Set - Small Input Payload
 */
struct pt_payload_alarm_thresholds {

	/*
	 * Used to enable/disable alarms. If the alarms are not enabled,
	 * even depending on the thresholds set, no notification will be sent.
	 * Bit
	 * 0 - Spare Block
	 * 1 - Media Temperature
	 * 2 - Controller Temperature
	 * 16:3 RSVD
	 *
	 * 0	Disabled
	 * 1	Enabled
	 */
	unsigned short enable;

	/*
	 * When spare levels fall below this percentage based value,
	 * asynchronous events may be triggered and may cause a
	 * transition in the overall health state
	 * Default 50
	 */
	unsigned char spare;

	/*
	 * Temperatures (in Celsius) above this threshold trigger asynchronous
	 * events and may cause a transition in the overall health state
	 * Default 0x540 = 84 C
	 *
	 * Bit
	 * 14-0	: Temperature in Celsius with 0.0625 C resolution
	 * 15	: Sign Bit(1 = Negative 0 = Positive)
	 */
	unsigned short media_temperature;

	/*
	 * Temperatures (in Celsius) above this threshold trigger asynchronous
	 * events and may cause a transition in the overall health state
	 * Default 0x680 = 104 C
	 *
	 * Bit
	 * 14-0	: Temperature in Celsius with 0.0625 C resolution
	 * 15	: Sign Bit(1 = Negative 0 = Positive)
	 */
	unsigned short controller_temperature;

	unsigned char reserved[121];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x04/0x05h (Get/Set Features)
 *		Sub-Opcode:	0x02h (Power Management Policy)
 *	Get - Small Output Payload
 *	Set - Small Input Payload
 */
struct pt_payload_power_mgmt_policy {
	/*
	 *	Reflects whether the power management policy is enabled or disabled.
	 *
	 *	Value
	 *	0x00 - Disabled
	 *	0x01 - Enabled (Default)
	 */
	unsigned char enabled;
	/* Shows the current TDP DIMM power limit in W (watts) */
	unsigned char tdp;
	/* Power budget in mW used in instantaneous power. */
	unsigned short peak_power_budget;
	/* Power budget in mW used for averaged power */
	unsigned short average_power_budget;
	unsigned char rsvd[122];
} __attribute__((packed));
/*
 * Passthrough Payload:
 *		Opcode:		0x04h (Get Features)
 *		Sub-Opcode:	0x03h (Die Sparing Policy)
 *	Small Output Payload
 */
struct pt_get_die_spare_policy {
	/*
	 * Reflects whether the die sparing policy is enabled or disabled
	 * 0x00 - Disabled
	 * 0x01 - Enabled
	 */
	unsigned char enable;
	/* How aggressive to be on die sparing (0...255), Default 128 */
	unsigned char aggressiveness;

	/*
	 * Bit:
	 * 0 - Rank 0 Spare
	 * 1 - Rank 1 Spare
	 * 2 - Rank 2 Spare
	 * 3 - Rank 3 Spare
	 * 7:4 - RSVD
	 *
	 * Designates whether or not each rank of the DIMM still supports die sparing.
	 * 0x00 - No longer available - die sparing has most likely occurred
	 * 0x01 - Still available - the dimm's corresponding rank has not had to take action yet
	 */
	unsigned char supported;
	unsigned char rsvd[125];

} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x04h/0x05h (Get/Set Features)
 *		Sub-Opcode:	0x05h (DDRT Alerts)
 *	Get - Small output payload
 *	Set - Small input payload
 */
struct pt_ddrt_alert {
	/*
	 * Each byte represents a transaction type 0..127
	 * Bit 1:0 Log/Alert Level: Specifies which log should receive this
	 * transaction type. Also indicates which Interrupt Packet to use for
	 * Interrupts if enabled.
	 * 0h = Do Not Log
	 * 1h = Low Priority Log
	 * 2h = High Priority Log
	 *
	 * Bit 2:7 Alert Policy Enables: Bitfields to enable signaling policy
	 * for this transaction type for different error types. Also allows
	 * configurability to signal Viral on this transaction/error type.
	 *
	 * (7) = UNC ERROR INTERRUPT ENABLE
	 * (6) = UNC ERROR VIRAL ENABLE
	 * (5) = DPA ERROR INTERRUPT ENABLE
	 * (4) = DPA ERROR VIRAL ENABLE
	 * (3) = AIT ERROR INTERRUPT ENABLE
	 * (2) = AIT ERROR VIRAL ENABLE
	 */
	unsigned char transaction_type[DEV_DDRT_ALERT_TYPES];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x04h (Get Features)
 *		Sub-Opcode:	0x06h (Optional Configuration Data Policy)
 *	Get - Small Output Payload
 */
struct pt_payload_get_config_data_policy {
	/*
	 * Current state of acceleration of the first refresh cycle
	 * 0x00 - Disabled (Default)
	 * 0x01 - Enabled
	 */
	unsigned char first_fast_refresh;

	/*
	 * Current state of Viral Policies of the NVM DIMM
	 * 0x00 - Disable (default)
	 * 0x01 - Enable
	 */
	unsigned char viral_policy_enable;

	/*
	 * Current Viral status of the NVM DIMM
	 * 0x00 - Not viral
	 * 0x01 - Viral
	 */
	unsigned char viral_status;
	unsigned char rsvd[125];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x05h (Set Features)
 *		Sub-Opcode:	0x06h (Optional Configuration Data Policy)
 *	Set - Small Input Payload
 */
struct pt_payload_set_config_data_policy {
	/*
	 * Enable/disable acceleration of the first refresh cycle
	 * 0x00 - Disabled (Default)
	 * 0x01 - Enabled
	 */
	unsigned char first_fast_refresh;

	/*
	 *  Enable/disable the Viral Policies of the NVM DIMM
	 *  0x00 - Disable (default)
	 *  0x01 - Enable
	 */
	unsigned char viral_policy_enable;

	/*
	 * Clear the viral status of the NVM DIMM
	 * 0x00 - Do Not Clear
	 * 0x01 - Clear Viral
	 */
	unsigned char viral_clear;
	unsigned char rsvd[125];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x05h (Set Features)
 *		Sub-Opcode:	0x04h (Die Sparing Policy)
 *	Small Input Payload
 */
struct pt_set_die_spare_policy {
	/*
	 * Reflects whether the die sparing policy is enabled or disabled
	 * 0x00 - Disabled
	 * 0x01 - Enabled
	 */
	unsigned char enable;
	/* How aggressive to be on die sparing (0...255), Default 128 */
	unsigned char aggressiveness;
	unsigned char rsvd[126];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x06h/0x07h (Get/Set Admin Features)
 *		Sub-Opcode:	0x00h (System Time)
 *	Get - Small Output Payload
 *	Set - Small Input Payload
 */
struct pt_payload_system_time {
	/*
	 * The number of seconds since 1 January 1970
	 */
	unsigned long long time;

	unsigned char rsvd[120];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x06h (Get Admin Features)
 *		Sub-Opcode:	0x01h (Platform Config Data)
 *	Get - Small Input Payload + Large Ouput Payload
 */
struct pt_payload_get_platform_cfg_data {
	/*
	 * Which Partition to access
	 * 0x00 - BIOS
	 * 0x01 - OEM
	 * 0x02 - Namespace Label Storage Area
	 * All other values are reserved
	 */
	unsigned char partition_id;

	/*
	 * Command Options
	 * Bits 0:0 - Payload Type
	 * 	0 - Large payload (Default)
	 * 	1 - Small payload
	 * Bits 1:1 - Retrieve Options
	 *  0 - Partition Data (Default)
	 *  1 - Partition Size
	 * Bits 7:2 - Reserved
	 */
	unsigned char options;

	/*
	 * Offset (Small payload Type only)
	 * Offset in bytes of the partition to start reading from.
	 */
	unsigned int offset;

	unsigned char reserved[122];

} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x07h (Set Admin Features)
 *		Sub-Opcode:	0x01h (Platform Config Data)
  *	Set - Small Input Payload + Large Input Payload
 */
struct pt_payload_set_platform_cfg_data {
	/*
	 * Which Partition to access
	 * 0x00 - BIOS
	 * 0x01 - OEM
	 * 0x02 - Namespace Label Storage Area
	 * All other values are reserved
	 */
	unsigned char partition_id;

	/*
	 * The payload area to use for data transfer
	 * 	0 - Large payload (Default)
	 * 	1 - Small payload
	 */
	unsigned char payload_type;

	/* Offset (Small payload only)
	 * Offset in bytes of the partition to start reading from.
	 */
	unsigned int offset;

	unsigned char reserved[58];

	/*
	 * Data (Small payload only)
	 * The data to write.
	 */
	unsigned char data[DEV_PLT_CFG_SMALL_PAYLOAD_WRITE_SIZE];

} __attribute__((packed));

/*
 * Passthrough CR Payload:
 *		Opcode:		0x06h (Get Admin Features)
 *		Sub-Opcode:	0x02h (DIMM Partition Info)
 *	Small Output Payload
 */
struct pt_payload_get_dimm_partition_info {
	/*
	 * DIMM volatile memory capacity (in 4KB multiples of bytes).
	 * Special Values:
	 * 0x0000000000000000 - No volatile capacity
	 */
	unsigned int volatile_capacity;
	unsigned char rsvd1[4];

	/* The DPA start address of the volatile region */
	unsigned long long start_volatile;

	/*
	 * DIMM PMEM capacity (in 4KB multiples of bytes).
	 * Special Values:
	 * 0x0000000000000000 - No persistent capacity
	 */
	unsigned int pmem_capacity;
	unsigned char rsvd2[4];

	/* The DPA start address of the PMEM region */
	unsigned long long start_pmem;

	/* The raw usable size of the DIMM (Volatile + PMEM)
	 * (in 4KB multiples of bytes) */
	unsigned int raw_capacity;
	unsigned char rsvd3[92];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x06h (Get Admin Features)
 *		Sub-Opcode:	0x03h (FW Debug Log Level)
 *	Small Input Payload
 */
struct pt_payload_input_get_fw_dbg_log_level {
	/*
	 * This is the ID(0 to 255) for the log from which to retrieve the log
	 * level. If no log ID is sent then log ID 0 will be assumed.
	 */
	unsigned char log_id;
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x06h (Get Admin Features)
 *		Sub-Opcode:	0x03h (FW Debug Log Level)
 *	Small Output Payload
 */
struct pt_payload_output_get_fw_dbg_log_level {
	/*
	 * The current logging level of the  FW (0-255).
	 *
	 * 0 = Disabled
	 * 1 = Error
	 * 2 = Warning
	 * 3 = Info
	 * 4 = Debug
	 */
	unsigned char log_level;
	/*
	 * The number of logs available for which to change the level(0 to 255).
	 */
	unsigned char logs;
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x06/0x07h (Get/Set Admin Features)
 *		Sub-Opcode:	0x04h (Persistent Partition)
 *	Get - Small Output Payload
 *	Set - Small Input Payload
 */
struct pt_payload_persistent_partition {

	/*
	 * The current state of the persistent partition
	 * 0x0 = Disabled (default)
	 * 0x1 = Enabled
	 */
	unsigned char enabled;
	unsigned char rsvd[127];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x06/0x07h (Get/Set Admin Features)
 *		Sub-Opcode:	0x06h (DDRT IO Init Info)
 *	Small Output Payload
 */
struct pt_payload_ddrt_init_info {

	/*
	 * Information required to configure DDRT
	 * Bit 3:0 - Operating Frequency (default 0x0)
	 * Valid Values:
	 * x000 - f < 1600MT/s
	 * x001 - 1600 MT/s < f < 1867 MT/s
	 * x010 - 1867 MT/s < f < 2134 MT/s
	 * x011 - 2134 MT/s < f < 2400 MT/s
	 * x100 - 2400 MT/s < f < 2667 MT/s
	 * x101 - 2667 MT/s < f < 3200 MT/s
	 * x110 - Reserved
	 * x111 - Reserved
	 * 0xxx - Reserved
	 * 1xxx - Reserved
	 *
	 * Bit 4 - VDDQ: Encoding for DDRT Voltage
	 * Valid Values
	 * 0 - 1.2V (Default)
	 * 1 - Reserved for low voltage
	 *
	 * Bit 5 - Write Preamble: DDRT Mode Register for Write Preamble.
	 * 0 - 1 nCk (default)
	 * 1 - 2 nCk
	 *
	 * Bit 6 - Read Preamble: DDRT Mode Register for Read Preamble.
	 * 0 - 1 nCk (default)
	 * 1 - 2 nCk
	 *
	 * Bit 7 Reserved
	 */
	unsigned char ddrt_io_info;
	unsigned char rsvd[127];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *	Opcode:		0x06/0x07h (Get/Set Admin Features)
 *	Sub-Opcode:	0x07h (FW Load Flag)
 *	Small Output Payload
 */
struct pt_payload_fw_load_flag {
	/*
	 * Type of FW load that is requested.
	 * 0x00 - Default FW (Default Value)
	 * 0x01 - Debug FW
	 * 0x02 - Mfg FW
	 */
	unsigned char load_flag;
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x07h (Set Admin Features)
 *		Sub-Opcode:	0x02h (DIMM Partition Info)
 *	Small Input Payload
 */
struct pt_payload_set_dimm_partion_info {
	/*
	 * DIMM volatile memory capacity (in 4KB multiples of bytes).
	 * Special Values:
	 * 0x0000000000000000 - No volatile capacity
	 */
	unsigned int volatile_capacity;

	/*
	 * DIMM PMEM capacity (in 4KB multiples of bytes).
	 *
	 * Special Values:
	 * 0x0000000000000000 - No persistent capacity
	 */
	unsigned int pmem_capacity;

	unsigned char rsvd[120];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *	Opcode:		0x07h (Set Admin Features)
 *	Sub-Opcode:	0x03h (FW Debug Log Level)
 *	Small Payload Input
 */
struct pt_payload_set_fw_dbg_log_level {
	/*
	 * The current logging level of the FW (0-255).
	 *
	 * 0 = Disabled
	 * 1 = Error
	 * 2 = Warning
	 * 3 = Info
	 * 4 = Debug
	 */
	unsigned char log_level;
	/*ID of the log you are changing the level for(0-255)*/
	unsigned char log_id;
} __attribute__((packed));

/*
 * Passthrough Input Payload:
 *		Opcode:		0x0E2h (Get Debug Features)
 *		Sub-Opcode:	0x00h (Read CSR)
 *	Small Input Payload
 */
struct pt_input_payload_read_csr {
	/* The address of the CSR register to read */
	unsigned int csr_register_address;
} __attribute__((packed));

/*
 * Passthrough Output Payload:
 *		Opcode:		0x0E2h (Get Debug Features)
 *		Sub-Opcode:	0x00h (Read CSR)
 *
 *		Opcode:		0x0E3h (Set Debug Features)
 *		Sub-Opcode:	0x00h (Write CSR)
 *	Small Output Payload
 */
struct pt_output_payload_csr {
	unsigned int csr_value; /* The value of the CSR register */
} __attribute__((packed));

/*
 * Passthrough Payload
 *		Opcode:		0x0E2h/0x0E3h (Get/Set Debug Features)
 *		Sub-Opcode:	0x01h (Error Correction/Erasure Policy)
 *	Get - Small Output Payload
 *	Set - Small Input Payload
 */
struct pt_payload_err_correct_erasure_policy {
	/**
	 * Which policy to retrieve the policy values for.
	 * Bit 0 - Error Correction Policy
	 * Bit 1 - Erasure Coding Policy
	 */
	unsigned char policy_selection;

	/**
	 * The values for the selected policy
	 *
	 * Bit 1 - Forced Write on Unrefreshed Lines
	 * Bit 2 - Forced Write on Refreshed Lines
	 * Bit 3 - Unrefreshed Enabled
	 * Bit 4 - Refreshed Enabled
	 */
	unsigned char policy;
} __attribute__((packed));

/**
 * Passthrough Payload:
 *		Opcode:		0x0E2h (Get Debug Features)
 *		Sub-Opcode:	0x02h (Thermal Policy)
 *	Small output payload
 *		Opcode:		0x0E3h (Set Debug Features)
 *		Sub-Opcode:	0x02h (Thermal Policy)
 *	Small input payload
 */
struct pt_payload_thermal_policy {
	/**
	 * Reflects whether the thermal policy is enabled or disabled
	 * Bit
	 * 0 - Thermal Throttling:
	 * Specifies whether or not throttling should be enabled or disabled.
	 * 1 - Thermal Alerting:
	 * Specifies whether or not to contine with thermal alerts
	 * 2 - Critical Shutdown Action:
	 * Specifies whether or not to take critical shutdown actions
	 * 7:3 RSVD
	 */
	unsigned char enable_flags;
} __attribute__((packed));

/*
 * Passthrough Input Payload:
 *		Opcode:		0xE3h (Set Debug Features)
 *		Sub-Opcode:	0x00h (Write CSR)
 *	Small Input Payload
 */
struct pt_input_payload_write_csr {
	unsigned int csr_register_address; /* The address of the CSR register */
	unsigned int csr_value; /* The value to be written to the CSR register */
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x0Ah (Inject Error)
 *		Sub-Opcode:	0x00h (Enable Injection)
 *	Small Input Payload
 */
struct pt_payload_enable_injection {
	/*
	 * Used to turn off/on injection functionality
	 * 0x00h - Off ( default)
	 * 0x01h - On
	 */
	unsigned char enable;
	unsigned char reserved[127];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x0Ah (Inject Error)
 *		Sub-Opcode:	0x01h (Poison Error)
 *	Small Input Payload
 */
struct pt_payload_poison_err {
	/*
	 * Allows the enabling or disabling of poison for this address
	 * 0x00h - Clear
	 * 0x01h - Set
	 */
	unsigned char enable;
	unsigned char reserved_a[3];
	unsigned long long dpa_address; /* Address to set the poison bit for */
	unsigned char reserved_b[116];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x0Ah (Inject Error)
 *		Sub-Opcode:	0x02h (Media Temperature Error)
 */
struct pt_payload_temp_err {
	/*
		 * Allows the enabling or disabling of the temperature error
		 * 0x00h - Off (default)
		 * 0x01h - On
		 */
	unsigned char enable;
	/*
	 * A number representing the temperature (Celsius) to inject
	 * Bit 14-0: Temperature in Celsius with 0.0625 C resolution
	 * Bit 15: Sign Bit ( 1 = negative, 0 = positive)
	 */
	unsigned short temperature;
	unsigned char reserved[125];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x0Ah (Inject Error)
 *		Sub-Opcode:	0x03h (Software Triggers)
 */
struct pt_payload_sw_triggers {
	/*
	 * Spoofs FW to initiate a Die Sparing.
	 * Will trigger all ranks to spare.
	 * 0x0h - Do Not/Disable Trigger
	 * 0x1h - Enable Trigger
	 */
	unsigned char die_sparing_trigger;

	unsigned char reserved;

	/*
	 * Spoofs FW to trigger a a spare block trip.
	 * 0x0h - Do Not/Disable Trigger
	 * 0x1h - Enable Trigger
	 */
	unsigned char user_spare_block_alarm_trip_trigger;

	/*
	 * Spoofs FW to trigger a fatal media error.
	 * 0x0h - Do Not/Disable Trigger
	 * 0x1h - Enable Trigger
	 */
	unsigned char fatal_error_trigger;

	unsigned char reserved_1[124];
} __attribute__((packed));

/*
 * If the corresponding validation flag is not set in this field, it is indication to the
 * software that the corresponding field is not valid and must not be interpreted.
 */
typedef union
{
	struct smart_validation_flags
	{
		unsigned int health_status_field:1;
		unsigned int spare_block_field:1;
		unsigned int percentage_used_field:1;
		unsigned int media_temperature_field:1;
		unsigned int controller_temperature_field:1;
		unsigned int rsvd_a:4;
		unsigned int alarm_trips_field:1;
		unsigned int last_shutdown_status_field:1;
		unsigned int sizeof_vendor_data_field:1;
		unsigned int rsvd_b:20;
	} parts;
	unsigned int flags;
} SMART_VALIDATION_FLAGS;

struct intel_smart_vendor_data
{
	unsigned long long power_cycles; /* Number of NVM DIMM power cycles */
	/*
	 * Lifetime hours the DIMM has been powered on. Time is represented in Seconds
	 */
	unsigned long long power_on_seconds;
	unsigned long long uptime;
	unsigned int unsafe_shutdowns;
	unsigned char lss_details;
	unsigned long long last_shutdown_time; /* seconds since 1 January 1970 */
	unsigned char reserved_b[55];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x00h (SMART & Health Info)
 *	Small payload output
 */
struct pt_payload_smart_health {

	SMART_VALIDATION_FLAGS validation_flags;

	unsigned int rsvd_a;

	/*
	 * Overall health summary
	 * Bit meaning if set:
	 * 0 = Non-Critical (maintenance required)
	 * 1 = Critical (features or performance degraded due to failure)
	 * 2 = Fatal (data loss has occurred or is imminent)
	 * 7 - 3 Reserved
	 *
	 * No bit set = Normal Health
	 */
	unsigned char health_status;

	/*
	 * Remaining spare capacity as a percentage of factory configured spare
	 */
	unsigned char spare;

	/*
	 * Device life span as a percentage.
	 * 100 = warranted life span of device has been reached however values
	 * up to 255 can be used.
	 */
	unsigned char percentage_used;

	/*
	 * Alarm trips - Bits to signify whether or not values have tripped their respective alarm thresholds.
	 * Bit 0                : Spare Block Trips
	 * Bit 1                : Media Temperature Trips
	 * Bit 2                : Controller Temperature Trips
	 * Bit 7 - 3    : Reserved
	 */
	unsigned char alarm_trips;

	/*
	 * 	Current media temperature in Celsius
	 * 	bits 14 - 0 = temperature in Celsius
	 * 	bit 15 = sign bit (1 = negative, 0 = positive)
	 */
	unsigned short media_temperature;

	/*
	 *      Current controller temperature in Celsius
	 *      bits 14 - 0 = temperature in Celsius
	 *      bit 15 = sign bit (1 = negative, 0 = positive)
	 */
	unsigned short controller_temperature;

	unsigned char rsvd_b[15];

	/*
	 * Last Shutdown Status: Displays the last shutdown that occured
	 * 00h = Clean Shutdown
	 * FF - 01h = Not clean shutdown
	 */
	unsigned char lss;

	unsigned int vendor_specific_data_size;

	struct intel_smart_vendor_data vendor_data;
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x01h (Firmware Image Info)
 *	Small output payload
 */
struct pt_payload_fw_image_info {
	/*
	 * Contains BCD formatted revision of the FW in the following format:
	 * aa.bb.cc.dddd
	 *
	 * aa = 2 digit Major Version
	 * bb = 2 digit Minor Version
	 * cc = 2 digit Hotfix Version
	 * dddd = 4 digit Build Version
	 */
	unsigned char fw_rev[DEV_FW_REV_LEN];

	/*
	 * Contains value designating FW type:
	 * 		0x29 - production
	 * 		0x30 - dfx
	 * 		0x32 - debug
	 */
	unsigned char fw_type;

	unsigned char rsvd1[10];

	/*
	 * Contains info regarding updated/downloaded/staged FW via update FW cmd:
	 * 		0x00 - no new fw staged
	 * 		0x01 - new fw has been staged for execution
	 * 		0xFF-0x02 - reserved
	 */
	unsigned char staged_fw_status;

	/*
	 * Contains BCD formatted revision of the FW in aa.bb.cc.dddd format
	 */
	unsigned char staged_fw_rev[DEV_FW_REV_LEN];

	/*
	 * Contains value designating the staged FW type
	 */
	unsigned char staged_fw_type;

	unsigned char rsvd2[9];

	/*
	 * Contains commit identifier of the active FW for debug/troubleshooting purposes
	 */
	char commit_id[DEV_FW_COMMIT_ID_LEN];

	/*
	 * Contains the build configuration of the active firmware for debug and troubleshoot purpose
	 */
	char build_configuration[DEV_FW_BUILD_CONFIGURATION_LEN];

	unsigned char rsvd3[40];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x02h (Firmware Debug Log)
 *	Small Input Payload
 */
struct pt_payload_input_get_fw_dbg_log {
#define RETRIEVE_LOG_SIZE 0x00
#define GET_LOG_PAGE 0x01
	/*
	 *  This is used to tell the FW Debug Log command what to do
	 *  0x00 = Retrieve Log Size (default)
	 *  0x01 = Get Log Page
	 */
	unsigned char log_action;
	/*
	 * The 128 byte offset into the log over SMBus or a 1MB offset into the log over DDRT.
	 * ( Not used when retrieving the log size) ( 0 based offset)
	 */
	unsigned char log_page_offset[2];
	/*
	 * This is the ID(0 to 255) for the log from which to retrieve the log
	 * level. If no log ID is sent then log ID 0 will be assumed.
	 */
	unsigned char log_id;
	unsigned char rsvd[122];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x02h (Firmware Debug Log)
 *	Small Output Payload
 */
struct pt_payload_output_get_fw_dbg_log {
	/* The size of the log in MB. */
	unsigned char log_size;
	unsigned char rsvd[127];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x03h (Memory Info)
 *	Small Output Payload
 */
struct pt_payload_input_memory_info {
	/* The page of memory information you want to retrieve */
	unsigned char memory_page;
	unsigned char rsvd[127];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x03h (Memory Info)
 *	Small Output Payload
 */
struct pt_payload_memory_info_page0 {
	unsigned char bytes_read[16]; /* Number of  64 bytes read from the DIMM. */
	unsigned char bytes_written[16]; /* Number of 64 bytes written to the DIMM. */
	unsigned char read_reqs[16]; /* Number of DDRT read transactions DIMM has serviced. */
	unsigned char write_reqs[16]; /* Number of DDRT write transactions DIMM has serviced. */
	unsigned char block_read_reqs[16]; /* Number of BW read requests DIMM has serviced. */
	unsigned char block_write_reqs[16]; /* Number of BW write requests DIMM has serviced. */
	unsigned char reserved[32];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x03h (Memory Info)
 *	Small Output Payload
 */
struct pt_payload_memory_info_page1 {
	unsigned char total_bytes_read[16]; /* Lifetime nu of 64 bytes read from the DIMM */
	unsigned char total_bytes_written[16]; /* Lifetime num of 64 bytes written to the DIMM */
	unsigned char total_read_reqs[16]; /* Lifetime num DDRT read transactions serviced by DIMM*/
	unsigned char total_write_reqs[16]; /* Lifetime num DDRT write transactions serviced by DIMM*/
	unsigned char total_block_read_reqs[16]; /* Lifetime num BW read requests serviced by DIMM */
	unsigned char total_block_write_reqs[16]; /* Lifetime num BW write requests serviced by DIMM */
	unsigned char rsvd[32];

} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x03h (Memory Info)
 *	Small Output Payload
 */
struct pt_payload_memory_info_page2 {
	unsigned long long write_count_max; /* Largest num data writes to a single block across DIMM */
	unsigned long long write_count_average; /* Avg num of data writes to all blocks across DIMM */
	unsigned int uncorrectable_host; /* ECC errors encountered by host requests only */
	unsigned int uncorrectable_non_host; /* ECC errors encountered by non-host requests only */
	unsigned int media_errors_uc; /* ECC uncorrectable errors. */
	unsigned char media_errors_ce[16]; /* ECC corrected errors */
	unsigned char media_errors_ecc[16]; /* Erasure Code Corrected Errors */

	unsigned char rsvd[68];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x04h (Long Operations Status)
 *	Small Output Payload
 */
struct pt_payload_long_op_stat {
	/*
	 * This will coincide with the opcode & sub-opcode
	 * Byte 0 - Opcode
	 * Byte 1 - Sub-Opcode
	 */
	unsigned short command;

	/*
	 * The % complete of the current command
	 * BCD Format = XXX
	 */
	unsigned short percent_complete;

	/*
	 * Estimated Time to Completion.
	 */
	unsigned int etc;

	/*
	 * The status code that would normally be found in
	 * the mailbox status register*/
	unsigned char status_code;

	unsigned char command_specific_data[DEV_COMM_SPECIFIC_DATA];
} __attribute__((packed));

/*
 * return payload:
 *		Opcode:		0x04h (Get Features)
 *		Sub-Opcode:	0x04h (Address Range Scrub)
 */
struct pt_return_address_range_scrub {
	/*
	 * Indicates number of errors found during address range scrub
	 */
	unsigned char num_errors;

	/*
	 * State bits to describe other ARS information
	 * 0x0 = Normal
	 * 0x1 = Ended Early
	 */
	unsigned char ars_state;

	/*
	 * List of 14 DPA addresses (8 bytes each) encountered
	 */
	unsigned long long dpa_error_address[14];

	/*
	 * Reserved
	 */
	unsigned char rsvd[5];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x05h (Get Error Log)
 *	Small Input Payload
 */
struct pt_input_payload_fw_error_log {
	/*
	 * Bit 0 - Log Level
	 * 0b = Low
	 * 1b = High
	 *
	 * Bit 1 - Log Type
	 * 0b = Media Log
	 * 1b = Thermal Log
	 *
	 * Bit 2 - Log Info
	 * 0b = Retrieve Entries
	 * 1b = Retrieve Log Info Data
	 *
	 * Bit 3 - Log Entries Payload Return
	 * Only affects retrieval of log entries
	 * 0b = Small Payload
	 * 1b = Large Payload
	 *
	 * 7:4 RSVD
	 */
	unsigned char params;
	/*
	 * Offset in into log to retrieve.
	 * Reads log from specified entry offset.
	 */
	unsigned char offset;
	/*
	 *  Request Count: Max number of log entries requested for this access.
	 *  ‘0’ can be used to request the count fields without reading the
	 *  log entries.
	 */
	unsigned char request_count;

	unsigned char rsvd[124];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x05h (Get Error Log)
 */
struct pt_fw_media_log_entry {
	/*
	 * Unix Epoch Time of the log entry
	 */
	unsigned long long system_timestamp;

	/*
	 * Specifies the DPA address of the error
	 */
	unsigned long long dpa;

	/*
	 * Specifies the PDA address of the error
	 */
	unsigned long long pda;

	/*
	 * Specifies the length in address space of this error. Ranges will be
	 * encoded as a power of 2. Range is in 2^X bytes. Typically X=8 for
	 * one NGNVM 256B error
	 */
	unsigned char range;

	/*
	 * Indicates what kind of error was logged. Entry includes
	 * error type and Flags.
	 * 		0h = Uncorrectable
	 *		1h = DPA Mismatch
	 *		2h = AIT Error
	 *		3h = Data Path Error
	 *		4h = Locked/Illegal Access
	 *		5h = User Spare BLock Alarm Trip
	 *		6h = Smart Health Status Change
	 *		All other values reserved
	 */
	unsigned char error_type;

	/*
	 * Indicates the error flags for this entry.
	 * Bits
	 * 0 - PDA VALID: Indicates the PDA address is VALID
	 * 1 - DPA VALID: Indicates the DPA address is VALID
	 * 2 - INTERRUPT: Indicates this error generated an interrupt packet
	 * 3 - INJECT: Indicates this was an injected error entry
	 * 4 - VIRAL: Indicates Viral was signaled this error
	 * 7:5 Reserved
	 */
	unsigned char error_flags;

	/*
	 * Indicates what transaction caused the error
	 *		0h=2LM READ
	 *		1h=2LM WRITE (Uncorrectable on a partial write)
	 *		2h=PMEM READ
	 *		3h=PMEM WRITE
	 *		4h=BW READ
	 *		5h=BW WRITE
	 *		6h=AIT READ
	 *		7h=AIT WRITE
	 *		8h=Wear Level Move
	 *		Ah=CSR Read
	 *		Bh=CSR Write
	 *		All other values reserved
	 */
	unsigned char transaction_type;

	char rsvd[4];
} __attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x05h (Get Error Log)
 */
struct pt_fw_thermal_log_entry {
	/*
	 * Unix Epoch Time of the log entry
	 */
	unsigned long long system_timestamp;

	/*
	 * Bit
	 * 14:0 Temperature: In degrees Celsius with 0.0625 C resolution
	 * 15	Sign: Positive or Negative
	 *	0 - Positive
	 *	1 - Negative
	 * 18:16 Reported: Temperature being reported
	 *	000b - User Alarm Trip
	 *	001b - Low
	 *	010b - High
	 * 	100b - Critcal
	 * 20:19 Temperature type: Denotes which device the temperature is for
	 * 	00b - Media temperature
	 * 	01b - Controller temperature
	 * 	10b - Reserved
	 * 	11b - Reserved
	 * 31:21 Reserved
	 */
	unsigned int host_reported_temp_data;

	char rsvd[4];
}__attribute__((packed));

/*
 * Passthrough Payload:
 *		Opcode:		0x08h (Get Log Page)
 *		Sub-Opcode:	0x05h (Get Error Log)
 *	Note: When the Log Entries Payload Return is set, only bytes 2-0 will be returned in the
 *	small payload. The log entries will all be returned in the large payload.
 */
struct pt_output_payload_fw_error_log {
	/*
	 * Number Total Entries: Specifies the total number of valid entries
	 * in the Log (New or Old)
	 */
	unsigned short number_total_entries;

	/*
	 * Bit
	 * 6:0 Return Count: Number of Log entries returned
	 * 7 Overrun Flag: Flag to indicate that the Log FIFO had an overrun
	 *		condition. Occurs if new entries exceed the LOG size
	 */
	unsigned char return_info;

	unsigned char rsvd[125];
}__attribute__((packed));

/*
 * Passthrough Payload:
 *              Opcode:         0x08h (Get Log Page)
 *              Sub-Opcode:     0x05h (Get Error Log)
 *      Note: If the System Time has not been set, the timestamps will be based on the current
 *      boot time of the FW.
 */
struct pt_payload_fw_log_info_data {
	/*
	 * Specifies the total number of log entries that the FW has the ability to log for the
	 * specified Log Type before an overrun condition occurs.
	 */
        unsigned short max_log_entries;
	/*
	 * Specifies the number of new log entries since the last time the log was read. It can
	 * also be noted that this may exceed the Max Log Entries and thus indicate an overflow
	 * condition.
	 */
        unsigned short new_log_entries;
	/*
	 * Unix Epoch Time of the oldest log entry for this Log
	 */
        unsigned long long oldest_log_entry_timestamp;
	/*
	 * Unix Epoch Time of the newest log entry for this Log
	 */
        unsigned long long newest_log_entry_timestamp;

        unsigned char rsvd[108];
}__attribute__((packed));

struct pt_bios_get_size {
	unsigned int large_input_payload_size;
	unsigned int large_output_payload_size;
	unsigned int rw_size;
}__attribute__((packed));


struct pt_update_fw_small_payload {
	/*
	 * Bit
	 * 1:0 Transfer type
	 * 15:2 Packet #
	 */
	unsigned short transfer_header;
	unsigned char payload_selector;
	unsigned char reserved;
	unsigned char data[64];
	unsigned char reserved_2[60];

}__attribute__((packed));
#endif // CR_MGMT_FIS_TYPES_H
