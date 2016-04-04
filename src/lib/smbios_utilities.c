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
 * Implementations of common functions to translate data from the SMBIOS table into NVM
 * structures and values.
 */

#include "smbios_utilities.h"
#include "nvm_types.h"
#include "system.h"
#include <persistence/logging.h>
#include <smbios/smbios.h>
#include <string/s_str.h>

int get_dimm_physical_id_from_handle(const NVM_NFIT_DEVICE_HANDLE device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	rc = get_topology_count();
	if (rc > 0)
	{
		int topo_count = rc;
		struct nvm_topology topology[topo_count];
		rc = get_topology(topo_count, topology);
		if (rc > 0)
		{
			rc = NVM_ERR_BADDEVICE;
			for (int i = 0; i < topo_count; i++)
			{
				if (topology[i].device_handle.handle == device_handle.handle)
				{
					rc = topology[i].id;
					break;
				}
			}
		}
		else if (rc == 0)
		{
			COMMON_LOG_ERROR("Empty topology after we got a count");
			rc = NVM_ERR_UNKNOWN;
		}
	}
	else if (rc == 0)
	{
		COMMON_LOG_ERROR("Asked for a handle, but topology count == 0");
		rc = NVM_ERR_BADDEVICE;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_dimm_details_for_physical_id(const NVM_UINT16 physical_id,
		struct nvm_details *p_dimm_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_UINT8 *p_smbios_table = NULL;
	size_t smbios_table_size = 0;
	rc = get_smbios_table_alloc(&p_smbios_table, &smbios_table_size);
	if (rc == NVM_SUCCESS)
	{
		rc = get_dimm_details_from_smbios_table(p_smbios_table, smbios_table_size,
				physical_id, p_dimm_details);
	}

	if (p_smbios_table)
	{
		free(p_smbios_table);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_dimm_details_from_smbios_table(const NVM_UINT8 *p_smbios_table,
		const size_t smbios_data_length,
		const NVM_UINT16 physical_id, struct nvm_details *p_dimm_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	size_t remaining_length = smbios_data_length;
	const struct smbios_structure_header *p_device =
			smbios_get_structure_with_handle(physical_id,
			(struct smbios_structure_header *)p_smbios_table, &remaining_length);
	if (p_device && p_device->type == SMBIOS_STRUCT_TYPE_MEMORY_DEVICE)
	{
		smbios_memory_device_to_nvm_details(
				(const struct smbios_memory_device *)p_device,
				remaining_length, p_dimm_details);
		rc = NVM_SUCCESS;
	}
	else
	{
		COMMON_LOG_ERROR_F("Memory Device with SMBIOS handle %hu not found", physical_id);
		rc = NVM_ERR_BADDEVICE;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_device_memory_type_from_smbios_table(const NVM_UINT8 *p_smbios_table,
		const size_t smbios_data_length,
		const NVM_UINT16 physical_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct nvm_details details;
	memset(&details, 0, sizeof (details));
	rc = get_dimm_details_from_smbios_table(p_smbios_table,
			smbios_data_length, physical_id, &details);
	if (rc == NVM_SUCCESS)
	{
		rc = details.type;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int smbios_table_to_nvm_details_array(const NVM_UINT8 *p_smbios_table,
		const size_t smbios_data_length,
		struct nvm_details *p_details, const size_t num_details)
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	size_t remaining_length = smbios_data_length;
	const struct smbios_structure_header *p_header =
			smbios_get_first_structure_of_type(SMBIOS_STRUCT_TYPE_MEMORY_DEVICE,
			(struct smbios_structure_header *)p_smbios_table, &remaining_length);
	while (p_header && remaining_length >= sizeof (struct smbios_memory_device))
	{
		const struct smbios_memory_device *p_device = (struct smbios_memory_device *)p_header;
		if (p_device->size != SMBIOS_SIZE_EMPTY)
		{
			if (rc < num_details)
			{
				smbios_memory_device_to_nvm_details(p_device, remaining_length,
						&(p_details[rc]));
				rc++;
			}
			else
			{
				COMMON_LOG_ERROR_F(
						"Too many SMBIOS type 17 devices for nvm_details array size = %llu",
						num_details);
				rc = NVM_ERR_ARRAYTOOSMALL;
				break;
			}
		}

		p_header = smbios_get_next_structure_of_type(SMBIOS_STRUCT_TYPE_MEMORY_DEVICE,
				p_header, &remaining_length);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int smbios_get_populated_memory_device_count(const NVM_UINT8 *p_smbios_table,
		const size_t smbios_data_length)
{
	COMMON_LOG_ENTRY();
	int count = 0;

	size_t remaining_length = smbios_data_length;
	const struct smbios_structure_header *p_header =
			smbios_get_first_structure_of_type(SMBIOS_STRUCT_TYPE_MEMORY_DEVICE,
			(struct smbios_structure_header *)p_smbios_table, &remaining_length);
	while (p_header && remaining_length >= sizeof (struct smbios_memory_device))
	{
		const struct smbios_memory_device *p_device = (struct smbios_memory_device *)p_header;
		if (p_device->size != SMBIOS_SIZE_EMPTY)
		{
			count++;
		}

		p_header = smbios_get_next_structure_of_type(
				SMBIOS_STRUCT_TYPE_MEMORY_DEVICE, p_header, &remaining_length);
	}

	COMMON_LOG_EXIT_RETURN_I(count);
	return count;
}

void smbios_memory_device_to_nvm_details(const struct smbios_memory_device *p_smbios_dev,
		const size_t smbios_data_length,
		struct nvm_details *p_details)
{
	COMMON_LOG_ENTRY();

	memset(p_details, 0, sizeof (struct nvm_details));

	p_details->id = p_smbios_dev->header.handle;
	p_details->type = p_smbios_dev->memory_type;
	p_details->type_detail_bits = p_smbios_dev->type_detail;
	p_details->form_factor = p_smbios_dev->form_factor;
	p_details->data_width = p_smbios_dev->data_width;
	p_details->total_width = p_smbios_dev->total_width;
	p_details->speed = p_smbios_dev->speed;
	p_details->size = smbios_get_memory_device_size_in_bytes(p_smbios_dev);

	smbios_copy_structure_string_to_buffer(&(p_smbios_dev->header),
			smbios_data_length, p_smbios_dev->part_number_str_num,
			p_details->part_number, NVM_PART_NUM_LEN);
	smbios_copy_structure_string_to_buffer(&(p_smbios_dev->header),
			smbios_data_length, p_smbios_dev->device_locator_str_num,
			p_details->device_locator, NVM_DEVICE_LOCATOR_LEN);
	smbios_copy_structure_string_to_buffer(&(p_smbios_dev->header),
			smbios_data_length, p_smbios_dev->bank_locator_str_num,
			p_details->bank_label, NVM_BANK_LABEL_LEN);
	smbios_copy_structure_string_to_buffer(&(p_smbios_dev->header),
			smbios_data_length, p_smbios_dev->manufacturer_str_num,
			p_details->manufacturer, NVM_MANUFACTURERSTR_LEN);

	COMMON_LOG_EXIT();
}

COMMON_UINT64 smbios_get_memory_device_size_in_bytes(
		const struct smbios_memory_device *p_smbios_dev)
{
	COMMON_LOG_ENTRY();
	COMMON_UINT64 size_bytes = 0;

	if (p_smbios_dev->size == SMBIOS_SIZE_UNKNOWN)
	{
		size_bytes = NVM_DETAILS_SIZE_UNKNOWN;
	}
	else if (p_smbios_dev->size == SMBIOS_SIZE_EXTENDED) // extended size - always MB
	{
		COMMON_UINT32 size_mb = (p_smbios_dev->extended_size & SMBIOS_EXTENDED_SIZE_MASK);
		size_bytes = (COMMON_UINT64)size_mb * BYTES_PER_MB;
	}
	else // may be KB or MB
	{
		COMMON_UINT64 factor = BYTES_PER_MB;
		if (p_smbios_dev->size & SMBIOS_SIZE_KB_GRANULARITY_MASK)
		{
			factor = BYTES_PER_KB;
		}

		COMMON_UINT16 size = (p_smbios_dev->size & SMBIOS_SIZE_MASK);
		size_bytes = (COMMON_UINT64)size * factor;
	}

	COMMON_LOG_EXIT();
	return size_bytes;
}
