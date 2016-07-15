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
 * This file defines helper functions for
 * reading/writing the BIOS platform capabilities table (PCAT).
 */

#ifndef PLATFORM_CAPABILITIES_H_
#define	PLATFORM_CAPABILITIES_H_

#include <persistence/schema.h>
#include "nvm_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define	PCAT_MAX_LEN	4096
#define	PCAT_SIGNATURE_LEN	4
#define	PCAT_OEM_ID_LEN	6
#define	PCAT_OEM_TABLE_ID_LEN	8
#define	PCAT_CREATOR_ID_LEN	4
#define	PCAT_TABLE_SIGNATURE	"PCAT"
#define	PCAT_TABLE_SIZE	sizeof (struct bios_capabilities_header) // size without extension tables
#define	PCAT_CHECKSUM_OFFSET	9 // checksum offset
#define	PLATFORM_INFO_TABLE_SIZE	sizeof (struct platform_capabilities_ext_table)
#define	MEMORY_INTERLEAVE_TABLE_SIZE	sizeof (struct memory_interleave_capabilities_ext_table)
#define	RT_CONFIG_TABLE_SIZE	sizeof (struct reconfig_input_validation_ext_table)
#define	MGMT_SW_CONFIG_SUPPORTED(bits)	((bits) & 1)
#define	RUNTIME_CONFIG_SUPPORTED(bits) ((bits >> 1) & 1)
#define	PMEM_MIRROR_SUPPORTED(bits)	((bits) & 1)
#define	PMEM_SPARE_SUPPORTED(bits)	((bits >> 1) & 1)
#define	PMEM_MIGRATION_SUPPORTED(bits)	((bits >> 2) & 1)
#define	PCAT_FORMAT_CHANNEL_MASK	0xff
#define	PCAT_FORMAT_IMC_MASK	0xff
#define	PCAT_FORMAT_WAYS_MASK	0x1ff // bits 8:0
#define	PCAT_FORMAT_RECOMMENDED_SHIFT	31
#define	PCAT_FORMAT_IMC_SHIFT	8
#define	PCAT_FORMAT_WAYS_SHIFT	16
#define	CURRENT_VOLATILE_MODE(bits)	((bits) & 0b11) // bits 1:0

/*
 * Extension Table Type
 */
enum pcat_ext_table_type
{
	PCAT_TABLE_PLATFORM_INFO = 0,
	PCAT_TABLE_MEMORY_INTERLEAVE_INFO = 1,
	PCAT_TABLE_RECONFIG_INPUT_VALIDATION = 2,
	PCAT_TABLE_MGMT_ATTRIBUTES = 3
};

/*
 * Defines the various memory mode capabilities bits
 */
enum mem_mode_capabilities_bits
{
	MEM_MODE_1LM = 1, /* 1LM mode supported */
	MEM_MODE_MEMORY = 1 << 1, /* Memory mode supported */
	MEM_MODE_APP_DIRECT = 1 << 2, /* App Direct mode supported */
	MEM_MODE_RESERVED = 1 << 3,
	MEM_MODE_STORAGE = 1 << 4, /* Storage mode supported */
	MEM_MODE_SUBNUMA = 1 << 5
};

enum mem_mode_interleave
{
	INTERLEAVE_MEM_MODE_1LM = 0,
	INTERLEAVE_MEM_MODE_2LM = 1,
	INTERLEAVE_MEM_MODE_APP_DIRECT = 3,
};

/*
 * Defines the interleave capabilities bits
 */
enum bitmap_interleave_size
{
	BITMAP_INTERLEAVE_SIZE_64B = 1, /* 64 Bytes interleave */
	BITMAP_INTERLEAVE_SIZE_128B = 1 << 1, /* 128 Bytes interleave */
	BITMAP_INTERLEAVE_SIZE_256B = 1 << 2, /* 256 Bytes interleave */
	BITMAP_INTERLEAVE_SIZE_4KB = 1 << 6, /* 4K Bytes interleave */
	BITMAP_INTERLEAVE_SIZE_1GB = 1 << 7, /* 1G Bytes interleave */
};

enum bitmap_interleave_ways
{
	BITMAP_INTERLEAVE_WAYS_1  = 1,
	BITMAP_INTERLEAVE_WAYS_2  = 1 << 1,
	BITMAP_INTERLEAVE_WAYS_3  = 1 << 2,
	BITMAP_INTERLEAVE_WAYS_4  = 1 << 3,
	BITMAP_INTERLEAVE_WAYS_6  = 1 << 4,
	BITMAP_INTERLEAVE_WAYS_8  = 1 << 5,
	BITMAP_INTERLEAVE_WAYS_12 = 1 << 6,
	BITMAP_INTERLEAVE_WAYS_16 = 1 << 7,
	BITMAP_INTERLEAVE_WAYS_24 = 1 << 8
};

/*
 * Defines management software BIOS config support
 */
enum mgmt_sw_config_support_bits
{
	BIOS_SUPPORT_CONFIG_CHANGE = 1,
	BIOS_SUPPORT_RUNTIME_INTERFACE = 1 << 1
};

enum pmem_ras_capabilities
{
	PMEM_MIRROR = 1,
	PMEM_SPARE = 1 << 1,
	PMEM_MIGRATION = 1 << 2
};

/*
 * Header for PCAT extension tables
 */
struct pcat_extension_table_header
{
	/*
	 * Type of the extension table
	 * 0 - Platform Capability Information Table
	 * 1 - Memory Interleave Capability Information Table
	 * 2 - Re-Configuration Input Validation Interface Table
	 * 3 - Configuration Management Attributes Extension Table
	 */
	NVM_UINT16 type;

	/*
	 * Length in bytes for entire table.
	 */
	NVM_UINT16 length;

}__attribute__((packed));

/*
 * Platform Capability Information Extension Table
 * Type 0
 */
struct platform_capabilities_ext_table
{
	/*
	 * Header
	 */
	struct pcat_extension_table_header header;

	/*
	 * Management SW Config Input Support
	 * Bit0 - BIOS support configuration through Mgmt SW
	 * Bit1 - BIOS runtime check support
	 */
	NVM_UINT8 mgmt_sw_config_support;

	/*
	 * Memory Mode Capabilities
	 * Bit0 - 1LM Mode
	 * Bit1 - 2LM Mode
	 * Bit2 - App Direct Mode
	 * Bit3 - Reserved
	 * Bit4 - Storage Mode
	 * Bit5 - SubNUMA Cluster
	 * Bit7:6 - Reserved
	 */
	NVM_UINT8 mem_mode_capabilities;

	/*
	 * Current Memory mode selected by the BIOS
	 * Bits1:0 - Volatile Memory Mode
	 * 	00b - 1LM Mode
	 * 	01b - Memory Mode
	 * 	10b - Auto (Memory if DDR4+Intel NVDIMM with memory mode present, 1LM otherwise)
	 * 	11b - Reserved
	 * Bits3:2 - App Direct Memory Mode
	 *  00b - Disabled
	 *  01b - App Direct Mode
	 *  10b/11b - Reserved
	 * Bits6:4 - Reserved
	 * Bits7 - SubNUMA Cluster Mode Enabled
	 */
	NVM_UINT8 current_mem_mode;

	/*
	 * Persistent Memory RAS Capability
	 * Bit0: PM Mirror Support
	 * Bit1: PM Memory Spare Support
	 * Bit2: PM Migration Support
	 */
	NVM_UINT8 pmem_ras_capabilities;

	NVM_UINT8 reserved[8];

} __attribute__((packed));

/*
 * Memory Interleave Capability Extension Information
 * Type 1
 */
struct memory_interleave_capabilities_ext_table
{
	/*
	 * Header
	 */
	struct pcat_extension_table_header header;

	/*
	 * Value defines memory mode
	 * 0 - 1LM
	 * 1 - 2LM
	 * 3 - App Direct
	 * 4 - Reserved
	 * Other values reserved
	 */
	NVM_UINT8 memory_mode;

	NVM_UINT8 reserved[3];

	/*
	 * Interleave alignment size in 2^n bytes.
	 * n=26 for 64MB
	 * n=27 for 128MB
	 */
	NVM_UINT16 interleave_alignment_size;

	/*
	 * Number of interleave formats supported by
	 * BIOS for the above memory mode. The variable
	 * body of this structure contains m number of
	 * interleave formats.
	 */
	NVM_UINT16 supported_interleave_count;

	/*
	 * This field will have a list of 4byte values
	 * that provide information about BIOS supported
	 * interleave formats and the recommended
	 * interleave informations.
	 * Byte0 - Channel interleave size
	 * Bit0 - 64B
	 * Bit1 - 128B
	 * Bit2 - 256B
	 * Bit5:3 - Reserved
	 * Bit6 - 4KB
	 * Bit7 - Reserved
	 *
	 * Byte1 - iMC interleave size
	 * Bit0 - 64B
	 * Bit1 - 128B
	 * Bit2 - 256B
	 * Bit5:3 - Reserved
	 * Bit6 - 4KB
	 * Bit7 - Reserved
	 *
	 * Byte2-3 - Number of channel ways
	 * Bit0 - 1way
	 * Bit1 - 2way
	 * Bit2 - 3way
	 * Bit3 - 4way
	 * Bit4 - 6way
	 * Bit5 - 8way
	 * Bit6 - 12way
	 * Bit7 - 16way
	 * Bit8 - 24way
	 * Bit14:9 - Reserved
	 * Bit[15] - If clear, the interleave format
	 * is supported but not recommended. If set,
	 * the interleave format is recommended.
	 */
	NVM_UINT32 interleave_format_list[0];

} __attribute__((packed));


/*
 * Re-configuration Input Validation Extension Table
 * Type 2
 */
struct reconfig_input_validation_ext_table
{
	/*
	 * Header
	 */
	struct pcat_extension_table_header header;

	/*
	 * Address space type of command register
	 * 1: System I/O
	 * All other values reserved
	 */
	NVM_UINT8 address_space_id;

	/*
	 * The size in bits of the command register
	 */
	NVM_UINT8 bit_width;

	/*
	 * The bit offset command register at the given
	 * address
	 */
	NVM_UINT8 bit_offset;

	/*
	 * Command register access size
	 *	0 Undefined
	 *	1 Byte Access
	 *	2 Word Access
	 *	3 Dword Access
	 *	4 Qword Access
	 */
	NVM_UINT8 access_size;

	/*
	 * Register in the given address space
	 */
	NVM_UINT64 address;

	/*
	 * Type of register operation to submit the
	 * command
	 * 0 - Read register
	 * 1 - Write register
	 */
	NVM_UINT8 operation_type_1;

	NVM_UINT8 reserved_2[7];

	/*
	 * If operation type is write, this field
	 * provides the data to be written
	 */
	NVM_UINT64 value;

	/*
	 * Mask value to be used to preserve the bits
	 * on the write.
	 * Note: If the bits are not 1, read the value
	 * from the address space, mask the value part
	 * and then do the write
	 */
	NVM_UINT64 mask_1;

	/*
	 * ACPI GAS structure with Address Space ID
	 * 0: System Memory
	 * All other values reserved
	 */
	NVM_UINT8 gas_structure[12];

	/*
	 * Type of register operation to submit the
	 * command
	 * 3 - Read Memory
	 */
	NVM_UINT8 operation_type_2;

	NVM_UINT8 reserved_3[3];

	/*
	 * Read the value from given address and mask
	 * using this value
	 * Resulting Status:
	 * 0 - None
	 * 1 - Busy
	 * 2 - Done,results are updated on DIMMs
	 * config output structures
	 */
	NVM_UINT64 mask_2;
} __attribute__((packed));

/*
 * Re-configuration Input Validation Extension Table
 * Type 2
 */
struct mgmt_attributes_ext_table
{
	/*
	 * Header
	 */
	struct pcat_extension_table_header header;

	NVM_UINT8 reserved[2];

	/*
	 * Generator of UID who maintains the format of the UID data
	 */
	NVM_UINT16 vendor_id;

	/*
	 * Vendor Specific UID
	 */
	NVM_UINT8 uid[16];

	/*
	 * 8 byte aligned vendor specific data
	 */
	NVM_UINT8 uid_data[0];

} __attribute__((packed));

/*
 * BIOS PCAT Table Header
 */
struct bios_capabilities_header
{
	char signature[PCAT_SIGNATURE_LEN]; /* 'PCAT' is Signature for this table */
	NVM_UINT32 length; /* Length in bytes for entire table */
	NVM_UINT8 revision; /* 1 */
	NVM_UINT8 checksum; /* Must sum to zero */
	char oem_id[PCAT_OEM_ID_LEN]; /* OEM ID */
	char oem_table_id[PCAT_OEM_TABLE_ID_LEN]; /* The table ID is the manufacturer model ID */
	NVM_UINT32 oem_revision; /* OEM revision of table for supplied OEM Table ID */
	NVM_UINT8 creator_id[PCAT_CREATOR_ID_LEN]; /* Vendor ID of utility that created the table */
	NVM_UINT32 creator_revision; /* Revision of utility that created the table */
	NVM_UINT8 reserved[4]; /* Reserved */
}__attribute__((packed));

/*
 * BIOS PCAT Table
 */
struct bios_capabilities
{
	struct bios_capabilities_header header;
	/* Variable extension Tables */
	NVM_UINT8 p_ext_tables[PCAT_MAX_LEN - sizeof (struct bios_capabilities_header)];

}__attribute__((packed));

/*
 * Helper function to retrieve pcat from db
 */
int get_pcat_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		const NVM_UINT32 cap_len);

/*
 * Helper function to get the PCAT memory interleave extension tables
 * from the database and copy the data to the bios_capabilities structure
 */
int get_pcat_interleave_tables_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		NVM_UINT32 *p_offset, const NVM_UINT32 cap_len);

/*
 * Helper function to get the PCAT platform capabilities extension table
 * from the database and copy the data to the bios_capabilities struct
 */
int get_pcat_platform_info_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		NVM_UINT32 *p_offset, const NVM_UINT32 cap_len);

/*
 * Helper function to get the PCAT runtime configuration extension table
 * from the database and copy the data to the bios_capabilities struct
 */
int get_pcat_runtime_validation_table_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		NVM_UINT32 *p_offset, const NVM_UINT32 cap_len);

/*
 * Update the platform capabilities data stored in the db
 * Tries to store as much data as possible while propagating any errors.
 */
int update_pcat_in_db(PersistentStore *p_db,
		const struct bios_capabilities *p_capabilities,
		const int history_id);

/*
 * Helper fn to check for valid platform capabilities data
 */
int check_pcat(const struct bios_capabilities *p_capabilities);

/*
 * Given a starting offset of the extension tables, get the offset of the specfied table
 */
NVM_UINT32 get_offset_of_ext_table(const struct bios_capabilities *p_capabilities,
		enum pcat_ext_table_type type, NVM_UINT32 offset);


#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_CAPABILITIES_H_ */
