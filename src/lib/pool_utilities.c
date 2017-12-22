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
#include <guid/guid.h>
#include "system.h"
#include <string.h>
#include <string/s_str.h>
#include "device_utilities.h"
#include "config_goal.h"

#define	MAX_PERSISTENT_POOLS_PER_SOCKET 2
#define	MAX_POOL_UID_INPUT_LEN	512

/*
 * Helper to initialize a new pool with its UUID and type
 */
int init_pool(struct pool *p_pool, const char *host_name,
		const enum pool_type type, const int socket)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	memset(p_pool, 0, sizeof (struct pool));

	char buffer[MAX_POOL_UID_INPUT_LEN];
	if (type == POOL_TYPE_VOLATILE)
	{
		s_snprintf(buffer, sizeof (buffer), "%svolatile", host_name);
	}
	else if (type == POOL_TYPE_PERSISTENT)
	{
		s_snprintf(buffer, sizeof (buffer), "persistent%d", socket);
	}
	else
	{
		s_snprintf(buffer, sizeof (buffer), "mirrored%d", socket);
	}

	if (!guid_hash_str((unsigned char *) buffer, strlen((const char *) buffer),
		p_pool->pool_uid))
	{
		// should never get here
		COMMON_LOG_ERROR("Pool uid hash creation FAILED");
		rc = NVM_ERR_UNKNOWN;
	}

	p_pool->type = type;
	p_pool->socket_id = socket;
	p_pool->health = POOL_HEALTH_NORMAL;

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
		struct nvm_interleave_set *interleaves = malloc(interleave_count * sizeof(struct nvm_interleave_set));
		interleave_count = get_interleave_sets(interleave_count, interleaves);
		if (interleave_count > 0)
		{
			for (int i = 0; i < interleave_count; i++)
			{
				if (interleaves[i].id == driver_id)
				{
					memmove(p_interleave, &(interleaves[i]), sizeof (struct nvm_interleave_set));
					rc = NVM_SUCCESS;
					break;
				}
			}
		}

        free(interleaves);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to get pool  corresponding to a namespace,
 * given the device handle or interleave set index, based on the namespace type.
 */
int get_pool_from_namespace_details(
		const struct nvm_namespace_details *p_details, struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// get all the pools and then find the one we want
	int pool_count = nvm_get_pool_count();
	if (pool_count < 0)
	{
		rc = pool_count;
	}
	else if (pool_count > 0)
	{
		struct pool *pools =
				(struct pool *)calloc(pool_count, sizeof (struct pool));
		if (pools == NULL)
		{
			COMMON_LOG_ERROR("Not enough memory to get namespace details.");
			rc = NVM_ERR_NOMEMORY;
		}
		else
		{
			pool_count = nvm_get_pools(pools, pool_count);
			if (pool_count < 0)
			{
				rc = pool_count;
			}
			else if (pool_count > 0)
			{
				int tmprc = NVM_ERR_NOTFOUND;
				for (int pool_idx = 0; pool_idx < pool_count; pool_idx++)
				{
					for (int iset_idx = 0; iset_idx <
						pools[pool_idx].ilset_count; iset_idx++)
					{
						if (p_details->namespace_creation_id.interleave_setid
								== pools[pool_idx].ilsets[iset_idx].driver_id)
						{
							memmove(p_pool, &pools[pool_idx], sizeof (struct pool));
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
			}
		}
		free(pools);
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

int fill_interleave_set_settings_and_id_from_dimm(
		struct interleave_set *p_interleave_set,
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
	else
	{
		if ((rc = get_interleave_settings_from_platform_config_data(handle,
			interleave_set_pcd_offset,
			&(p_interleave_set->set_index),
			&(p_interleave_set->settings),
			NULL)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR("Couldn't get PCD info");
			rc = NVM_ERR_DRIVERFAILED;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL dimm_is_in_interleave_set(const NVM_UID device_uid,
		const struct interleave_set *p_ilset)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	for (int dimm_index = 0; dimm_index < p_ilset->dimm_count; dimm_index++)
	{
		if (uid_cmp(device_uid, p_ilset->dimms[dimm_index]))
		{
			result = 1;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

int get_dimm_free_capacities(
		const struct nvm_capabilities *p_nvm_caps,
		const struct device_discovery *p_discovery,
		const struct pool *p_pool,
		const struct nvm_namespace_details *p_namespaces,
		const int ns_count,
		struct device_capacities *p_dimm_caps,
		struct device_free_capacities *p_dimm_free_caps)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_nvm_caps->nvm_features.app_direct_mode)
	{
		// calculate app direct capacities
		for (int iset_idx = 0; iset_idx < p_pool->ilset_count; iset_idx++)
		{
			struct interleave_set iset = p_pool->ilsets[iset_idx];
			// is this dimm in the interleave set?
			for (int dimm_idx = 0; dimm_idx < iset.dimm_count; dimm_idx++)
			{
				if (uid_cmp(p_discovery->uid, iset.dimms[dimm_idx]))
				{
					if (iset.mirrored)
					{
						p_dimm_free_caps->app_direct_mirrored_capacity +=
							(iset.available_size / iset.dimm_count);
					}
					else
					{
						NVM_UINT64 used_ad = 0;
						if (iset.settings.ways == INTERLEAVE_WAYS_1)
						{
							p_dimm_free_caps->app_direct_byone_capacity +=
								iset.available_size;
							used_ad = iset.size;
							REDUCE_CAPACITY(used_ad, iset.available_size)
						}
						else
						{
							p_dimm_free_caps->app_direct_interleaved_capacity +=
								(iset.available_size / iset.dimm_count);
							used_ad = (iset.size / iset.dimm_count);
							REDUCE_CAPACITY(used_ad,
								p_dimm_free_caps->app_direct_interleaved_capacity);

						}
					}
					break;
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Return 1 if the specified interleave set already contains a namespaces, 0 if not.
 */
NVM_BOOL interleave_set_has_namespace(const NVM_UINT32 interleave_set_driver_id,
	const struct nvm_namespace_details *p_namespaces, int ns_count)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL ns_exists = 0;

	for (int i = 0; i < ns_count; i++)
	{
		if (p_namespaces[i].type == NAMESPACE_TYPE_APP_DIRECT &&
			(p_namespaces[i].namespace_creation_id.interleave_setid == interleave_set_driver_id))
		{
			ns_exists = 1;
			break;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(ns_exists);
	return ns_exists;
}
