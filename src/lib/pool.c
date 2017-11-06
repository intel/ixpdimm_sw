/*
 * Copyright (c) 2015 2017, Intel Corporation
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
 * This file contains the implementation of native API functions to get pools
 * of capacity, dump configuration and load configuration.
 */

#include "nvm_management.h"
#include "device_adapter.h"
#include "pool_utilities.h"
#include "config_goal.h"
#include "utility.h"
#include "platform_config_data.h"
#include <uid/uid.h>
#include <persistence/lib_persistence.h>
#include <string/s_str.h>
#include <string/x_str.h>
#include <file_ops/file_ops_adapter.h>
#include "device_utilities.h"
#include "capabilities.h"
#include "nvm_context.h"
#include "system.h"
#include "nfit_utilities.h"
#include "namespace_utils.h"

// defined in device.c
extern int get_device_status_by_handle(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct device_status *p_status, struct nvm_capabilities *p_capabilities);

#define	MAX_POOLS	9
/*
 * Data required to create pools
 */
struct pool_data
{
	struct host host;
	struct nvm_capabilities capabilities;
	int dimm_count;
	struct device_discovery *dimm_list;
	struct device_capacities *dimm_capacities_list;
	struct device_status *dimm_status_list;
	struct config_goal *dimm_goal_list;
	int iset_count;
	struct nvm_interleave_set *iset_list;
	int namespace_count;
	struct nvm_namespace_details *namespace_list;
};

// Macro used in interleave calculations - hold onto more severe health
#define	KEEP_INTERLEAVE_HEALTH(health, new_health)	\
{ \
	if ((new_health > health) || (health == INTERLEAVE_HEALTH_NORMAL)) \
	{ \
		health = new_health; \
	} \
}

// Macro used in interleave calculations - hold onto more severe health
#define	KEEP_POOL_HEALTH(health, new_health)	\
{ \
	if ((new_health > health) || (health == POOL_HEALTH_NORMAL)) \
	{ \
		health = new_health; \
	} \
}

int update_pool_security_from_iset(struct pool *p_pool,
		const struct interleave_set *p_set,
		const struct pool_data *p_pool_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// can an app direct ns be created on this iset
	if (p_set->available_size >
		p_pool_data->capabilities.sw_capabilities.min_namespace_size)
	{
		if (p_set->encryption == NVM_ENCRYPTION_ON)
		{
			p_pool->encryption_enabled = 1;
		}
		if (p_set->erase_capable == 1)
		{
			p_pool->erase_capable = 1;
		}

		p_pool->encryption_capable = 1;
		for (int dimm_pool_idx = 0; dimm_pool_idx < p_set->dimm_count &&
				p_pool->encryption_capable; dimm_pool_idx++)
		{
			// find the dimm in the pool data
			for (int dimm_data_idx = 0; dimm_data_idx < p_pool_data->dimm_count;
					dimm_data_idx++)
			{
				if (uid_cmp(p_pool->dimms[dimm_pool_idx],
						p_pool_data->dimm_list[dimm_data_idx].uid))
				{
					if (!p_pool_data->dimm_list[dimm_data_idx].\
							security_capabilities.passphrase_capable)
					{
						p_pool->encryption_capable = 0;
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
 * Deallocate the memory in a pool_data structure
 */
void free_pool_data(struct pool_data *p_pool_data)
{
	COMMON_LOG_ENTRY();
	if (p_pool_data)
	{
		if (p_pool_data->dimm_count)
		{
			if (p_pool_data->dimm_list)
			{
				free(p_pool_data->dimm_list);
			}
			if (p_pool_data->dimm_capacities_list)
			{
				free(p_pool_data->dimm_capacities_list);
			}
			if (p_pool_data->dimm_status_list)
			{
				free(p_pool_data->dimm_status_list);
			}
			if (p_pool_data->dimm_goal_list)
			{
				free(p_pool_data->dimm_goal_list);
			}
		}
		if (p_pool_data->iset_count && p_pool_data->iset_list)
		{
				free(p_pool_data->iset_list);
		}
		if (p_pool_data->namespace_count && p_pool_data->namespace_list)
		{
				free(p_pool_data->namespace_list);
		}
		free(p_pool_data);
	}
	COMMON_LOG_EXIT();
}

/*
 * Retrieve namespaces and store them in the pool data structure
 */
int collect_namespaces(struct pool_data **pp_pool_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	// get namespaces
	(*pp_pool_data)->namespace_count = get_namespace_count();
	if ((*pp_pool_data)->namespace_count < 0)
	{
		rc = (*pp_pool_data)->namespace_count;
	}
	else if ((*pp_pool_data)->namespace_count > 0)
	{
		struct nvm_namespace_discovery namespaces[(*pp_pool_data)->namespace_count];
		(*pp_pool_data)->namespace_count =
				get_namespaces((*pp_pool_data)->namespace_count, namespaces);
		if ((*pp_pool_data)->namespace_count < 0)
		{
			rc = (*pp_pool_data)->namespace_count;
		}
		else if ((*pp_pool_data)->namespace_count > 0)
		{
			(*pp_pool_data)->namespace_list = calloc((*pp_pool_data)->namespace_count,
					sizeof (struct nvm_namespace_details));
			if (!(*pp_pool_data)->namespace_list)
			{
				COMMON_LOG_ERROR("No memory to collect pool information");
				rc = NVM_ERR_NOMEMORY;
			}
			else
			{
				for (int i = 0; i < (*pp_pool_data)->namespace_count; i++)
				{
					rc = get_namespace_details(namespaces[i].namespace_uid,
							&(*pp_pool_data)->namespace_list[i]);
					if (rc != NVM_SUCCESS)
					{
						break;
					}
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve interleave sets and store them in the pool data structure
 */
int collect_interleave_sets(struct pool_data **pp_pool_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	(*pp_pool_data)->iset_count = get_interleave_set_count();
	if ((*pp_pool_data)->iset_count < 0)
	{
		rc = (*pp_pool_data)->iset_count;
		(*pp_pool_data)->iset_count = 0;
	}
	else if ((*pp_pool_data)->iset_count > 0)
	{
		(*pp_pool_data)->iset_list = calloc((*pp_pool_data)->iset_count,
				sizeof (struct nvm_interleave_set));
		if (!(*pp_pool_data)->iset_list)
		{
			COMMON_LOG_ERROR("No memory to collect pool information");
			rc = NVM_ERR_NOMEMORY;
		}
		else
		{
			(*pp_pool_data)->iset_count = get_interleave_sets(
					(*pp_pool_data)->iset_count, (*pp_pool_data)->iset_list);
			if ((*pp_pool_data)->iset_count < 0)
			{
				rc = (*pp_pool_data)->iset_count;
				(*pp_pool_data)->iset_count = 0;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve config goal for each manageable DIMM in the pool data struct
 * and store them.
 */
int collect_dimm_goals(struct pool_data **pp_pool_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if ((*pp_pool_data)->dimm_count > 0)
	{
		(*pp_pool_data)->dimm_goal_list = calloc(
			(*pp_pool_data)->dimm_count, sizeof (struct config_goal));
		if (!(*pp_pool_data)->dimm_goal_list)
		{
			COMMON_LOG_ERROR("No memory to collect pool information");
			rc = NVM_ERR_NOMEMORY;
		}
		else
		{
			for (int i = 0; i < (*pp_pool_data)->dimm_count; i++)
			{
				// ignore failures because not all dimms have goals
				nvm_get_config_goal(
					(*pp_pool_data)->dimm_list[i].uid,
					&(*pp_pool_data)->dimm_goal_list[i]);
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve status for each manageable DIMM in the pool data struct
 * and store them.
 */
int collect_dimm_statuses(struct pool_data **pp_pool_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if ((*pp_pool_data)->dimm_count > 0)
	{
		(*pp_pool_data)->dimm_status_list = calloc(
			(*pp_pool_data)->dimm_count, sizeof (struct device_status));
		if (!(*pp_pool_data)->dimm_status_list)
		{
			COMMON_LOG_ERROR("No memory to collect pool information");
			rc = NVM_ERR_NOMEMORY;
		}
		else
		{
			for (int i = 0; i < (*pp_pool_data)->dimm_count; i++)
			{
				// ignore failures because not all status can be retrieved
				get_device_status_by_handle(
					(*pp_pool_data)->dimm_list[i].device_handle,
					&(*pp_pool_data)->dimm_status_list[i],
					&(*pp_pool_data)->capabilities);
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve capacities for each manageable DIMM in the pool data struct
 * and store them.
 */
int collect_dimm_capacities(struct pool_data **pp_pool_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if ((*pp_pool_data)->dimm_count > 0)
	{
		(*pp_pool_data)->dimm_capacities_list = calloc(
			(*pp_pool_data)->dimm_count, sizeof (struct device_capacities));
		if (!(*pp_pool_data)->dimm_capacities_list)
		{
			COMMON_LOG_ERROR("No memory to collect pool information");
			rc = NVM_ERR_NOMEMORY;
		}
		else
		{
			for (int i = 0; i < (*pp_pool_data)->dimm_count; i++)
			{
				rc = get_dimm_capacities(&(*pp_pool_data)->dimm_list[i],
						&(*pp_pool_data)->capabilities,
						&(*pp_pool_data)->dimm_capacities_list[i]);
				if (rc != NVM_SUCCESS)
				{
					break;
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve a list of manageable DIMMs and store them in the pool data struct
 */
int collect_manageable_dimms(struct pool_data **pp_pool_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	(*pp_pool_data)->dimm_count = get_manageable_dimms(&(*pp_pool_data)->dimm_list);
	if ((*pp_pool_data)->dimm_count < 0)
	{
		rc = (*pp_pool_data)->dimm_count;
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve system capabilities and store them in the pool data structure
 */
int collect_capabilities(struct pool_data **pp_pool_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	rc = nvm_get_nvm_capabilities(&(*pp_pool_data)->capabilities);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve host info and store them in the pool data struct
 */
int collect_host(struct pool_data **pp_pool_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	rc = get_host(&(*pp_pool_data)->host);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Gather all the required data to populate the pool information
 */
int collect_required_pool_data(struct pool_data **pp_pool_data,
		const NVM_BOOL count_only)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	*pp_pool_data = calloc(1, sizeof (struct pool_data));
	if (!(*pp_pool_data))
	{
		COMMON_LOG_ERROR("No memory to collect pool information");
		rc = NVM_ERR_NOMEMORY;
	}
	// get the manageable DIMMs
	else if ((rc = collect_manageable_dimms(pp_pool_data)) != NVM_SUCCESS)
	{
		free_pool_data(*pp_pool_data);
	}
	// no reason to continue if no manageable DIMMs
	else if ((*pp_pool_data)->dimm_count)
	{
		// get the system capabilities
		if ((rc = collect_capabilities(pp_pool_data)) != NVM_SUCCESS)
		{
			free_pool_data(*pp_pool_data);
		}
		// get DIMM capacities for each manageable DIMM
		else if ((rc = collect_dimm_capacities(pp_pool_data)) != NVM_SUCCESS)
		{
			free_pool_data(*pp_pool_data);
		}
		// get interleave sets
		else if ((rc = collect_interleave_sets(pp_pool_data)) != NVM_SUCCESS)
		{
			free_pool_data(*pp_pool_data);
		}


		// this data is not needed when just counting pools
		if (rc == NVM_SUCCESS && !count_only)
		{
			// get the host
			if ((rc = collect_host(pp_pool_data)) != NVM_SUCCESS)
			{
				free_pool_data(*pp_pool_data);
			}
			// collect DIMM goals
			else if ((rc = collect_dimm_goals(pp_pool_data)) != NVM_SUCCESS)
			{
				free_pool_data(*pp_pool_data);
			}
			// get DIMM status for each manageable DIMM
			else if ((rc = collect_dimm_statuses(pp_pool_data)) != NVM_SUCCESS)
			{
				free_pool_data(*pp_pool_data);
			}
			// get namespaces
			else if ((rc = collect_namespaces(pp_pool_data)) != NVM_SUCCESS)
			{
				free_pool_data(*pp_pool_data);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the index of a pool from the pool list
 */
int get_pool_index(const NVM_INT16 pool_socket_id,
		const enum pool_type pool_type, const struct pool *p_pools,
		const int count)
{
	COMMON_LOG_ENTRY();
	int index = -1;
	for (int i = 0; i < count; i++)
	{
		if (p_pools[i].socket_id == pool_socket_id &&
			p_pools[i].type == pool_type)
		{
			index = i;
			break;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(index);
	return index;
}

/*
 * Retrieve the index of a DIMM from a pool
 */
int get_dimm_index_from_pool(const NVM_UID dimm_uid, struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int index = -1;
	for (int i = 0; i < p_pool->dimm_count; i++)
	{
		if (uid_cmp(p_pool->dimms[i], dimm_uid))
		{
			index = i;
			break;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(index);
	return index;
}

/*
 * Add a DIMM to a pool
 */
void add_dimm_to_pool(const struct pool_data *p_pool_data,
		int data_idx, struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int dimm_index = get_dimm_index_from_pool(
			p_pool_data->dimm_list[data_idx].uid, p_pool);
	if (dimm_index < 0)
	{
		// update the pool based on this new DIMM
		struct device_discovery dimm = p_pool_data->dimm_list[data_idx];
		struct device_capacities caps = p_pool_data->dimm_capacities_list[data_idx];
		struct device_status status = p_pool_data->dimm_status_list[data_idx];
		struct config_goal goal = p_pool_data->dimm_goal_list[data_idx];

		struct device_free_capacities free_caps;
		memset(&free_caps, 0, sizeof (struct device_free_capacities));
		get_dimm_free_capacities(
				&p_pool_data->capabilities,
				&dimm,
				p_pool,
				p_pool_data->namespace_list,
				p_pool_data->namespace_count,
				&caps,
				&free_caps);

		dimm_index = p_pool->dimm_count;
		uid_copy(dimm.uid, p_pool->dimms[p_pool->dimm_count]);
		p_pool->raw_capacities[dimm_index] = caps.capacity;
		if (p_pool->type == POOL_TYPE_VOLATILE)
		{
			p_pool->memory_capacities[dimm_index] = caps.memory_capacity;
			p_pool->capacity += caps.memory_capacity;
			p_pool->free_capacity += caps.memory_capacity;
		}
		else if (p_pool->type == POOL_TYPE_PERSISTENT)
		{
			NVM_UINT64 ad_cap = caps.app_direct_capacity;
			REDUCE_CAPACITY(ad_cap, caps.mirrored_app_direct_capacity * 2llu);
			p_pool->capacity += ad_cap;
			p_pool->free_capacity += free_caps.app_direct_byone_capacity +
				free_caps.app_direct_interleaved_capacity;
		}
		else if (p_pool->type == POOL_TYPE_PERSISTENT_MIRROR)
		{
			p_pool->capacity += caps.mirrored_app_direct_capacity;
			p_pool->free_capacity += free_caps.app_direct_mirrored_capacity;
		}

		if (dimm.lock_state == LOCK_STATE_LOCKED ||
			dimm.lock_state == LOCK_STATE_PASSPHRASE_LIMIT)
		{
			KEEP_POOL_HEALTH(p_pool->health, POOL_HEALTH_LOCKED);
		}
		if (goal.status == CONFIG_GOAL_STATUS_NEW)
		{
			KEEP_POOL_HEALTH(p_pool->health, POOL_HEALTH_PENDING);
		}
		if (status.health == DEVICE_HEALTH_UNKNOWN)
		{
			KEEP_POOL_HEALTH(p_pool->health, POOL_HEALTH_UNKNOWN);
		}
		else if (status.health != DEVICE_HEALTH_NORMAL ||
				status.viral_state)
		{
			KEEP_POOL_HEALTH(p_pool->health, POOL_HEALTH_ERROR);
		}
		if ((status.config_status != CONFIG_STATUS_NOT_CONFIGURED) &&
			(status.config_status != CONFIG_STATUS_VALID))
		{
			KEEP_POOL_HEALTH(p_pool->health, POOL_HEALTH_ERROR);
		}
		p_pool->dimm_count++;
	}
	COMMON_LOG_EXIT();
}

int get_dimm_index_from_handle(const struct pool_data *p_pool_data,
	const NVM_UINT32 device_handle)
{
	COMMON_LOG_ENTRY();
	int index = -1;
	for (int i = 0; i < p_pool_data->dimm_count; i++)
	{
		if (p_pool_data->dimm_list[i].device_handle.handle == device_handle)
		{
			index = i;
			break;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(index);
	return index;
}

void calculate_iset_free_capacity(const struct pool_data *p_pool_data,
		struct interleave_set *p_iset)
{
	p_iset->available_size = p_iset->size;
	for (int i = 0; i < p_pool_data->namespace_count; i++)
	{
		struct nvm_namespace_details ns = p_pool_data->namespace_list[i];
		if (ns.type == NAMESPACE_TYPE_APP_DIRECT &&
			ns.namespace_creation_id.interleave_setid == p_iset->driver_id)
		{
			NVM_UINT64 ns_capacity = (ns.block_count * ns.block_size);
			REDUCE_CAPACITY(p_iset->available_size, ns_capacity);
		}
	}
}

void nvm_interleave_to_interleave(
		const struct pool_data *p_pool_data,
		const struct nvm_interleave_set *p_nvm_iset,
		struct interleave_set *p_iset)
{
	COMMON_LOG_ENTRY();

	p_iset->driver_id = p_nvm_iset->id;
	p_iset->size = p_nvm_iset->size;
	calculate_iset_free_capacity(p_pool_data, p_iset);
	p_iset->socket_id = p_nvm_iset->socket_id;
	if (p_nvm_iset->dimm_count)
	{
		p_iset->health = INTERLEAVE_HEALTH_NORMAL;
		p_iset->encryption = NVM_ENCRYPTION_ON;
		p_iset->erase_capable = 1;

		for (int dimm_idx = 0; dimm_idx < p_nvm_iset->dimm_count; dimm_idx++)
		{
			NVM_BOOL iset_settings_filled = 0;
			// find the dimm by the handle
			for (int data_idx = 0; data_idx < p_pool_data->dimm_count; data_idx++)
			{
				if (p_nvm_iset->dimms[dimm_idx] ==
						p_pool_data->dimm_list[data_idx].device_handle.handle)
				{
					uid_copy(p_pool_data->dimm_list[data_idx].uid, p_iset->dimms[dimm_idx]);
					if (!p_pool_data->dimm_list[data_idx].\
							security_capabilities.passphrase_capable)
					{
						p_iset->erase_capable = 0;
						p_iset->encryption = NVM_ENCRYPTION_OFF;
					}
					else
					{
						if (!p_pool_data->dimm_list[data_idx].\
								security_capabilities.erase_crypto_capable)
						{
							p_iset->erase_capable = 0;
						}
						if (!device_is_encryption_enabled(
								p_pool_data->dimm_list[data_idx].lock_state))
						{
							p_iset->encryption = NVM_ENCRYPTION_OFF;
						}
					}

					if (p_pool_data->dimm_status_list[data_idx].health ==
						DEVICE_HEALTH_UNKNOWN)
					{
						KEEP_INTERLEAVE_HEALTH(p_iset->health, INTERLEAVE_HEALTH_UNKNOWN)
					}
					else if (p_pool_data->dimm_status_list[data_idx].health ==
						DEVICE_HEALTH_FATAL)
					{
						KEEP_INTERLEAVE_HEALTH(p_iset->health, INTERLEAVE_HEALTH_FAILED)
					}
					else if (p_pool_data->dimm_status_list[data_idx].health !=
							DEVICE_HEALTH_NORMAL)
					{
						KEEP_INTERLEAVE_HEALTH(p_iset->health, INTERLEAVE_HEALTH_DEGRADED)
					}

					p_iset->mirrored = MIRRORED_INTERLEAVE(p_nvm_iset->attributes);

					// fill the interleave settings from pcd
					if (!iset_settings_filled)
					{
						int rc = fill_interleave_set_settings_and_id_from_dimm(
							p_iset, p_pool_data->dimm_list[data_idx].device_handle,
							p_nvm_iset->dimm_region_pdas[dimm_idx]);
						if (rc == NVM_SUCCESS)
						{
							iset_settings_filled = 1;
						}
					}
					p_iset->dimm_count++;
				}
			}
		}
	}
	if (p_iset->dimm_count != p_nvm_iset->dimm_count)
	{
		p_iset->health = INTERLEAVE_HEALTH_FAILED;
		p_iset->encryption = NVM_ENCRYPTION_OFF;
		p_iset->erase_capable = 0;
	}

#if 0
	printf("p_iset->socket_id = 0x%x\n", p_iset->socket_id);
	printf("p_iset->driver_id = 0x%x\n", p_iset->driver_id);
	printf("p_iset->size = 0x%llx\n", p_iset->size);
	printf("p_iset->available_size = 0x%llx\n", p_iset->available_size);
	printf("p_iset->encryption = 0x%x\n", p_iset->encryption);
	printf("p_iset->erase_capable = 0x%x\n", p_iset->erase_capable);
	printf("p_iset->health = 0x%x\n", p_iset->health);
	printf("p_iset->dimm_count = 0x%x\n", p_iset->dimm_count);
	printf("p_iset->set_index = 0x%x\n", p_iset->set_index);
	printf("p_iset->mirrored = 0x%x\n", p_iset->mirrored);
	printf("p_iset->settings.ways = 0x%x\n", p_iset->settings.ways);
	printf("p_iset->settings.channel = 0x%x\n", p_iset->settings.channel);
	printf("p_iset->settings.imc = 0x%x\n", p_iset->settings.imc);
#endif

	COMMON_LOG_EXIT();
}

/*
 * Add an interleave set to a pool
 */
void add_interleave_set_to_pool(const struct pool_data *p_pool_data,
		struct pool *p_pool, struct nvm_interleave_set *p_iset)
{
	COMMON_LOG_ENTRY();

	// convert the NVM interleave and fill missing data
	nvm_interleave_to_interleave(p_pool_data, p_iset,
			&p_pool->ilsets[p_pool->ilset_count]);
	struct interleave_set set = p_pool->ilsets[p_pool->ilset_count];
	p_pool->ilset_count++;
	// update the pool based on the interleave set
	for (int iset_dimm = 0; iset_dimm < set.dimm_count; iset_dimm++)
	{
		for (int data_dimm = 0; data_dimm < p_pool_data->dimm_count; data_dimm++)
		{
			if (uid_cmp(p_pool_data->dimm_list[data_dimm].uid, set.dimms[iset_dimm]))
			{
				add_dimm_to_pool(p_pool_data, data_dimm, p_pool);
				break;
			}
		}
	}
	if (set.health == INTERLEAVE_HEALTH_UNKNOWN)
	{
		KEEP_POOL_HEALTH(p_pool->health, POOL_HEALTH_UNKNOWN);
	}
	else if (set.health == INTERLEAVE_HEALTH_DEGRADED ||
			set.health == INTERLEAVE_HEALTH_FAILED)
	{
		KEEP_POOL_HEALTH(p_pool->health, POOL_HEALTH_ERROR);
	}
	update_pool_security_from_iset(p_pool, &set, p_pool_data);

	COMMON_LOG_EXIT();
}

void zero_pool_free_capacity_if_locked(struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	if (p_pool->health == POOL_HEALTH_LOCKED)
	{
		p_pool->free_capacity = 0;
	}
	COMMON_LOG_EXIT();
}

/*
 * Create pools from interleave sets
 */
int add_interleave_sets_to_pools(struct pool_data *p_pool_data,
		struct pool *p_pools, const NVM_UINT8 count, int *p_pool_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// create pools from interleave sets
	for (int i = 0; i < p_pool_data->iset_count; i++)
	{
		enum pool_type pool_type = POOL_TYPE_PERSISTENT;
		if (MIRRORED_INTERLEAVE(p_pool_data->iset_list[i].attributes))
		{
			pool_type = POOL_TYPE_PERSISTENT_MIRROR;
		}
		// if not already created, create a new pool
		int pool_index = get_pool_index(p_pool_data->iset_list[i].socket_id,
				pool_type, p_pools, *p_pool_count);
		if (pool_index < 0)
		{
			if (*p_pool_count >= count)
			{
				rc = NVM_ERR_ARRAYTOOSMALL;
			}
			else
			{
				pool_index = *p_pool_count;
				rc = init_pool(&p_pools[*p_pool_count], p_pool_data->host.name, pool_type,
						p_pool_data->iset_list[i].socket_id);
				if (rc != NVM_SUCCESS)
				{
					break;
				}
				(*p_pool_count)++;
			}
		}
		// add this interleave set to the pool
		if (pool_index >= 0)
		{
			add_interleave_set_to_pool(p_pool_data, &p_pools[pool_index],
				&p_pool_data->iset_list[i]);
			zero_pool_free_capacity_if_locked(&p_pools[pool_index]);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int add_dimms_volatile_capacity_to_pools(struct pool_data *p_pool_data,
	struct pool *p_pools, const NVM_UINT8 count, int *p_pool_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	for (int dimm_idx = 0; dimm_idx < p_pool_data->dimm_count; dimm_idx++)
	{
		struct device_capacities cap = p_pool_data->dimm_capacities_list[dimm_idx];
		if (cap.memory_capacity)
		{
			int pool_index = get_pool_index(-1, POOL_TYPE_VOLATILE, p_pools, *p_pool_count);
			if (pool_index < 0)
			{
				if (*p_pool_count >= count)
				{
					rc = NVM_ERR_ARRAYTOOSMALL;
					break; // only one volatile pool, so just exit on failure
				}
				pool_index = *p_pool_count;
				rc = init_pool(&p_pools[pool_index], p_pool_data->host.name,
						POOL_TYPE_VOLATILE, -1);
				if (rc != NVM_SUCCESS)
				{
					break;
				}
				(*p_pool_count)++;
			}
			add_dimm_to_pool(p_pool_data, dimm_idx, &p_pools[pool_index]);
			zero_pool_free_capacity_if_locked(&p_pools[pool_index]);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Given all the required data, populate the pool list
 */
int populate_pools(struct pool_data *p_pool_data,
		struct pool *p_pools, const NVM_UINT8 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	int pool_count = 0;

	rc = add_dimms_volatile_capacity_to_pools(p_pool_data, p_pools, count, &pool_count);
	if (rc == NVM_SUCCESS)
	{
		rc = pool_count;
	}
	if (rc >= 0 && p_pool_data->capabilities.nvm_features.app_direct_mode)
	{
		rc = add_interleave_sets_to_pools(p_pool_data, p_pools, count, &pool_count);
		if (rc == NVM_SUCCESS)
		{
			rc = pool_count;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Count the pools in the system given a pool data struct
 *
 * Returns: Number of pools (integer >= 0) if success
 *          NVM_ERR_BADPOOLHEALTH if we have an invalid number of pools
 */
int count_pools(struct pool_data *p_pool_data)
{
	COMMON_LOG_ENTRY();
	int pool_count = 0;

	struct pool *pools_list = calloc(MAX_POOLS, sizeof (struct pool));
	if (pools_list)
	{
		for (int i = 0; i < p_pool_data->dimm_count; i++)
		{
			if (p_pool_data->dimm_capacities_list[i].memory_capacity)
			{
				int pool_index = get_pool_index(-1, POOL_TYPE_VOLATILE, pools_list, pool_count);
				if (pool_index < 0)
				{
					pools_list[pool_count].socket_id = -1;
					pools_list[pool_count].type = POOL_TYPE_VOLATILE;
					pool_count++;
				}
			}
		}

		// is pm pool on each socket
		if (p_pool_data->capabilities.nvm_features.app_direct_mode)
		{
			for (int i = 0; i < p_pool_data->iset_count; i++)
			{
				enum pool_type pool_type = POOL_TYPE_PERSISTENT;
				if (MIRRORED_INTERLEAVE(p_pool_data->iset_list[i].attributes))
				{
					pool_type = POOL_TYPE_PERSISTENT_MIRROR;
				}
				// if not already created, create a new pool
				int pool_index = get_pool_index(p_pool_data->iset_list[i].socket_id,
						pool_type, pools_list, pool_count);
				if (pool_index < 0)
				{
					pools_list[pool_count].socket_id = p_pool_data->iset_list[i].socket_id;
					pools_list[pool_count].type = pool_type;
					pool_count++;

					// Check for too many pools
					if (pool_count > MAX_POOLS)
					{
						pool_count = NVM_ERR_BADPOOLHEALTH;
						break;
					}
				}
			}
		}

		// clean up
		free(pools_list);
	}

	COMMON_LOG_EXIT_RETURN_I(pool_count);
	return pool_count;
}

/*
 * Retrieve the number of configured pools of NVM-DIMM capacity in the host server.
 */
int nvm_get_pool_count()
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(get_pools)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving pools is not supported.");
	}
	else if ((rc = is_ars_in_progress()) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving pools is not supported when an ARS is in progress.");
	}
	else if ((rc = get_nvm_context_pool_count()) < 0)
	{
		struct pool_data *p_pool_data = NULL;
		rc = collect_required_pool_data(&p_pool_data, 1);
		if (rc == NVM_SUCCESS && p_pool_data)
		{
			rc = count_pools(p_pool_data);
			free_pool_data(p_pool_data);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 *
 * Retrieve a list of the configured pools of NVM-DIMM capacity in host server.
 */
int nvm_get_pools(struct pool *p_pools, const NVM_UINT8 count)
{
	COMMON_LOG_ENTRY();
	int rc = 0; // 0 pools

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(get_pools)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving pools is not supported.");
	}
	else if (p_pools == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_pools is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (count == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, count is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = is_ars_in_progress()) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving pools is not supported when an ARS is in progress.");
	}
	else if ((rc = get_nvm_context_pools(p_pools, count)) < 0 &&
			rc != NVM_ERR_ARRAYTOOSMALL)
	{
		struct pool_data *p_pool_data = NULL;
		rc = collect_required_pool_data(&p_pool_data, 0);
		if (rc == NVM_SUCCESS && p_pool_data)
		{
			rc = populate_pools(p_pool_data, p_pools, count);
			// update the context
			if (rc > 0)
			{
				set_nvm_context_pools(p_pools, rc);
			}
			free_pool_data(p_pool_data);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve a list of the configured pools of NVM-DIMM capacity in host server.
 */
int nvm_get_pool(const NVM_UID pool_uid, struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_BADPOOL;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(get_pools)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving pools is not supported.");
	}
	else if (pool_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, pool_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_pool == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_pool is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = is_ars_in_progress()) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving pools is not supported when an ARS is in progress.");
	}
	else if ((rc = get_nvm_context_pool(pool_uid, p_pool)) != NVM_SUCCESS)
	{
		struct pool_data *p_pool_data = NULL;
		rc = collect_required_pool_data(&p_pool_data, 0);
		if (rc == NVM_SUCCESS && p_pool_data)
		{
			rc = count_pools(p_pool_data);
			if (rc > 0)
			{
				int pool_count = rc;
				struct pool *pools = (struct pool *)calloc(pool_count, sizeof (struct pool));
				if (!pools)
				{
					COMMON_LOG_ERROR("Not enough memory to allocate pool list");
					rc = NVM_ERR_NOMEMORY;
				}
				else
				{
					rc = populate_pools(p_pool_data, pools, pool_count);
					if (rc > 0)
					{
						pool_count = rc;
						rc = NVM_ERR_BADPOOL;
						for (int i = 0; i < pool_count; i++)
						{
							if (uid_cmp(pools[i].pool_uid, pool_uid))
							{
								memmove(p_pool, &pools[i], sizeof (struct pool));
								rc = NVM_SUCCESS;
								break;
							}
						}
					}
					free(pools);
				}
			}
			free_pool_data(p_pool_data);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Store the configuration settings from the specified NVM-DIMM
 * to a file in order to duplicate the configuration elsewhere.
 */
int nvm_dump_config(const NVM_UID device_uid,
		const NVM_PATH path, const NVM_SIZE path_len,
		const NVM_BOOL append)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(get_device_capacity)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving device capacity is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path == NULL)
	{
		COMMON_LOG_ERROR("File path is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR_F(
				"Invalid parameter, path length is too big: %d; <= %d",
				path_len, NVM_PATH_LEN);
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, path length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		// get the platform config data for this dimm
		struct platform_config_data *p_cfg_data = NULL;
		int tmp_rc = get_dimm_platform_config(discovery.device_handle, &p_cfg_data);
		if (tmp_rc != NVM_SUCCESS)
		{
			rc = tmp_rc; // propagate the error
		}
		else if (!p_cfg_data) // this should never happen, but just in case.
		{
			COMMON_LOG_ERROR("Config data structure returned is NULL");
			rc = NVM_ERR_BADDEVICECONFIG;
		}
		else
		{
			struct current_config_table *p_current_config =
					cast_current_config(p_cfg_data);
			// no current config is not valid
			if (!p_current_config)
			{
				COMMON_LOG_ERROR("No current config data");
				rc = NVM_ERR_NOTFOUND;
			}
			else
			{
				// Unconfigured dimms and unapplied memory allocation goals are not included
				if (p_current_config->config_status != CURRENT_CONFIG_STATUS_UNCONFIGURED)
				{
					// convert current config into config_goal struct
					struct config_goal goal;
					memset(&goal, 0, sizeof (goal));
					// default memory size to the current in case there
					// is no partition size change table
					goal.memory_size =
							(p_current_config->mapped_memory_capacity / BYTES_PER_GIB);
					rc = config_input_table_to_config_goal(device_uid,
							(unsigned char *)p_current_config, sizeof (struct current_config_table),
							p_current_config->header.length, &goal);
					if (rc == NVM_SUCCESS)
					{
						// write it to the path
						rc = write_dimm_config(&discovery, &goal, path, path_len, append);
					}
				}
				else
				{
					COMMON_LOG_ERROR("dimm is not configured.");
					rc = NVM_ERR_NOTFOUND;
				}
			}
		}
		if (p_cfg_data)
		{
			free(p_cfg_data);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int fill_goal_dimm_uids(FILE *p_file, struct config_goal *p_goal)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	int p1_count = 0;
	int p2_count = 0;

	rewind(p_file); // reset the file to the beginning
	NVM_UINT16 socket = 0;
	NVM_UINT32 dimm_handle = 0;
	NVM_UINT64 dimm_size_gb = 0;
	struct config_goal tmp;
	memset(&tmp, 0, sizeof (tmp));
	while ((rc = read_dimm_config(p_file, &tmp, &socket, &dimm_handle, &dimm_size_gb)) > 0)
	{
		// if our goal has a P1, check for a matching index
		if (p_goal->app_direct_count > 0 &&
			p_goal->app_direct_1_set_id == tmp.app_direct_1_set_id)
		{
			// convert the dimm_handle to a uid
			NVM_NFIT_DEVICE_HANDLE handle;
			handle.handle = dimm_handle;
			struct device_discovery discovery;
			if (lookup_dev_handle(handle, &discovery) == NVM_SUCCESS)
			{
				memmove(p_goal->app_direct_1_settings.dimms[p1_count],
						discovery.uid, NVM_MAX_UID_LEN);
				p1_count++;
			}
		}
		// if our goal has a P2 index and it matches this one
		if (p_goal->app_direct_count > 1 &&
				p_goal->app_direct_2_set_id == tmp.app_direct_2_set_id)
		{
			NVM_NFIT_DEVICE_HANDLE handle;
			handle.handle = dimm_handle;
			struct device_discovery discovery;
			if (lookup_dev_handle(handle, &discovery) == NVM_SUCCESS)
			{
				memmove(p_goal->app_direct_2_settings.dimms[p2_count],
						discovery.uid, NVM_MAX_UID_LEN);
				p2_count++;
			}
		}
	}

	// make sure we found all dimms in the interleave set for P1
	if (p_goal->app_direct_count > 0)
	{
		if (p1_count != p_goal->app_direct_1_settings.interleave.ways)
		{
			COMMON_LOG_ERROR(
					"Not all the DIMMs in the p1 interleave set were found in the file");
			rc = NVM_ERR_BADDEVICECONFIG;
		}
	}
	// make sure we found all dimms in the interleave set for P2
	if (p_goal->app_direct_count > 1)
	{
		if (p2_count != p_goal->app_direct_2_settings.interleave.ways)
		{
			COMMON_LOG_ERROR(
					"Not all the DIMMs in the p2 interleave set were found in the file");
			rc = NVM_ERR_BADDEVICECONFIG;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Apply a previously stored configuration goal from a file onto the specified NVM-DIMM.
 */
int nvm_load_config(const NVM_UID device_uid,
		const NVM_PATH path, const NVM_SIZE path_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_capacity)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying device capacity is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path == NULL)
	{
		COMMON_LOG_ERROR("File path is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR_F(
				"Invalid parameter, path length is too big: %d; <= %d",
				path_len, NVM_PATH_LEN);
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, path length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (!file_exists(path, path_len))
	{
		COMMON_LOG_ERROR_F("File %s does not exist", path);
		rc = NVM_ERR_BADFILE;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		// read the file to get the desired config goal for this dimm
		FILE *p_file = NULL;
		p_file = open_file(path, path_len, "r");
		if (!p_file)
		{
			COMMON_LOG_ERROR_F("Failed to open file %s", path);
			rc = NVM_ERR_BADFILE;
		}
		else
		{
			if (lock_file(p_file, FILE_LOCK_MODE_READ) != COMMON_SUCCESS)
			{
				COMMON_LOG_ERROR_F("Failed to lock file %s for reading", path);
				rc = NVM_ERR_BADFILE;
			}
			else
			{
				NVM_BOOL dimm_found = 0;
				NVM_UINT16 socket_id = 0;
				NVM_UINT32 dimm_handle = 0;
				NVM_UINT64 dimm_size_gb = 0;
				struct config_goal goal;
				memset(&goal, 0, sizeof (goal));
				while ((rc = read_dimm_config(p_file, &goal, &socket_id,
						&dimm_handle, &dimm_size_gb)) > 0)
				{
					// is this the dimm we're looking for?
					if (dimm_handle == discovery.device_handle.handle &&
						socket_id == discovery.device_handle.parts.socket_id)
					{
						dimm_found = 1;
						struct device_discovery discovery;
						if ((rc = nvm_get_device_discovery(device_uid, &discovery))
								== NVM_SUCCESS)
						{
							// make sure dimm is big enough for config
							if ((discovery.capacity / BYTES_PER_GIB) >= dimm_size_gb)
							{
								// set App Direct count
								if (goal.app_direct_1_size)
								{
									goal.app_direct_count = 1;
								}
								if (goal.app_direct_2_size)
								{
									goal.app_direct_count = 2;
								}
								// find matching dimm uids for the interleave sets
								if ((rc = fill_goal_dimm_uids(p_file, &goal)) == NVM_SUCCESS)
								{
									rc = nvm_create_config_goal(discovery.uid, &goal);
								}
							} // end dimm big enough
							else
							{
								COMMON_LOG_ERROR_F("The requested goal DIMM size in file %s is too"
									" big to fit on the DIMM in the system", path);
								rc = NVM_ERR_BADDEVICECONFIG;
							}
						} // end get discovery
						break;
					} // end match dimm/socket
				} // end getline
				if (rc == NVM_SUCCESS && !dimm_found)
				{
					COMMON_LOG_ERROR_F("The dimm config was not found in the file %s", path);
					rc = NVM_ERR_BADDEVICECONFIG;
				}
				// clean up the file
				lock_file(p_file, FILE_LOCK_MODE_UNLOCK);
			}
			fclose(p_file);
		} // end open file
	} // end exists and manageable

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
