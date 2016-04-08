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
 * This file implements the Windows driver adapter interface for managing interleave
 * sets and pools.
 */

#include "device_adapter.h"
#include "pool_utilities.h"
#include "platform_config_data.h"
#include "device_utilities.h"
#include "win_adapter.h"
#include "utility.h"

/*
 * Fetch the interleave set count
 */
int get_interleave_set_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	GET_INTERLEAVE_SET_LIST_IOCTL ioctl_data;

	size_t buf_size = sizeof (GET_INTERLEAVE_SET_LIST_IOCTL)
		- sizeof (GET_INTERLEAVE_SET_LIST_OUTPUT_PAYLOAD);

	memset(&ioctl_data, 0, sizeof (ioctl_data));

	// Interleave count == 0 is a flag - driver will tell us how many there are
	ioctl_data.InputPayload.InteleaveSetCount = 0;

	if ((rc = execute_ioctl(buf_size, &ioctl_data, IOCTL_CR_GET_INTERLEAVE_SET_LIST))
		== NVM_SUCCESS)
	{
		// For windows driver if the buffer provided is less than the
		// amount of space the driver would expect underrun is returned
		// and the count variable is set to the expected number of elements
		if (ioctl_data.ReturnCode == CR_RETURN_CODE_BUFFER_OVERRUN ||
				ioctl_data.ReturnCode == CR_RETURN_CODE_BUFFER_UNDERRUN ||
				ioctl_data.ReturnCode == CR_RETURN_CODE_SUCCESS)
		{
			rc = ioctl_data.InputPayload.InteleaveSetCount;
		}
		else // Any other error is bad news
		{
			COMMON_LOG_ERROR_F("unexpected driver error code = %u",
				ioctl_data.ReturnCode);
			rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int interleave_set_ioctl_to_nvm(NVM_INTERLEAVE_SET *p_ioctl_interleave_set,
		struct nvm_interleave_set *p_nvm_interleave_set)
{
	int rc = NVM_SUCCESS;

	NVM_UINT8 num_dimms = (NVM_UINT8)p_ioctl_interleave_set->DeviceCount;

	p_nvm_interleave_set->driver_id = p_ioctl_interleave_set->InterleaveSetId;
	p_nvm_interleave_set->socket_id = p_ioctl_interleave_set->ProximityDomain;
	p_nvm_interleave_set->available_size = p_ioctl_interleave_set->FreeSpace;

	if (p_ioctl_interleave_set->Attributes &
			MAPPING_ATTRIBUTE_MIRRORED)
	{
		p_nvm_interleave_set->mirrored = 1;
	}

	p_nvm_interleave_set->dimm_count = num_dimms;
	for (NVM_UINT8 j = 0; (j < num_dimms) && (j < NVM_MAX_DEVICES_PER_SOCKET); j++)
	{
		NVM_NFIT_DEVICE_HANDLE dimm_handle;
		dimm_handle.handle = p_ioctl_interleave_set->DeviceList[j].DeviceHandle;
		p_nvm_interleave_set->dimms[j] = dimm_handle;
		p_nvm_interleave_set->size += p_ioctl_interleave_set->RegionSize[j];

		if (p_nvm_interleave_set->socket_id != dimm_handle.parts.socket_id)
		{
			COMMON_LOG_ERROR_F("found dimm on socket %u, expected socket "
					"ID %hhu",
					dimm_handle.parts.socket_id,
					p_nvm_interleave_set->socket_id);
			rc = NVM_ERR_BADDEVICECONFIG;
			break;
		}
	}

	if (num_dimms > 0)
	{
		// Data should be the same on all DIMMs in the interleave
		KEEP_ERROR(rc, fill_interleave_set_settings_and_id_from_dimm(
				p_nvm_interleave_set,
				p_nvm_interleave_set->dimms[0],
				p_ioctl_interleave_set->PhysicalBaseAddress[0]));
	}

	return rc;
}

/*
 * Fetch the list of interleave sets
 */
int get_interleave_sets(const NVM_UINT32 count, struct nvm_interleave_set *p_interleaves)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (p_interleaves == NULL)
	{
		COMMON_LOG_ERROR("p_interleaves is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = get_interleave_set_count()) > 0) // there are interleave sets to fetch
	{
		memset(p_interleaves, 0, sizeof (struct nvm_interleave_set) * count);

		int actual_count = rc;

		size_t buf_size = sizeof (GET_INTERLEAVE_SET_LIST_IOCTL) +
				(actual_count - 1) * sizeof (NVM_INTERLEAVE_SET);
		GET_INTERLEAVE_SET_LIST_IOCTL *p_ioctl_data = calloc(1, buf_size);

		if (p_ioctl_data)
		{
			p_ioctl_data->InputPayload.InteleaveSetCount = actual_count;

			if ((rc = execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_GET_INTERLEAVE_SET_LIST))
				== NVM_SUCCESS &&
				(rc = ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == NVM_SUCCESS)
			{
				int return_count = 0;
				if (count < actual_count)
				{
					return_count = count;
					COMMON_LOG_ERROR("array too small to hold all interleave sets");
					rc = NVM_ERR_ARRAYTOOSMALL;
				}
				else
				{
					return_count = actual_count;
					rc = actual_count;
				}

				for (int i = 0; i < return_count; i++)
				{
					int tmp_rc = interleave_set_ioctl_to_nvm(
							&(p_ioctl_data->OutputPayload.InterleaveSetList[i]),
							&(p_interleaves[i]));

					if (tmp_rc != NVM_SUCCESS)
					{
						COMMON_LOG_ERROR_F("Error converting Win IOCTL interleave set %u, rc = %d",
								p_ioctl_data->OutputPayload.InterleaveSetList[i].InterleaveSetId,
								tmp_rc);
						rc = tmp_rc;
						break;
					}
				}
			}

			free(p_ioctl_data);
		}
		else
		{
			COMMON_LOG_ERROR("failed to dynamically allocate output payload");
			rc = NVM_ERR_NOMEMORY;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_dimm_free_storage_capacity(NVM_NFIT_DEVICE_HANDLE handle,
		NVM_UINT64 *free_storage_capacity)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (free_storage_capacity == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, free_storage_capacity is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		CR_GET_DIMM_DETAILS_IOCTL ioctl_data;
		memset(&ioctl_data, 0, sizeof (ioctl_data));

		ioctl_data.NfitDeviceHandle.DeviceHandle = handle.handle;

		if ((rc = execute_ioctl(sizeof (ioctl_data), &ioctl_data, IOCTL_CR_GET_DIMM_DETAILS))
			== NVM_SUCCESS && (rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode)) == NVM_SUCCESS)
		{
			*free_storage_capacity = ioctl_data.OutputPayload.FreeBlockCapacity;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
/*
 * Retrieve the storage capacities of all DIMMs
 */
int get_dimm_storage_capacities(const NVM_UINT32 count, struct nvm_storage_capacities *p_capacities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if (p_capacities == NULL)
	{
		COMMON_LOG_ERROR("capacities is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		memset(p_capacities, 0, count * sizeof (struct nvm_storage_capacities));

		if ((rc = get_topology_count()) > 0)
		{
			int dimm_count = rc;
			struct nvm_topology topology[dimm_count];
			if ((rc = get_topology(dimm_count, topology)) == dimm_count)
			{
				for (int i = 0; i < dimm_count; i++)
				{
					if (i >= count)
					{
						rc = NVM_ERR_UNKNOWN;
						COMMON_LOG_ERROR_F("too many dimms for array, expected count = %u",
								count);
						break;
					}

					NVM_NFIT_DEVICE_HANDLE handle = topology[i].device_handle;
					struct pt_payload_get_dimm_partition_info pi;

					NVM_UINT64 memory_capacity = 0;
					NVM_UINT64 ilset_capacity = 0; // unmirrored interleaves
					NVM_UINT64 mirrored_capacity = 0; // mirrored interleaves

					p_capacities[i].device_handle = handle;

					if (get_dimm_memory_capacity(handle, &memory_capacity) != NVM_SUCCESS)
					{
						COMMON_LOG_ERROR("Failed to retrieve dimm memory capacity");
						rc = NVM_ERR_DRIVERFAILED;
						break; // don't continue on failure
					}
					else if (get_dimm_ilset_capacity(handle, &mirrored_capacity, &ilset_capacity)
							!= NVM_SUCCESS)
					{
						COMMON_LOG_ERROR("Failed to retrieve dimm interleave set capacity");
						rc = NVM_ERR_DRIVERFAILED;
						break; // don't continue on failure
					}
					else if ((rc = get_partition_info(handle, &pi)) != NVM_SUCCESS)
					{
						COMMON_LOG_ERROR("Failed to retrieve dimm partition info");
						rc = NVM_ERR_DRIVERFAILED;
						break; // don't continue on failure
					}
					else
					{
						NVM_UINT64 usable_capacity = USABLE_CAPACITY_BYTES(
								(NVM_UINT64)pi.raw_capacity * BYTES_PER_4K_CHUNK);
						if ((memory_capacity + ilset_capacity + mirrored_capacity) <=
								usable_capacity)
						{
							p_capacities[i].total_storage_capacity = usable_capacity -
								memory_capacity - mirrored_capacity;
							p_capacities[i].storage_only_capacity = usable_capacity -
									memory_capacity - mirrored_capacity - ilset_capacity;

							rc = get_dimm_free_storage_capacity(handle,
								&p_capacities[i].free_storage_capacity);
						}
						else
						{
							COMMON_LOG_ERROR("Memory and Ilset capacity greater than total "
									"dimm capacity");
							rc = NVM_ERR_UNKNOWN;
						}
					}
				}

				if (rc >= 0)
				{
					rc = dimm_count;
				}
			}
			else if (rc >= 0)
			{
				COMMON_LOG_ERROR_F("topology count changed! old = %d, new = %d",
						dimm_count, rc);
				rc = NVM_ERR_DRIVERFAILED;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
