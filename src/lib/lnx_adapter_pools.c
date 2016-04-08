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
 * This file implements the Linux driver adapter interface for managing pools and
 * interleave sets.
 */

#include "device_adapter.h"
#include "lnx_adapter.h"
#include "platform_config_data.h"
#include "utility.h"
#include "pool_utilities.h"

/*
 * Count the number of interleave sets as provided by the driver
 */
int get_interleave_set_count()
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	struct ndctl_ctx *ctx;

	if ((rc = ndctl_new(&ctx)) >= 0)
	{
		struct ndctl_bus *bus;
		rc = 0;
		ndctl_bus_foreach(ctx, bus)
		{
			struct ndctl_interleave_set *interleave_set;
			ndctl_interleave_set_foreach(bus, interleave_set)
			{
				rc++;
			}
		}
		ndctl_unref(ctx);
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_interleave_set_dimm_info(struct ndctl_interleave_set *p_ndctl_interleave,
		struct nvm_interleave_set *p_nvm_interleave_set)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_BOOL socket_set = 0;
	int dimm_index = 0;

	struct ndctl_dimm *dimm;
	ndctl_dimm_foreach_in_interleave_set(p_ndctl_interleave, dimm)
	{
		if (dimm_index >= NVM_MAX_DEVICES_PER_SOCKET)
		{
			rc = NVM_ERR_DRIVERFAILED;
			COMMON_LOG_ERROR("Invalid Interleave Set, "
				"Number of dimms exceeds max allowed per socket");
			break;
		}

		NVM_NFIT_DEVICE_HANDLE device_handle;
		device_handle.handle = ndctl_dimm_get_handle(dimm);

		p_nvm_interleave_set->dimms[dimm_index].handle =
			device_handle.handle;

		if (!socket_set)
		{
			p_nvm_interleave_set->socket_id = device_handle.parts.socket_id;
			socket_set = 1;
		}
		else if (p_nvm_interleave_set->socket_id != device_handle.parts.socket_id)
		{
			rc = NVM_ERR_DRIVERFAILED;
			COMMON_LOG_ERROR("Invalid Interleave Set, "
				"Dimms are not all on the same socket");
			break;
		}

		dimm_index++;
	}

	if (rc == NVM_SUCCESS)
	{
		p_nvm_interleave_set->dimm_count = dimm_index;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_interleave_dimm_offset(struct ndctl_region *p_region, NVM_UINT64 *p_mapping_offset)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (ndctl_region_get_mappings(p_region) > 0)
	{
		// offset is the same for all DIMMs in the interleave set
		struct ndctl_mapping *mapping = ndctl_mapping_get_first(p_region);
		*p_mapping_offset = ndctl_mapping_get_offset(mapping);
	}
	else
	{
		COMMON_LOG_ERROR("found no mappings for region");
		rc = NVM_ERR_DRIVERFAILED;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int interleave_set_ndctl_to_nvm(struct ndctl_interleave_set *p_ndctl_interleave,
		struct nvm_interleave_set *p_nvm_interleave_set)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	rc = get_interleave_set_dimm_info(p_ndctl_interleave, p_nvm_interleave_set);
	if (rc == NVM_SUCCESS)
	{
		struct ndctl_region *region = ndctl_interleave_set_get_region(p_ndctl_interleave);
		if (region)
		{
			p_nvm_interleave_set->size = ndctl_region_get_size(region);
			p_nvm_interleave_set->available_size = ndctl_region_get_available_size(region);
			p_nvm_interleave_set->driver_id = ndctl_region_get_range_index(region);

			NVM_UINT64 interleave_set_driver_offset = 0;
			rc = get_interleave_dimm_offset(region, &interleave_set_driver_offset);
			if (rc == NVM_SUCCESS)
			{
				rc = fill_interleave_set_settings_and_id_from_dimm(p_nvm_interleave_set,
						p_nvm_interleave_set->dimms[0],
						interleave_set_driver_offset);
			}
		}
		else
		{
			COMMON_LOG_ERROR("Couldn't get corresponding region for interleave set");
			rc = NVM_ERR_DRIVERFAILED;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get interleave sets as provided by the driver. Returns the number of interleave sets returning,
 * or fail on first error code
 */
int get_interleave_sets(const NVM_UINT32 count, struct nvm_interleave_set *sets)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	int interleave_index = 0;
	struct ndctl_ctx *ctx;

	if (sets == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, interleave set array is null");
		rc = NVM_ERR_UNKNOWN;
	}
	else if ((count != 0) && ((rc = ndctl_new(&ctx)) >= 0))
	{
		memset(sets, 0, count * sizeof (struct nvm_interleave_set));

		struct ndctl_bus *bus;
		ndctl_bus_foreach(ctx, bus)
		{
			struct ndctl_interleave_set *interleave_set;
			ndctl_interleave_set_foreach(bus, interleave_set)
			{
				// ensure only count is returned
				if (interleave_index >= count)
				{
					rc = NVM_ERR_ARRAYTOOSMALL;
					COMMON_LOG_ERROR_F(
							"count %u is smaller than number of real interleave sets", count);
					break;
				}

				if ((rc = interleave_set_ndctl_to_nvm(
						interleave_set, &(sets[interleave_index]))) != NVM_SUCCESS)
				{
					break;
				}

				interleave_index++;
			}
			if (rc != NVM_SUCCESS)
			{
				break;
			}
		} // end for buses

		if (rc == NVM_SUCCESS)
		{
			rc = interleave_index;
		}

		ndctl_unref(ctx);
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the storage capacity for each DIMM
 */
int get_dimm_storage_capacities(const NVM_UINT32 count, struct nvm_storage_capacities *p_capacities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct ndctl_ctx *ctx;

	if (p_capacities == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_capacities is null");
		rc = NVM_ERR_UNKNOWN;
	}
	else if ((rc = ndctl_new(&ctx)) >= 0)
	{
		memset(p_capacities, 0, count * sizeof (struct nvm_storage_capacities));

		int cap_count = 0;
		struct ndctl_bus *bus;
		rc = NVM_SUCCESS;
		ndctl_bus_foreach(ctx, bus)
		{
			struct ndctl_region *region;
			ndctl_region_foreach(bus, region)
			{
				// Only count regions that can create a storage namespace
				if (ndctl_region_is_enabled(region) &&
					ndctl_region_get_type(region) == ND_DEVICE_REGION_BLK &&
					ndctl_region_get_mappings(region) > 0)
				{
					struct ndctl_dimm *dimm = ndctl_region_get_first_dimm(region);

					if (dimm == NULL)
					{
						rc = NVM_ERR_DRIVERFAILED;
						COMMON_LOG_ERROR("Can't get Storage region with associated dimm");
						break;
					}
					else
					{
						NVM_NFIT_DEVICE_HANDLE handle;
						handle.handle = ndctl_dimm_get_handle(dimm);

						// see if we already have an index for the dimm
						int found = 0;
						int cap_idx = cap_count;
						for (int i = 0; i < cap_count; i++)
						{
							if (p_capacities[cap_count].device_handle.handle ==
									handle.handle)
							{
								cap_idx = i;
								found = 1;
								break;
							}
						}

						if (found)
						{
							rc = NVM_ERR_DRIVERFAILED;
							COMMON_LOG_ERROR_F("Driver reported DIMM with handle %u has multiple "
									"storage regions",
									handle.handle);
							break;
						}

						if (cap_idx >= count)
						{
							rc = NVM_ERR_DRIVERFAILED;
							COMMON_LOG_ERROR_F("Too many DIMMs (expected count = %u)", count);
							break;
						}

						// Get interleave info so we can calculate storage-only capacity
						NVM_UINT64 ilset_capacity = 0;
						NVM_UINT64 mirrored_capacity = 0;
						if ((rc = get_dimm_ilset_capacity(handle, &mirrored_capacity,
								&ilset_capacity)) != NVM_SUCCESS)
						{
							COMMON_LOG_ERROR("Failed to retrieve dimm interleave set capacity");
							break;
						}

						p_capacities[cap_idx].total_storage_capacity =
								ndctl_region_get_size(region);
						p_capacities[cap_idx].free_storage_capacity =
								ndctl_region_get_available_size(region);
						if (p_capacities[cap_idx].total_storage_capacity > ilset_capacity)
						{
							p_capacities[cap_idx].storage_only_capacity =
									p_capacities[cap_idx].total_storage_capacity - ilset_capacity;
						}
						else
						{
							p_capacities[cap_idx].storage_only_capacity = 0;
						}
						p_capacities[cap_idx].device_handle = handle;
						cap_count++;
					}
				}
			} // end region loop
		} // end bus loop
		ndctl_unref(ctx);

		if (rc >= 0)
		{
			rc = cap_count;
		}
	}
	else
	{
		rc = linux_err_to_nvm_lib_err(rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
