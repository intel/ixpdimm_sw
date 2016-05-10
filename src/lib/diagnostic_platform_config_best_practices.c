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
 * This file contains the implementation of the platform configuration diagnostic's
 * best practices checks for the native API.
 */

#include "nvm_management.h"
#include "device_adapter.h"
#include "utility.h"
#include <uid/uid.h>
#include <persistence/event.h>
#include <string/s_str.h>
#include "pool_utilities.h"
#include "system.h"

NVM_BOOL pool_interleave_sets_need_namespace(
		const struct nvm_namespace_details *p_namespaces,
		const NVM_UINT32 namespace_count,
		const struct nvm_pool *p_pool)
{
	NVM_BOOL result = 0;

	if ((p_pool->type == POOL_TYPE_PERSISTENT) ||
			(p_pool->type == POOL_TYPE_PERSISTENT_MIRROR))
	{
		for (NVM_UINT16 i = 0; i < p_pool->ilset_count; i++)
		{
			NVM_UINT32 interleave_id = p_pool->ilsets[i].driver_id;
			NVM_BOOL namespace_found = 0;
			for (NVM_UINT32 j = 0; j < namespace_count; j++)
			{
				if ((p_namespaces[j].type == NAMESPACE_TYPE_APP_DIRECT) &&
						(p_namespaces[j].namespace_creation_id.interleave_setid == interleave_id))
				{
					namespace_found = 1;
					break;
				}
			}

			if (!namespace_found)
			{
				result = 1;
				break;
			}
		}
	}

	return result;
}

NVM_BOOL pool_storage_regions_need_namespace(
		const struct nvm_namespace_details *p_namespaces,
		const NVM_UINT32 namespace_count,
		const struct nvm_pool *p_pool)
{
	NVM_BOOL result = 0;

	if (p_pool->type == POOL_TYPE_PERSISTENT)
	{
		for (NVM_UINT16 i = 0; i < p_pool->dimm_count; i++)
		{
			if (p_pool->storage_capacities[i] > 0)
			{
				NVM_BOOL namespace_found = 0;
				for (NVM_UINT32 j = 0; j < namespace_count; j++)
				{
					if ((p_namespaces[j].type == NAMESPACE_TYPE_STORAGE) &&
							(p_namespaces[j].namespace_creation_id.device_handle.handle ==
									p_pool->dimms[i].handle))
					{
						namespace_found = 1;
						break;
					}
				}

				if (!namespace_found)
				{
					result = 1;
					break;
				}
			}
		}
	}

	return result;
}

void check_if_pool_interleave_sets_need_namespace(NVM_UINT32 *p_results,
		const struct nvm_namespace_details *p_namespaces,
		const NVM_UINT32 namespace_count,
		const struct nvm_pool *p_pool)
{
	if (pool_interleave_sets_need_namespace(p_namespaces, namespace_count, p_pool))
	{
		NVM_UID pool_uid_str;
		uid_copy(p_pool->pool_uid, pool_uid_str);

		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_INFO,
				EVENT_CODE_DIAG_PCONFIG_POOL_NEEDS_APP_DIRECT_NAMESPACES,
				p_pool->pool_uid,
				0,
				pool_uid_str, NULL, NULL,
				DIAGNOSTIC_RESULT_WARNING);

		(*p_results)++;
	}
}

void check_if_pool_storage_regions_need_namespace(NVM_UINT32 *p_results,
		const struct nvm_namespace_details *p_namespaces,
		const NVM_UINT32 namespace_count,
		const struct nvm_pool *p_pool)
{
	if (pool_storage_regions_need_namespace(p_namespaces, namespace_count, p_pool))
	{
		NVM_UID pool_uid_str;
		uid_copy(p_pool->pool_uid, pool_uid_str);

		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_INFO,
				EVENT_CODE_DIAG_PCONFIG_POOL_NEEDS_STORAGE_NAMESPACES,
				p_pool->pool_uid,
				0,
				pool_uid_str, NULL, NULL,
				DIAGNOSTIC_RESULT_WARNING);

		(*p_results)++;
	}
}

void check_if_pools_need_namespaces(NVM_UINT32 *p_results,
		const struct nvm_namespace_details *p_namespaces,
		const NVM_UINT32 namespace_count,
		const struct nvm_pool *p_pools,
		const NVM_UINT32 pool_count)
{
	COMMON_LOG_ENTRY();

	for (NVM_UINT32 i = 0; i < pool_count; i++)
	{
		if (p_pools[i].type != POOL_TYPE_VOLATILE)
		{
			check_if_pool_interleave_sets_need_namespace(p_results,
					p_namespaces, namespace_count,
					&(p_pools[i]));

			check_if_pool_storage_regions_need_namespace(p_results,
					p_namespaces, namespace_count,
					&(p_pools[i]));
		}
	}

	COMMON_LOG_EXIT();
}

const struct nvm_pool *get_namespace_pool(const NVM_UID namespace_uid,
		const struct nvm_pool *p_pools, const NVM_UINT32 pool_count)
{
	const struct nvm_pool *p_pool = NULL;

	struct nvm_namespace_details details;
	memset(&details, 0, sizeof (details));
	int rc = get_namespace_details(namespace_uid, &details);
	if (rc == NVM_SUCCESS)
	{
		NVM_UID pool_uid;
		rc = get_pool_uid_from_namespace_details(&details, &pool_uid);
		if (rc == NVM_SUCCESS)
		{
			for (NVM_UINT32 i = 0; i < pool_count; i++)
			{
				if (uid_cmp(pool_uid, p_pools[i].pool_uid) == 1)
				{
					p_pool = &(p_pools[i]);
					break;
				}
			}
		}
	}
	else
	{
		NVM_UID namespace_uid_str;
		uid_copy(namespace_uid, namespace_uid_str);
		COMMON_LOG_ERROR_F("couldn't get namespace details for '%s', rc = %d",
				namespace_uid_str, rc);
	}

	return p_pool;
}

NVM_UINT64 get_size_bytes_difference_between_namespace_and_containing_interleave_set(
		const struct nvm_namespace_details *p_namespace,
		const struct nvm_pool *p_pool)
{
	NVM_UINT64 difference_bytes = 0;

	NVM_UINT32 interleave_set_id = p_namespace->namespace_creation_id.interleave_setid;
	for (NVM_UINT16 i = 0; i < p_pool->ilset_count; i++)
	{
		if (p_pool->ilsets[i].driver_id == interleave_set_id)
		{
			NVM_UINT64 namespace_bytes = p_namespace->block_count * p_namespace->block_size;
			if (namespace_bytes < p_pool->ilsets[i].size)
			{
				difference_bytes = p_pool->ilsets[i].size - namespace_bytes;
			}
			break;
		}
	}

	return difference_bytes;
}

void check_for_namespaces_smaller_than_containing_interleave_set(NVM_UINT32 *p_results,
		const struct nvm_namespace_details *p_namespaces,
		const NVM_UINT32 namespace_count,
		const struct nvm_pool *p_pools,
		const NVM_UINT32 pool_count)
{
	COMMON_LOG_ENTRY();

	for (NVM_UINT32 i = 0; i < namespace_count; i++)
	{
		if (p_namespaces[i].type == NAMESPACE_TYPE_APP_DIRECT)
		{
			NVM_UID namespace_uid_str;
			uid_copy(p_namespaces[i].discovery.namespace_uid, namespace_uid_str);

			const struct nvm_pool *p_pool = get_namespace_pool(
					p_namespaces[i].discovery.namespace_uid,
					p_pools, pool_count);
			if (p_pool)
			{
				NVM_UINT64 difference_bytes =
					get_size_bytes_difference_between_namespace_and_containing_interleave_set(
					&(p_namespaces[i]), p_pool);
				if (difference_bytes > 0)
				{
					NVM_EVENT_ARG difference_mb_str;
					s_snprintf(difference_mb_str, sizeof (difference_mb_str),
							"%llu MB", difference_bytes / BYTES_PER_MB);

					store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
							EVENT_SEVERITY_INFO,
							EVENT_CODE_DIAG_PCONFIG_APP_DIRECT_NAMESPACE_TOO_SMALL,
							p_namespaces[i].discovery.namespace_uid,
							0,
							namespace_uid_str, difference_mb_str, NULL,
							DIAGNOSTIC_RESULT_WARNING);

					(*p_results)++;
				}
			}
			else
			{
				COMMON_LOG_ERROR_F("No pool found for namespace %s", namespace_uid_str);
			}
		}
	}

	COMMON_LOG_EXIT();
}

int get_all_driver_namespace_details(struct nvm_namespace_details *p_namespaces,
		const int namespace_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct nvm_namespace_discovery namespace_discovery[namespace_count];
	if ((rc = get_namespaces(namespace_count, namespace_discovery)) > 0)
	{
		for (int i = 0; i < namespace_count; i++)
		{
			rc = get_namespace_details(namespace_discovery[i].namespace_uid, &(p_namespaces[i]));
			if (rc < 0)
			{
				NVM_UID uid_str;
				uid_copy(namespace_discovery[i].namespace_uid, uid_str);
				COMMON_LOG_ERROR_F("error fetching namespace details for namespace %s, rc = %d",
						uid_str, rc);

				break;
			}
		}
	}
	else
	{
		COMMON_LOG_ERROR_F("error getting namespaces, rc = %d", rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void check_namespace_best_practices_with_pools(NVM_UINT32 *p_results,
		const struct nvm_pool *p_pools, const NVM_UINT32 pool_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	int ns_count = get_namespace_count();
	if (ns_count < 0)
	{
		rc = ns_count;
	}
	else if (ns_count > 0)
	{
		struct nvm_namespace_details namespaces[ns_count];
		rc = get_all_driver_namespace_details(namespaces, ns_count);
		if (rc == NVM_SUCCESS)
		{
			check_for_namespaces_smaller_than_containing_interleave_set(p_results,
					namespaces, ns_count, p_pools, pool_count);
			check_if_pools_need_namespaces(p_results, namespaces, ns_count,
					p_pools, pool_count);
		}
	}

	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Error fetching namespaces, rc = %d", rc);
		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_PCONFIG_NAMESPACES_FAILED,
				NULL,
				0,
				NULL,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
	}

	COMMON_LOG_EXIT();
}

int check_namespace_best_practices(NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// Need pool info to compare to namespace info
	int pool_count = get_pool_count();
	if (pool_count > 0)
	{
		struct nvm_pool *p_pools = calloc(pool_count, sizeof (struct nvm_pool));
		if (p_pools)
		{
			pool_count = get_pools(pool_count, p_pools);
			if (pool_count > 0)
			{
				check_namespace_best_practices_with_pools(p_results, p_pools, pool_count);
			}
			free(p_pools);
		}
		else
		{
			COMMON_LOG_ERROR("couldn't allocate memory for pools");
			rc = NVM_ERR_NOMEMORY;
		}
	}

	if (pool_count < 0)
	{
		COMMON_LOG_ERROR_F(
			"Failed to retrieve the pool information, error %d",
			pool_count);

		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_PCONFIG_POOLS_FAILED,
				NULL,
				0,
				NULL,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void create_event_for_interleave_size(const enum event_code_diag_platform_config event_code,
		const enum interleave_size size,
		const NVM_UINT32 set_index)
{
	COMMON_LOG_ENTRY();

	// These are "best practices" issues - not errors
	enum diagnostic_result result = DIAGNOSTIC_RESULT_WARNING;

	// Set up interleave index as string argument for the event
	NVM_EVENT_ARG interleave_set_index_str;
	s_snprintf(interleave_set_index_str, sizeof (interleave_set_index_str),
			"%u", set_index);

	store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
			EVENT_SEVERITY_INFO,
			event_code,
			NULL, // no uid
			0, // no action required
			interleave_set_index_str, // arg1
			get_string_for_interleave_size(size), // arg2
			NULL, // arg3
			result);

	COMMON_LOG_EXIT();
}

void check_interleave_size(NVM_UINT32 *p_results,
		const enum interleave_size size, const NVM_UINT32 set_index,
		const enum event_code_diag_platform_config result_event_code)
{
	COMMON_LOG_ENTRY();

	if (size < INTERLEAVE_SIZE_4KB)
	{
		create_event_for_interleave_size(result_event_code, size, set_index);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

/*
 * Checks the interleave iMC and channel size recommendations and generates an event.
 */
void check_interleave_imc_and_channel_sizes(NVM_UINT32 *p_results,
		const struct nvm_interleave_set *p_interleave_set)
{
	COMMON_LOG_ENTRY();

	check_interleave_size(p_results, p_interleave_set->settings.imc,
			p_interleave_set->set_index,
			EVENT_CODE_DIAG_PCONFIG_RECOMMENDED_INTERLEAVE_SIZE_IMC);

	check_interleave_size(p_results, p_interleave_set->settings.channel,
			p_interleave_set->set_index,
			EVENT_CODE_DIAG_PCONFIG_RECOMMENDED_INTERLEAVE_SIZE_CHANNEL);

	COMMON_LOG_EXIT();
}

/*
 * Get the number of manageable DIMMs assigned to a specific memory controller on a socket.
 */
NVM_UINT32 get_num_manageable_dimms_on_socket_mem_controller(const NVM_UINT8 socket_id,
		const NVM_UINT32 mem_controller,
		const struct device_discovery *p_devices,
		const NVM_UINT8 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_UINT32 count = 0;

	for (NVM_UINT8 i = 0; i < device_count; i++)
	{
		if ((p_devices[i].manageability == MANAGEMENT_VALIDCONFIG) &&
				(p_devices[i].socket_id == socket_id) &&
				(p_devices[i].device_handle.parts.memory_controller_id == mem_controller))
		{
			count++;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(count);
	return count;
}

NVM_BOOL device_channel_ids_are_partners(const NVM_UINT32 device1_channel_id,
		const NVM_UINT32 device2_channel_id)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	// The channel IDs are sequential numbers - i.e. MC 1 has 1, 2, 3,
	// MC 2 has 4, 5, 6. Channel 1's partner on the other MC is channel 4.
	if ((device1_channel_id % NVM_MEMORY_CONTROLLER_CHANNEL_COUNT) ==
			(device2_channel_id % NVM_MEMORY_CONTROLLER_CHANNEL_COUNT))
	{
		result = 1;
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

/*
 * Determine whether the DIMM with a given handle has a partner DIMM on the same channel on the
 * other MC on the same socket.
 */
NVM_BOOL device_with_handle_has_partner_on_other_mem_controller(
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct device_discovery *p_devices, const NVM_UINT8 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	NVM_UINT16 socket_id = device_handle.parts.socket_id;
	NVM_UINT32 mc = device_handle.parts.memory_controller_id;
	NVM_UINT32 channel = device_handle.parts.mem_channel_id;
	NVM_UINT32 channel_position = device_handle.parts.mem_channel_dimm_num;

	for (NVM_UINT8 i = 0; i < device_count; i++)
	{
		// a "partner" has the same socket ID and channel position and a corresponding channel ID,
		// but a different memory controller
		if (p_devices[i].socket_id == socket_id &&
				p_devices[i].device_handle.parts.memory_controller_id != mc &&
				device_channel_ids_are_partners(p_devices[i].device_handle.parts.mem_channel_id,
						channel) &&
				p_devices[i].device_handle.parts.mem_channel_dimm_num == channel_position)
		{
			result = 1;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

/*
 * Determine if the socket has a balanced DIMM layout - i.e. symmetrical by channel.
 */
NVM_BOOL socket_is_balanced(const NVM_UINT16 socket_id, const struct device_discovery *p_devices,
		const NVM_UINT8 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 1;

	for (NVM_UINT8 i = 0; (i < device_count) && result; i++)
	{
		if (p_devices[i].socket_id == socket_id)
		{
			// if any DIMM doesn't have a partner, it's unbalanced
			result = device_with_handle_has_partner_on_other_mem_controller(
					p_devices[i].device_handle,
					p_devices, device_count);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

/*
 * Get the number of DIMMs on the socket that are partnered across memory controllers.
 */
NVM_UINT32 get_number_of_balanced_dimms_on_socket(const NVM_UINT16 socket_id,
		const struct device_discovery *p_devices,
		const NVM_UINT8 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_UINT32 count = 0;

	for (NVM_UINT8 i = 0; i < device_count; i++)
	{
		if (p_devices[i].socket_id == socket_id &&
				p_devices[i].manageability == MANAGEMENT_VALIDCONFIG)
		{
			if (device_with_handle_has_partner_on_other_mem_controller(
					p_devices[i].device_handle,
					p_devices, device_count))
			{
				count++;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(count);
	return count;
}

/*
 * Get the number of DIMMs on this memory controller that don't have a partner on the other
 * memory controller.
 */
NVM_UINT32 get_number_of_unbalanced_dimms_on_mem_controller(const NVM_UINT16 socket_id,
		const NVM_UINT16 memory_controller_id,
		const struct device_discovery *p_devices,
		const NVM_UINT8 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_UINT32 count = 0;

	for (NVM_UINT8 i = 0; i < device_count; i++)
	{
		if (p_devices[i].socket_id == socket_id &&
				p_devices[i].memory_controller_id == memory_controller_id &&
				p_devices[i].manageability == MANAGEMENT_VALIDCONFIG)
		{
			if (!device_with_handle_has_partner_on_other_mem_controller(
					p_devices[i].device_handle,
					p_devices, device_count))
			{
				count++;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(count);
	return count;
}

NVM_UINT32 get_minimum_recommended_ways_for_nonmirrored_ad_interleave(
		const struct nvm_interleave_set *p_interleave_set,
		const struct device_discovery *p_devices,
		const NVM_UINT8 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_UINT32 recommended_interleave_ways = 0;
	NVM_UINT8 socket_id = p_interleave_set->socket_id;

	// A balanced socket includes all DIMMs in its recommended interleave.
	// On an unbalanced socket we expect up to one overall "balanced" interleave
	// consisting of partnered DIMMs, and up to one unbalanced interleave
	// for each memory controller
	NVM_UINT32 balanced_dimm_interleave_size = get_number_of_balanced_dimms_on_socket(socket_id,
			p_devices, device_count);
	NVM_UINT32 interleave_set_dimm_count = p_interleave_set->dimm_count;

	for (NVM_UINT32 i = 0; i < interleave_set_dimm_count; i++)
	{
		NVM_UINT32 device_interleave_ways = 0;
		NVM_NFIT_DEVICE_HANDLE device_handle = p_interleave_set->dimms[i];
		if (device_with_handle_has_partner_on_other_mem_controller(device_handle,
				p_devices, device_count))
		{
			device_interleave_ways = balanced_dimm_interleave_size;
		}
		else
		{
			device_interleave_ways = get_number_of_unbalanced_dimms_on_mem_controller(socket_id,
					device_handle.parts.memory_controller_id, p_devices, device_count);
		}

		if (device_interleave_ways > recommended_interleave_ways)
		{
			recommended_interleave_ways = device_interleave_ways;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(recommended_interleave_ways);
	return recommended_interleave_ways;
}

NVM_UINT32 get_minimum_recommended_interleave_ways(
		const struct nvm_interleave_set *p_interleave_set,
		const struct device_discovery *p_devices,
		const NVM_UINT8 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_UINT32 recommended_interleave_ways = 0;

	// Mirrored interleave sets are interleaved across one memory controller,
	// not whole sockets.
	if (p_interleave_set->mirrored)
	{
		NVM_UINT8 socket_id = p_interleave_set->socket_id;
		NVM_UINT32 memory_controller_id = p_interleave_set->dimms[0].parts.memory_controller_id;
		recommended_interleave_ways = get_num_manageable_dimms_on_socket_mem_controller(socket_id,
				memory_controller_id,
				p_devices, device_count);
	}
	else
	{
		recommended_interleave_ways = get_minimum_recommended_ways_for_nonmirrored_ad_interleave(
				p_interleave_set, p_devices, device_count);
	}

	COMMON_LOG_EXIT_RETURN_I(recommended_interleave_ways);
	return recommended_interleave_ways;
}

void check_interleave_ways(NVM_UINT32 *p_results,
		const struct nvm_interleave_set *p_interleave_set,
		const struct device_discovery *p_devices,
		const NVM_UINT8 device_count)
{
	COMMON_LOG_ENTRY();

	NVM_UINT32 interleave_ways = p_interleave_set->dimm_count;
	NVM_UINT32 recommended_interleave_ways = get_minimum_recommended_interleave_ways(
			p_interleave_set,
			p_devices, device_count);
	if (interleave_ways < recommended_interleave_ways)
	{
		// These are "best practices" issues - not errors
		enum diagnostic_result result = DIAGNOSTIC_RESULT_WARNING;

		// Set up string arguments for the event
		NVM_EVENT_ARG interleave_set_index_str;
		s_snprintf(interleave_set_index_str, sizeof (interleave_set_index_str),
				"%u", p_interleave_set->set_index);
		NVM_EVENT_ARG interleave_ways_str;
		s_snprintf(interleave_ways_str, sizeof (interleave_ways_str),
				"%u", interleave_ways);
		NVM_EVENT_ARG recommended_interleave_ways_str;
		s_snprintf(recommended_interleave_ways_str, sizeof (recommended_interleave_ways_str),
				"%u", recommended_interleave_ways);

		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_INFO,
				EVENT_CODE_DIAG_PCONFIG_RECOMMENDED_INTERLEAVE_WAYS,
				NULL, // no uid
				0, // no action required
				interleave_set_index_str, // arg1
				interleave_ways_str, // arg2
				recommended_interleave_ways_str, // arg3
				result);

		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

void check_all_interleave_set_best_practices_against_device_list(NVM_UINT32 *p_results,
		const struct nvm_interleave_set *p_interleave_sets,
		const NVM_UINT32 interleave_set_count,
		const struct device_discovery *p_devices,
		const NVM_UINT8 device_count)
{
	COMMON_LOG_ENTRY();

	for (NVM_UINT32 i = 0; i < interleave_set_count; i++)
	{
		check_interleave_imc_and_channel_sizes(p_results,
				&(p_interleave_sets[i]));
		check_interleave_ways(p_results,
				&(p_interleave_sets[i]),
				p_devices, device_count);
	}

	COMMON_LOG_EXIT();
}

/*
 * Verify existing interleave set configuration matches with best practices
 * guidelines.
 */
int check_all_interleave_set_best_practices(NVM_UINT32 *p_results,
		const struct device_discovery *p_devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();

	int rc = get_interleave_set_count();
	if (rc > 0)
	{
		NVM_UINT32 interleave_set_count = rc;
		struct nvm_interleave_set interleave_sets[interleave_set_count];
		if ((rc = get_interleave_sets(interleave_set_count, interleave_sets)) > 0)
		{
			check_all_interleave_set_best_practices_against_device_list(p_results,
					interleave_sets, interleave_set_count,
					p_devices, device_count);
			rc = NVM_SUCCESS;
		}
		else
		{
			COMMON_LOG_ERROR_F("get_interleave_sets errored after get_interleave_set_count passed, "
					"rc=%d", rc);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL dimms_on_socket_are_different_sizes(const NVM_UINT16 socket_id,
		const struct device_discovery *p_devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL sizes_are_different = 0;

	// flag so we only compare size after it's initialized
	NVM_BOOL capacity_initialized = 0;
	NVM_UINT64 capacity = 0;
	for (NVM_UINT32 i = 0; i < device_count; i++)
	{
		// Only look at manageable DIMMs
		if ((p_devices[i].socket_id == socket_id) &&
				(p_devices[i].manageability == MANAGEMENT_VALIDCONFIG))
		{
			if (capacity_initialized)
			{
				if (p_devices[i].capacity != capacity)
				{
					sizes_are_different = 1;
					break;
				}
			}
			else
			{
				capacity = p_devices[i].capacity;
				capacity_initialized = 1;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(sizes_are_different);
	return sizes_are_different;
}

void check_dimm_socket_capacities(NVM_UINT32 *p_results,
		const struct device_discovery *p_devices, const NVM_UINT32 device_count,
		const struct socket *p_sockets, const NVM_UINT16 socket_count)
{
	COMMON_LOG_ENTRY();

	for (NVM_UINT16 i = 0; i < socket_count; i++)
	{
		NVM_UINT16 socket_id = p_sockets[i].id;
		if (dimms_on_socket_are_different_sizes(socket_id, p_devices, device_count))
		{
			NVM_EVENT_ARG socket_id_str;
			s_snprintf(socket_id_str, NVM_EVENT_ARG_LEN, "%hu", socket_id);

			store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
					EVENT_SEVERITY_INFO,
					EVENT_CODE_DIAG_PCONFIG_DIMMS_DIFFERENT_SIZES,
					NULL,
					0,
					socket_id_str, NULL, NULL,
					DIAGNOSTIC_RESULT_WARNING);

			(*p_results)++;
		}
	}

	COMMON_LOG_EXIT();
}

NVM_BOOL all_dimms_on_socket_on_one_memory_controller(const NVM_UINT16 socket_id,
		const struct device_discovery *p_devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 1;

	int last_memory_controller_seen = NVM_ERR_NOTFOUND;
	for (NVM_UINT32 i = 0; i < device_count; i++)
	{
		if (p_devices[i].socket_id == socket_id)
		{
			NVM_UINT16 memory_controller_id = p_devices[i].memory_controller_id;
			if (last_memory_controller_seen == NVM_ERR_NOTFOUND)
			{
				last_memory_controller_seen = memory_controller_id;
			}
			else if (memory_controller_id != last_memory_controller_seen)
			{
				result = 0;
				break;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

void check_dimm_socket_layout_balance(NVM_UINT32 *p_results,
		const struct device_discovery *p_devices, const NVM_UINT32 device_count,
		const struct socket *p_sockets, const NVM_UINT16 socket_count)
{
	COMMON_LOG_ENTRY();

	for (NVM_UINT16 i = 0; i < socket_count; i++)
	{
		NVM_UINT16 socket_id = p_sockets[i].id;
		if (!socket_is_balanced(socket_id, p_devices, device_count))
		{
			NVM_EVENT_ARG socket_id_str;
			s_snprintf(socket_id_str, sizeof (socket_id_str), "%hu", socket_id);

			store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
					EVENT_SEVERITY_INFO,
					EVENT_CODE_DIAG_PCONFIG_DIMM_CONFIG_UNBALANCED,
					NULL,
					0,
					socket_id_str, NULL, NULL,
					DIAGNOSTIC_RESULT_WARNING);
			(*p_results)++;
		}
	}

	COMMON_LOG_EXIT();
}

int check_dimm_socket_layout_best_practices(NVM_UINT32 *p_results,
		const struct device_discovery *p_devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();

	int rc = get_socket_count();
	if (rc > 0)
	{
		NVM_UINT16 socket_count = rc;
		struct socket sockets[socket_count];
		if ((rc = get_sockets(sockets, socket_count)) > 0)
		{
			check_dimm_socket_layout_balance(p_results, p_devices,
					device_count, sockets, socket_count);
			check_dimm_socket_capacities(p_results, p_devices,
					device_count, sockets, socket_count);
		}
		else if (rc < 0)
		{
			COMMON_LOG_ERROR_F("couldn't get sockets, rc=%d", rc);
		}
	}
	else if (rc < 0)
	{
		COMMON_LOG_ERROR_F("couldn't get socket count, rc=%d", rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL dimm_mode_skus_are_different(const struct device_discovery *p_device1,
		const struct device_discovery *p_device2)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL skus_different = 0;

	if ((p_device1->device_capabilities.app_direct_mode_capable !=
			p_device2->device_capabilities.app_direct_mode_capable) ||
			(p_device1->device_capabilities.storage_mode_capable !=
			p_device2->device_capabilities.storage_mode_capable) ||
			(p_device1->device_capabilities.memory_mode_capable !=
			p_device2->device_capabilities.memory_mode_capable))
	{
		skus_different = 1;
	}

	COMMON_LOG_EXIT_RETURN_I(skus_different);
	return skus_different;
}

NVM_BOOL dimm_sparing_skus_are_different(const struct device_discovery *p_device1,
		const struct device_discovery *p_device2)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL skus_different = 0;

	if (p_device1->device_capabilities.die_sparing_capable !=
			p_device2->device_capabilities.die_sparing_capable)
	{
		skus_different = 1;
	}

	COMMON_LOG_EXIT_RETURN_I(skus_different);
	return skus_different;
}

NVM_BOOL dimm_security_skus_are_different(const struct device_discovery *p_device1,
		const struct device_discovery *p_device2)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL skus_different = 0;

	if ((p_device1->security_capabilities.erase_crypto_capable !=
			p_device2->security_capabilities.erase_crypto_capable) ||
			(p_device1->security_capabilities.passphrase_capable !=
					p_device2->security_capabilities.passphrase_capable) ||
			(p_device1->security_capabilities.unlock_device_capable !=
					p_device2->security_capabilities.unlock_device_capable))
	{
		skus_different = 1;
	}

	COMMON_LOG_EXIT_RETURN_I(skus_different);
	return skus_different;
}

/*
 * Perform the appropriate system-wide SKU check based on a custom SKU comparison function.
 */
NVM_BOOL system_has_mixed_skus(const struct device_discovery *p_devices,
		const NVM_UINT32 device_count,
		NVM_BOOL (*dimms_skus_are_different)(const struct device_discovery *,
				const struct device_discovery *))
{
	COMMON_LOG_ENTRY();
	NVM_BOOL has_mixed_skus = 0;

	// Grab the initial settings from the first one off the stack.
	// We only care about whether any DIMMs are different.
	const struct device_discovery *p_reference_device = &(p_devices[0]);

	for (NVM_UINT32 i = 1; i < device_count; i++)
	{
		if (dimms_skus_are_different(p_reference_device, &(p_devices[i])))
		{
			has_mixed_skus = 1;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(has_mixed_skus);
	return has_mixed_skus;
}

NVM_BOOL system_has_mixed_mode_skus(const struct device_discovery *p_devices,
		const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL has_mixed_skus = system_has_mixed_skus(p_devices,
			device_count, dimm_mode_skus_are_different);

	COMMON_LOG_EXIT_RETURN_I(has_mixed_skus);
	return has_mixed_skus;
}

NVM_BOOL system_has_mixed_sparing_skus(const struct device_discovery *p_devices,
		const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL has_mixed_skus = system_has_mixed_skus(p_devices,
			device_count, dimm_sparing_skus_are_different);

	COMMON_LOG_EXIT_RETURN_I(has_mixed_skus);
	return has_mixed_skus;
}

NVM_BOOL system_has_mixed_security_skus(const struct device_discovery *p_devices,
		const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL has_mixed_skus = system_has_mixed_skus(p_devices,
			device_count, dimm_security_skus_are_different);

	COMMON_LOG_EXIT_RETURN_I(has_mixed_skus);
	return has_mixed_skus;
}

void check_dimm_skus(NVM_UINT32 *p_results,
		const struct device_discovery *p_devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();

	// Mixed SKUs are a system-wide condition - only log each event once
	if (system_has_mixed_mode_skus(p_devices, device_count))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_PCONFIG_DIMM_MODE_SKUS_MIXED,
				NULL,
				0,
				NULL, NULL, NULL,
				DIAGNOSTIC_RESULT_WARNING);

		(*p_results)++;
	}

	if (system_has_mixed_sparing_skus(p_devices, device_count))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_PCONFIG_DIMM_DIE_SPARING_SKUS_MIXED,
				NULL,
				0,
				NULL, NULL, NULL,
				DIAGNOSTIC_RESULT_WARNING);

		(*p_results)++;
	}

	if (system_has_mixed_security_skus(p_devices, device_count))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_PCONFIG_DIMM_SECURITY_SKUS_MIXED,
				NULL,
				0,
				NULL, NULL, NULL,
				DIAGNOSTIC_RESULT_WARNING);

		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

/*
 * Generate diagnostic events if the user's DIMM/pool configuration doesn't match up
 * with best practices guidelines for platform configuration.
 */
int check_platform_config_best_practices(NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	int rc = nvm_get_device_count();
	if (rc > 0)
	{
		NVM_UINT32 num_devices = rc;
		struct device_discovery devices[num_devices];
		memset(devices, 0, sizeof (devices));
		if ((rc = nvm_get_devices(devices, num_devices)) > 0)
		{
			KEEP_ERROR(rc, check_all_interleave_set_best_practices(p_results,
					devices, num_devices));

			KEEP_ERROR(rc, check_dimm_socket_layout_best_practices(p_results,
					devices, num_devices));

			check_dimm_skus(p_results, devices, num_devices);
		}
		else
		{
			COMMON_LOG_ERROR_F("couldn't get %u devices, rc=%d", num_devices, rc);
		}
	}
	else if (rc < 0)
	{
		COMMON_LOG_ERROR_F("couldn't get device count, rc=%d", rc);
	}

	// Namespace best practices aren't directly dependent on DIMM list
	KEEP_ERROR(rc, check_namespace_best_practices(p_results));

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
