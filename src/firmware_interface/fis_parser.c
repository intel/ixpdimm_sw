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

#define PARSING_SUCCESS(rc) (rc == FIS_PARSER_CODES_SUCCESS)

#include "fis_commands.h"
#include "fis_parser.h"
#include "fw_commands.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int fwcmd_parse_identify_dimm(
	const struct pt_output_identify_dimm *p_output_payload,
	struct fwcmd_identify_dimm_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->vendor_id = p_output_payload->vendor_id;
	p_data->device_id = p_output_payload->device_id;
	p_data->revision_id = p_output_payload->revision_id;
	p_data->interface_format_code = p_output_payload->interface_format_code;
	memmove(p_data->firmware_revision, p_output_payload->firmware_revision, 5);
	p_data->reserved_old_api = p_output_payload->reserved_old_api;
	p_data->feature_sw_required_mask = p_output_payload->feature_sw_required_mask;
	p_data->feature_sw_required_mask_invalidate_before_block_read = (unsigned char)((p_data->feature_sw_required_mask >> 0) & 0x01);
	p_data->feature_sw_required_mask_readback_of_bw_address_register_required_before_use = (unsigned char)((p_data->feature_sw_required_mask >> 1) & 0x01);
	p_data->number_of_block_windows = p_output_payload->number_of_block_windows;
	p_data->number_of_write_flush_addresses = p_output_payload->number_of_write_flush_addresses;
	p_data->write_flush_address_start = p_output_payload->write_flush_address_start;
	p_data->offset_of_block_mode_control_region = p_output_payload->offset_of_block_mode_control_region;
	p_data->raw_capacity = p_output_payload->raw_capacity;
	p_data->manufacturer = p_output_payload->manufacturer;
	p_data->serial_number = p_output_payload->serial_number;
	memmove(p_data->part_number, p_output_payload->part_number, 20);
	p_data->part_number[20] = '\0';
	p_data->dimm_sku = p_output_payload->dimm_sku;
	p_data->dimm_sku_memory_mode_enabled = (unsigned char)((p_data->dimm_sku >> 0) & 0x01);
	p_data->dimm_sku_storage_mode_enabled = (unsigned char)((p_data->dimm_sku >> 1) & 0x01);
	p_data->dimm_sku_app_direct_mode_enabled = (unsigned char)((p_data->dimm_sku >> 2) & 0x01);
	p_data->dimm_sku_die_sparing_capable = (unsigned char)((p_data->dimm_sku >> 3) & 0x01);
	p_data->dimm_sku_soft_programmable_sku = (unsigned char)((p_data->dimm_sku >> 16) & 0x01);
	p_data->dimm_sku_encryption_enabled = (unsigned char)((p_data->dimm_sku >> 17) & 0x01);
	p_data->interface_format_code_extra = p_output_payload->interface_format_code_extra;
	p_data->api_ver = p_output_payload->api_ver;
	return rc;
}


int fwcmd_parse_identify_dimm_characteristics(
	const struct pt_output_identify_dimm_characteristics *p_output_payload,
	struct fwcmd_identify_dimm_characteristics_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->controller_temp_shutdown_threshold = p_output_payload->controller_temp_shutdown_threshold;
	p_data->media_temp_shutdown_threshold = p_output_payload->media_temp_shutdown_threshold;
	p_data->throttling_start_threshold = p_output_payload->throttling_start_threshold;
	p_data->throttling_stop_threshold = p_output_payload->throttling_stop_threshold;
	return rc;
}


int fwcmd_parse_get_security_state(
	const struct pt_output_get_security_state *p_output_payload,
	struct fwcmd_get_security_state_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->security_state = p_output_payload->security_state;
	p_data->security_state_enabled = (unsigned char)((p_data->security_state >> 1) & 0x01);
	p_data->security_state_locked = (unsigned char)((p_data->security_state >> 2) & 0x01);
	p_data->security_state_frozen = (unsigned char)((p_data->security_state >> 3) & 0x01);
	p_data->security_state_count_expired = (unsigned char)((p_data->security_state >> 4) & 0x01);
	p_data->security_state_not_supported = (unsigned char)((p_data->security_state >> 5) & 0x01);
	return rc;
}


int fwcmd_parse_get_alarm_threshold(
	const struct pt_output_get_alarm_threshold *p_output_payload,
	struct fwcmd_get_alarm_threshold_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->enable = p_output_payload->enable;
	p_data->enable_spare_block = (unsigned char)((p_data->enable >> 0) & 0x01);
	p_data->enable_media_temp = (unsigned char)((p_data->enable >> 1) & 0x01);
	p_data->enable_controller_temp = (unsigned char)((p_data->enable >> 2) & 0x01);
	p_data->spare_block_threshold = p_output_payload->spare_block_threshold;
	p_data->media_temp_threshold = p_output_payload->media_temp_threshold;
	p_data->controller_temp_threshold = p_output_payload->controller_temp_threshold;
	return rc;
}


int fwcmd_parse_power_management_policy(
	const struct pt_output_power_management_policy *p_output_payload,
	struct fwcmd_power_management_policy_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->enable = p_output_payload->enable;
	p_data->peak_power_budget = p_output_payload->peak_power_budget;
	p_data->average_power_budget = p_output_payload->average_power_budget;
	p_data->max_power = p_output_payload->max_power;
	return rc;
}


int fwcmd_parse_die_sparing_policy(
	const struct pt_output_die_sparing_policy *p_output_payload,
	struct fwcmd_die_sparing_policy_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->enable = p_output_payload->enable;
	p_data->aggressiveness = p_output_payload->aggressiveness;
	p_data->supported = p_output_payload->supported;
	p_data->supported_rank_0 = (unsigned char)((p_data->supported >> 0) & 0x01);
	p_data->supported_rank_1 = (unsigned char)((p_data->supported >> 1) & 0x01);
	p_data->supported_rank_2 = (unsigned char)((p_data->supported >> 2) & 0x01);
	p_data->supported_rank_3 = (unsigned char)((p_data->supported >> 3) & 0x01);
	return rc;
}


int fwcmd_parse_address_range_scrub(
	const struct pt_output_address_range_scrub *p_output_payload,
	struct fwcmd_address_range_scrub_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->enable = p_output_payload->enable;
	p_data->dpa_start_address = p_output_payload->dpa_start_address;
	p_data->dpa_end_address = p_output_payload->dpa_end_address;
	p_data->dpa_current_address = p_output_payload->dpa_current_address;
	return rc;
}


int fwcmd_parse_optional_configuration_data_policy(
	const struct pt_output_optional_configuration_data_policy *p_output_payload,
	struct fwcmd_optional_configuration_data_policy_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->first_fast_refresh = p_output_payload->first_fast_refresh;
	p_data->viral_policy_enabled = p_output_payload->viral_policy_enabled;
	p_data->viral_status = p_output_payload->viral_status;
	return rc;
}


int fwcmd_parse_pmon_registers(
	const struct pt_output_pmon_registers *p_output_payload,
	struct fwcmd_pmon_registers_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->pmon_retreive_mask = p_output_payload->pmon_retreive_mask;
	p_data->pmon_0_counter = p_output_payload->pmon_0_counter;
	p_data->pmon_0_control = p_output_payload->pmon_0_control;
	p_data->pmon_1_counter = p_output_payload->pmon_1_counter;
	p_data->pmon_1_control = p_output_payload->pmon_1_control;
	p_data->pmon_2_counter = p_output_payload->pmon_2_counter;
	p_data->pmon_2_control = p_output_payload->pmon_2_control;
	p_data->pmon_3_counter = p_output_payload->pmon_3_counter;
	p_data->pmon_3_control = p_output_payload->pmon_3_control;
	p_data->pmon_4_counter = p_output_payload->pmon_4_counter;
	p_data->pmon_4_control = p_output_payload->pmon_4_control;
	p_data->pmon_5_counter = p_output_payload->pmon_5_counter;
	p_data->pmon_5_control = p_output_payload->pmon_5_control;
	p_data->pmon_6_counter = p_output_payload->pmon_6_counter;
	p_data->pmon_6_control = p_output_payload->pmon_6_control;
	p_data->pmon_7_counter = p_output_payload->pmon_7_counter;
	p_data->pmon_7_control = p_output_payload->pmon_7_control;
	p_data->pmon_8_counter = p_output_payload->pmon_8_counter;
	p_data->pmon_8_control = p_output_payload->pmon_8_control;
	p_data->pmon_9_counter = p_output_payload->pmon_9_counter;
	p_data->pmon_9_control = p_output_payload->pmon_9_control;
	p_data->pmon_10_counter = p_output_payload->pmon_10_counter;
	p_data->pmon_10_control = p_output_payload->pmon_10_control;
	p_data->pmon_11_counter = p_output_payload->pmon_11_counter;
	p_data->pmon_11_control = p_output_payload->pmon_11_control;
	p_data->pmon_14_counter = p_output_payload->pmon_14_counter;
	p_data->pmon_14_control = p_output_payload->pmon_14_control;
	return rc;
}


int fwcmd_parse_system_time(
	const struct pt_output_system_time *p_output_payload,
	struct fwcmd_system_time_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->unix_time = p_output_payload->unix_time;
	return rc;
}


int fwcmd_parse_dimm_info_for_interleave_set(
	const struct pt_output_dimm_info_for_interleave_set *p_output_payload,
	struct fwcmd_dimm_info_for_interleave_set_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->manufacturer_id = p_output_payload->manufacturer_id;
	p_data->serial_number = p_output_payload->serial_number;
	memmove(p_data->model_number, p_output_payload->model_number, 20);
	p_data->model_number[20] = '\0';
	p_data->partition_offset = p_output_payload->partition_offset;
	p_data->partition_size = p_output_payload->partition_size;
	return rc;
}

int fwcmd_parse_dimm_interleave_information(
	const struct pt_output_dimm_interleave_information *p_output_payload,
	struct fwcmd_dimm_interleave_information_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->type = p_output_payload->type;
	p_data->length = p_output_payload->length;
	p_data->index = p_output_payload->index;
	p_data->number_of_dimms = p_output_payload->number_of_dimms;
	p_data->memory_type = p_output_payload->memory_type;
	p_data->format = p_output_payload->format;
	p_data->mirror_enabled = p_output_payload->mirror_enabled;
	p_data->change_status = p_output_payload->change_status;
	p_data->memory_spare = p_output_payload->memory_spare;
// Count Based
	p_data->dimm_info_for_interleave_set = realloc(p_data->dimm_info_for_interleave_set,
				sizeof(struct fwcmd_dimm_info_for_interleave_set_data) * (p_data->dimm_info_for_interleave_set_count + 1));
	unsigned char *base = (unsigned char *) p_output_payload;
	int current_offset = sizeof(*p_output_payload); // start at end of parent payload
	for (int i = 0; i < (int)p_data->number_of_dimms; i++)
	{
		struct pt_output_dimm_info_for_interleave_set *p_sub_payloads =
			((struct pt_output_dimm_info_for_interleave_set *) (base + current_offset));
		fwcmd_parse_dimm_info_for_interleave_set(p_sub_payloads, &p_data->dimm_info_for_interleave_set[p_data->dimm_info_for_interleave_set_count]);
		p_data->dimm_info_for_interleave_set_count++;
		current_offset += (i + 1) * sizeof(struct pt_output_dimm_info_for_interleave_set);
	}
	return rc;
}

int fwcmd_parse_dimm_partition_size_change(
	const struct pt_output_dimm_partition_size_change *p_output_payload,
	struct fwcmd_dimm_partition_size_change_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->type = p_output_payload->type;
	p_data->length = p_output_payload->length;
	p_data->partition_size_change_status = p_output_payload->partition_size_change_status;
	p_data->persistent_memory_partition_size = p_output_payload->persistent_memory_partition_size;
	return rc;
}

int fwcmd_parse_current_config(
	const struct pt_output_current_config *p_output_payload,
	struct fwcmd_current_config_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	memmove(p_data->signature, p_output_payload->signature, 4);
	p_data->signature[4] = '\0';
	p_data->length = p_output_payload->length;
	p_data->revision = p_output_payload->revision;
	p_data->checksum = p_output_payload->checksum;
	memmove(p_data->oem_id, p_output_payload->oem_id, 6);
	p_data->oem_id[6] = '\0';
	memmove(p_data->oem_table_id, p_output_payload->oem_table_id, 8);
	p_data->oem_table_id[8] = '\0';
	p_data->oem_revision = p_output_payload->oem_revision;
	p_data->creator_id = p_output_payload->creator_id;
	p_data->creator_revision = p_output_payload->creator_revision;
	p_data->config_status = p_output_payload->config_status;
	p_data->volatile_memory_size = p_output_payload->volatile_memory_size;
	p_data->persistent_memory_size = p_output_payload->persistent_memory_size;
	// Is Type Based
	unsigned char *base = (unsigned char *) p_output_payload;
	const int type_offset = 0;
	int current_offset = sizeof(*p_output_payload); // start at end of parent payload

	while (current_offset < (int)p_data->length)
	{
		int type_id = *(base + current_offset + type_offset);
		if (type_id == 5) // 5 = dimm_interleave_information
		{
			p_data->dimm_interleave_information = realloc(p_data->dimm_interleave_information,
				sizeof(struct fwcmd_dimm_interleave_information_data) * (p_data->dimm_interleave_information_count + 1));

			struct pt_output_dimm_interleave_information *p_sub_payloads =
				((struct pt_output_dimm_interleave_information *) (base + current_offset));
			struct fwcmd_dimm_interleave_information_data *p_converted = &p_data->dimm_interleave_information[p_data->dimm_interleave_information_count];
			fwcmd_parse_dimm_interleave_information(p_sub_payloads, p_converted);
			p_data->dimm_interleave_information_count++;
			current_offset += p_converted->length;
		}
		else
		{
			rc = FIS_PARSER_CODES_PARSING_TYPE_NOT_FOUND;
			break;
		}
	}
	return rc;
}

int fwcmd_parse_input_config(
	const struct pt_output_input_config *p_output_payload,
	struct fwcmd_input_config_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	memmove(p_data->signature, p_output_payload->signature, 4);
	p_data->signature[4] = '\0';
	p_data->length = p_output_payload->length;
	p_data->revision = p_output_payload->revision;
	p_data->checksum = p_output_payload->checksum;
	memmove(p_data->oem_id, p_output_payload->oem_id, 6);
	p_data->oem_id[6] = '\0';
	memmove(p_data->oem_table_id, p_output_payload->oem_table_id, 8);
	p_data->oem_table_id[8] = '\0';
	p_data->oem_revision = p_output_payload->oem_revision;
	p_data->creator_id = p_output_payload->creator_id;
	p_data->creator_revision = p_output_payload->creator_revision;
	p_data->sequence_number = p_output_payload->sequence_number;
	// Is Type Based
	unsigned char *base = (unsigned char *) p_output_payload;
	const int type_offset = 0;
	int current_offset = sizeof(*p_output_payload); // start at end of parent payload

	while (current_offset < (int)p_data->length)
	{
		int type_id = *(base + current_offset + type_offset);
		if (type_id == 5) // 5 = dimm_interleave_information
		{
			p_data->dimm_interleave_information = realloc(p_data->dimm_interleave_information,
				sizeof(struct fwcmd_dimm_interleave_information_data) * (p_data->dimm_interleave_information_count + 1));

			struct pt_output_dimm_interleave_information *p_sub_payloads =
				((struct pt_output_dimm_interleave_information *) (base + current_offset));
			struct fwcmd_dimm_interleave_information_data *p_converted = &p_data->dimm_interleave_information[p_data->dimm_interleave_information_count];
			fwcmd_parse_dimm_interleave_information(p_sub_payloads, p_converted);
			p_data->dimm_interleave_information_count++;
			current_offset += p_converted->length;
		}
		else if (type_id == 4) // 4 = dimm_partition_size_change
		{
			p_data->dimm_partition_size_change = realloc(p_data->dimm_partition_size_change,
				sizeof(struct fwcmd_dimm_partition_size_change_data) * (p_data->dimm_partition_size_change_count + 1));

			struct pt_output_dimm_partition_size_change *p_sub_payloads =
				((struct pt_output_dimm_partition_size_change *) (base + current_offset));
			struct fwcmd_dimm_partition_size_change_data *p_converted = &p_data->dimm_partition_size_change[p_data->dimm_partition_size_change_count];
			fwcmd_parse_dimm_partition_size_change(p_sub_payloads, p_converted);
			p_data->dimm_partition_size_change_count++;
			current_offset += p_converted->length;
		}
		else
		{
			rc = FIS_PARSER_CODES_PARSING_TYPE_NOT_FOUND;
			break;
		}
	}
	return rc;
}

int fwcmd_parse_output_config(
	const struct pt_output_output_config *p_output_payload,
	struct fwcmd_output_config_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	memmove(p_data->signature, p_output_payload->signature, 4);
	p_data->signature[4] = '\0';
	p_data->length = p_output_payload->length;
	p_data->revision = p_output_payload->revision;
	p_data->checksum = p_output_payload->checksum;
	memmove(p_data->oem_id, p_output_payload->oem_id, 6);
	p_data->oem_id[6] = '\0';
	memmove(p_data->oem_table_id, p_output_payload->oem_table_id, 8);
	p_data->oem_table_id[8] = '\0';
	p_data->oem_revision = p_output_payload->oem_revision;
	p_data->creator_id = p_output_payload->creator_id;
	p_data->creator_revision = p_output_payload->creator_revision;
	p_data->sequence_number = p_output_payload->sequence_number;
	p_data->validation_status = p_output_payload->validation_status;
	// Is Type Based
	unsigned char *base = (unsigned char *) p_output_payload;
	const int type_offset = 0;
	int current_offset = sizeof(*p_output_payload); // start at end of parent payload

	while (current_offset < (int)p_data->length)
	{
		int type_id = *(base + current_offset + type_offset);
		if (type_id == 5) // 5 = dimm_interleave_information
		{
			p_data->dimm_interleave_information = realloc(p_data->dimm_interleave_information,
				sizeof(struct fwcmd_dimm_interleave_information_data) * (p_data->dimm_interleave_information_count + 1));

			struct pt_output_dimm_interleave_information *p_sub_payloads =
				((struct pt_output_dimm_interleave_information *) (base + current_offset));
			struct fwcmd_dimm_interleave_information_data *p_converted = &p_data->dimm_interleave_information[p_data->dimm_interleave_information_count];
			fwcmd_parse_dimm_interleave_information(p_sub_payloads, p_converted);
			p_data->dimm_interleave_information_count++;
			current_offset += p_converted->length;
		}
		else if (type_id == 4) // 4 = dimm_partition_size_change
		{
			p_data->dimm_partition_size_change = realloc(p_data->dimm_partition_size_change,
				sizeof(struct fwcmd_dimm_partition_size_change_data) * (p_data->dimm_partition_size_change_count + 1));

			struct pt_output_dimm_partition_size_change *p_sub_payloads =
				((struct pt_output_dimm_partition_size_change *) (base + current_offset));
			struct fwcmd_dimm_partition_size_change_data *p_converted = &p_data->dimm_partition_size_change[p_data->dimm_partition_size_change_count];
			fwcmd_parse_dimm_partition_size_change(p_sub_payloads, p_converted);
			p_data->dimm_partition_size_change_count++;
			current_offset += p_converted->length;
		}
		else
		{
			rc = FIS_PARSER_CODES_PARSING_TYPE_NOT_FOUND;
			break;
		}
	}
	return rc;
}

int fwcmd_parse_platform_config_data(
	const struct pt_output_platform_config_data *p_output_payload,
	struct fwcmd_platform_config_data_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	memmove(p_data->signature, p_output_payload->signature, 4);
	p_data->signature[4] = '\0';
	p_data->length = p_output_payload->length;
	p_data->revision = p_output_payload->revision;
	p_data->checksum = p_output_payload->checksum;
	memmove(p_data->oem_id, p_output_payload->oem_id, 6);
	p_data->oem_id[6] = '\0';
	memmove(p_data->oem_table_id, p_output_payload->oem_table_id, 8);
	p_data->oem_table_id[8] = '\0';
	p_data->oem_revision = p_output_payload->oem_revision;
	p_data->creator_id = p_output_payload->creator_id;
	p_data->creator_revision = p_output_payload->creator_revision;
	p_data->current_config_size = p_output_payload->current_config_size;
	p_data->current_config_offset = p_output_payload->current_config_offset;
	p_data->input_config_size = p_output_payload->input_config_size;
	p_data->input_config_offset = p_output_payload->input_config_offset;
	p_data->output_config_size = p_output_payload->output_config_size;
	p_data->output_config_offset = p_output_payload->output_config_offset;
	// Is Offset
	if (PARSING_SUCCESS(rc))
	{
		if (p_output_payload->current_config_offset < sizeof(*p_output_payload))
    	{
    		struct pt_output_current_config *p_current_config  =
    			((struct pt_output_current_config *)
    				((unsigned char *) p_output_payload + p_data->current_config_offset));
    		rc = fwcmd_parse_current_config(p_current_config, &(p_data->current_config));
    	}
    	else
    	{
    		rc = FIS_PARSER_CODES_PARSING_WRONG_OFFSET;
    	}
	}

	// Is Offset
	if (PARSING_SUCCESS(rc))
	{
		if (p_output_payload->input_config_offset < sizeof(*p_output_payload))
    	{
    		struct pt_output_input_config *p_input_config  =
    			((struct pt_output_input_config *)
    				((unsigned char *) p_output_payload + p_data->input_config_offset));
    		rc = fwcmd_parse_input_config(p_input_config, &(p_data->input_config));
    	}
    	else
    	{
    		rc = FIS_PARSER_CODES_PARSING_WRONG_OFFSET;
    	}
	}

	// Is Offset
	if (PARSING_SUCCESS(rc))
	{
		if (p_output_payload->output_config_offset < sizeof(*p_output_payload))
    	{
    		struct pt_output_output_config *p_output_config  =
    			((struct pt_output_output_config *)
    				((unsigned char *) p_output_payload + p_data->output_config_offset));
    		rc = fwcmd_parse_output_config(p_output_config, &(p_data->output_config));
    	}
    	else
    	{
    		rc = FIS_PARSER_CODES_PARSING_WRONG_OFFSET;
    	}
	}

	return rc;
}


int fwcmd_parse_dimm_partition_info(
	const struct pt_output_dimm_partition_info *p_output_payload,
	struct fwcmd_dimm_partition_info_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->volatile_capacity = p_output_payload->volatile_capacity;
	p_data->volatile_start = p_output_payload->volatile_start;
	p_data->pm_capacity = p_output_payload->pm_capacity;
	p_data->pm_start = p_output_payload->pm_start;
	p_data->raw_capacity = p_output_payload->raw_capacity;
	p_data->enabled_capacity = p_output_payload->enabled_capacity;
	return rc;
}


int fwcmd_parse_fw_debug_log_level(
	const struct pt_output_fw_debug_log_level *p_output_payload,
	struct fwcmd_fw_debug_log_level_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->log_level = p_output_payload->log_level;
	p_data->logs = p_output_payload->logs;
	return rc;
}


int fwcmd_parse_fw_load_flag(
	const struct pt_output_fw_load_flag *p_output_payload,
	struct fwcmd_fw_load_flag_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->load_flag = p_output_payload->load_flag;
	return rc;
}


int fwcmd_parse_config_lockdown(
	const struct pt_output_config_lockdown *p_output_payload,
	struct fwcmd_config_lockdown_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->locked = p_output_payload->locked;
	return rc;
}


int fwcmd_parse_ddrt_io_init_info(
	const struct pt_output_ddrt_io_init_info *p_output_payload,
	struct fwcmd_ddrt_io_init_info_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->ddrt_io_info = p_output_payload->ddrt_io_info;
	p_data->ddrt_training_complete = p_output_payload->ddrt_training_complete;
	return rc;
}


int fwcmd_parse_get_supported_sku_features(
	const struct pt_output_get_supported_sku_features *p_output_payload,
	struct fwcmd_get_supported_sku_features_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->dimm_sku = p_output_payload->dimm_sku;
	return rc;
}


int fwcmd_parse_enable_dimm(
	const struct pt_output_enable_dimm *p_output_payload,
	struct fwcmd_enable_dimm_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->enable = p_output_payload->enable;
	return rc;
}


int fwcmd_parse_smart_health_info(
	const struct pt_output_smart_health_info *p_output_payload,
	struct fwcmd_smart_health_info_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->validation_flags = p_output_payload->validation_flags;
	p_data->validation_flags_health_status = (unsigned char)((p_data->validation_flags >> 0) & 0x01);
	p_data->validation_flags_spare_blocks = (unsigned char)((p_data->validation_flags >> 1) & 0x01);
	p_data->validation_flags_percent_used = (unsigned char)((p_data->validation_flags >> 2) & 0x01);
	p_data->validation_flags_media_temp = (unsigned char)((p_data->validation_flags >> 3) & 0x01);
	p_data->validation_flags_controller_temp = (unsigned char)((p_data->validation_flags >> 4) & 0x01);
	p_data->validation_flags_unsafe_shutdown_counter = (unsigned char)((p_data->validation_flags >> 5) & 0x01);
	p_data->validation_flags_ait_dram_status = (unsigned char)((p_data->validation_flags >> 6) & 0x01);
	p_data->validation_flags_alarm_trips = (unsigned char)((p_data->validation_flags >> 9) & 0x01);
	p_data->validation_flags_last_shutdown_status = (unsigned char)((p_data->validation_flags >> 10) & 0x01);
	p_data->validation_flags_vendor_specific_data_size = (unsigned char)((p_data->validation_flags >> 11) & 0x01);
	p_data->health_status = p_output_payload->health_status;
	p_data->health_status_normal = (unsigned char)((p_data->health_status >> 0) & 0x01);
	p_data->health_status_noncritical = (unsigned char)((p_data->health_status >> 1) & 0x01);
	p_data->health_status_critical = (unsigned char)((p_data->health_status >> 2) & 0x01);
	p_data->health_status_fatal = (unsigned char)((p_data->health_status >> 3) & 0x01);
	p_data->spare_blocks = p_output_payload->spare_blocks;
	p_data->percent_used = p_output_payload->percent_used;
	p_data->alarm_trips = p_output_payload->alarm_trips;
	p_data->alarm_trips_spare_block_trip = (unsigned char)((p_data->alarm_trips >> 0) & 0x01);
	p_data->alarm_trips_media_temperature_trip = (unsigned char)((p_data->alarm_trips >> 1) & 0x01);
	p_data->alarm_trips_controller_temperature_trip = (unsigned char)((p_data->alarm_trips >> 2) & 0x01);
	p_data->media_temp = p_output_payload->media_temp;
	p_data->controller_temp = p_output_payload->controller_temp;
	p_data->unsafe_shutdown_count = p_output_payload->unsafe_shutdown_count;
	p_data->ait_dram_status = p_output_payload->ait_dram_status;
	p_data->last_shutdown_status = p_output_payload->last_shutdown_status;
	p_data->vendor_specific_data_size = p_output_payload->vendor_specific_data_size;
	p_data->power_cycles = p_output_payload->power_cycles;
	p_data->power_on_time = p_output_payload->power_on_time;
	p_data->uptime = p_output_payload->uptime;
	p_data->unsafe_shutdowns = p_output_payload->unsafe_shutdowns;
	p_data->last_shutdown_status_details = p_output_payload->last_shutdown_status_details;
	p_data->last_shutdown_status_details_pm_adr_command_received = (unsigned char)((p_data->last_shutdown_status_details >> 0) & 0x01);
	p_data->last_shutdown_status_details_pm_s3_received = (unsigned char)((p_data->last_shutdown_status_details >> 1) & 0x01);
	p_data->last_shutdown_status_details_pm_s5_received = (unsigned char)((p_data->last_shutdown_status_details >> 2) & 0x01);
	p_data->last_shutdown_status_details_ddrt_power_fail_command_received = (unsigned char)((p_data->last_shutdown_status_details >> 3) & 0x01);
	p_data->last_shutdown_status_details_pmic_12v_power_fail = (unsigned char)((p_data->last_shutdown_status_details >> 4) & 0x01);
	p_data->last_shutdown_status_details_pm_warm_reset_received = (unsigned char)((p_data->last_shutdown_status_details >> 5) & 0x01);
	p_data->last_shutdown_status_details_thermal_shutdown_received = (unsigned char)((p_data->last_shutdown_status_details >> 6) & 0x01);
	p_data->last_shutdown_status_details_flush_complete = (unsigned char)((p_data->last_shutdown_status_details >> 7) & 0x01);
	p_data->last_shutdown_time = p_output_payload->last_shutdown_time;
	return rc;
}


int fwcmd_parse_firmware_image_info(
	const struct pt_output_firmware_image_info *p_output_payload,
	struct fwcmd_firmware_image_info_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	memmove(p_data->firmware_revision, p_output_payload->firmware_revision, 5);
	p_data->firmware_type = p_output_payload->firmware_type;
	memmove(p_data->staged_fw_revision, p_output_payload->staged_fw_revision, 5);
	p_data->staged_firmware_type = p_output_payload->staged_firmware_type;
	memmove(p_data->commit_id, p_output_payload->commit_id, 40);
	p_data->commit_id[40] = '\0';
	memmove(p_data->build_configuration, p_output_payload->build_configuration, 16);
	p_data->build_configuration[16] = '\0';
	return rc;
}


int fwcmd_parse_firmware_debug_log(
	const struct pt_output_firmware_debug_log *p_output_payload,
	struct fwcmd_firmware_debug_log_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->log_size = p_output_payload->log_size;
	return rc;
}


int fwcmd_parse_long_operation_status(
	const struct pt_output_long_operation_status *p_output_payload,
	struct fwcmd_long_operation_status_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->command = p_output_payload->command;
	p_data->percent_complete = p_output_payload->percent_complete;
	p_data->estimate_time_to_completion = p_output_payload->estimate_time_to_completion;
	p_data->status_code = p_output_payload->status_code;
	memmove(p_data->command_specific_return_data, p_output_payload->command_specific_return_data, 119);
	return rc;
}


int fwcmd_parse_bsr(
	const struct pt_output_bsr *p_output_payload,
	struct fwcmd_bsr_data *p_data)
{
	memset(p_data, 0, sizeof (*p_data));
	int rc = FIS_PARSER_CODES_SUCCESS;
	p_data->major_checkpoint = p_output_payload->major_checkpoint;
	p_data->minor_checkpoint = p_output_payload->minor_checkpoint;
	p_data->rest1 = p_output_payload->rest1;
	p_data->rest1_media_ready_1 = (unsigned char)((p_data->rest1 >> 0) & 0x01);
	p_data->rest1_media_ready_2 = (unsigned char)((p_data->rest1 >> 1) & 0x01);
	p_data->rest1_ddrt_io_init_complete = (unsigned char)((p_data->rest1 >> 2) & 0x01);
	p_data->rest1_pcr_lock = (unsigned char)((p_data->rest1 >> 3) & 0x01);
	p_data->rest1_mailbox_ready = (unsigned char)((p_data->rest1 >> 4) & 0x01);
	p_data->rest1_watch_dog_status = (unsigned char)((p_data->rest1 >> 5) & 0x01);
	p_data->rest1_first_fast_refresh_complete = (unsigned char)((p_data->rest1 >> 6) & 0x01);
	p_data->rest1_credit_ready = (unsigned char)((p_data->rest1 >> 7) & 0x01);
	p_data->rest1_media_disabled = (unsigned char)((p_data->rest1 >> 8) & 0x01);
	p_data->rest1_opt_in_enabled = (unsigned char)((p_data->rest1 >> 9) & 0x01);
	p_data->rest1_opt_in_was_enabled = (unsigned char)((p_data->rest1 >> 10) & 0x01);
	p_data->rest1_assertion = (unsigned char)((p_data->rest1 >> 16) & 0x01);
	p_data->rest1_mi_stall = (unsigned char)((p_data->rest1 >> 17) & 0x01);
	p_data->rest1_ait_dram_ready = (unsigned char)((p_data->rest1 >> 18) & 0x01);
	p_data->rest2 = p_output_payload->rest2;
	return rc;
}


