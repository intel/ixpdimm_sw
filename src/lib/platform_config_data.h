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
 * This file describes the format of the platform configuration data
 * and defines helper functions for reading/writing such data.
 * The platform configuration data is stored on each
 * NVM-DIMM and is the Native API interface with the BIOS.
 */

#ifndef	_NVM_PLATFORM_CONFIG_DATA_H_
#define	_NVM_PLATFORM_CONFIG_DATA_H_

#include "nvm_types.h"
#include "device_adapter.h"
#include "device_utilities.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define	SIGNATURE_LEN	4
#define	OEM_ID_LEN	6
#define	OEM_TABLE_ID_LEN	8
#define	CURRENT_CONFIG_TABLE_SIGNATURE	"CCUR"
#define	CONFIG_INPUT_TABLE_SIGNATURE	"CIN_"
#define	CONFIG_OUTPUT_TABLE_SIGNATURE	"COUT"
#define	PLATFORM_CONFIG_TABLE_SIGNATURE	"DMHD"
#define	CHECKSUM_OFFSET	9
#define	INTEL_VENDOR_ID 0x8086

enum config_table_type
{
	TABLE_TYPE_CURRENT_CONFIG = 0,
	TABLE_TYPE_CONFIG_INPUT = 1,
	TABLE_TYPE_CONFIG_OUTPUT = 2
};

enum interleave_memory_type
{
	INTERLEAVE_MEMORY_TYPE_UNKNOWN = 0,
	INTERLEAVE_MEMORY_TYPE_MEMORY = 1,
	INTERLEAVE_MEMORY_TYPE_APP_DIRECT = 2
};

enum extension_table_type
{
	PLATFORM_CAPABILITY_TABLE = 0,
	INTERLEAVE_CAPABILITY_TABLE = 1,
	RUNTIME_INTERFACE_VALIDATION_TABLE = 2,
	CONFIGURATION_MANAGEMENT_TABLE = 3,
	PARTITION_CHANGE_TABLE = 4,
	INTERLEAVE_TABLE = 5
};

/*
 * Header for extension tables
 */
struct extension_table_header
{
	NVM_UINT16 type;
	NVM_UINT16 length;
}__attribute__((packed));

/*
 * Partition size change status
 */
enum partition_size_change_status
{
	PARTITION_SIZE_CHANGE_STATUS_UNDEFINED = 0,
	PARTITION_SIZE_CHANGE_STATUS_SUCCESS = 1,
	// 2 is reserved
	PARTITION_SIZE_CHANGE_STATUS_DIMMS_NOT_FOUND = 3,
	PARTITION_SIZE_CHANGE_STATUS_DIMM_INTERLEAVE_INFO_BAD = 4,
	PARTITION_SIZE_CHANGE_STATUS_SIZE_TOO_BIG = 5,
	PARTITION_SIZE_CHANGE_STATUS_FW_ERROR = 6,
	PARTITION_SIZE_CHANGE_STATUS_NOT_ENOUGH_DRAM_DECODERS = 7,
	PARTITION_SIZE_CHANGE_STATUS_BAD_ALIGNMENT = 8
};

/*
 * Partition size change table
 */
struct partition_size_change_extension_table
{
	struct extension_table_header header;

	/*
	 * Operation status - only valid in config output table
	 * Byte 1:0
	 *		0 - Undefined
	 *		1 - Success
	 *		2 - Reserved
	 *		3 - Not all the DIMMs in the interleave set found
	 *		4 - Matching interleave set not found
	 *		5 - Total size exceeds input
	 *		6 - FW returned error
	 *		7 - Out of DRAM Decoders
	 *		8 - PM partition size not aligned
	 * Byte 3:2: FW Error response code
	 */
	NVM_UINT32 status;

	NVM_UINT64 partition_size; // PM partition size in bytes
}__attribute__((packed));


/*
 * Describes a dimm in an interleave set
 * NOTE: total size is 48
 */
struct dimm_info_extension_table
{
	NVM_UINT8 manufacturer[2]; // NVM-DIMM manufacturer
	NVM_UINT8 serial_number[4]; // NVM-DIMM serial number
	char model_number[20]; // NVM-DIMM serial number
	NVM_UINT8 reserved[6];
	NVM_UINT64 offset; // Logical offset from the base of the partition
	NVM_UINT64 size; // Size in bytes to add to interleave set
}__attribute__((packed));

enum interleave_status
{
	INTERLEAVE_STATUS_UNKNOWN = 0,
	INTERLEAVE_STATUS_SUCCESS = 1,
	INTERLEAVE_STATUS_NOT_PROCESSED = 2,
	INTERLEAVE_STATUS_DIMMS_NOT_FOUND = 3,
	INTERLEAVE_STATUS_DIMM_INTERLEAVE_INFO_BAD = 4,
	INTERLEAVE_STATUS_NOT_ENOUGH_DRAM_DECODERS = 5,
	INTERLEAVE_STATUS_NOT_ENOUGH_SPA_SPACE = 6,
	INTERLEAVE_STATUS_UNAVAILABLE_RESOURCES = 7,
	INTERLEAVE_STATUS_PARTITIONING_FAILED = 8,
	INTERLEAVE_STATUS_DIMM_INTERLEAVE_MISSING = 9,
	INTERLEAVE_STATUS_CHANNEL_INTERLEAVE_MISMATCH = 10,
	INTERLEAVE_STATUS_BAD_ALIGNMENT = 11,
};

/*
 * Describes an interleave set (pool)
 * Valid for current config, config input and output
 */
struct interleave_info_extension_table
{
	struct extension_table_header header;

	NVM_UINT16 index; // logical index number
	NVM_UINT8 dimm_count; // number of dimms in interleave
	NVM_UINT8 memory_type; // 1 - memory mode, 2 - app direct mode
	NVM_UINT32 interleave_format; // interleave format
	NVM_UINT8 mirror_enable; // 0 - disabled, 1 enabled

	/*
	 * Operation status - only valid for config output
	 *  0 - Undefined
	 *	1 - successfully interleaved the request
	 *	2 - info not processed
	 *	3 - unable to find matching dimms in the interleave
	 *	4 - matching dimms found, but interleave info does not match
	 *	5 - insufficient number of DRAM decoders available to map
	 *	6 - memory mapping failed due to unavailable system address space
	 *	7 - mirror mapping failed due to unavailable resources
	 *	8 - partitioning request failed
	 *	9 - matching dimms found, but CIN_ missing a DIMM in the interleave list
	 *	10 - Channel intereave doesn't match between MCs being interleaved
	 *	11 - Bad alignment
	 */
	NVM_UINT8 status;

	/*
	 * Memory Spare - config input
	 * 0 - this interleave set is not a spare
	 * 1 - reserve this interleave set as a spare
	 */
	NVM_UINT8 mem_spare;
	NVM_UINT8 rsvd[9];
	NVM_UINT8 p_dimms[0];
}__attribute__((packed));

/*
 * Header for main config tables
 */
struct config_data_table_header
{
	char signature[SIGNATURE_LEN]; // Table signature, 'DMHD' for this table
	NVM_UINT32 length; // Length in bytes for the entire table
	NVM_UINT8 revision; // Table definition revision
	NVM_UINT8 checksum; // Entire Table must sum to zero
	char oem_id[OEM_ID_LEN]; // OEM ID
	char oem_table_id[OEM_TABLE_ID_LEN]; // The manufacturer model ID
	NVM_UINT32 oem_revision; // OEM revision of table
	NVM_UINT32 creator_id; // Vendor ID of utility that created table
	NVM_UINT32 creator_revision; // Revision of utility that created table
}__attribute__((packed));

enum current_config_status
{
	CURRENT_CONFIG_STATUS_UNKNOWN = 0,
	CURRENT_CONFIG_STATUS_SUCCESS = 1,
	// 2 is reserved
	CURRENT_CONFIG_STATUS_DIMMS_NOT_FOUND = 3,
	CURRENT_CONFIG_STATUS_INTERLEAVE_NOT_FOUND = 4,
	CURRENT_CONFIG_STATUS_UNCONFIGURED = 5,
	CURRENT_CONFIG_STATUS_ERROR_USING_OLD = 6,
	CURRENT_CONFIG_STATUS_ERROR_UNMAPPED = 7,
	CURRENT_CONFIG_STATUS_BAD_INPUT_CHECKSUM = 8,
	CURRENT_CONFIG_STATUS_BAD_INPUT_REVISION = 9,
	CURRENT_CONFIG_STATUS_BAD_CURRENT_CHECKSUM = 10
};

/*
 * BIOS CR Mgmt Interface Current Configuration Table
 */
struct current_config_table
{
	struct config_data_table_header header;

	/*
	 * Error status
	 * 	00 - Undefined
	 * 	01 - Dimm is configured successfully
	 * 	02 - Reserved
	 * 	03 - All the DIMMs in the iset not found.
	 * 	04 - Matching iset not found.
	 * 	05 - New Dimm
	 * 	06 - Config input had errors, old config used
	 * 	07 - Config input had errors, dimm is not mapped
	 * 	08 - Config input checksum did not match
	 * 	09 - Config input data revision is not supported
	 * 	10 - Current config checksum did not match
	 */
	NVM_UINT16 config_status;

	NVM_UINT8 reserved[2];

	NVM_UINT64 mapped_memory_capacity; // memory mode capacity in bytes mapped into the SPA
	NVM_UINT64 mapped_app_direct_capacity; // app direct capacity in bytes mapped into the SPA

	// Extension tables
	NVM_UINT8 p_ext_tables[0];

}__attribute__((packed));

/*
 * Mgmt software request to BIOS
 */
struct config_input_table
{
	struct config_data_table_header header;

	// body
	NVM_UINT32 sequence_number; // Request sequence number
	NVM_UINT8 reserved[8];

	// Extension tables
	NVM_UINT8 p_ext_tables[0];

}__attribute__((packed));


enum config_output_status
{
	CONFIG_OUTPUT_STATUS_UNKNOWN = 0,
	CONFIG_OUTPUT_STATUS_SUCCESS = 1,
	CONFIG_OUTPUT_STATUS_BOOTTIME_ERRORS = 2,
	CONFIG_OUTPUT_STATUS_RUNTIME_PROGRESS = 3,
	CONFIG_OUTPUT_STATUS_RUNTIME_OK = 4,
	CONFIG_OUTPUT_STATUS_RUNTIME_ERRORS = 5
};

/*
 * Specific error state derived from the config output extension tables
 */
enum config_error
{
	CONFIG_ERROR_NOTFOUND = 0, // No config output extension table reported an error.
	CONFIG_ERROR_BADREQUEST = 1, // The request was invalid or the output table was corrupt.
	CONFIG_ERROR_INSUFFICIENTRESOURCES = 2, // Not enough resources to apply the configuration.
	CONFIG_ERROR_FW = 3, // Failed to apply the configuration due to a FW error.
	CONFIG_ERROR_BROKENINTERLEAVE = 4, // Interleave set is broken due to missing/failed DIMMs.
	CONFIG_ERROR_UNKNOWN = 5, // Failed for an unknown reason.
};

/*
 * BIOS response to mgmt software
 */
struct config_output_table
{
	struct config_data_table_header header;

	// body
	NVM_UINT32 sequence_number; // Request sequence number

	/*
	 * BIOS response after config input processing
	 *	0 - Undefined
	 *	1 - Config change applied successfully
	 *	2 - Boot time processing complete, errors found
	 *	3 - Runtime validation is in progress
	 *	4 - Runtime validation is complete, no errors found
	 *	5 - Runtime validation complete, errors found
	 */
	NVM_UINT8 validation_status;

	NVM_UINT8 reserved[7];

	// Extension tables
	NVM_UINT8 p_ext_tables[0];

}__attribute__((packed));

/*
 * The BIOS platform config data table written to the DIMM
 * during the MRC phase to represent the current configuration
 */
struct platform_config_data
{
	// Header
	struct config_data_table_header header;

	// Body
	NVM_UINT32 current_config_size; // current config table size
	NVM_UINT32 current_config_offset; // start location of current config table

	NVM_UINT32 config_input_size; // config input table size
	NVM_UINT32 config_input_offset; // start location of config input table

	NVM_UINT32 config_output_size; // config output table size
	NVM_UINT32 config_output_offset; // start location of config output table

}__attribute__((packed));


/*
 * Cast a current config table from the platform config data
 */
static inline struct current_config_table *cast_current_config(
		struct platform_config_data *p_config)
{
	struct current_config_table *p_current = NULL;
	if (p_config->current_config_size > 0)
	{
		p_current = (struct current_config_table *)
				((NVM_UINT8 *)p_config + p_config->current_config_offset);
	}
	return p_current;
}

/*
 * Cast the config input table from the platform config data
 */
static inline struct config_input_table *cast_config_input(
		struct platform_config_data *p_config)
{
	struct config_input_table *p_input = NULL;
	if (p_config->config_input_size > 0)
	{
		p_input = (struct config_input_table *)
				((NVM_UINT8 *)p_config + p_config->config_input_offset);
	}
	return p_input;
}

/*
 * Cast the config output table from the platform config data
 */
static inline struct config_output_table *cast_config_output(
		struct platform_config_data *p_config)
{
	struct config_output_table *p_output = NULL;
	if (p_config->config_output_size > 0)
	{
		p_output = (struct config_output_table *)
				((NVM_UINT8 *)p_config + p_config->config_output_offset);
	}
	return p_output;
}

/*
 * *****************************************************************************
 *	Helper functions
 * ***************************************************************************
 */

/*
 * Create a platform config data table built from the three sub-tables.
 *
 * Sub-tables are expected to be fully-populated and have accurate checksums/lengths.
 *
 * NOTE: Caller must free the platform_config_data structure.
 */
int build_platform_config_data(const struct current_config_table *p_current_config,
		const struct config_input_table *p_config_input,
		const struct config_output_table *p_config_output,
		struct platform_config_data **pp_config);
/*
 * Call the FW commands to retrieve the platform configuration data
 * and parse it into the structure.
 *
 * NOTE: Callers must free the platform_config_data structure to avoid memory leaks
 */
int get_hw_dimm_platform_config_alloc(const unsigned int handle, NVM_SIZE *p_pcd_size,
		void **pp_config);

/*
 * Retrieve a copy of the PCD data from the global device table
 *
 * NOTE: Callers must free the platform_config_data structure to avoid memory leaks
 */
int get_dimm_platform_config(const NVM_NFIT_DEVICE_HANDLE handle,
		struct platform_config_data **pp_config);

/*
 * Write the platform configuration data to the specified dimm
 */
int set_dimm_platform_config(const NVM_NFIT_DEVICE_HANDLE handle,
		const struct platform_config_data *p_config);

/*
 * Verify the data is good
 */
int check_platform_config(struct platform_config_data *p_config);

/*
 * Generate a byte checksum on a table and store the checksum at the
 * specified offset.
 */
void generate_checksum(NVM_UINT8 *p_raw_data,
		const NVM_UINT32 length, const NVM_UINT32 checksum_offset);

/*
 * Verify a table sums to 0.
 */
int verify_checksum(const NVM_UINT8 *p_raw_data, const NVM_UINT32 length);

/*
 * Returns the sequence number from the config output table.
 * I.e. last BIOS response.
 * If there is no config output table, returns 0.
 */
NVM_UINT32 get_last_config_output_sequence_number(struct platform_config_data *p_config);

/*
 * Calculate the next sequence number for a DIMM based on existing platform config.
 * Returns NVM_SUCCESS if the calculation is successful.
 * If successful, the next sequence number is returned in *p_seq_num.
 */
int get_next_config_input_sequence_number(const struct device_discovery *p_dev_entry,
		NVM_UINT32 *p_seq_num);

/*
 * Write the dimm configuration to the file specified
 */
int write_dimm_config(const struct device_discovery *p_discovery,
		const struct config_goal *p_goal,
		const NVM_PATH path, const NVM_SIZE path_len, const NVM_BOOL append);

/*
 * Get the next interleave index
 */
int get_dimm_interleave_info_max_set_index(const NVM_UID device_uid,
		NVM_UINT32 *p_set_index);

/*
 * Harvest format and settings data for a specific interleave set from the platform config data.
 */
int get_interleave_settings_from_platform_config_data(
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const NVM_UINT64 interleave_set_offset,
		NVM_UINT32 *p_interleave_set_id,
		struct interleave_format *p_format, NVM_BOOL *p_mirrored);

/*
 * Translate the config output table error status to a config_error.
 * Assumes caller has checked for NULL on the config output table.
 */
enum config_error get_config_error_from_config_output(
		struct config_output_table *p_config_output);

/*
 * Translates the current config table status to a config_status.
 */
enum config_status get_config_status_from_current_config(
		struct current_config_table *p_current_config);

#ifdef __cplusplus
}
#endif

#endif /* _NVM_PLATFORM_CONFIG_DATA_H_ */
