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

#include "nfit_printer.h"
#include <guid/guid.h>
#include <string.h>
#include <stdio.h>

/*
 * Print a parsed_nfit structure
 */
void nfit_print_parsed_nfit(const struct parsed_nfit *p_nfit)
{
	if (p_nfit)
	{
		print_nfit_table(p_nfit->nfit, 0);
		for (size_t i = 0; i < p_nfit->spa_count; i++)
		{
			print_spa_table(p_nfit->spa_list[i], 1);
		}
		for (size_t i = 0; i < p_nfit->region_mapping_count; i++)
		{
			print_region_mapping_table(p_nfit->region_mapping_list[i], 1);
		}
		for (size_t i = 0; i < p_nfit->interleave_count; i++)
		{
			print_interleave_table(p_nfit->interleave_list[i], 1);
		}
		for (size_t i = 0; i < p_nfit->smbios_management_info_count; i++)
		{
			print_smbios_management_info_table(p_nfit->smbios_management_info_list[i], 1);
		}
		for (size_t i = 0; i < p_nfit->control_region_count; i++)
		{
			print_control_region_table(p_nfit->control_region_list[i], 1);
		}
		for (size_t i = 0; i < p_nfit->block_data_window_region_count; i++)
		{
			print_block_data_window_region_table(p_nfit->block_data_window_region_list[i], 1);
		}
		for (size_t i = 0; i < p_nfit->flush_hint_address_count; i++)
		{
			print_flush_hint_address_table(p_nfit->flush_hint_address_list[i], 1);
		}
	}
	else
	{
		printf("NFIT is NULL\n");
	}
}

/*
 * Print an NFIT nfit extension table
 */
void print_nfit_table(
	const struct nfit table,
	const int indent_count)
{
	// print the table name
	for (int i = 0; i < indent_count; i++)
	{
		printf("\t");
	}
	printf("nfit:\n");

	// print the values
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	char signature_str[4+1]; // add a null terminator
	memmove(signature_str, table.signature, 4);
	signature_str[4] = '\0';
	printf("signature: %s\n", signature_str);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("length: 0x%x\n",
		table.length);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("revision: 0x%x\n",
		table.revision);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("checksum: 0x%x\n",
		table.checksum);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	char oem_id_str[6+1]; // add a null terminator
	memmove(oem_id_str, table.oem_id, 6);
	oem_id_str[6] = '\0';
	printf("oem_id: %s\n", oem_id_str);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	char oem_table_id_str[8+1]; // add a null terminator
	memmove(oem_table_id_str, table.oem_table_id, 8);
	oem_table_id_str[8] = '\0';
	printf("oem_table_id: %s\n", oem_table_id_str);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("oem_revision: 0x%x\n",
		table.oem_revision);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	char creator_id_str[4+1]; // add a null terminator
	memmove(creator_id_str, table.creator_id, 4);
	creator_id_str[4] = '\0';
	printf("creator_id: %s\n", creator_id_str);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("creator_revision: 0x%x\n",
		table.creator_revision);
}
/*
 * Print an NFIT spa extension table
 */
void print_spa_table(
	const struct spa table,
	const int indent_count)
{
	// print the table name
	for (int i = 0; i < indent_count; i++)
	{
		printf("\t");
	}
	printf("spa:\n");

	// print the values
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("type: 0x%x\n",
		table.type);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("length: 0x%x\n",
		table.length);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("spa_range_index: 0x%x\n",
		table.spa_range_index);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("flags: 0x%x\n",
		table.flags);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("proximity_domain: 0x%x\n",
		table.proximity_domain);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	COMMON_GUID_STR address_range_type_guid_guid_str;
	guid_to_str(table.address_range_type_guid, address_range_type_guid_guid_str);
	printf("address_range_type_guid: %s\n",
		address_range_type_guid_guid_str);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("spa_range_base: 0x%llx\n",
		table.spa_range_base);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("spa_range_length: 0x%llx\n",
		table.spa_range_length);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("address_range_memory_mapping_attribute: 0x%llx\n",
		table.address_range_memory_mapping_attribute);
}
/*
 * Print an NFIT region_mapping extension table
 */
void print_region_mapping_table(
	const struct region_mapping table,
	const int indent_count)
{
	// print the table name
	for (int i = 0; i < indent_count; i++)
	{
		printf("\t");
	}
	printf("region_mapping:\n");

	// print the values
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("type: 0x%x\n",
		table.type);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("length: 0x%x\n",
		table.length);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("handle: 0x%x\n",
		table.handle);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("physical_id: 0x%x\n",
		table.physical_id);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("region_id: 0x%x\n",
		table.region_id);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("spa_index: 0x%x\n",
		table.spa_index);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("control_region_index: 0x%x\n",
		table.control_region_index);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("region_size: 0x%llx\n",
		table.region_size);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("region_offset: 0x%llx\n",
		table.region_offset);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("physical_address_region_base: 0x%llx\n",
		table.physical_address_region_base);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("interleave_index: 0x%x\n",
		table.interleave_index);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("interleave_ways: 0x%x\n",
		table.interleave_ways);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("state_flag: 0x%x\n",
		table.state_flag);
}
/*
 * Print an NFIT interleave extension table
 */
void print_interleave_table(
	const struct interleave table,
	const int indent_count)
{
	// print the table name
	for (int i = 0; i < indent_count; i++)
	{
		printf("\t");
	}
	printf("interleave:\n");

	// print the values
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("type: 0x%x\n",
		table.type);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("length: 0x%x\n",
		table.length);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("interleave_index: 0x%x\n",
		table.interleave_index);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("number_of_lines: 0x%x\n",
		table.number_of_lines);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("line_size_bytes: 0x%x\n",
		table.line_size_bytes);
}
/*
 * Print an NFIT smbios_management_info extension table
 */
void print_smbios_management_info_table(
	const struct smbios_management_info table,
	const int indent_count)
{
	// print the table name
	for (int i = 0; i < indent_count; i++)
	{
		printf("\t");
	}
	printf("smbios_management_info:\n");

	// print the values
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("type: 0x%x\n",
		table.type);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("length: 0x%x\n",
		table.length);
}
/*
 * Print an NFIT control_region extension table
 */
void print_control_region_table(
	const struct control_region table,
	const int indent_count)
{
	// print the table name
	for (int i = 0; i < indent_count; i++)
	{
		printf("\t");
	}
	printf("control_region:\n");

	// print the values
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("type: 0x%x\n",
		table.type);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("length: 0x%x\n",
		table.length);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("index: 0x%x\n",
		table.index);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("vendor_id: 0x%x\n",
		table.vendor_id);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("device_id: 0x%x\n",
		table.device_id);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("revision_id: 0x%x\n",
		table.revision_id);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("subsystem_vendor_id: 0x%x\n",
		table.subsystem_vendor_id);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("subsystem_device_id: 0x%x\n",
		table.subsystem_device_id);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("subsystem_revision_id: 0x%x\n",
		table.subsystem_revision_id);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("valid_fields: 0x%x\n",
		table.valid_fields);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("manufacturing_location: 0x%x\n",
		table.manufacturing_location);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("manufacturing_date: 0x%x\n",
		table.manufacturing_date);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("serial_number: 0x%x\n",
		table.serial_number);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("ifc: 0x%x\n",
		table.ifc);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("block_window_count: 0x%x\n",
		table.block_window_count);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("block_window_size: 0x%llx\n",
		table.block_window_size);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("register_offset: 0x%llx\n",
		table.register_offset);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("register_size: 0x%llx\n",
		table.register_size);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("status_register_offset: 0x%llx\n",
		table.status_register_offset);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("status_register_size: 0x%llx\n",
		table.status_register_size);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("region_flag: 0x%x\n",
		table.region_flag);
}
/*
 * Print an NFIT block_data_window_region extension table
 */
void print_block_data_window_region_table(
	const struct block_data_window_region table,
	const int indent_count)
{
	// print the table name
	for (int i = 0; i < indent_count; i++)
	{
		printf("\t");
	}
	printf("block_data_window_region:\n");

	// print the values
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("type: 0x%x\n",
		table.type);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("length: 0x%x\n",
		table.length);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("control_region_index: 0x%x\n",
		table.control_region_index);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("windows_count: 0x%x\n",
		table.windows_count);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("window_start_offset: 0x%llx\n",
		table.window_start_offset);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("window_size: 0x%llx\n",
		table.window_size);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("memory_capacity: 0x%llx\n",
		table.memory_capacity);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("first_block_address: 0x%llx\n",
		table.first_block_address);
}
/*
 * Print an NFIT flush_hint_address extension table
 */
void print_flush_hint_address_table(
	const struct flush_hint_address table,
	const int indent_count)
{
	// print the table name
	for (int i = 0; i < indent_count; i++)
	{
		printf("\t");
	}
	printf("flush_hint_address:\n");

	// print the values
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("type: 0x%x\n",
		table.type);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("length: 0x%x\n",
		table.length);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("handle: 0x%x\n",
		table.handle);
	for (int i = 0; i < indent_count+1; i++)
	{
		printf("\t");
	}
	printf("address_count: 0x%x\n",
		table.address_count);
}
