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
 * This file contains definitions for structures and values necessary for perusing
 * the raw SMBIOS table data.
 *
 * These structures and values are defined in DMTF SMBIOS spec 3.0.0.
 */

#ifndef SRC_COMMON_SMBIOS_SMBIOS_TYPES_H_
#define SRC_COMMON_SMBIOS_SMBIOS_TYPES_H_

#include <common_types.h>

/*
 * For finding the SMBIOS entry point(s) in physical memory.
 * There may be a 32-bit entry point, a 64-bit entry point, or both.
 */
#define	SMBIOS_ADDR_RANGE_START	0x000F0000
#define	SMBIOS_ADDR_RANGE_END	0x000FFFFF

#define	SMBIOS_32BIT_ANCHOR_STR_SIZE	4
#define	SMBIOS_64BIT_ANCHOR_STR_SIZE	5

extern unsigned char SMBIOS_32BIT_ANCHOR_STR[SMBIOS_32BIT_ANCHOR_STR_SIZE];
extern unsigned char SMBIOS_64BIT_ANCHOR_STR[SMBIOS_64BIT_ANCHOR_STR_SIZE];

/*
 * Entry point structures - used to find the SMBIOS table in raw physical memory
 */
struct smbios_32bit_entry_point
{
	COMMON_UINT8 anchor_str[SMBIOS_32BIT_ANCHOR_STR_SIZE];
	COMMON_UINT8 checksum;
	COMMON_UINT8 entry_point_length;
	COMMON_UINT8 smbios_major_version;
	COMMON_UINT8 smbios_minor_version;
	COMMON_UINT16 max_structure_size;
	COMMON_UINT8 entry_point_revision;
	COMMON_UINT8 formatted_area[5];
	COMMON_UINT8 intermediate_anchor_str[5];
	COMMON_UINT8 intermediate_checksum;
	COMMON_UINT16 structure_table_length;
	COMMON_UINT32 structure_table_address;
	COMMON_UINT16 num_smbios_structures;
	COMMON_UINT8 bcd_revision;
} __attribute__((packed));

struct smbios_64bit_entry_point
{
	COMMON_UINT8 anchor_str[SMBIOS_64BIT_ANCHOR_STR_SIZE];
	COMMON_UINT8 checksum;
	COMMON_UINT8 entry_point_length;
	COMMON_UINT8 smbios_major_version;
	COMMON_UINT8 smbios_minor_version;
	COMMON_UINT8 smbios_docrev;
	COMMON_UINT8 entry_point_revision;
	COMMON_UINT8 reserved;
	COMMON_UINT32 structure_table_max_length;
	COMMON_UINT64 structure_table_address;
} __attribute__((packed));

/*
 * Convenience structure that can be used for either 32-bit or 64-bit entry point
 */
enum smbios_entry_point_type
{
	SMBIOS_ENTRY_POINT_32BIT,
	SMBIOS_ENTRY_POINT_64BIT
};

struct smbios_entry_point
{
	enum smbios_entry_point_type type;
	union
	{
		struct smbios_32bit_entry_point entry_point_32_bit;
		struct smbios_64bit_entry_point entry_point_64_bit;
	} data;
};

/*
 * SMBIOS structure table values and structures
 */

enum smbios_structure_type
{
	SMBIOS_STRUCT_TYPE_BIOS_INFO = 0,
	SMBIOS_STRUCT_TYPE_SYSTEM_INFO = 1,
	SMBIOS_STRUCT_TYPE_SYSTEM_ENCLOSURE = 3,
	SMBIOS_STRUCT_TYPE_PROCESSOR_INFO = 4,
	SMBIOS_STRUCT_TYPE_CACHE_INFO = 7,
	SMBIOS_STRUCT_TYPE_SYSTEM_SLOTS = 9,
	SMBIOS_STRUCT_TYPE_PHYSICAL_MEMORY_ARRAY = 16,
	SMBIOS_STRUCT_TYPE_MEMORY_DEVICE = 17,
	SMBIOS_STRUCT_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS = 19,
	SMBIOS_STRUCT_TYPE_SYSTEM_BOOT_INFO = 32
};

struct smbios_structure_header
{
	COMMON_UINT8 type;
	COMMON_UINT8 length;
	COMMON_UINT16 handle;
} __attribute__((packed));

// Special values for the SMBIOS Type 17 Memory Device fields
#define SMBIOS_SIZE_KB_GRANULARITY_MASK	0x8000
#define SMBIOS_SIZE_MASK				0x7FFF
#define SMBIOS_EXTENDED_SIZE_MASK		0x7FFFFFFF
#define	SMBIOS_ATTRIBUTES_RANK_MASK		0x0F

#define SMBIOS_MEM_ERROR_INFO_NOT_PROVIDED	0xFFFE
#define SMBIOS_MEM_ERROR_INFO_NONE			0xFFFF

#define SMBIOS_WIDTH_UNKNOWN	0xFFFF

#define	SMBIOS_SIZE_EMPTY		0x0
#define SMBIOS_SIZE_UNKNOWN		0xFFFF
#define SMBIOS_SIZE_EXTENDED	0x7FFF

#define SMBIOS_DEVICE_SET_NONE		0x0
#define SMBIOS_DEVICE_SET_UNKNOWN	0xFF

#define	SMBIOS_SPEED_UNKNOWN	0x0

#define SMBIOS_RANK_UNKNOWN		0x0

#define	SMBIOS_VOLTAGE_UNKNOWN	0x0

// SMBIOS Type 17 - Memory Device
struct smbios_memory_device
{
	struct smbios_structure_header header;
	COMMON_UINT16 physical_mem_array_handle;
	COMMON_UINT16 mem_error_info_handle;
	COMMON_UINT16 total_width;
	COMMON_UINT16 data_width;
	COMMON_UINT16 size;
	COMMON_UINT8 form_factor;
	COMMON_UINT8 device_set;
	COMMON_UINT8 device_locator_str_num;
	COMMON_UINT8 bank_locator_str_num;
	COMMON_UINT8 memory_type;
	COMMON_UINT16 type_detail;
	COMMON_UINT16 speed;
	COMMON_UINT8 manufacturer_str_num;
	COMMON_UINT8 serial_number_str_num;
	COMMON_UINT8 asset_tag_str_num;
	COMMON_UINT8 part_number_str_num;
	COMMON_UINT8 attributes;
	COMMON_UINT32 extended_size;
	COMMON_UINT16 configured_mem_clock_speed;
	COMMON_UINT16 min_voltage;
	COMMON_UINT16 max_voltage;
	COMMON_UINT16 configured_voltage;

} __attribute__((packed));

#endif /* SRC_COMMON_SMBIOS_SMBIOS_TYPES_H_ */
