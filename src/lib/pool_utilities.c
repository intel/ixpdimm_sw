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
 * This file contains the implementation of helper functions
 * to convert interleave sets to pools.
 */

#include "pool_utilities.h"
#include <persistence/logging.h>
#include <uid/uid.h>
#include "system.h"
#include <string.h>
#include <string/s_str.h>
#include <uid/uid.h>
#include <guid/guid.h>
#include "device_utilities.h"
#include "platform_config_data.h"

#define	MAX_PERSISTENT_POOLS_PER_SOCKET 2

extern int get_device_status_by_handle(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct device_status *p_status, struct nvm_capabilities *p_capabilities);

/*
 * Get the number of pools allocated
 */
int get_pool_count()
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	int v_rc = 0;
	int m_rc = 0;
	int p_rc = 0;
	if ((v_rc = has_memory_pool()) < 0)
	{
		rc = v_rc;
	}
	else if ((m_rc = get_mirrored_pools_count()) < 0)
	{
		rc = m_rc;
	}
	else if ((p_rc = get_persistent_pools_count()) < 0)
	{
		rc = p_rc;
	}
	else
	{
		// all good ... add them up
		rc = v_rc + m_rc + p_rc;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve a list of pools
 */
int get_pools(const NVM_UINT32 count, struct nvm_pool *p_pools)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// check input parameters
	if (p_pools == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_pools array is null");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		memset(p_pools, 0, sizeof (struct nvm_pool) * count);

		// volatile pool
		struct nvm_pool v_pool;
		NVM_BOOL has_volatile_pool = 0;
		if ((rc = get_memory_pool(&v_pool)) == NVM_SUCCESS)
		{
			has_volatile_pool = v_pool.capacity > 0 ? 1 : 0;
		}

		// mirrored pools
		struct nvm_pool *p_mirrored_pools = NULL;
		int mirrored_count = 0;
		if ((rc >= 0) && (rc = get_mirrored_pools_count()) > 0)
		{
			mirrored_count = rc;
			p_mirrored_pools = calloc(mirrored_count, sizeof (struct nvm_pool));
			if (p_mirrored_pools)
			{
				if ((rc = get_mirrored_pools(p_mirrored_pools, mirrored_count)) != mirrored_count)
				{
					COMMON_LOG_ERROR_BAD_COUNT("get_mirrored_pools", mirrored_count, rc);
				}
			}
			else
			{
				COMMON_LOG_ERROR("couldn't allocate array for mirrored pools");
				rc = NVM_ERR_NOMEMORY;
			}
		}

		// PM pools
		struct nvm_pool *p_pm_pools = NULL;
		int persistent_count = 0;
		if ((rc >= 0) && (rc = get_persistent_pools_count()) > 0)
		{
			persistent_count = rc;
			p_pm_pools = calloc(persistent_count, sizeof (struct nvm_pool));
			if (p_pm_pools)
			{
				if ((rc = get_persistent_pools(p_pm_pools, persistent_count))
						!= persistent_count)
				{
					COMMON_LOG_ERROR_BAD_COUNT("get_persistent_pools",
							persistent_count, rc);
				}
			}
			else
			{
				COMMON_LOG_ERROR("couldn't allocate array for persistent pools");
				rc = NVM_ERR_NOMEMORY;
			}
		}

		// Successfully got all pools
		if (rc >= 0)
		{
			rc = has_volatile_pool + mirrored_count + persistent_count;
			if (rc != count)
			{
				COMMON_LOG_ERROR_F("size of array (%d) does not "
						"equal number of pools (%d)", count, rc);
				rc = NVM_ERR_INVALIDPARAMETER;
			}
			else
			{
				int pool_idx = 0;
				if (has_volatile_pool)
				{
					memmove(&(p_pools[pool_idx]), &v_pool, sizeof (struct nvm_pool));
					pool_idx++;
				}
				for (int i = 0; i < mirrored_count; i++)
				{
					memmove(&(p_pools[pool_idx]), &(p_mirrored_pools[i]),
							sizeof (struct nvm_pool));
					pool_idx++;
				}
				for (int i = 0; i < persistent_count; i++)
				{
					memmove(&(p_pools[pool_idx]), &(p_pm_pools[i]),
							sizeof (struct nvm_pool));
					pool_idx++;
				}
			}
		}

		// Cleanup
		if (p_mirrored_pools)
		{
			free(p_mirrored_pools);
		}

		if (p_pm_pools)
		{
			free(p_pm_pools);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
/*
 * returns 1 if a volatile pool exists, 0 if not, or an appropriate error code
 */
int has_memory_pool()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct nvm_pool pool;

	if ((rc = get_memory_pool(&pool)) == NVM_SUCCESS)
	{
		rc = pool.capacity > 0 ? 1 : 0;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_memory_pool(struct nvm_pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if (p_pool == NULL)
	{
		COMMON_LOG_ERROR("p_pool is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		memset(p_pool, 0, sizeof (struct nvm_pool));

		// build uuid src
		struct host host;
		if ((rc = get_host(&host)) == NVM_SUCCESS)
		{
			char *volatile_str = "volatile";
			char buffer[NVM_COMPUTERNAME_LEN + strlen(volatile_str)];
			s_snprintf(buffer, sizeof (buffer), "%s%s", host.name, volatile_str);
			if ((rc = init_pool(p_pool, buffer, POOL_TYPE_VOLATILE, -1)) == NVM_SUCCESS)
			{
				if ((rc = get_topology_count()) > 0)
				{
					int count = rc;
					struct nvm_topology topology[count];
					rc = get_topology(count, topology);
					if (rc != count)
					{
						COMMON_LOG_ERROR_BAD_COUNT("get_topology", count, rc);
					}
					else
					{
						rc = NVM_SUCCESS;
						for (int i = 0; i < count && rc == NVM_SUCCESS; i++)
						{
							if (is_device_manageable(topology[i].device_handle) > 0)
							{
								NVM_UINT64 dimm_memory_size = 0;
								if ((rc = get_dimm_memory_capacity(topology[i].device_handle,
										&dimm_memory_size)) == NVM_SUCCESS &&
										dimm_memory_size)
								{
									p_pool->capacity += dimm_memory_size;
									rc = add_dimm_to_pool(p_pool, topology[i].device_handle);
								}
							}
						}
						p_pool->free_capacity = p_pool->capacity;
					}
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Return the memory mode capacity on the dimm
 */
int get_dimm_memory_capacity(NVM_NFIT_DEVICE_HANDLE handle, NVM_UINT64 *p_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	*p_size = 0;

	// get BIOS mapped memory capacity from the platform config data
	struct platform_config_data *p_cfg_data = NULL;
	if ((rc = get_dimm_platform_config(handle, &p_cfg_data)) == NVM_SUCCESS)
	{
		struct current_config_table *p_current_config = cast_current_config(p_cfg_data);
		if (!p_current_config)
		{
			COMMON_LOG_WARN("Failed to retrieve mapped memory capacity - no PCD "
					"current config");
		}
		else
		{
			*p_size = p_current_config->mapped_memory_capacity;
		}
		free(p_cfg_data);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the number of mirrored pools
 */
int get_mirrored_pools_count()
{
	COMMON_LOG_ENTRY();
	int rc = get_mirrored_pools(NULL, 0);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to copy one set of pools to another. Handles the case when
 * caller doesn't want to copy the pools, but just wants to know the number of source
 * pools. Also handles the case if the caller passes an invalid destination array size
 *
 * Return the number of source pools, or an error code.
 */
int copy_pools(struct nvm_pool dst_pools[], const NVM_UINT32 dst_count,
		struct nvm_pool src_pools[], const NVM_UINT32 src_count)
{
	COMMON_LOG_ENTRY();
	int rc = src_count;
	if ((dst_pools != NULL) && (dst_count > 0))
	{
		NVM_UINT32 num_pools = src_count;
		if (src_count > dst_count)
		{
			rc = NVM_ERR_UNKNOWN;
			COMMON_LOG_ERROR_F("size of array (%d) smaller than number of "
					"pools (%d)", dst_count, src_count);
			num_pools = dst_count;
		}

		for (NVM_UINT32 i = 0; i < num_pools; i++)
		{
			memmove(&(dst_pools[i]), &(src_pools[i]),
					sizeof (struct nvm_pool));
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Construct mirrored pools, and if requesting, copy to pools[].
 *
 * Return number of pools or an error code
 */
int get_mirrored_pools(struct nvm_pool *p_pools, const NVM_UINT32 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if ((rc = get_interleave_set_count()) > 0)
	{
		int set_count = rc;
		struct nvm_interleave_set sets[set_count];
		if ((rc = get_interleave_sets(set_count, sets)) != set_count)
		{
			COMMON_LOG_ERROR_BAD_COUNT("get_interleave_sets", set_count, rc);
		}
		else
		{
			// can only have 1 mirrored per pool per socket. Going to build the pools in
			// a locally scoped pools array, then will copy over if needed
			struct nvm_pool *p_mirrored_pools = calloc(NVM_MAX_SOCKETS, sizeof (struct nvm_pool));
			int mirrored_pools_count = 0; // count of mirrored pools

			// loop through each interleave set
			rc = NVM_SUCCESS;
			for (int i = 0; i < set_count && rc == NVM_SUCCESS; i++)
			{
				if (sets[i].mirrored)
				{
					rc = add_ilset_to_pools(p_mirrored_pools,
							&mirrored_pools_count, &sets[i],
							POOL_TYPE_PERSISTENT_MIRROR, "mirrored");
				}
			}
			if (rc == NVM_SUCCESS)
			{
				rc = mirrored_pools_count;
				// Calculate mirrored pool capacity based on ilset information
				for (int i = 0; i < mirrored_pools_count; i++)
				{
					for (int j = 0; j < p_mirrored_pools[i].ilset_count; j++)
					{
						p_mirrored_pools[i].capacity += p_mirrored_pools[i].ilsets[j].size;
						p_mirrored_pools[i].free_capacity +=
							p_mirrored_pools[i].ilsets[j].available_size;
					}
				}
				if (p_pools)
				{
					memset(p_pools, 0, count * sizeof (struct nvm_pool));
					rc = copy_pools(p_pools, count, p_mirrored_pools, mirrored_pools_count);
				}
			}
			free(p_mirrored_pools);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the number of persistent pools
 */
int get_persistent_pools_count()
{
	COMMON_LOG_ENTRY();
	int rc = get_persistent_pools(NULL, 0);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the combined capacity of all the interleave sets on the dimm
 */
int get_dimm_ilset_capacity(NVM_NFIT_DEVICE_HANDLE handle, NVM_UINT64 *p_mirrored_size,
		NVM_UINT64 *p_unmirrored_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!p_mirrored_size || !p_unmirrored_size)
	{
		COMMON_LOG_ERROR("Invalid parameter, size is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		*p_mirrored_size = 0;
		*p_unmirrored_size = 0;

		if ((rc = get_interleave_set_count()) > 0)
		{
			int interleave_count = rc;
			struct nvm_interleave_set interleaves[interleave_count];
			if ((rc = get_interleave_sets(interleave_count, interleaves)) > 0)
			{
				for (int i = 0; i < interleave_count; i++)
				{
					NVM_UINT8 dimm_count = interleaves[i].dimm_count;
					NVM_UINT64 size_per_dimm = interleaves[i].size / (NVM_UINT64)dimm_count;
					if (interleaves[i].mirrored) // mirrored takes up 2x raw capacity
					{
						size_per_dimm *= 2llu;
					}
					for (NVM_UINT8 j = 0; j < dimm_count; j++)
					{
						// Found our dimm
						if (interleaves[i].dimms[j].handle == handle.handle)
						{
							if (interleaves[i].mirrored)
							{
								*p_mirrored_size += size_per_dimm;
							}
							else
							{
								*p_unmirrored_size += size_per_dimm;
							}
							break;
						}
					}
				}

				rc = NVM_SUCCESS;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * For persistent pools there may be storage regions that are not part of an
 * app direct interleave set and thus not accounted for in the pool
 */
int add_storage_regions_to_pools(struct nvm_pool *p_pools, int *p_pool_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_pools == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, pool array is null");
		rc = NVM_ERR_UNKNOWN;
	}
	else if ((rc = get_topology_count()) > 0)
	{
		// At max there could be storage capacity on each DIMM
		int num_dimms = rc;
		struct nvm_storage_capacities capacities[num_dimms];
		memset(&capacities, 0, sizeof (capacities));
		if ((rc = get_dimm_storage_capacities(num_dimms, capacities)) > 0)
		{
			int num_caps = rc;
			for (int i = 0; i < num_caps; i++)
			{
				NVM_NFIT_DEVICE_HANDLE handle = capacities[i].device_handle;

				// skip unmanageable dimms
				if (is_device_manageable(handle))
				{
					if (capacities[i].total_storage_capacity > 0)
					{
						rc = add_dimm_to_pools(p_pools, p_pool_count,
								handle,	POOL_TYPE_PERSISTENT, "persistent");
						if (rc != NVM_SUCCESS)
						{
							break;
						}
					}
				}
			} // end capacities loop
		} // either error or no storage capacities returned
	} // else no dimms in topology

	if (rc >= 0)
	{
		rc = NVM_SUCCESS;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Add the non-mirrored App Direct and Storage capacities to the pool
 */
int calculate_pm_capacities(struct nvm_pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if ((rc = get_topology_count()) > 0)
	{
		int total_dimm_count = rc;
		struct nvm_storage_capacities capacities[total_dimm_count];
		if ((rc = get_dimm_storage_capacities(total_dimm_count, capacities)) > 0)
		{
			int capacity_count = total_dimm_count < rc ? total_dimm_count : rc;
			for (int i = 0; i < p_pool->dimm_count; i++)
			{
				// look for this DIMM's storage capacity
				// if we couldn't find it, it might mean the capacity is all App Direct
				for (int j = 0; j < capacity_count; j++)
				{
					if (capacities[j].device_handle.handle == p_pool->dimms[i].handle)
					{
						// Total storage capacity = storage-only + app direct
						// If there's ever a DIMM with no storage regions, this could break.
						p_pool->storage_capacities[i] = capacities[j].storage_only_capacity;
						if (p_pool->type == POOL_TYPE_PERSISTENT)
						{
							p_pool->capacity += capacities[j].total_storage_capacity;
							p_pool->free_capacity += capacities[j].free_storage_capacity;
						}
					}
				}
			}

			rc = NVM_SUCCESS;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Construct pools for each socket containing all persistent memory,
 * and if requesting, copy to pools[].
 *
 * Return number of pools or an error code
 */
int get_persistent_pools(struct nvm_pool *p_pools, const NVM_UINT32 count)
{
	COMMON_LOG_ENTRY();

	struct nvm_pool *p_pm_pools = calloc(NVM_MAX_SOCKETS, sizeof (struct nvm_pool));

	int pm_pools_count = 0; // actual count of pm pools found

	int rc = NVM_SUCCESS;
	if ((rc = get_interleave_set_count()) > 0)
	{
		int set_count = rc;
		struct nvm_interleave_set sets[set_count];
		if ((rc = get_interleave_sets(set_count, sets)) != set_count)
		{
			COMMON_LOG_ERROR_BAD_COUNT("get_interleave_sets", set_count, rc);
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{

			// loop through each interleave set
			rc = NVM_SUCCESS;
			for (int i = 0; i < set_count && rc == NVM_SUCCESS; i++)
			{
				if (!sets[i].mirrored)
				{
					rc = add_ilset_to_pools(p_pm_pools,
							&pm_pools_count, &sets[i],
							POOL_TYPE_PERSISTENT, "persistent");
				}
			}
		}
	}

	if (rc == NVM_SUCCESS)
	{
		// cycle through all storage regions, make sure the dimm is in a pool
		// and calculate overall pool capacity
		if ((rc = add_storage_regions_to_pools(p_pm_pools, &pm_pools_count)) == NVM_SUCCESS)
		{
			for (int j = 0; j < pm_pools_count && rc == NVM_SUCCESS; j++)
			{
				if ((rc = calculate_pm_capacities(&p_pm_pools[j]))
					!= NVM_SUCCESS)
				{
					COMMON_LOG_ERROR("Failed to calculate storage only capacity");
					break; // don't continue on failure
				}
			}
			if (rc == NVM_SUCCESS)
			{
				rc = pm_pools_count;
				if (p_pools)
				{
					memset(p_pools, 0, count * sizeof (struct nvm_pool));
					rc = copy_pools(p_pools, count, p_pm_pools, pm_pools_count);
				}
			}
		}
	}
	free(p_pm_pools);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int add_dimm_to_pool(struct nvm_pool *p_pool, NVM_NFIT_DEVICE_HANDLE handle)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// check if p_set->dimms[i] has already been added ... if not add it
	int found = 0;
	for (int j = 0; j < p_pool->dimm_count && !found; j++)
	{
		if (p_pool->dimms[j].handle == handle.handle)
		{
			found = 1;
		}
	}

	if (!found)
	{
		enum device_ars_status ars_status;
		rc = get_ars_status(handle, &ars_status);
		if (rc == NVM_SUCCESS)
		{
			if (ars_status == DEVICE_ARS_STATUS_INPROGRESS)
			{
				rc = NVM_ERR_ARSINPROGRESS;
			}
			else
			{
				struct pt_payload_get_dimm_partition_info pi;
				if ((rc = get_partition_info(handle, &pi)) == NVM_SUCCESS)
				{
					p_pool->dimms[p_pool->dimm_count].handle = handle.handle;

					get_dimm_memory_capacity(handle,
							&p_pool->memory_capacities[p_pool->dimm_count]);
					p_pool->raw_capacities[p_pool->dimm_count] =
							pi.raw_capacity * BYTES_PER_4K_CHUNK;

					p_pool->dimm_count++;

					// calculate pool health based on dimm health
					enum device_health dimm_health;

					rc = get_dimm_health(handle, &dimm_health);
					if (rc == NVM_SUCCESS)
					{
						enum pool_health health;
						switch (dimm_health)
						{
						case DEVICE_HEALTH_NORMAL:
							health = POOL_HEALTH_NORMAL;
							break;
						case DEVICE_HEALTH_NONCRITICAL:
						case DEVICE_HEALTH_CRITICAL:
							health = POOL_HEALTH_WARNING;
							break;
						case DEVICE_HEALTH_FATAL:
							health = p_pool->type == POOL_TYPE_PERSISTENT_MIRROR ?
									POOL_HEALTH_DEGRADED : POOL_HEALTH_FAILED;
							break;
						default:
							health = POOL_HEALTH_UNKNOWN;
							break;
						}

						// keep the greater (worse) health state
						p_pool->health = p_pool->health < health ? health : p_pool->health;
					}
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Search through the pools to find the appropriate one to add the details to. If
 * one doesn't exist, add a new one and increment p_pools_count
 */
int add_dimm_to_pools(struct nvm_pool *p_pools, int *p_pools_count, NVM_NFIT_DEVICE_HANDLE handle,
		enum pool_type type, char *uuid_src_prefix)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct nvm_capabilities capabilities;
	struct device_status dev_status;

	if ((rc = nvm_get_nvm_capabilities(&capabilities)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the system capabilities.");
	}
	else if (!capabilities.nvm_features.get_device_health)
	{
		COMMON_LOG_ERROR("Retrieving device status is not supported.");
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else if ((rc = get_device_status_by_handle(handle, &dev_status, &capabilities)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the device status.");
	}
	else
	{
		if (dev_status.ars_status == DEVICE_ARS_STATUS_INPROGRESS)
		{
			COMMON_LOG_ERROR("Failed to retrieve the device status.");
			rc = NVM_ERR_ARSINPROGRESS;
		}
		// ignore dimm if it's unconfigured
		else if (dev_status.config_status != CONFIG_STATUS_NOT_CONFIGURED)
		{
			// find the pool that represents the socket this set is on
			int pool_idx = -1;
			for (int j = 0; j < *p_pools_count && pool_idx < 0; j++)
			{
				if (p_pools[j].socket_id == handle.parts.socket_id)
				{
					pool_idx = j;
				}
			}

			if (pool_idx == -1) // not found so need to add
			{
				pool_idx = *p_pools_count;
				(*p_pools_count) ++;

				// generate uuid src to init pool with
				char buffer[sizeof (uuid_src_prefix) + NVM_MAX_SOCKET_DIGIT_COUNT];
				s_snprintf(buffer, sizeof (buffer),
						"%s%d", uuid_src_prefix, handle.parts.socket_id);

				rc = init_pool(p_pools + pool_idx, buffer, type, handle.parts.socket_id);
			}

			if (rc == NVM_SUCCESS)
			{
				rc = add_dimm_to_pool(p_pools + pool_idx, handle);
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * add the details to the pool
 */
int add_ilset_to_pool(struct nvm_pool *p_pool, struct nvm_interleave_set *p_ilset)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	int ilset_found = 0;
	// Make sure ilset doesn't exist already
	for (int k = 0; k < p_pool->ilset_count; k++)
	{
		if (p_pool->ilsets[k].set_index == p_ilset->set_index)
		{
			ilset_found = 1;
		}
	}

	if (!ilset_found)
	{
		memmove(&p_pool->ilsets[p_pool->ilset_count], p_ilset, sizeof (struct nvm_interleave_set));
		p_pool->ilset_count++;

		// add all the interleave set's dimms
		for (int i = 0; i < p_ilset->dimm_count && rc == NVM_SUCCESS; i++)
		{
			rc = add_dimm_to_pool(p_pool, p_ilset->dimms[i]);
		}
	}


	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Search through the pools to find the appropriate one to add the details to. If
 * one doesn't exist, add a new one and increment p_pools_count
 */
int add_ilset_to_pools(struct nvm_pool *p_pools, int *p_pools_count,
	struct nvm_interleave_set *p_ilset,	enum pool_type type, char *uuid_src_prefix)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// find the pool that represents the socket this set is on
	int pool_idx = -1;
	for (int j = 0; j < *p_pools_count && pool_idx < 0; j++)
	{
		if (p_pools[j].socket_id == p_ilset->socket_id)
		{
			pool_idx = j;
			break;
		}
	}

	if (pool_idx == -1) // not found so need to add
	{
		pool_idx = *p_pools_count;
		(*p_pools_count)++;

		// generate uuid src to init pool with
		char buffer[sizeof (uuid_src_prefix) + NVM_MAX_SOCKET_DIGIT_COUNT];
		s_snprintf(buffer, sizeof (buffer), "%s%d", uuid_src_prefix, p_ilset->socket_id);

		rc = init_pool(p_pools + pool_idx, buffer, type, p_ilset->socket_id);
	}

	if (rc == NVM_SUCCESS)
	{
		rc = add_ilset_to_pool(p_pools + pool_idx, p_ilset);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper to initialize a new pool with its UUID and type
 */
int init_pool(struct nvm_pool *p_pool, const char *uuid_src, const enum pool_type type,
		const int socket)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	memset(p_pool, 0, sizeof (struct nvm_pool));

	if (!guid_hash_str((unsigned char *) uuid_src, strlen((const char *) uuid_src),
		p_pool->pool_uid))
	{
		// should never get here
		COMMON_LOG_ERROR("Pool uid hash creation FAILED");
		rc = NVM_ERR_UNKNOWN;
	}

	p_pool->type = type;
	p_pool->socket_id = socket;

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Calculate security attributes of all dimms in the interleave set
 */
int calculate_security_attributes_all_dimms(struct nvm_interleave_set *p_interleave,
	enum encryption_status *p_encryption_enabled,
	enum erase_capable_status *p_erase_capable,
	enum encryption_status *p_encryption_capable /* could be NULL */)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_BOOL encryption_enabled_flag = 1;
	NVM_BOOL erasecapable_flag = 1;
	NVM_BOOL encryption_capable_flag = 1;

	for (int dimm_index = 0; dimm_index < p_interleave->dimm_count; dimm_index++)
	{
		struct device_discovery discovery;
		if ((lookup_dev_handle(p_interleave->dimms[dimm_index], &discovery)) >= 0)
		{
			// Encryption: on if NVDIMM1.LockState && NVDIMM2.Lockstate && .. = enabled.
			if (!device_is_encryption_enabled(discovery.lock_state))
			{
				encryption_enabled_flag = 0;
			}

			if (!discovery.security_capabilities.passphrase_capable)
			{
				encryption_capable_flag = 0;
			}

			// EraseCapable: true if all NVDIMMs in the set support erase.
			if (!device_is_erase_capable(discovery.security_capabilities))
			{
				erasecapable_flag = 0;
			}
		}
		else
		{
			COMMON_LOG_ERROR_F(
				"Failed to find the device from the device handle %u",
				p_interleave->dimms[dimm_index].handle);
			rc = NVM_ERR_DRIVERFAILED;
			break;
		}
	}

	*p_encryption_enabled = NVM_ENCRYPTION_OFF;
	if (p_encryption_capable != NULL)
	{
		*p_encryption_capable = NVM_ENCRYPTION_OFF;
	}
	*p_erase_capable = NVM_ERASE_CAPABLE_FALSE;
	if (rc == NVM_SUCCESS)
	{
		if (encryption_enabled_flag)
		{
			*p_encryption_enabled = NVM_ENCRYPTION_ON;
		}

		if ((p_encryption_capable != NULL) && encryption_capable_flag)
		{
			*p_encryption_capable = NVM_ENCRYPTION_ON;
		}

		if (erasecapable_flag)
		{
			*p_erase_capable = NVM_ERASE_CAPABLE_TRUE;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the given interleave set
 */
int get_interleaveset_by_driver_id(NVM_UINT32 driver_id, struct nvm_interleave_set *p_interleave)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;

	int interleave_count = get_interleave_set_count();
	if (interleave_count >= 0)
	{
		struct nvm_interleave_set interleaves[interleave_count];
		interleave_count = get_interleave_sets(interleave_count, interleaves);
		if (interleave_count > 0)
		{
			for (int i = 0; i < interleave_count; i++)
			{
				if (interleaves[i].driver_id == driver_id)
				{
					memmove(p_interleave, &(interleaves[i]), sizeof (struct nvm_interleave_set));
					rc = NVM_SUCCESS;
					break;
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Calculate security information about the interleave set specified.
 */
int calculate_app_direct_interleave_security(
	NVM_UINT32 interleave_setid,
	enum encryption_status *p_encryption_enabled,
	enum erase_capable_status *p_erase_capable,
	enum encryption_status *p_encryption_capable /* could be NULL */)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;

	struct nvm_interleave_set interleave;
	memset(&interleave, 0, sizeof (interleave));
	if (get_interleaveset_by_driver_id(interleave_setid, &interleave) == NVM_SUCCESS)
	{
		if (calculate_security_attributes_all_dimms(&interleave,
				p_encryption_enabled, p_erase_capable, p_encryption_capable) == NVM_SUCCESS)
		{
			rc = NVM_SUCCESS;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to get pool uid corresponding to a namespace,
 * given the device handle or interleave set index, based on the namespace type.
 */
int get_pool_uid_from_namespace_details(
		const struct nvm_namespace_details *p_details, NVM_UID *p_pool_uid)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;

	// get all the pools and then find the one we want
	int pool_count = get_pool_count();
	if (pool_count > 0)
	{
		struct nvm_pool *pools =
				(struct nvm_pool *)calloc(pool_count, sizeof (struct nvm_pool));
		if (pools == NULL)
		{
			COMMON_LOG_ERROR("Not enough memory to get namespace details.");
			rc = NVM_ERR_NOMEMORY;
		}
		else
		{
			pool_count = get_pools(pool_count, pools);
			if (pool_count > 0)
			{
				if (p_details->type == NAMESPACE_TYPE_STORAGE)
				{
					struct device_discovery discovery;
					if (lookup_dev_handle(p_details->namespace_creation_id.device_handle,
							&discovery) == NVM_SUCCESS)
					{
						int tmprc = NVM_ERR_NOTFOUND;
						for (int i = 0; i < pool_count; i++)
						{
							for (int j = 0; j < pools[i].dimm_count; j++)
							{
								if ((p_details->namespace_creation_id.device_handle.handle ==
										pools[i].dimms[j].handle) &&
										(pools[i].type == POOL_TYPE_PERSISTENT))
								{
									memmove(p_pool_uid, pools[i].pool_uid, NVM_MAX_UID_LEN);
									tmprc = NVM_SUCCESS;
									break;
								}
							}
						}
						if (tmprc == NVM_ERR_NOTFOUND)
						{
							NVM_UID ns_uid_str;
							uid_copy(p_details->discovery.namespace_uid, ns_uid_str);
							COMMON_LOG_ERROR_F("Failed to find the pool associated with the \
									namespace %s.", ns_uid_str);
							rc = NVM_ERR_DRIVERFAILED;
						}
						else
						{
							rc = NVM_SUCCESS;
						}
					}
				}
				else if (p_details->type == NAMESPACE_TYPE_APP_DIRECT)
				{
					int tmprc = NVM_ERR_NOTFOUND;
					for (int i = 0; i < pool_count; i++)
					{
						for (int j = 0; j < pools[i].ilset_count; j++)
						{
							if (p_details->namespace_creation_id.interleave_setid
									== pools[i].ilsets[j].driver_id)
							{
								memmove(p_pool_uid, pools[i].pool_uid, NVM_MAX_UID_LEN);
								tmprc = NVM_SUCCESS;
								break;
							}
						}
					}
					if (tmprc == NVM_ERR_NOTFOUND)
					{
						NVM_UID ns_uid_str;
						uid_copy(p_details->discovery.namespace_uid, ns_uid_str);
						COMMON_LOG_ERROR_F("Failed to find the pool associated with the namespace \
								 %s.", ns_uid_str);
						rc = NVM_ERR_DRIVERFAILED;
					}
					else
					{
						rc = NVM_SUCCESS;
					}
				}
			}
			free(pools);
		}
	}
	else if (pool_count < 0)
	{
		rc = pool_count;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_interleave_set_offset_from_pm_partition_start(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const NVM_UINT64 offset_from_dimm_start,
		NVM_UINT64* p_offset_from_partition_start)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_get_dimm_partition_info partition_info;
	rc = get_partition_info(device_handle, &partition_info);
	if (rc == NVM_SUCCESS)
	{
		if (partition_info.start_pmem > offset_from_dimm_start)
		{
			COMMON_LOG_WARN_F("DPA 0x%llx is before the start of PM partition (0x%llx)",
					offset_from_dimm_start, partition_info.start_pmem);
			// VMware gives us the offset from the partition start
			*p_offset_from_partition_start = offset_from_dimm_start;
		}
		else
		{
			*p_offset_from_partition_start =
					offset_from_dimm_start - partition_info.start_pmem;
		}
	}
	else
	{
		COMMON_LOG_ERROR_F("Couldn't get partition info for dimm %u, rc=%d",
				device_handle.handle, rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int fill_interleave_set_settings_and_id_from_dimm(struct nvm_interleave_set *p_interleave_set,
		const NVM_NFIT_DEVICE_HANDLE handle,
		const NVM_UINT64 interleave_dpa_offset)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_UINT64 interleave_set_pcd_offset = 0;
	if ((rc = get_interleave_set_offset_from_pm_partition_start(handle,
			interleave_dpa_offset,
			&interleave_set_pcd_offset)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Couldn't get partition info");
		rc = NVM_ERR_DRIVERFAILED;
	}
	else if ((rc = get_interleave_settings_from_platform_config_data(handle,
			interleave_set_pcd_offset,
			&(p_interleave_set->set_index),
			&(p_interleave_set->settings), &(p_interleave_set->mirrored))) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Couldn't get PCD info");
		rc = NVM_ERR_DRIVERFAILED;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
