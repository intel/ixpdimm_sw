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
 * This file contains the implementation of the helper functions for
 * reading/writing the platform configuration data.
 * The platform configuration data is stored on each
 * NVM-DIMM and is the Native API interface with the BIOS.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string/s_str.h>

#include "nvm_types.h"
#include "platform_config_data.h"
#include "utility.h"
#include <os/os_adapter.h>
#include <persistence/logging.h>
#include <file_ops/file_ops_adapter.h>
#include "device_utilities.h"
#include "nvm_context.h"

/*
 * Line format for a single dimm's configuration in a file.
 * SocketID
 * DeviceHandle
 * Capacity (GiB)
 * MemorySize (GiB)
 * AppDirect1Size (GiB)
 * AppDirect1Format
 * AppDirect1Mirrored
 * AppDirect1Index
 * AppDirect2Size (GiB)
 * AppDirect2Format
 * AppDirect2Mirrored
 * AppDirect2Index
 */
#define	config_line_format	"%hu,%u,%llu,%llu,%llu,%u,%hhu,%hu,%llu,%u,%hhu,%hu\n"

/*
 * ****************************************************************************
 * Utility functions
 * ****************************************************************************
 */

void generate_checksum(
		NVM_UINT8 *p_raw_data,
		const NVM_UINT32 length,
		const NVM_UINT32 checksum_offset)
{
	COMMON_LOG_ENTRY();

	NVM_UINT8 checksum = 0;
	for (int i = 0; i < length; i++)
	{
		if (i != checksum_offset) // skip the checksums
		{
			checksum += p_raw_data[i];
		}
	}
	p_raw_data[checksum_offset] = ((0xFF - checksum) + 1);
	COMMON_LOG_EXIT();
}

int verify_checksum(
		const NVM_UINT8 *p_raw_data,
		const NVM_UINT32 length)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_UINT8 sum = 0;
	for (int i = 0; i < length; i++)
	{
		sum += p_raw_data[i];
	}
	if (sum != 0)
	{
		COMMON_LOG_ERROR_F("Table checksum failed, sum = %hhu", sum);
		rc = NVM_ERR_BADDEVICECONFIG;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void print_config_data_table_header(struct config_data_table_header header)
{
	COMMON_LOG_DEBUG_F("Config Data Table Header - Signature: %s", header.signature);
	COMMON_LOG_DEBUG_F("Config Data Table Header - Length: %u", header.length);
	COMMON_LOG_DEBUG_F("Config Data Table Header - Revision: %hhu", header.revision);
	COMMON_LOG_DEBUG_F("Config Data Table Header - Checksum: %hhu", header.checksum);
	COMMON_LOG_DEBUG_F("Config Data Table Header - OEM_ID: %s", header.oem_id);
	COMMON_LOG_DEBUG_F("Config Data Table Header - OEM_TABLE_ID: %s", header.oem_table_id);
	COMMON_LOG_DEBUG_F("Config Data Table Header - OEM_REVISION: %u", header.oem_revision);
	COMMON_LOG_DEBUG_F("Config Data Table Header - Creator Id: %u", header.creator_id);
	COMMON_LOG_DEBUG_F("Config Data Table Header - Creator Revision: %u", header.creator_revision);
}

void print_extension_tables(void *p_ext_table, NVM_SIZE total_length)
{
	struct partition_size_change_extension_table *p_part_size_tbl;
	struct interleave_info_extension_table *p_interleave_info_tbl;
	struct dimm_info_extension_table *p_dimms;
	NVM_UINT64 offset = 0;

	while (offset < total_length)
	{
		struct extension_table_header *p_header =
			(struct extension_table_header *)((NVM_UINT8 *)p_ext_table + offset);

		COMMON_LOG_DEBUG_F("Extension Table - Type: %hu", p_header->type);
		COMMON_LOG_DEBUG_F("Extension Table - Length: %hu", p_header->length);

		switch (p_header->type)
		{
		case PARTITION_CHANGE_TABLE:
			p_part_size_tbl = (struct partition_size_change_extension_table *)p_header;

			COMMON_LOG_DEBUG_F("Partition Change Table - Status: %hu", p_part_size_tbl->status);
			COMMON_LOG_DEBUG_F("Partition Change Table - Size: %llu",
				p_part_size_tbl->partition_size);

			offset += p_part_size_tbl->header.length;

			break;
		case INTERLEAVE_TABLE:
			p_interleave_info_tbl = (struct interleave_info_extension_table *)p_header;

			COMMON_LOG_DEBUG_F("Interleave Info Extension Table - Index: %hu",
				p_interleave_info_tbl->index);
			COMMON_LOG_DEBUG_F("Interleave Info Extension Table - Dimm Count: %hhu",
				p_interleave_info_tbl->dimm_count);
			COMMON_LOG_DEBUG_F("Interleave Info Extension Table - Memory Type: %hhu",
				p_interleave_info_tbl->memory_type);
			COMMON_LOG_DEBUG_F("Interleave Info Extension Table - Interleave Format: %u",
				p_interleave_info_tbl->interleave_format);
			COMMON_LOG_DEBUG_F("Interleave Info Extension Table - Mirror Enable: %hhu",
				p_interleave_info_tbl->mirror_enable);
			COMMON_LOG_DEBUG_F("Interleave Info Extension Table - Status: %hhu",
				p_interleave_info_tbl->status);

			offset += p_interleave_info_tbl->header.length;

			p_dimms = (struct dimm_info_extension_table *)&p_interleave_info_tbl->p_dimms;

			for (int i = 0; i < p_interleave_info_tbl->dimm_count; i++)
			{
				COMMON_LOG_DEBUG_F("DIMM Info Extension Table - Manufacturer: 0x%02x%02x",
					p_dimms[i].manufacturer[1],
					p_dimms[i].manufacturer[0]);
				COMMON_LOG_DEBUG_F(
					"DIMM Info Extension Table - Serial Number: 0x%02x%02x%02x%02x",
					p_dimms[i].serial_number[3],
					p_dimms[i].serial_number[2],
					p_dimms[i].serial_number[1],
					p_dimms[i].serial_number[0]);
				COMMON_LOG_DEBUG_F("DIMM Info Extension Table - Model Number: %s",
					p_dimms[i].model_number);
				COMMON_LOG_DEBUG_F("DIMM Info Extension Table - Offset: %llu", p_dimms[i].offset);
				COMMON_LOG_DEBUG_F("DIMM Info Extension Table - Size: %llu", p_dimms[i].size);
			}
			break;
		default:
			COMMON_LOG_DEBUG("Unknown Extension Table");
			offset = -1;
			break;
		}
	}
}

void print_pcd_output(struct config_output_table *p_output)
{
	print_config_data_table_header(p_output->header);
	COMMON_LOG_DEBUG_F("Config Output Table - Sequence Number: %u", p_output->sequence_number);
	COMMON_LOG_DEBUG_F("Config Output Table - Validation Status: %hhu",
		p_output->validation_status);

	if (p_output->header.length > sizeof (struct config_output_table))
	{
		print_extension_tables(&p_output->p_ext_tables,
			(p_output->header.length - sizeof (struct config_output_table)));
	}
}

/*
 * Write the data in the structure back to the specified database.
 */
int check_config_output(struct platform_config_data *p_config)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// config output table may not be present
	if (p_config->config_output_size > 0)
	{
		// check the minimum size
		if (p_config->config_output_size < sizeof (struct config_output_table))
		{
			COMMON_LOG_ERROR(
					"Config output table size is too small");
			rc = NVM_ERR_BADDEVICECONFIG;
		}
		else
		{
			struct config_output_table *p_output = (struct config_output_table *)
					((NVM_UINT8 *)p_config + p_config->config_output_offset);

			print_pcd_output(p_output);
			// check the checksum
			rc = verify_checksum((NVM_UINT8*)p_config + p_config->config_output_offset,
					p_output->header.length);
			if (rc == NVM_SUCCESS)
			{
				// check the table signature
				if (strncmp(CONFIG_OUTPUT_TABLE_SIGNATURE,
						p_output->header.signature, SIGNATURE_LEN) != 0)
				{
					COMMON_LOG_ERROR_F(
							"Config output header signature mismatch. Expected: %s, actual: %s",
							CONFIG_OUTPUT_TABLE_SIGNATURE, p_output->header.signature);
					rc = NVM_ERR_BADDEVICECONFIG;
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void print_pcd_input(struct config_input_table *p_input)
{
	print_config_data_table_header(p_input->header);
	COMMON_LOG_DEBUG_F("Config Input Table - Sequence_number: %u", p_input->sequence_number);

	if (p_input->header.length > sizeof (struct config_input_table))
	{
		print_extension_tables(&p_input->p_ext_tables,
			(p_input->header.length - sizeof (struct config_input_table)));
	}
}

int check_config_input(struct platform_config_data *p_config)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// current config table may not be present
	if (p_config->config_input_size > 0)
	{
		// check the minimum size
		if (p_config->config_input_size < sizeof (struct config_input_table))
		{
			COMMON_LOG_ERROR(
					"Config input table size is too small");
			rc = NVM_ERR_BADDEVICECONFIG;
		}
		else
		{
			struct config_input_table *p_input = (struct config_input_table *)
					((NVM_UINT8 *)p_config + p_config->config_input_offset);

			print_pcd_input(p_input);
			// length should match header
			if (p_config->config_input_size != p_input->header.length)
			{
				COMMON_LOG_ERROR(
					"Config input length doesn't match header length");
				rc = NVM_ERR_BADDEVICECONFIG;
			}
			else if ((rc = verify_checksum((NVM_UINT8*)p_config + p_config->config_input_offset,
				p_input->header.length)) == NVM_SUCCESS)
			{
				// check the table signature
				if (strncmp(CONFIG_INPUT_TABLE_SIGNATURE,
						p_input->header.signature, SIGNATURE_LEN) != 0)
				{
					COMMON_LOG_ERROR_F(
							"Config input header signature mismatch. Expected: %s, actual: %s",
							CONFIG_INPUT_TABLE_SIGNATURE, p_input->header.signature);
					rc = NVM_ERR_BADDEVICECONFIG;
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void print_pcd_current(struct current_config_table *p_current)
{
	print_config_data_table_header(p_current->header);
	COMMON_LOG_DEBUG_F("Current Config Table - Config Status: %hu", p_current->config_status);
	COMMON_LOG_DEBUG_F("Current Config Table - Config Mapped Memory: %llu",
		p_current->mapped_memory_capacity);
	COMMON_LOG_DEBUG_F("Current Config Table - Config Mapped App Direct: %llu",
		p_current->mapped_app_direct_capacity);

	if (p_current->header.length > sizeof (struct current_config_table))
	{
		print_extension_tables(&p_current->p_ext_tables,
			(p_current->header.length - sizeof (struct current_config_table)));
	}
}

int check_current_config(struct platform_config_data *p_config)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// current config table may not be present
	if (p_config->current_config_size > 0)
	{
		// check the minimum size
		if (p_config->current_config_size < sizeof (struct current_config_table))
		{
			COMMON_LOG_ERROR(
					"Current config table size is too small");
			rc = NVM_ERR_BADDEVICECONFIG;
		}
		else
		{
			struct current_config_table *p_current = (struct current_config_table *)
					((NVM_UINT8 *)p_config + p_config->current_config_offset);

			print_pcd_current(p_current);
			// check the checksum
			rc = verify_checksum((NVM_UINT8*)p_config + p_config->current_config_offset,
					p_current->header.length);
			if (rc == NVM_SUCCESS)
			{
				// check the table signature
				if (strncmp(CURRENT_CONFIG_TABLE_SIGNATURE,
						p_current->header.signature, SIGNATURE_LEN) != 0)
				{
					COMMON_LOG_ERROR_F(
							"Current config signature mismatch. Expected: %s, actual: %s",
							CURRENT_CONFIG_TABLE_SIGNATURE, p_current->header.signature);
					rc = NVM_ERR_BADDEVICECONFIG;
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void print_pcd_header(struct platform_config_data *p_config)
{
	print_config_data_table_header(p_config->header);
	COMMON_LOG_DEBUG_F("PCD Header - Current Config Size: %u", p_config->current_config_size);
	COMMON_LOG_DEBUG_F("PCD Header - Current Config Offset: %u", p_config->current_config_offset);
	COMMON_LOG_DEBUG_F("PCD Header - Input Config Size: %u", p_config->config_input_size);
	COMMON_LOG_DEBUG_F("PCD Header - Input Config Offset: %u", p_config->config_input_offset);
	COMMON_LOG_DEBUG_F("PCD Header - Output Config Size: %u", p_config->config_output_size);
	COMMON_LOG_DEBUG_F("PCD Header - Output Config Offset: %u", p_config->config_output_offset);
}

int check_platform_config_header(struct platform_config_data *p_config)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// check overall table length is at least as big as the header
	if (p_config->header.length < sizeof (struct platform_config_data))
	{
		COMMON_LOG_ERROR(
				"Platform config data size is too small");
		rc = NVM_ERR_BADDEVICECONFIG;
	}
	else
	{
		print_pcd_header(p_config);
		// TODO: US8549 BIOS Currently does not implement this checksum correctly.
		generate_checksum((NVM_UINT8*)p_config, p_config->header.length, CHECKSUM_OFFSET);
		// check overall table checksum
		rc = verify_checksum((NVM_UINT8*)p_config, p_config->header.length);
		if (rc == NVM_SUCCESS)
		{
			// check the table signature
			if (strncmp(PLATFORM_CONFIG_TABLE_SIGNATURE,
					p_config->header.signature, SIGNATURE_LEN) != 0)
			{
				COMMON_LOG_ERROR_F(
						"Platform config header signature mismatch. Expected: %s, actual: %s",
						PLATFORM_CONFIG_TABLE_SIGNATURE, p_config->header.signature);
				rc = NVM_ERR_BADDEVICECONFIG;
			}
		}
		else
		{
			COMMON_LOG_ERROR("Checksum failure for DMHD");
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int check_platform_config(struct platform_config_data *p_config)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!p_config)
	{
		COMMON_LOG_ERROR("p_config is NULL");
		rc = NVM_ERR_BADDEVICECONFIG;
	}
	else if ((rc = check_platform_config_header(p_config)) == NVM_SUCCESS)
	{
		// check current config table
		if ((rc = check_current_config(p_config)) == NVM_SUCCESS)
		{
			// check config input table
			if ((rc = check_config_input(p_config)) == NVM_SUCCESS)
			{
				// check config output table
				rc = check_config_output(p_config);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Create a platform config data table built from the three input tables.
 *
 * NOTE: Caller must free the platform_config_data structure.
 */
int build_platform_config_data(const struct current_config_table *p_current_config,
		const struct config_input_table *p_config_input,
		const struct config_output_table *p_config_output,
		struct platform_config_data **pp_config)
{
	int rc = NVM_SUCCESS;

	const NVM_UINT32 current_cfg_size = p_current_config ?
			p_current_config->header.length : 0;
	const NVM_UINT32 input_size = p_config_input ?
			p_config_input->header.length : 0;
	const NVM_UINT32 output_size = p_config_output ?
			p_config_output->header.length : 0;

	NVM_UINT32 offset = sizeof (struct platform_config_data); // offset for first sub-table

	struct platform_config_data *p_cfg_data = malloc(DEV_PLT_CFG_PART_SIZE);
	if (p_cfg_data)
	{
		memset(p_cfg_data, 0, DEV_PLT_CFG_PART_SIZE);

		// Fill out the header
		p_cfg_data->header.length = sizeof (struct platform_config_data);
		memmove(p_cfg_data->header.signature, PLATFORM_CONFIG_TABLE_SIGNATURE,
				SIGNATURE_LEN);

		p_cfg_data->header.revision = 1;
		memmove(&p_cfg_data->header.oem_id, "INTEL ", OEM_ID_LEN);
		memmove(&p_cfg_data->header.oem_table_id, "PURLEY  ", OEM_TABLE_ID_LEN);
		p_cfg_data->header.oem_revision = 2;
		memmove(&p_cfg_data->header.creator_id, "INTL", sizeof (NVM_UINT32));
		p_cfg_data->header.creator_revision = 0;

		// Fill out sub-tables
		if (current_cfg_size > 0)
		{
			p_cfg_data->current_config_offset = offset;
			p_cfg_data->current_config_size = current_cfg_size;

			// Copy over current config
			struct current_config_table *p_current_final = cast_current_config(p_cfg_data);
			if (p_current_final)
			{
				memmove(p_current_final, p_current_config, current_cfg_size);
			}
			else // shouldn't get here
			{
				COMMON_LOG_ERROR("Failed to get current config table ptr from platform cfg");
				rc = NVM_ERR_UNKNOWN;
			}

			offset += current_cfg_size;
		}

		if ((input_size > 0) && (rc == NVM_SUCCESS))
		{
			p_cfg_data->config_input_offset = offset;
			p_cfg_data->config_input_size = input_size;

			// Copy over config input
			struct config_input_table *p_input_final = cast_config_input(p_cfg_data);
			if (p_input_final)
			{
				memmove(p_input_final, p_config_input, input_size);
			}
			else // shouldn't get here
			{
				COMMON_LOG_ERROR("Failed to get config input table ptr from platform cfg");
				rc = NVM_ERR_UNKNOWN;
			}

			offset += input_size;
		}

		if ((output_size > 0) && (rc == NVM_SUCCESS))
		{
			p_cfg_data->config_output_offset = offset;
			p_cfg_data->config_output_size = output_size;

			// Copy over config output
			struct config_output_table *p_output_final = cast_config_output(p_cfg_data);
			if (p_output_final)
			{
				memmove(p_output_final, p_config_output, output_size);
			}
			else // shouldn't get here
			{
				COMMON_LOG_ERROR("Failed to get config output table ptr from platform cfg");
				rc = NVM_ERR_UNKNOWN;
			}

			offset += output_size;
		}

		// Successfully built up the table
		if (rc == NVM_SUCCESS)
		{
			// Checksum for platform config data
			generate_checksum((NVM_UINT8*)p_cfg_data, p_cfg_data->header.length,
					CHECKSUM_OFFSET);

			// If we are able to pass back what we created...
			if (pp_config && (*pp_config == NULL))
			{
				*pp_config = p_cfg_data;
			}
			else
			{
				COMMON_LOG_ERROR("Caller passed in bad pointer for platform_config_data");
				free(p_cfg_data);
				rc = NVM_ERR_INVALIDPARAMETER;
			}
		}
	}
	else
	{
		rc = NVM_ERR_NOMEMORY;
	}

	return rc;
}


/*
 * ****************************************************************************
 * Implementation of device_adapter interface for platform config data
 * ****************************************************************************
 */

int get_pcd_table_size(const unsigned int handle, NVM_SIZE *pcd_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	*pcd_size = 0;

	// All the information for pcd table size is in the first 128B
	struct fw_cmd cfg_cmd;
	memset(&cfg_cmd, 0, sizeof (cfg_cmd));
	cfg_cmd.device_handle = handle;
	cfg_cmd.opcode = PT_GET_ADMIN_FEATURES;
	cfg_cmd.sub_opcode = SUBOP_PLATFORM_DATA_INFO;

	struct pt_payload_get_platform_cfg_data cfg_input;
	memset(&cfg_input, 0, sizeof (cfg_input));
	char out_buf[DEV_SMALL_PAYLOAD_SIZE];
	NVM_UINT32 offset = 0;
	cfg_input.partition_id = DEV_OS_PARTITION;
	cfg_input.options = DEV_PLT_CFG_OPT_SMALL_DATA;
	cfg_input.offset = offset;
	cfg_cmd.input_payload_size = sizeof (cfg_input);
	cfg_cmd.input_payload = &cfg_input;
	cfg_cmd.output_payload_size = DEV_SMALL_PAYLOAD_SIZE;
	cfg_cmd.output_payload = &out_buf;
	if ((rc = ioctl_passthrough_cmd(&cfg_cmd)) == NVM_SUCCESS)
	{
		struct platform_config_data *tmp_pcd = (struct platform_config_data *)out_buf;

		*pcd_size = tmp_pcd->header.length + tmp_pcd->config_input_size
			+ tmp_pcd->current_config_size + tmp_pcd->config_output_size;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve and populate the platform config data structure
 * NOTE: Callers must free the platform_config_data structure by
 * calling free_platform_config!
 */
int get_hw_dimm_platform_config_alloc(const unsigned int handle, NVM_SIZE *p_pcd_size,
		void **pp_config)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_SIZE tmp_pcd_size;
	if ((rc = get_pcd_table_size(handle, &tmp_pcd_size)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve pcd table size");
	}
	else if (tmp_pcd_size == 0)
	{
		rc = build_platform_config_data(NULL, NULL, NULL,
				(struct platform_config_data **)pp_config);
	}
	else if (tmp_pcd_size < sizeof (struct platform_config_data) ||
			tmp_pcd_size > DEV_PLT_CFG_PART_SIZE)
	{
		COMMON_LOG_ERROR("PCD table size is invalid");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		*pp_config = calloc(1, tmp_pcd_size);

		if (*pp_config != NULL)
		{
			*p_pcd_size = tmp_pcd_size;
#if __LARGE_PAYLOAD__
			NVM_UINT8 *large_buffer = calloc(1, DEV_PLT_CFG_PART_SIZE);
			if (large_buffer != NULL)
			{
				// Use Large Payload to retrieve PCD
				struct fw_cmd cfg_cmd;
				memset(&cfg_cmd, 0, sizeof (cfg_cmd));
				cfg_cmd.device_handle = handle;
				cfg_cmd.opcode = PT_GET_ADMIN_FEATURES;
				cfg_cmd.sub_opcode = SUBOP_PLATFORM_DATA_INFO;

				struct pt_payload_get_platform_cfg_data cfg_input;
				memset(&cfg_input, 0, sizeof (cfg_input));
				cfg_input.partition_id = DEV_OS_PARTITION;
				cfg_input.options = DEV_PLT_CFG_OPT_LARGE_DATA;
				cfg_cmd.input_payload_size = sizeof (cfg_input);
				cfg_cmd.input_payload = &cfg_input;
				cfg_cmd.large_output_payload_size = DEV_PLT_CFG_PART_SIZE;
				cfg_cmd.large_output_payload = large_buffer;

				if ((rc = ioctl_passthrough_cmd(&cfg_cmd)) == NVM_SUCCESS)
				{
					memmove(*pp_config, large_buffer, *p_pcd_size);
				}

				free(large_buffer);
			}
			else
			{
				rc = NVM_ERR_NOMEMORY;
			}
#else
			// Use Small Payload to retrieve PCD
			struct fw_cmd cfg_cmd;
			memset(&cfg_cmd, 0, sizeof (cfg_cmd));
			cfg_cmd.device_handle = handle;
			cfg_cmd.opcode = PT_GET_ADMIN_FEATURES;
			cfg_cmd.sub_opcode = SUBOP_PLATFORM_DATA_INFO;

			struct pt_payload_get_platform_cfg_data cfg_input;
			memset(&cfg_input, 0, sizeof (cfg_input));
			char out_buf[DEV_SMALL_PAYLOAD_SIZE];
			NVM_UINT32 offset = 0;
			cfg_input.partition_id = DEV_OS_PARTITION;
			cfg_input.options = DEV_PLT_CFG_OPT_SMALL_DATA;
			cfg_input.offset = offset;
			cfg_cmd.input_payload_size = sizeof (cfg_input);
			cfg_cmd.input_payload = &cfg_input;
			cfg_cmd.output_payload_size = DEV_SMALL_PAYLOAD_SIZE;
			cfg_cmd.output_payload = &out_buf;

			while (offset < *p_pcd_size && rc == NVM_SUCCESS)
			{
				if (offset > (DEV_PLT_CFG_PART_SIZE - DEV_SMALL_PAYLOAD_SIZE))
				{
					rc = NVM_ERR_UNKNOWN;
					COMMON_LOG_ERROR("Trying to read outside PCD Partition");
					break;
				}
				memset(&out_buf, 0, DEV_SMALL_PAYLOAD_SIZE);
				cfg_input.offset = offset;

				if ((rc = ioctl_passthrough_cmd(&cfg_cmd)) == NVM_SUCCESS)
				{
					NVM_SIZE transfer_size = DEV_SMALL_PAYLOAD_SIZE;

					if ((offset + DEV_SMALL_PAYLOAD_SIZE) > *p_pcd_size)
					{
						transfer_size = *p_pcd_size - offset;
					}
					memmove((void *)*pp_config + offset, out_buf, transfer_size);
					offset += DEV_SMALL_PAYLOAD_SIZE;
				}
			}
#endif
		}
		else
		{
			rc = NVM_ERR_NOMEMORY;
		}

		// validate the returned data
		if (rc == NVM_SUCCESS)
		{
			rc = check_platform_config(*pp_config);
		}
		else
		{
			free(*pp_config);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve a copy of the platform config data from the global device table
 * NOTE: Callers must free the platform_config_data structure by
 * calling free_platform_config!
 */
int get_dimm_platform_config(const NVM_NFIT_DEVICE_HANDLE handle,
		struct platform_config_data **pp_config)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_SIZE pcd_size = 0;
	// get the UID from the handle
	struct device_discovery discovery;
	if ((rc = lookup_dev_handle(handle, &discovery)) == NVM_SUCCESS)
	{
		// look up pcd in context
		if (get_nvm_context_device_pcd(discovery.uid, pp_config, &pcd_size) != NVM_SUCCESS)
		{
			rc = get_hw_dimm_platform_config_alloc(handle.handle, &pcd_size, (void **)pp_config);
			if (rc == NVM_SUCCESS && *pp_config)
			{
				set_nvm_context_device_pcd(discovery.uid, *pp_config, pcd_size);
			}
		}
	}

	if (*pp_config == NULL)
	{
		COMMON_LOG_ERROR("PCD data is empty");
		rc = NVM_ERR_BADDEVICECONFIG;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Write the platform config data stored in the structure
 * to the dimm
 */
int set_dimm_platform_config(const NVM_NFIT_DEVICE_HANDLE handle,
		const struct platform_config_data *p_config)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_SIZE pcd_size = p_config->header.length + p_config->config_input_size
		+ p_config->config_output_size + p_config->current_config_size;

#if __LARGE_PAYLOAD__
	char *tmp_buffer = calloc(1, DEV_PLT_CFG_PART_SIZE);
	if (tmp_buffer != NULL)
	{
		memmove(tmp_buffer, p_config, pcd_size);

		// write the binary data to the dimm through large payload
		struct fw_cmd cfg_cmd;
		memset(&cfg_cmd, 0, sizeof (cfg_cmd));
		cfg_cmd.device_handle = handle.handle;
		cfg_cmd.opcode = PT_SET_ADMIN_FEATURES;
		cfg_cmd.sub_opcode = SUBOP_PLATFORM_DATA_INFO;

		struct pt_payload_set_platform_cfg_data cfg_input;
		memset(&cfg_input, 0, sizeof (cfg_input));
		cfg_input.partition_id = DEV_OS_PARTITION;
		cfg_input.payload_type = DEV_PLT_CFG_LARGE_PAY;
		cfg_cmd.input_payload_size = sizeof (cfg_input);
		cfg_cmd.input_payload = &cfg_input;
		cfg_cmd.large_input_payload_size = DEV_PLT_CFG_PART_SIZE;
		cfg_cmd.large_input_payload = tmp_buffer;

		rc = ioctl_passthrough_cmd(&cfg_cmd);
		free(tmp_buffer);
	}
	else
	{
		rc = NVM_ERR_NOMEMORY;
	}
#else
	struct fw_cmd cfg_cmd;
	memset(&cfg_cmd, 0, sizeof (cfg_cmd));
	cfg_cmd.device_handle = handle.handle;
	cfg_cmd.opcode = PT_SET_ADMIN_FEATURES;
	cfg_cmd.sub_opcode = SUBOP_PLATFORM_DATA_INFO;

	struct pt_payload_set_platform_cfg_data cfg_input;
	memset(&cfg_input, 0, sizeof (cfg_input));
	cfg_input.partition_id = DEV_OS_PARTITION;
	cfg_input.payload_type = DEV_PLT_CFG_SMALL_PAY;
	cfg_cmd.input_payload_size = sizeof (cfg_input);
	cfg_cmd.input_payload = &cfg_input;

	NVM_SIZE offset = 0;

	while (offset < pcd_size && rc == NVM_SUCCESS)
	{
		if (offset > (DEV_PLT_CFG_PART_SIZE - DEV_PLT_CFG_SMALL_PAYLOAD_WRITE_SIZE))
		{
			rc = NVM_ERR_UNKNOWN;
			COMMON_LOG_ERROR("Trying to write outside PCD Partition");
			break;
		}

		memmove(cfg_input.data, (void *)p_config + offset, DEV_PLT_CFG_SMALL_PAYLOAD_WRITE_SIZE);
		cfg_input.offset = offset;

		if ((rc = ioctl_passthrough_cmd(&cfg_cmd)) == NVM_SUCCESS)
		{
			offset += DEV_PLT_CFG_SMALL_PAYLOAD_WRITE_SIZE;
		}
	}
#endif

	if (rc == NVM_SUCCESS)
	{
		// invalidate context
		struct device_discovery discovery;
		if ((rc = lookup_dev_handle(handle, &discovery)) == NVM_SUCCESS)
		{
			invalidate_device_pcd(discovery.uid);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * Returns the sequence number from the config output table in the platform_config_data.
 * This represents the last BIOS response to us -- therefore the last sequence number
 * it has seen.
 * If there is no config output table, returns 0.
 */
NVM_UINT32 get_last_config_output_sequence_number(struct platform_config_data *p_config)
{
	NVM_UINT32 seq_num = 0;

	struct config_output_table *p_config_output = cast_config_output(p_config);
	if (p_config_output) // config output table exists
	{
		seq_num = p_config_output->sequence_number;
	}

	return seq_num;
}

/*
 * Calculate the next sequence number for a DIMM based on existing platform config.
 */
int get_next_config_input_sequence_number(const struct device_discovery *p_discovery,
		NVM_UINT32 *p_seq_num)
{
	int rc = NVM_SUCCESS;

	struct platform_config_data *p_config = NULL;
	rc = get_dimm_platform_config(p_discovery->device_handle, &p_config);
	if (rc == NVM_SUCCESS)
	{
		if (p_seq_num)
		{
			*p_seq_num = get_last_config_output_sequence_number(p_config) + 1;
		}
		free(p_config);
	}

	return rc;
}


/*
 * Write the dimm config to the specified file
 */
int write_dimm_config(const struct device_discovery *p_discovery,
		const struct config_goal *p_goal,
		const NVM_PATH path, const NVM_SIZE path_len, const NVM_BOOL append)
{
	int rc = NVM_SUCCESS;

	FILE *p_file;
	// set the mode
	char mode[2];
	if (append)
	{
		s_strcpy(mode, "a", sizeof (mode));
	}
	else
	{
		s_strcpy(mode, "w", sizeof (mode));
	}

	if ((p_file = open_file(path, path_len, mode)) == NULL)
	{
		COMMON_LOG_ERROR_F("Failed to open file %s", path);
		rc = NVM_ERR_BADFILE;
	}
	else
	{
		if (lock_file(p_file, FILE_LOCK_MODE_WRITE) != COMMON_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to lock file %s for writing", path);
			rc = NVM_ERR_BADFILE;
		}
		else
		{
			// write the header
			if (!append)
			{
				fprintf(p_file,
						"#SocketID,"
						"DimmHandle,"
						"Capacity,"
						"MemorySize,"
						"AppDirect1Size,"
						"AppDirect1Format,"
						"AppDirect1Mirrored,"
						"AppDirect1Index,"
						"AppDirect2Size,"
						"AppDirect2Format,"
						"AppDirect2Mirrored,"
						"AppDirect2Index\n");
			}

			// DIMM capacity to GiB
			NVM_UINT64 dimm_size_gb = (p_discovery->capacity / BYTES_PER_GB);

			// convert interleave format structs to number
			NVM_UINT32 p1_format = 0;
			NVM_UINT32 p2_format = 0;
			interleave_struct_to_format(&p_goal->app_direct_1_settings.interleave,
					&p1_format);
			interleave_struct_to_format(&p_goal->app_direct_2_settings.interleave,
					&p2_format);

			// write to the file
			fprintf(p_file, config_line_format,
					p_discovery->socket_id,
					p_discovery->device_handle.handle,
					dimm_size_gb,
					p_goal->memory_size,
					p_goal->app_direct_1_size,
					p1_format,
					p_goal->app_direct_1_settings.mirrored,
					p_goal->app_direct_1_set_id,
					p_goal->app_direct_2_size,
					p2_format,
					p_goal->app_direct_2_settings.mirrored,
					p_goal->app_direct_2_set_id);

			lock_file(p_file, FILE_LOCK_MODE_UNLOCK);
		}
		fclose(p_file);
	}
	return rc;
}


/*
 * Fetch the highest-numbered interleave set index from the current config and any existing
 * config goals on a DIMM. The interleave set index is a unique number identifying the interleave
 * set.
 * Returns NVM_SUCCESS if it was able to successfully retrieve the highest set index.
 * If successful, the maximum set index is returned in *p_set_index.
 * If there are no interleave sets in current config or goal, *p_set_index will be 0.
 */
int get_dimm_interleave_info_max_set_index(const NVM_UID device_uid,
		NVM_UINT32 *p_set_index)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct device_discovery discovery;

	if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		// Fetch the platform config tables for the DIMM
		struct platform_config_data *p_config = NULL;
		rc = get_dimm_platform_config(discovery.device_handle, &p_config);
		*p_set_index = 0;

		if (rc == NVM_SUCCESS) // Got the platform config
		{
			// Inspect the config input table -- i.e. config goals
			if (p_config->config_input_size > 0)
			{
				struct config_input_table *p_input =
						(struct config_input_table *)((char *)p_config +
								p_config->config_input_offset);
				// has extension table(s)
				if (p_input->header.length > sizeof (struct config_input_table))
				{
					NVM_UINT32 offset = 0;
					char *p_extensions = (char *)&(p_input->p_ext_tables);

					// Walk through extension tables
					while (offset < (p_input->header.length - sizeof (struct config_input_table)) &&
							rc == NVM_SUCCESS)
					{
						struct extension_table_header *p_header =
								(struct extension_table_header *)((char *)p_extensions + offset);
						if (p_header->length == 0)
						{
							COMMON_LOG_ERROR("Expected extension table has length 0");
							rc = NVM_ERR_UNKNOWN;
							break; // can't progress any further down extension table list
						}
						else // valid extension table - inspect it
						{
							if (p_header->type == INTERLEAVE_TABLE)
							{
								// This is an interleave set table - inspect its index
								struct interleave_info_extension_table *p_interleave =
										(struct interleave_info_extension_table *)
										((char *)p_extensions + offset);

								*p_set_index = (*p_set_index < p_interleave->index) ?
										p_interleave->index : *p_set_index;
							}
							// offset to next config input extension table
							offset += p_header->length;
						}
					}

				}

			}

			// Inspect the current config table -- used to build the current pools
			if (p_config->current_config_size > 0)
			{
				struct current_config_table *p_current =
						(struct current_config_table *)((char *)p_config +
								p_config->current_config_offset);
				// has extension table(s)
				if (p_current->header.length > sizeof (struct current_config_table))
				{
					NVM_UINT32 offset = 0;
					char *p_extensions = (char *)&(p_current->p_ext_tables);

					// Walk through the extension tables
					while (offset < (p_current->header.length -
						sizeof (struct current_config_table)) && rc == NVM_SUCCESS)
					{
						struct extension_table_header *p_header =
								(struct extension_table_header *)(((char *)p_extensions) + offset);
						if (p_header->length == 0)
						{
							COMMON_LOG_ERROR("Expected extension table has length 0");
							rc = NVM_ERR_UNKNOWN;
							break; // can't progress any further
						}
						else // valid extension table - inspect it
						{
							if (p_header->type == INTERLEAVE_TABLE)
							{
								// This is an interleave set table - inspect its index
								struct interleave_info_extension_table *p_interleave =
										(struct interleave_info_extension_table *)
										(((char *)p_extensions) + offset);

								*p_set_index =  (*p_set_index < p_interleave->index) ?
										p_interleave->index : *p_set_index;
							}
							// offset to next current config extension table
							offset += p_header->length;
						}
					}
				}
			}
		}
		if (p_config)
		{
			free(p_config);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

enum config_error get_config_error_from_config_output(struct config_output_table *p_config_output)
{
	enum config_error status = CONFIG_ERROR_NOTFOUND;

	// Walk through the extension tables to get more specific error
	// break on first non-unknown error found
	NVM_UINT64 offset = 0;
	NVM_SIZE table_length = p_config_output->header.length - sizeof (struct config_output_table);
	while (offset < table_length && status == CONFIG_ERROR_NOTFOUND)
	{
		struct extension_table_header *p_header =
				(struct extension_table_header *)((NVM_UINT8 *)
					&p_config_output->p_ext_tables + offset);
		// Bad data
		if (p_header->length <= sizeof (struct extension_table_header))
		{
			status = CONFIG_ERROR_BADREQUEST;
			break;
		}
		// go to next extension table
		offset += p_header->length;

		// check partition change table status
		if (p_header->type == PARTITION_CHANGE_TABLE)
		{
			struct partition_size_change_extension_table *p_table =
					(struct partition_size_change_extension_table *)
							((NVM_UINT8 *)p_header);
			switch (p_table->status)
			{
				case PARTITION_SIZE_CHANGE_STATUS_DIMMS_NOT_FOUND:
					status = CONFIG_ERROR_BROKENINTERLEAVE;
					break;
				case PARTITION_SIZE_CHANGE_STATUS_DIMM_INTERLEAVE_INFO_BAD:
				case PARTITION_SIZE_CHANGE_STATUS_BAD_ALIGNMENT:
					status = CONFIG_ERROR_BADREQUEST;
					break;
				case PARTITION_SIZE_CHANGE_STATUS_SIZE_TOO_BIG:
				case PARTITION_SIZE_CHANGE_STATUS_NOT_ENOUGH_DRAM_DECODERS:
					status = CONFIG_ERROR_INSUFFICIENTRESOURCES;
					break;
				case PARTITION_SIZE_CHANGE_STATUS_FW_ERROR:
					status = CONFIG_ERROR_FW;
					break;
				case PARTITION_SIZE_CHANGE_STATUS_SUCCESS:
					// ignore
					break;
				case PARTITION_SIZE_CHANGE_STATUS_UNDEFINED:
				default:
					status = CONFIG_ERROR_UNKNOWN;
					break;
			}
		}
		// check interleave set change status
		else if (p_header->type == INTERLEAVE_TABLE)
		{
			struct interleave_info_extension_table *p_table =
					(struct interleave_info_extension_table *)
							((NVM_UINT8 *)p_header);
			switch (p_table->status)
			{
				case INTERLEAVE_STATUS_DIMMS_NOT_FOUND:
					status = CONFIG_ERROR_BROKENINTERLEAVE;
					break;
				case INTERLEAVE_STATUS_DIMM_INTERLEAVE_INFO_BAD:
				case INTERLEAVE_STATUS_DIMM_INTERLEAVE_MISSING:
				case INTERLEAVE_STATUS_CHANNEL_INTERLEAVE_MISMATCH:
				case INTERLEAVE_STATUS_BAD_ALIGNMENT:
					status = CONFIG_ERROR_BADREQUEST;
					break;
				case INTERLEAVE_STATUS_NOT_ENOUGH_DRAM_DECODERS:
				case INTERLEAVE_STATUS_NOT_ENOUGH_SPA_SPACE:
				case INTERLEAVE_STATUS_UNAVAILABLE_RESOURCES:
					status = CONFIG_ERROR_INSUFFICIENTRESOURCES;
					break;
				case INTERLEAVE_STATUS_NOT_PROCESSED:
				case INTERLEAVE_STATUS_SUCCESS:
				case INTERLEAVE_STATUS_PARTITIONING_FAILED:
					// Ignore
					break;
				default:
					status = CONFIG_ERROR_UNKNOWN;
					break;
			}
		}
	} // end for each extension table

	return status;
}

enum config_status get_config_status_from_current_config(
		struct current_config_table *p_current_config)
{
	enum config_status status = CONFIG_STATUS_ERR_NOT_SUPPORTED;

	switch (p_current_config->config_status)
	{
		case CURRENT_CONFIG_STATUS_SUCCESS:
			status = CONFIG_STATUS_VALID;
			break;

		case CURRENT_CONFIG_STATUS_DIMMS_NOT_FOUND:
			status = CONFIG_STATUS_ERR_BROKEN_INTERLEAVE;
			break;

		case CURRENT_CONFIG_STATUS_UNCONFIGURED:
		case CURRENT_CONFIG_STATUS_ERROR_UNMAPPED:
			status = CONFIG_STATUS_NOT_CONFIGURED;
			break;

		case CURRENT_CONFIG_STATUS_ERROR_USING_OLD:
		case CURRENT_CONFIG_STATUS_BAD_INPUT_CHECKSUM:
		case CURRENT_CONFIG_STATUS_BAD_INPUT_REVISION:
			status = CONFIG_STATUS_ERR_REVERTED;
			break;

		case CURRENT_CONFIG_STATUS_UNKNOWN:
		case CURRENT_CONFIG_STATUS_INTERLEAVE_NOT_FOUND:
		case CURRENT_CONFIG_STATUS_BAD_CURRENT_CHECKSUM:
			status = CONFIG_STATUS_ERR_CORRUPT;
			break;

		default: // unrecognized error code - unsupported BIOS
			break;
	}

	return status;
}

NVM_BOOL interleave_set_has_offset(struct interleave_info_extension_table *p_interleave_table,
		const NVM_UINT64 interleave_set_offset)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	// Only need to look at the first DIMM - offsets are the same across all DIMMs
	NVM_UINT16 table_offset = sizeof (struct interleave_info_extension_table);
	if ((table_offset + sizeof (struct dimm_info_extension_table)) <=
			p_interleave_table->header.length)
	{
		struct dimm_info_extension_table *p_dimm_table =
			(struct dimm_info_extension_table *)((NVM_UINT8 *)p_interleave_table + table_offset);
		if (interleave_set_offset == p_dimm_table->offset)
		{
			result = 1;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

int get_interleave_settings_from_platform_config_data(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const NVM_UINT64 interleave_set_offset,
		NVM_UINT32 *p_interleave_set_id,
		struct interleave_format *p_format, NVM_BOOL *p_mirrored)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct platform_config_data *p_cfg_data = NULL;
	if ((rc = get_dimm_platform_config(device_handle, &p_cfg_data)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve Platform config data");
	}
	else
	{
		struct current_config_table *p_current_cfg = cast_current_config(p_cfg_data);
		if (p_current_cfg)
		{
			NVM_UINT32 offset = sizeof (struct current_config_table);
			NVM_BOOL found = 0;

			// loop through all extension tables
			while ((offset < p_current_cfg->header.length) && (rc == NVM_SUCCESS))
			{
				struct extension_table_header *p_header =
						(struct extension_table_header *)((NVM_UINT8 *) p_current_cfg + offset);

				if (!p_header->length || (offset + p_header->length) > p_current_cfg->header.length)
				{
					COMMON_LOG_ERROR_F("Extension table length %d invalid", p_header->length);
					rc = NVM_ERR_BADDEVICECONFIG; // bad data
					break;
				}
				// Only care about interleave tables
				else if (p_header->type == INTERLEAVE_TABLE)
				{
					struct interleave_info_extension_table *p_interleave_table =
							(struct interleave_info_extension_table *)((void*) p_header);

					if (p_interleave_table->memory_type == INTERLEAVE_MEMORY_TYPE_APP_DIRECT &&
							interleave_set_has_offset(p_interleave_table, interleave_set_offset))
					{
						found = 1;

						if (p_interleave_set_id)
						{
							*p_interleave_set_id = p_interleave_table->index;
						}

						if (p_mirrored)
						{
							*p_mirrored = p_interleave_table->mirror_enable;
						}

						interleave_format_to_struct(p_interleave_table->interleave_format,
								p_format);
						break;
					}
				}

				// go to next extension table
				offset += p_header->length;
			} // end extension table loop

			if (!found)
			{
				COMMON_LOG_ERROR_F("Unable to find interleave set with offset %llu on DIMM %u",
						interleave_set_offset, device_handle.handle);
				rc = NVM_ERR_NOTFOUND;
			}
		}
	}

	if (p_cfg_data)
	{
		free(p_cfg_data);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
