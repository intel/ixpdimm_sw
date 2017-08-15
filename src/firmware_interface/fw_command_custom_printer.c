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
#include "fw_command_custom_printer.h"
#include "fw_command_printer.h"
#include <stdio.h>

void custom_identification_information_table_field_printer(
	const struct fwcmd_id_info_table_data *p_value,
	int indent_count, int version);

void custom_interleave_info_printer(
	const struct fwcmd_interleave_information_table_data *interleave_information_table_data,
	int interleave_information_table_count, unsigned char table_revision, int indent_count);


void fwcmd_custom_platform_config_data_printer(
	const struct fwcmd_platform_config_data_data *p_value,
	int indent_count)
{
	fwcmd_platform_config_data_field_printer(p_value, indent_count);
	indent_count++;

	const struct fwcmd_current_config_table_data *p_current_config = &p_value->current_config_table;
	fwcmd_current_config_table_field_printer(p_current_config, indent_count);
	custom_interleave_info_printer(
		p_current_config->interleave_information_table,
		p_current_config->interleave_information_table_count,
		p_current_config->revision, indent_count + 1);

	const struct fwcmd_config_input_table_data *p_input = &p_value->config_input_table;

	fwcmd_config_input_table_field_printer(p_input, indent_count);
	custom_interleave_info_printer(
		p_input->interleave_information_table,
		p_input->interleave_information_table_count,
		p_input->revision, indent_count + 1);

	const struct fwcmd_config_output_table_data *p_output = &p_value->config_output_table;
	fwcmd_config_output_table_field_printer(p_output, indent_count);
	custom_interleave_info_printer(
		p_output->interleave_information_table,
		p_output->interleave_information_table_count,
		p_output->revision, indent_count + 1);
}

void custom_interleave_info_printer(
	const struct fwcmd_interleave_information_table_data *interleave_information_table_data,
	int interleave_information_table_count, unsigned char table_revision, int indent_count)
{
	for (size_t i = 0; i < interleave_information_table_count; i++)
	{
		const struct fwcmd_interleave_information_table_data *p_i = &interleave_information_table_data[i];
		fwcmd_interleave_information_table_field_printer(p_i, indent_count);

		for (size_t j = 0; j < p_i->id_info_table_count; j++)
		{
			custom_identification_information_table_field_printer(&p_i->id_info_table[i],
				indent_count + 1,
				table_revision);
		}
	}
}

void custom_identification_information_table_field_printer(
	const struct fwcmd_id_info_table_data *p_value,
	int indent_count, int version)
{
	print_tabs(indent_count);
	printf("PlatformConfigDataIdentificationInformationTable:\n");
	//	print_tabs(indent_count + 1);
	if (version == 1)
	{
		fwcmd_device_identification_v1_field_printer(
			&(p_value->device_identification.device_identification_v1), indent_count + 1);
	}
	else if (version == 2)
	{
		fwcmd_device_identification_v2_field_printer(
			&(p_value->device_identification.device_identification_v2), indent_count + 1);
	}
	print_tabs(indent_count + 1);
	printf("PartitionOffset: 0x%llx\n", p_value->partition_offset);
	print_tabs(indent_count + 1);
	printf("PartitionSize: 0x%llx\n", p_value->partition_size);
}