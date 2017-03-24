/*
 * Copyright (c) 2017, Intel Corporation
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

#ifndef _NFIT_INTERFACE_NFIT_TABLES_H_
#define _NFIT_INTERFACE_NFIT_TABLES_H_

#ifdef __cplusplus
extern "C"
{
#endif

struct nfit
{
	unsigned char signature[4];
	unsigned int length;
	unsigned char revision;
	unsigned char checksum;
	unsigned char oem_id[6];
	unsigned char oem_table_id[8];
	unsigned int oem_revision;
	unsigned char creator_id[4];
	unsigned int creator_revision;
	unsigned int reserved;
} __attribute__((packed));

struct spa
{
	unsigned short type;
	unsigned short length;
	unsigned short spa_range_index;
	unsigned short flags;
	unsigned int reserved;
	unsigned int proximity_domain;
	unsigned char address_range_type_guid[16];
	unsigned long long spa_range_base;
	unsigned long long spa_range_length;
	unsigned long long address_range_memory_mapping_attribute;
} __attribute__((packed));

struct region_mapping
{
	unsigned short type;
	unsigned short length;
	unsigned int handle;
	unsigned short physical_id;
	unsigned short region_id;
	unsigned short spa_index;
	unsigned short control_region_index;
	unsigned long long region_size;
	unsigned long long region_offset;
	unsigned long long physical_address_region_base;
	unsigned short interleave_index;
	unsigned short interleave_ways;
	unsigned short state_flag;
	unsigned short reserved;
} __attribute__((packed));

struct interleave
{
	unsigned short type;
	unsigned short length;
	unsigned short interleave_index;
	unsigned short reserved;
	unsigned int number_of_lines;
	unsigned int line_size_bytes;
} __attribute__((packed));

struct smbios_management_info
{
	unsigned short type;
	unsigned short length;
	unsigned int reserved;
} __attribute__((packed));

struct control_region
{
	unsigned short type;
	unsigned short length;
	unsigned short index;
	unsigned short vendor_id;
	unsigned short device_id;
	unsigned short revision_id;
	unsigned short subsystem_vendor_id;
	unsigned short subsystem_device_id;
	unsigned short subsystem_revision_id;
	unsigned char valid_fields;
	unsigned char manufacturing_location;
	unsigned short manufacturing_date;
	unsigned short reserved;
	unsigned int serial_number;
	unsigned short ifc;
	unsigned short block_window_count;
	unsigned long long block_window_size;
	unsigned long long register_offset;
	unsigned long long register_size;
	unsigned long long status_register_offset;
	unsigned long long status_register_size;
	unsigned short region_flag;
	unsigned char reserved_1[6];
} __attribute__((packed));

struct block_data_window_region
{
	unsigned short type;
	unsigned short length;
	unsigned short control_region_index;
	unsigned short windows_count;
	unsigned long long window_start_offset;
	unsigned long long window_size;
	unsigned long long memory_capacity;
	unsigned long long first_block_address;
} __attribute__((packed));

struct flush_hint_address
{
	unsigned short type;
	unsigned short length;
	unsigned int handle;
	unsigned short address_count;
	unsigned char reserved[6];
} __attribute__((packed));

struct parsed_nfit
{
	struct nfit nfit;
	int spa_count;
	struct spa *spa_list;
	int region_mapping_count;
	struct region_mapping *region_mapping_list;
	int interleave_count;
	struct interleave *interleave_list;
	int smbios_management_info_count;
	struct smbios_management_info *smbios_management_info_list;
	int control_region_count;
	struct control_region *control_region_list;
	int block_data_window_region_count;
	struct block_data_window_region *block_data_window_region_list;
	int flush_hint_address_count;
	struct flush_hint_address *flush_hint_address_list;
};

#ifdef __cplusplus
}
#endif

#endif /* _NFIT_INTERFACE_NFIT_TABLES_H_ */