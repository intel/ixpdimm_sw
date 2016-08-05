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
 * This file contains the implementation of App Direct and Storage namespace
 * management functions of the Native API.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "nvm_management.h"
#include <persistence/logging.h>
#include "monitor.h"
#include <persistence/lib_persistence.h>
#include "device_adapter.h"
#include "platform_config_data.h"
#include "utility.h"
#include <file_ops/file_ops_adapter.h>
#include <uid/uid.h>
#include <string/s_str.h>
#include "device_utilities.h"
#include "string/x_str.h"
#include "pool_utilities.h"
#include "capabilities.h"
#include <os/os_adapter.h>
#include "system.h"
#include "nvm_context.h"

int validate_namespace_block_count(const NVM_UID namespace_uid,
		NVM_UINT64 *block_count, const NVM_BOOL allow_adjustment);

int find_interleave_with_capacity(const struct pool *p_pool,
		const struct nvm_capabilities *p_nvm_caps,
		struct namespace_create_settings *p_settings, NVM_UINT32 *namespace_creation_id,
		const struct interleave_format *pFormat, NVM_BOOL allow_adjustment);

int find_dimm_with_capacity(const struct pool *p_pool,
		const struct nvm_capabilities *p_nvm_caps,
		struct namespace_create_settings *p_settings,
		NVM_UINT32 *namespace_creation_id, NVM_BOOL allow_adjustment);

int find_id_for_ns_creation(const struct pool *p_pool,
		const struct nvm_capabilities *p_nvm_caps,
		struct namespace_create_settings *p_settings, NVM_UINT32 *p_namespace_creation_id,
		const struct interleave_format *p_format, NVM_BOOL allow_adjustment);

NVM_BOOL interleave_meets_app_direct_settings(const struct interleave_set *p_interleave,
		const struct interleave_format *p_format);

void adjust_namespace_block_count_if_allowed(NVM_UINT64 *p_block_count,
		const NVM_UINT16 block_size,
		NVM_UINT8 ways, const NVM_BOOL allow_adjustment);

NVM_UINT64 get_minimum_ns_size(NVM_UINT8 ways, NVM_UINT64 driver_min_ns_size);

NVM_BOOL check_namespace_alignment(NVM_UINT64 capacity, NVM_UINT32 block_size, NVM_UINT8 ways);

NVM_UINT32 get_alignment_size(NVM_UINT32 block_size, NVM_UINT32 ways);

int get_pool_supported_size_ranges(const struct pool *p_pool,
		const struct nvm_capabilities *p_caps,
		struct possible_namespace_ranges *p_range);
/*
 * Retrieve the number of PM namespaces allocated across all devices.
 */
int nvm_get_namespace_count()
{
	COMMON_LOG_ENTRY();
	int rc = 0; // return count

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(get_namespaces)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving namespaces is not supported.");
	}
	else
	{
		rc = nvm_get_pool_count();
		if (rc >= 0)
		{
			rc = get_namespace_count();
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve basic information on all PM namespaces.
 */
int nvm_get_namespaces(struct namespace_discovery *p_namespaces, const NVM_UINT8 count)
{
	COMMON_LOG_ENTRY();
	int rc = 0; // return count

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(get_namespaces)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving namespaces is not supported.");
	}
	else if (p_namespaces == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_namespaces is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (count == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, count is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	// read from the cache
	else if ((rc = get_nvm_context_namespaces(p_namespaces, count)) < 0 &&
			rc != NVM_ERR_ARRAYTOOSMALL)
	{
		// zero out return data
		memset(p_namespaces, 0, (sizeof (struct namespace_discovery) * count));

		// get the actual count
		int ns_count = nvm_get_namespace_count();
		if (ns_count <= 0)
		{
			rc = ns_count;
		}
		else
		{
			struct nvm_namespace_discovery nvm_namespaces[ns_count];
			ns_count = get_namespaces(ns_count, nvm_namespaces);
			if (ns_count <= 0)
			{
				rc = ns_count;
			}
			else
			{
				rc = 0;
				for (int i = 0; i < ns_count; i++)
				{
					// check array size
					if (i >= count)
					{
						COMMON_LOG_ERROR("The provided namespace array is too small.");
						rc = NVM_ERR_ARRAYTOOSMALL;
						break;
					}

					// simple copy - same size struct
					memmove(&p_namespaces[i], &nvm_namespaces[i], sizeof (p_namespaces[i]));
					rc++; // return number of namespaces

				} // end for each ns
				if (rc >= 0)
				{
					// update the context
					set_nvm_context_namespaces(p_namespaces, rc);
				}

			} // end get_namespaces succeeded
		} // end get namespace count succeeded
	} // end valid input parameters

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


enum encryption_status namespace_encryption_to_enum(const NVM_BOOL passphrase_capable)
{
	COMMON_LOG_ENTRY();

	enum encryption_status enum_val;
	if (passphrase_capable)
	{
		enum_val = NVM_ENCRYPTION_ON;
	}
	else
	{
		enum_val = NVM_ENCRYPTION_OFF;
	}

	COMMON_LOG_EXIT_RETURN_I(enum_val);
	return enum_val;
}

/*
 * Retrieve security information about the device specified.
 */
int get_security_attributes_from_device(
	const NVM_NFIT_DEVICE_HANDLE device_handle,
	enum encryption_status *p_encryption,
	enum erase_capable_status *p_erase_capable)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;

	// Encryption is on if the parent NVDIMM lock state indicates encryption enabled
	// EraseCapable is true if true the namespace is on a NVDIMM that supports erase
	struct device_discovery discovery;
	if (lookup_dev_handle(device_handle, &discovery) == NVM_SUCCESS)
	{
		*p_encryption = device_is_encryption_enabled(discovery.lock_state)?
				NVM_ENCRYPTION_ON:NVM_ENCRYPTION_OFF;
		*p_erase_capable = device_is_erase_capable(discovery.security_capabilities)?
				NVM_ERASE_CAPABLE_TRUE:NVM_ERASE_CAPABLE_FALSE;
		rc = NVM_SUCCESS;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve detailed information about the namespace specified.
 */
int nvm_get_namespace_details(const NVM_UID namespace_uid, struct namespace_details *p_namespace)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(get_namespace_details)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving namespaces is not supported.");
	}
	else if (namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, namespace_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_namespace == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_namespace is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (get_nvm_context_namespace_details(namespace_uid, p_namespace) != NVM_SUCCESS)
	{
		memset(p_namespace, 0, sizeof (*p_namespace));

		// get the namespace from the driver
		struct nvm_namespace_details nvm_details;
		memset(&nvm_details, 0, sizeof (nvm_details));
		NVM_UID pool_uid;
		if ((rc = get_namespace_details(namespace_uid, &nvm_details)) == NVM_SUCCESS &&
			(rc = get_pool_uid_from_namespace_details(&nvm_details, &pool_uid)) == NVM_SUCCESS)
		{
			memmove(p_namespace->pool_uid, pool_uid, NVM_MAX_UID_LEN);
			s_strcpy(p_namespace->discovery.friendly_name,
					nvm_details.discovery.friendly_name, NVM_NAMESPACE_NAME_LEN);
			memmove(p_namespace->discovery.namespace_uid,
					nvm_details.discovery.namespace_uid, NVM_MAX_UID_LEN);
			p_namespace->block_count = nvm_details.block_count;
			p_namespace->block_size = nvm_details.block_size;
			p_namespace->type = nvm_details.type;
			p_namespace->health = nvm_details.health;
			p_namespace->enabled = nvm_details.enabled;
			p_namespace->btt = nvm_details.btt;
			p_namespace->memory_page_allocation = nvm_details.memory_page_allocation;

			int temprc;
			if (p_namespace->type == NAMESPACE_TYPE_APP_DIRECT)
			{
				if ((temprc = calculate_app_direct_interleave_security(
						nvm_details.namespace_creation_id.interleave_setid,
						&p_namespace->security_features.encryption, // enabled
						&p_namespace->security_features.erase_capable,
						NULL)) != NVM_SUCCESS)
				{
					KEEP_ERROR(rc, temprc);
				}

				struct nvm_interleave_set interleave;
				memset(&interleave, 0, sizeof (interleave));
				temprc = get_interleaveset_by_driver_id(
						nvm_details.namespace_creation_id.interleave_setid, &interleave);
				if (temprc == NVM_SUCCESS)
				{
					p_namespace->mirrored = interleave.mirrored;
					memmove(&p_namespace->interleave_format, &interleave.settings,
							sizeof (interleave.settings));
				}
				else
				{
					KEEP_ERROR(rc, temprc);
				}

				p_namespace->creation_id.interleave_setid =
										nvm_details.namespace_creation_id.interleave_setid;
			}
			else if (p_namespace->type == NAMESPACE_TYPE_STORAGE)
			{
				if ((temprc = get_security_attributes_from_device(
						nvm_details.namespace_creation_id.device_handle,
						&p_namespace->security_features.encryption,
						&p_namespace->security_features.erase_capable)) != NVM_SUCCESS)
				{
					KEEP_ERROR(rc, temprc);
				}
				struct device_discovery device;
				lookup_dev_handle(nvm_details.namespace_creation_id.device_handle,
						&device);
				memmove(p_namespace->creation_id.device_uid, device.uid, NVM_MAX_UID_LEN);
			}
			// update the context
			if (rc)
			{
				set_nvm_context_namespace_details(namespace_uid, p_namespace);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to validate namespace enable state
 */
int validate_ns_enabled_state(enum namespace_enable_state enabled)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if ((enabled != NAMESPACE_ENABLE_STATE_ENABLED) &&
			(enabled != NAMESPACE_ENABLE_STATE_DISABLED))
	{
		COMMON_LOG_ERROR("Invalid enable state to create a namespace.");
		rc = NVM_ERR_BADNAMESPACEENABLESTATE;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Return the capacity of the largest storage namespace that can be created on the
 * specified dimm.
 * Expect p_pool and p_size to not be NULL.
 */
int get_largest_storage_namespace_on_a_dimm(const struct pool *p_pool,
		const NVM_UID device_uid, NVM_UINT64 *p_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;
	int num_dimms = 0;

	struct device_discovery discovery;
	if (lookup_dev_uid(device_uid, &discovery) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to find the device in the lookup table");
	}
	else if (p_pool->type == POOL_TYPE_PERSISTENT_MIRROR ||
		p_pool->type == POOL_TYPE_VOLATILE)
	{
		*p_size = 0;
	}
	else if ((num_dimms = get_topology_count()) > 0)
	{
		*p_size = 0;

		struct nvm_storage_capacities capacities[num_dimms];
		memset(capacities, 0, sizeof (capacities));
		int num_caps = 0;
		if ((num_caps = get_dimm_storage_capacities(num_dimms, capacities)) > 0)
		{
			for (int j = 0; j < num_caps; j++)
			{
				if (discovery.device_handle.handle
						== capacities[j].device_handle.handle)
				{
					*p_size = capacities[j].free_storage_capacity;
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
 * Helper function to check if a device meets a requested encryption requirement
 */
NVM_BOOL device_meets_encryption_criteria(const enum encryption_status criteria,
		const struct device_discovery *discovery)
{
	NVM_BOOL criteria_met = 1;
	NVM_BOOL encryption_enabled = device_is_encryption_enabled(discovery->lock_state);
	if (criteria == NVM_ENCRYPTION_ON && !encryption_enabled)
	{
		criteria_met = 0;
	}
	else if (criteria == NVM_ENCRYPTION_OFF && encryption_enabled)
	{
		criteria_met = 0;
	}
	// else NVM_ENCRYPTION_IGNORE
	return criteria_met;
}

/*
 * Helper function to check if a device meets a requested encryption requirement
 */
NVM_BOOL device_meets_erase_capable_criteria(const enum erase_capable_status criteria,
		const struct device_discovery *discovery)
{
	NVM_BOOL criteria_met = 1;
	NVM_BOOL erase_capable = device_is_erase_capable(discovery->security_capabilities);
	if (criteria == NVM_ERASE_CAPABLE_TRUE && !erase_capable)
	{
		criteria_met = 0;
	}
	else if (criteria == NVM_ERASE_CAPABLE_FALSE && erase_capable)
	{
		criteria_met = 0;
	}
	// else NVM_ERASE_CAPABLE_IGNORE
	return criteria_met;
}

/*
 * Helper function to check if dimm meets security requirements to create
 * a storage NS
 */
NVM_BOOL dimm_meets_security_criteria(const NVM_UID dimm,
		struct namespace_security_features security_features)
{
	COMMON_LOG_ENTRY();

	NVM_BOOL security_criteria_met = 1;
	// ignore security features if these fields are set to ignore
	if ((security_features.erase_capable != NVM_ERASE_CAPABLE_IGNORE) ||
		(security_features.encryption != NVM_ENCRYPTION_IGNORE))
	{
		struct device_discovery discovery;
		if (nvm_get_device_discovery(dimm, &discovery) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR("Failed to get device discovery information.");
			security_criteria_met = 0;
		}
		if (security_criteria_met)
		{
			security_criteria_met = device_meets_encryption_criteria(
				security_features.encryption, &discovery);
		}
		if (security_criteria_met)
		{
			security_criteria_met = device_meets_erase_capable_criteria(
				security_features.erase_capable, &discovery);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(security_criteria_met);
	return security_criteria_met;
}

/*
 * Helper function to check if interleave set meets security requirements to create
 * App Direct NS
 */
NVM_BOOL interleave_meets_security_criteria(const struct interleave_set *p_ilset,
		const struct namespace_security_features *p_security_features)
{
	COMMON_LOG_ENTRY();

	NVM_BOOL security_criteria_met = 1;
	// ignore security features if these fields are set to ignore
	if ((p_security_features->erase_capable != NVM_ERASE_CAPABLE_IGNORE) ||
		(p_security_features->encryption != NVM_ENCRYPTION_IGNORE))
	{
		for (int dimm_index = 0; dimm_index < p_ilset->dimm_count; dimm_index++)
		{
			// verify if the security features match with the pool
			if (!dimm_meets_security_criteria(p_ilset->dimms[dimm_index], *p_security_features))
			{
				security_criteria_met = 0;
				break;
			}
		} // end of for loop
	}

	COMMON_LOG_EXIT_RETURN_I(security_criteria_met);
	return security_criteria_met;
}

/*
 * Helper function to validate namespace size when creating a NS
 */
int validate_ns_size_for_creation(struct pool *p_pool,
		const struct namespace_create_settings *p_settings,
		const struct nvm_capabilities *p_nvm_caps,
		const struct possible_namespace_ranges *p_range)

{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_UINT64 namespace_capacity = p_settings->block_count * p_settings->block_size;
	NVM_BOOL namespace_blocksize_supported = 0;
	for (int i = 0; i < p_nvm_caps->sw_capabilities.block_size_count; i++)
	{
		if (p_nvm_caps->sw_capabilities.block_sizes[i] == p_settings->block_size)
		{
			namespace_blocksize_supported = 1;
			break;
		}
	}
	if ((namespace_blocksize_supported == 0) &&
			(p_settings->type == NAMESPACE_TYPE_STORAGE))
	{
		COMMON_LOG_ERROR("Invalid block size to create a Storage namespace.");
		rc = NVM_ERR_BADBLOCKSIZE;
	}
	else if ((p_settings->type == NAMESPACE_TYPE_APP_DIRECT) &&
			(p_settings->block_size != 1))
	{
		COMMON_LOG_ERROR("Invalid block size to create an App Direct namespace.");
		rc = NVM_ERR_BADBLOCKSIZE;
	}
	else if (namespace_capacity > p_pool->free_capacity)
	{
		COMMON_LOG_ERROR_F("Requested namespace capacity %llu bytes \
					is more than the maximum available size of %llu bytes",
				namespace_capacity, p_pool->free_capacity);
		rc = NVM_ERR_BADSIZE;
	}
	else if (p_settings->type == NAMESPACE_TYPE_APP_DIRECT &&
			namespace_capacity > p_range->largest_possible_app_direct_ns)
	{
		COMMON_LOG_ERROR_F("Requested namespace capacity %llu bytes \
					is more than the maximum available size of %llu bytes",
				namespace_capacity, p_range->largest_possible_app_direct_ns);
		rc = NVM_ERR_BADSIZE;
	}
	else if (p_settings->type == NAMESPACE_TYPE_STORAGE &&
			namespace_capacity > p_range->largest_possible_storage_ns)
	{
		COMMON_LOG_ERROR_F("Requested namespace capacity %llu bytes \
					is more than the maximum available size of %llu bytes",
				namespace_capacity, p_range->largest_possible_storage_ns);
		rc = NVM_ERR_BADSIZE;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int find_id_for_ns_creation(const struct pool *p_pool,
		const struct nvm_capabilities *p_nvm_caps,
		struct namespace_create_settings *p_settings,
		NVM_UINT32 *p_namespace_creation_id,
		const struct interleave_format *p_format,
		NVM_BOOL allow_adjustment)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_settings->type == NAMESPACE_TYPE_APP_DIRECT)
	{
		rc = find_interleave_with_capacity(p_pool, p_nvm_caps, p_settings,
				p_namespace_creation_id, p_format, allow_adjustment);
	}
	else if (p_settings->type == NAMESPACE_TYPE_STORAGE)
	{
		rc = find_dimm_with_capacity(p_pool, p_nvm_caps, p_settings,
				p_namespace_creation_id, allow_adjustment);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int check_namespace_capacity_and_security(const struct pool *p_pool,
		NVM_UINT64 namespace_capacity,
		NVM_UINT64 minimum_ns_size,
		NVM_UINT32 *p_namespace_creation_id,
		struct namespace_security_features *p_security_features)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;
	struct device_discovery discovery;
	NVM_BOOL are_all_dimms_locked = 1;

	// find a dimm with the requested capacity and security features
	for (int dimm_index = 0; dimm_index < p_pool->dimm_count; dimm_index++)
	{
		if (lookup_dev_uid(p_pool->dimms[dimm_index], &discovery) == NVM_SUCCESS)
		{
			if (discovery.lock_state != LOCK_STATE_LOCKED)
			{
				are_all_dimms_locked = 0;
				NVM_UINT64 current_size;
				// verify if the requested namespace capacity is available
				rc = get_largest_storage_namespace_on_a_dimm(p_pool,
						discovery.uid, &current_size);
				if (rc == NVM_SUCCESS)
				{
					rc = NVM_ERR_BADSIZE;
					if (current_size >= namespace_capacity &&
							namespace_capacity >= minimum_ns_size)
					{
						rc = NVM_ERR_BADSECURITYGOAL;
						if (dimm_meets_security_criteria(p_pool->dimms[dimm_index],
														 *p_security_features))
						{
							*p_namespace_creation_id = discovery.device_handle.handle;
							rc = NVM_SUCCESS;
							break;
						}
					}
				}
			}
		}
	} // end of for loop

	if (are_all_dimms_locked)
	{
		rc = NVM_ERR_BADSECURITYSTATE;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int find_dimm_with_capacity(const struct pool *p_pool,
		const struct nvm_capabilities *p_nvm_caps,
		struct namespace_create_settings *p_settings,
		NVM_UINT32 *p_namespace_creation_id, NVM_BOOL allow_adjustment)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_BADALIGNMENT;

	NVM_UINT8 ways = 1; // storage namespaces are restricted to a single DIMM
	NVM_UINT64 minimum_ns_size =
			get_minimum_ns_size(ways, p_nvm_caps->sw_capabilities.min_namespace_size);
	NVM_UINT64 new_block_count = p_settings->block_count;
	NVM_UINT32 real_block_size = get_real_block_size(p_settings->block_size);
	adjust_namespace_block_count_if_allowed(&new_block_count,
			real_block_size, ways, allow_adjustment);
	NVM_UINT64 namespace_capacity = new_block_count * real_block_size;

	if (check_namespace_alignment(namespace_capacity, real_block_size, ways))
	{
		if ((rc = check_namespace_capacity_and_security(p_pool,
				namespace_capacity, minimum_ns_size,
				p_namespace_creation_id, &(p_settings->security_features))) == NVM_SUCCESS)
		{
			p_settings->block_count = namespace_capacity / real_block_size;
		}
		else if (rc == NVM_ERR_BADSIZE)
		{
			// We rounded up to get an aligned size but if the user request was close to the largest
			// available size we may have gone over. We don't want to fail that request so we will
			// try rounding down to an aligned size and see if that works. If its still not happy
			// then we will fail
			NVM_UINT64 alignment_size = get_alignment_size(real_block_size, ways);
			namespace_capacity -= alignment_size;
			if ((rc = check_namespace_capacity_and_security(
					p_pool, namespace_capacity, minimum_ns_size,
					p_namespace_creation_id, &(p_settings->security_features))) == NVM_SUCCESS)
			{
				p_settings->block_count = namespace_capacity / real_block_size;
			}
		}
	}
	else
	{
		rc = NVM_ERR_BADALIGNMENT;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL check_namespace_alignment(NVM_UINT64 capacity, NVM_UINT32 block_size, NVM_UINT8 ways)
{
	COMMON_LOG_ENTRY();

	NVM_BOOL alignment_ok = 0;
	NVM_UINT64 alignment_size = get_alignment_size(block_size, ways);
	alignment_ok = !(capacity % alignment_size);

	COMMON_LOG_EXIT();
	return alignment_ok;
}

NVM_UINT64 get_minimum_ns_size(NVM_UINT8 ways, NVM_UINT64 driver_min_ns_size)
{
	COMMON_LOG_ENTRY();

	NVM_UINT64 nvm_min_ns_size = BYTES_PER_GB * ways;
	nvm_min_ns_size = (nvm_min_ns_size > driver_min_ns_size) ?
			nvm_min_ns_size : driver_min_ns_size;

	COMMON_LOG_EXIT();
	return nvm_min_ns_size;
}

/*
 * Return 1 if the specified interleave set already contains a namespaces, 0 if not.
 */
NVM_BOOL interleave_set_has_namespace(const NVM_UINT32 interleave_set_driver_id)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL ns_exists = 0;
	int ns_count = get_namespace_count();
	if (ns_count > 0)
	{
		struct nvm_namespace_discovery namespaces[ns_count];
		memset(&namespaces, 0, sizeof (struct namespace_discovery) * ns_count);
		if (get_namespaces(ns_count, namespaces) == ns_count)
		{
			for (int i = 0; i < ns_count; i++)
			{
				struct nvm_namespace_details details;
				memset(&details, 0, sizeof (details));
				if (get_namespace_details(namespaces[i].namespace_uid, &details) == NVM_SUCCESS)
				{
					if (details.namespace_creation_id.interleave_setid ==
							interleave_set_driver_id && details.type == NAMESPACE_TYPE_APP_DIRECT)
					{
						ns_exists = 1;
						break;
					}
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(ns_exists);
	return ns_exists;
}


NVM_BOOL interleave_set_has_locked_dimms(const struct interleave_set *p_ilset)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	struct device_discovery discovery;
	for (int dimm_index = 0; dimm_index < p_ilset->dimm_count; dimm_index++)
	{
		if (nvm_get_device_discovery(p_ilset->dimms[dimm_index], &discovery) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR("Failed to get device discovery information.");
		}
		else
		{
			if (discovery.lock_state == LOCK_STATE_LOCKED)
			{
				result = 1;
				break;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}


int find_interleave_with_capacity(const struct pool *p_pool,
		const struct nvm_capabilities *p_nvm_caps,
		struct namespace_create_settings *p_settings,
		NVM_UINT32 *namespace_creation_id,
		const struct interleave_format *p_format,
		NVM_BOOL allow_adjustment)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_BOOL at_least_one_security_met = 0;
	NVM_BOOL at_least_one_interleave_format_met = 0;
	NVM_BOOL at_least_one_alignment_met = 0;
	NVM_BOOL found_ilset = 0;
	NVM_BOOL at_least_size_is_good = 0;
	NVM_UINT64 new_block_count = 0;
	NVM_BOOL atleast_one_ilset_have_unlocked_dimms = 0;
	for (int ilset_index = 0; ilset_index < p_pool->ilset_count &&
			found_ilset == 0; ilset_index++)
	{
		const struct interleave_set *p_interleave = &(p_pool->ilsets[ilset_index]);
		if (!interleave_set_has_locked_dimms(p_interleave))
		{
			atleast_one_ilset_have_unlocked_dimms = 1;
			// only look for space if the iset doesn't already have a namespace
			if (!interleave_set_has_namespace(p_interleave->driver_id))
			{
				NVM_UINT64 minimum_ns_size = get_minimum_ns_size(p_interleave->dimm_count,
						p_nvm_caps->sw_capabilities.min_namespace_size);
				new_block_count = p_settings->block_count;
				adjust_namespace_block_count_if_allowed(&new_block_count, p_settings->block_size,
					p_interleave->dimm_count, allow_adjustment);
				NVM_UINT64 new_namespace_capacity = new_block_count * p_settings->block_size;

				// verify if the requested namespace capacity is available
				if (p_interleave->available_size >= new_namespace_capacity &&
						new_namespace_capacity >= minimum_ns_size)
				{
					at_least_size_is_good = 1;
					const NVM_BOOL is_security_met = interleave_meets_security_criteria(
							p_interleave, &p_settings->security_features);
					const NVM_BOOL is_interleave_format_met =
							interleave_meets_app_direct_settings(p_interleave, p_format);
					const NVM_BOOL namespace_alignment_is_good =
							check_namespace_alignment(new_namespace_capacity,
							p_settings->block_size, p_interleave->dimm_count);
					at_least_one_security_met |= is_security_met;
					at_least_one_interleave_format_met |= is_interleave_format_met;
					at_least_one_alignment_met |= namespace_alignment_is_good;

					if (is_security_met && is_interleave_format_met && namespace_alignment_is_good)
					{
						*namespace_creation_id = p_pool->ilsets[ilset_index].driver_id;
						found_ilset = 1;
					}
				}
			}
		}
	}
	// determine return code based on search results
	if (found_ilset)
	{
		p_settings->block_count = new_block_count;
		rc = NVM_SUCCESS;
	}
	else if (!atleast_one_ilset_have_unlocked_dimms)
	{
		// all interleave sets have one/more locked dimms
		rc = NVM_ERR_BADSECURITYSTATE;
	}
	else if (!at_least_size_is_good)
	{
		// namespace request was either too big or too small
		rc = NVM_ERR_BADSIZE;
	}
	else if (!at_least_one_security_met)
	{
		// never met security goals
		rc = NVM_ERR_BADSECURITYGOAL;
	}
	else if (!at_least_one_alignment_met)
	{
		// never met the alignment requirement
		rc = NVM_ERR_BADALIGNMENT;
	}
	else
	{
		rc = NVM_ERR_BADNAMESPACESETTINGS;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL interleave_meets_app_direct_settings(const struct interleave_set *p_interleave,
		const struct interleave_format *p_format)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	if (p_format == NULL)
	{
		result = 1;
	}
	else
	{
		enum interleave_size settings_imc_size = p_format->imc;
		enum interleave_size settings_channel_size = p_format->channel;

		enum interleave_size interleave_imc_size = p_interleave->settings.imc;
		enum interleave_size interleave_channel_size = p_interleave->settings.channel;

		// by 1 is the only interleave way that can be specified for creating a namespace.
		NVM_BOOL format_ways_match = 0;
		if (p_format->ways != INTERLEAVE_WAYS_1 ||
			(p_format->ways == p_interleave->settings.ways))
		{
			format_ways_match = 1;
		}

		if (settings_channel_size == interleave_channel_size &&
			settings_imc_size == interleave_imc_size &&
			format_ways_match)
		{
			result = 1;
		}
	}


	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

int validate_ns_type_for_pool(enum namespace_type type, const struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if ((type != NAMESPACE_TYPE_STORAGE) && (type != NAMESPACE_TYPE_APP_DIRECT))
	{
		COMMON_LOG_ERROR("Invalid namespace type.");
		rc = NVM_ERR_BADNAMESPACETYPE;
	}
	else if (type == NAMESPACE_TYPE_APP_DIRECT &&
			((p_pool->type != POOL_TYPE_PERSISTENT_MIRROR) &&
			(p_pool->type != POOL_TYPE_PERSISTENT)))
	{
		COMMON_LOG_ERROR("Invalid namespace type for the specified pool.");
		rc = NVM_ERR_BADNAMESPACETYPE;
	}
	else if (type == NAMESPACE_TYPE_STORAGE &&
			p_pool->type != POOL_TYPE_PERSISTENT)
	{
		COMMON_LOG_ERROR("Invalid namespace type for the specified pool.");
		rc = NVM_ERR_BADNAMESPACETYPE;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int allocatedPageStructsAreInDramOrPmem(struct namespace_create_settings *p_settings)
{
	return ((p_settings->memory_page_allocation == NAMESPACE_MEMORY_PAGE_ALLOCATION_DRAM) ||
	(p_settings->memory_page_allocation == NAMESPACE_MEMORY_PAGE_ALLOCATION_APP_DIRECT));
}

/*
 * Helper function to validate namespace settings
 * Also determines namespace_creation_id to be used by the driver
 */
int validate_namespace_create_settings(struct pool *p_pool,
		struct namespace_create_settings *p_settings,
		const struct interleave_format *p_format,
		NVM_UINT32 *p_ns_creation_id,
		NVM_BOOL allow_adjustment)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct nvm_capabilities nvm_caps;
	memset(&nvm_caps, 0, sizeof (nvm_caps));

	if ((p_pool->type == POOL_TYPE_PERSISTENT_MIRROR) &&
			(p_settings->type == NAMESPACE_TYPE_STORAGE))
	{
		COMMON_LOG_ERROR("Cannot create storage namespace in mirrored pool");
		rc = NVM_ERR_BADNAMESPACETYPE;
	}
	else if ((rc = nvm_get_nvm_capabilities(&nvm_caps)) == NVM_SUCCESS)
	{
		// get the pool supported size ranges
		struct possible_namespace_ranges range;
		memset(&range, 0, sizeof (range));
		if ((rc = get_pool_supported_size_ranges(p_pool, &nvm_caps, &range)) == NVM_SUCCESS)
		{
			// can we even create a ns on this pool?
			if (p_settings->type == NAMESPACE_TYPE_APP_DIRECT)
			{
				if (range.largest_possible_app_direct_ns == 0)
				{
					COMMON_LOG_ERROR("No more App Direct namespaces can be created on the pool");
					rc = NVM_ERR_TOOMANYNAMESPACES;
				}
				else if ((p_settings->btt) &&
					(allocatedPageStructsAreInDramOrPmem(p_settings)))
				{
					COMMON_LOG_ERROR("Namespace can either be claimed by pfn or btt configuration");
					rc = NVM_ERR_NOTSUPPORTED;
				}
				else if ((allocatedPageStructsAreInDramOrPmem(p_settings)) &&
					(!nvm_caps.sw_capabilities.namespace_memory_page_allocation_capable))
				{
					COMMON_LOG_ERROR("Memory page allocation is not supported.");
					rc = NVM_ERR_NOTSUPPORTED;
				}
			}
			else if (p_settings->type == NAMESPACE_TYPE_STORAGE)
			{
				if (range.largest_possible_storage_ns == 0)
				{
					COMMON_LOG_ERROR("No more Storage namespaces can be created on the pool");
					rc = NVM_ERR_TOOMANYNAMESPACES;
				}
				else if (allocatedPageStructsAreInDramOrPmem(p_settings))
				{
					COMMON_LOG_ERROR("storage namespace do not support memory mode");
					rc = NVM_ERR_NOTSUPPORTED;
				}
			}

			if (rc == NVM_SUCCESS)
			{
				if ((rc = validate_ns_type_for_pool(p_settings->type, p_pool)) == NVM_SUCCESS &&
					(rc = validate_ns_enabled_state(p_settings->enabled)) == NVM_SUCCESS &&
					(rc = validate_ns_size_for_creation(p_pool, p_settings,
							&nvm_caps, &range)) == NVM_SUCCESS)
				{
					rc = find_id_for_ns_creation(p_pool, &nvm_caps, p_settings,
							p_ns_creation_id, p_format, allow_adjustment);
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_UINT32 get_alignment_size(NVM_UINT32 block_size, NVM_UINT32 ways)
{
	return get_lowest_common_multiple(block_size, PAGE_SIZE_x86 * ways);
}

void adjust_namespace_block_count(NVM_UINT64 *p_block_count, const NVM_UINT16 block_size,
			const NVM_UINT8 ways)
{
	COMMON_LOG_ENTRY();

	NVM_UINT32 real_block_size = get_real_block_size(block_size);
	NVM_UINT64 alignment_size = get_alignment_size(real_block_size, ways);
	NVM_UINT64 capacity = *p_block_count * real_block_size;
	capacity = round_up(capacity, alignment_size);
	*p_block_count = capacity / real_block_size;

	COMMON_LOG_EXIT();
}

/*
 * Helper function to adjust the size of a namespace if the user will allow it
 */
void adjust_namespace_block_count_if_allowed(NVM_UINT64 *p_block_count, const NVM_UINT16 block_size,
		NVM_UINT8 ways, const NVM_BOOL allow_adjustment)
{
	COMMON_LOG_ENTRY();

	if (allow_adjustment)
	{
		adjust_namespace_block_count(p_block_count, block_size, ways);
	}

	COMMON_LOG_EXIT();
}

int nvm_adjust_create_namespace_block_count(const NVM_UID pool_uid,
		struct namespace_create_settings *p_settings,
		const struct interleave_format *p_format)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(create_namespace)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Creating a namespace is not supported.");
	}
	else if (pool_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, pool uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_settings == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_settings is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		struct pool *p_pool = (struct pool *)calloc(1, sizeof (struct pool));
		if (p_pool)
		{
			if ((rc = nvm_get_pool(pool_uid, p_pool)) == NVM_SUCCESS)
			{
				NVM_UINT32 namespace_creation_id;
				rc = validate_namespace_create_settings(p_pool, p_settings,
						p_format, &namespace_creation_id, 1);
			}
			else
			{
				COMMON_LOG_ERROR_F("nvm_get_pool failed with rc - %d", rc);
			}
			free(p_pool);
		}
		else
		{
			rc = NVM_ERR_NOMEMORY;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int nvm_adjust_modify_namespace_block_count(
		const NVM_UID namespace_uid, NVM_UINT64 *p_block_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct namespace_details details;
	memset(&details, 0, sizeof (details));

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if (namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, namespace uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_block_count == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, block_count pointer is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (*p_block_count <= 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, block_count must be greater than zero");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = nvm_get_namespace_details(namespace_uid, &details)) == NVM_SUCCESS)
	{
		if (details.block_count < *p_block_count &&
			(rc = IS_NVM_FEATURE_LICENSED(grow_namespace)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR("Increasing namespace size not supported.");
		}
		else if (details.block_count > *p_block_count &&
				(rc = IS_NVM_FEATURE_LICENSED(shrink_namespace)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR("Decreasing namespace size not supported.");
		}
		else
		{
			rc = validate_namespace_block_count(namespace_uid, p_block_count, 1);
		}
	}
	else
	{
		COMMON_LOG_ERROR_F("nvm_get_namespace_details failed with rc - %d", rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Create a new namespace from the specified pool.
 */
int nvm_create_namespace(NVM_UID *p_namespace_uid, const NVM_UID pool_uid,
		struct namespace_create_settings *p_settings,
		const struct interleave_format *p_format, const NVM_BOOL allow_adjustment)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	NVM_UINT32 namespace_creation_id; // the identifier used by the driver to create a namespace

	struct nvm_capabilities nvm_caps;
	memset(&nvm_caps, 0, sizeof (nvm_caps));

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(create_namespace)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Creating a namespace is not supported.");
	}
	else if (pool_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, pool uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_settings == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_settings is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_namespace is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		// check if pool exists
		struct pool *p_pool = (struct pool *)calloc(1, sizeof (struct pool));
		if (p_pool)
		{
			if ((rc = nvm_get_pool(pool_uid, p_pool)) == NVM_SUCCESS)
			{
				if ((rc = validate_namespace_create_settings(p_pool, p_settings, p_format,
							&namespace_creation_id, allow_adjustment)) == NVM_SUCCESS)
				{
					struct nvm_namespace_create_settings nvm_settings;
					memset(&nvm_settings, 0, sizeof (nvm_settings));

					nvm_settings.type = p_settings->type;
					if (nvm_settings.type == NAMESPACE_TYPE_APP_DIRECT)
					{
						nvm_settings.namespace_creation_id.interleave_setid
							= namespace_creation_id;
						COMMON_LOG_DEBUG_F("Creating App Direct namespace on interleave set %u",
								namespace_creation_id);
					}
					else
					{
						nvm_settings.namespace_creation_id.device_handle.handle
							= namespace_creation_id;
						COMMON_LOG_DEBUG_F("Creating storage namespace on DIMM %u",
								namespace_creation_id);
					}
					if (s_strnlen(p_settings->friendly_name, NAMESPACE_FRIENDLY_NAME_LEN) == 0)
					{
						char friendly_name[NVM_NAMESPACE_NAME_LEN];
						int max_unique_id = 0;
						int namespace_count = nvm_get_namespace_count();
						if (namespace_count < 0)
						{
							namespace_count = 0;
						}

						if (namespace_count > 0)
						{
							// Figure out a unique ID for the default namespace name
							// Find the max value of the ID currently present,
							// and generate max_unique_id+1 as the ID
							struct nvm_namespace_discovery nvm_namespaces[namespace_count];
							get_namespaces(namespace_count, nvm_namespaces);
							for (int ns = 0; ns < namespace_count; ns++)
							{
								// Determine ID only if it is a default namespace name
								if (!s_strncmp(NVM_DEFAULT_NAMESPACE_NAME,
										nvm_namespaces[ns].friendly_name,
										s_strnlen(NVM_DEFAULT_NAMESPACE_NAME,
												NVM_NAMESPACE_NAME_LEN)))
								{
									unsigned int id = 0;
									s_strtoui(nvm_namespaces[ns].friendly_name,
										s_strnlen(nvm_namespaces[ns].friendly_name,
											NVM_NAMESPACE_NAME_LEN),
										NULL,
										&id);
									if (id > max_unique_id)
									{
										max_unique_id = id;
									}
								}
							}
						}
						// create a unique friendly name - NvDimmVolN.
						char namespace_name[NVM_NAMESPACE_NAME_LEN];
						s_strcpy(namespace_name, NVM_DEFAULT_NAMESPACE_NAME,
								NVM_NAMESPACE_NAME_LEN);
						s_snprintf(friendly_name, NVM_NAMESPACE_NAME_LEN,
								s_strcat(namespace_name, NVM_NAMESPACE_NAME_LEN, "%d"),
										max_unique_id + 1);
						s_strcpy(nvm_settings.friendly_name,
								friendly_name, NVM_NAMESPACE_NAME_LEN);
					}
					else
					{
						s_strncpy(nvm_settings.friendly_name, NVM_NAMESPACE_NAME_LEN,
								p_settings->friendly_name, NVM_NAMESPACE_NAME_LEN);
					}
					nvm_settings.enabled = p_settings->enabled;
					nvm_settings.block_size = p_settings->block_size;
					nvm_settings.block_count = p_settings->block_count;
					nvm_settings.btt = p_settings->btt;
					nvm_settings.memory_page_allocation = p_settings->memory_page_allocation;

					rc = create_namespace(p_namespace_uid, &nvm_settings);
					if (rc == NVM_SUCCESS)
					{
						// the context is no longer valid
						invalidate_namespaces();

						// Log an event indicating we successfully created a namespace
						NVM_EVENT_ARG ns_uid_arg;
						uid_to_event_arg(*p_namespace_uid, ns_uid_arg);
						NVM_EVENT_ARG ns_name_arg;
						s_strncpy(ns_name_arg, NVM_EVENT_ARG_LEN,
								nvm_settings.friendly_name, NVM_NAMESPACE_NAME_LEN);
						log_mgmt_event(EVENT_SEVERITY_INFO,
								EVENT_CODE_MGMT_NAMESPACE_CREATED,
								*p_namespace_uid,
								0, // no action required
								ns_name_arg, ns_uid_arg, NULL);
					}
				}
			}
			else
			{
				COMMON_LOG_ERROR_F("couldn't get pool, rc = %d", rc);
			}
			free(p_pool);
		}
		else
		{
			rc = NVM_ERR_NOMEMORY;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int validate_app_direct_namespace_size_for_modification(
		const struct pool *p_pool,
		NVM_UINT64 *p_block_count,
		struct nvm_namespace_details *p_nvm_details,
		struct nvm_capabilities *p_nvm_caps,
		const NVM_BOOL allow_adjustment)
{
	int rc = NVM_SUCCESS;

	for (int i = 0; i < p_pool->ilset_count; i++)
	{
		if (p_pool->ilsets[i].driver_id == p_nvm_details->namespace_creation_id.interleave_setid)
		{
			NVM_UINT64 minimum_ns_size = get_minimum_ns_size(p_pool->ilsets[i].dimm_count,
					p_nvm_caps->sw_capabilities.min_namespace_size);
			NVM_UINT64 old_namespace_capacity =
					p_nvm_details->block_count * (NVM_UINT64)p_nvm_details->block_size;
			NVM_UINT64 new_block_count = *p_block_count;
			adjust_namespace_block_count_if_allowed(&new_block_count, p_nvm_details->block_size,
					p_pool->ilsets[i].dimm_count, allow_adjustment);
			NVM_UINT64 namespace_capacity = new_block_count * p_nvm_details->block_size;

			if ((namespace_capacity	> old_namespace_capacity) &&
					(p_pool->ilsets[i].available_size <
					(namespace_capacity - old_namespace_capacity)))
			{
				COMMON_LOG_ERROR_F("Caller requested namespace capacity \
					%llu bytes is more than the available size on the \
					interleave set %llu bytes",
					namespace_capacity, p_pool->ilsets[i].available_size);
				rc = NVM_ERR_BADSIZE;
			}
			else if (namespace_capacity < minimum_ns_size)
			{
					COMMON_LOG_ERROR_F("Caller requested namespace capacity \
						%llu bytes is less than the smallest \
						supported namespace size %llu bytes",
						namespace_capacity, minimum_ns_size);
					rc = NVM_ERR_BADSIZE;
			}
			else if (!check_namespace_alignment(namespace_capacity,
					p_nvm_details->block_size, p_pool->ilsets[i].dimm_count))
			{
				COMMON_LOG_ERROR_F("Caller requested namespace capacity %llu bytes is \
						not in alignment with the namespace alignment size \
						supported by the driver %llu bytes",
						namespace_capacity, p_pool->free_capacity);
				rc = NVM_ERR_BADALIGNMENT;
			}
			else
			{
				*p_block_count = new_block_count;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int validate_storage_namespace_size_for_modification(const struct pool *p_pool,
		NVM_UINT64 *p_block_count,
		struct nvm_namespace_details *p_nvm_details,
		struct nvm_capabilities *p_nvm_caps,
		const NVM_BOOL allow_adjustment)
{
	int rc = NVM_SUCCESS;

	struct device_discovery discovery;
	if (lookup_dev_handle(p_nvm_details->namespace_creation_id.device_handle,
			&discovery) >= 0)
	{
		// storage namespaces are restricted to one DIMM
		NVM_UINT64 minimum_ns_size =
				get_minimum_ns_size(1, p_nvm_caps->sw_capabilities.min_namespace_size);
		NVM_UINT32 real_block_size =
				get_real_block_size(p_nvm_details->block_size);
		NVM_UINT64 old_namespace_capacity =
				p_nvm_details->block_count * (NVM_UINT64)real_block_size;
		NVM_UINT64 new_block_count = *p_block_count;
		adjust_namespace_block_count_if_allowed(&new_block_count,
				real_block_size, 1, allow_adjustment);
		NVM_UINT64 namespace_capacity = new_block_count * real_block_size;
		NVM_UINT64 available_size = 0;

		if ((rc = get_largest_storage_namespace_on_a_dimm(p_pool, discovery.uid, &available_size))
				== NVM_SUCCESS)
		{
			if ((namespace_capacity > old_namespace_capacity) &&
					(available_size < (namespace_capacity - old_namespace_capacity)))
			{
				COMMON_LOG_ERROR_F("Caller requested namespace capacity %llu \
						bytes is more than the available size on the dimm %llu \
						bytes", namespace_capacity, available_size);
				rc = NVM_ERR_BADSIZE;
			}
			else if (namespace_capacity < minimum_ns_size)
			{
				COMMON_LOG_ERROR_F("Caller requested namespace capacity %llu bytes \
						is smaller than the supported minimum namespace size %llu bytes.",
						namespace_capacity, minimum_ns_size);
				rc = NVM_ERR_BADSIZE;
			}
			else if (!check_namespace_alignment(namespace_capacity, real_block_size, 1))
			{
				COMMON_LOG_ERROR_F("Caller requested namespace capacity %llu \
						bytes is not in alignment with the namespace alignment \
						size supported by the driver %llu bytes",
						namespace_capacity, p_pool->free_capacity);

				rc = NVM_ERR_BADALIGNMENT;
			}
			else
			{
				*p_block_count = new_block_count;
			}
		}
		else
		{
			COMMON_LOG_ERROR_F("get_largest_namespace_on_a_dimm failed with error code %d", rc);
		}
	}
	else
	{
		COMMON_LOG_ERROR("Failed to find device");
		rc = NVM_ERR_BADDEVICE;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to validate namespace size when modifying a NS
 */
int validate_namespace_size_for_modification(const struct pool *p_pool, NVM_UINT64 *p_block_count,
				struct nvm_namespace_details *p_nvm_details, const NVM_BOOL allow_adjustment)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	NVM_UINT64 namespace_capacity = *p_block_count * (NVM_UINT64)p_nvm_details->block_size;
	struct nvm_capabilities nvm_caps;
	memset(&nvm_caps, 0, sizeof (nvm_caps));
	if ((rc = nvm_get_nvm_capabilities(&nvm_caps)) == NVM_SUCCESS)
	{
		NVM_UINT32 real_block_size = get_real_block_size(p_nvm_details->block_size);
		NVM_UINT64 old_namespace_capacity =
				(NVM_UINT64)real_block_size * p_nvm_details->block_count;

		if ((namespace_capacity > old_namespace_capacity) &&
				(p_pool->free_capacity < (namespace_capacity - old_namespace_capacity)))
		{
			COMMON_LOG_ERROR_F("Caller requested namespace capacity %llu bytes \
					is more than the available size of pool %llu bytes",
					namespace_capacity, p_pool->free_capacity);
			rc = NVM_ERR_BADSIZE;
		}
		else if (p_nvm_details->type == NAMESPACE_TYPE_APP_DIRECT)
		{
			rc = validate_app_direct_namespace_size_for_modification(p_pool, p_block_count,
					p_nvm_details, &nvm_caps, allow_adjustment);
		}
		else if (p_nvm_details->type == NAMESPACE_TYPE_STORAGE)
		{
			rc = validate_storage_namespace_size_for_modification(p_pool, p_block_count,
					p_nvm_details, &nvm_caps, allow_adjustment);
		}
		else
		{
			COMMON_LOG_ERROR("The namespace type is not valid");
			rc = NVM_ERR_BADNAMESPACETYPE;
		}
	}
	else
	{
		COMMON_LOG_ERROR_F("nvm_get_nvm_capabilities failed with error code %d", rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
/*
 * Helper function to validate modify namespace settings
 */
int validate_namespace_block_count(const NVM_UID namespace_uid,
		NVM_UINT64 *p_block_count, const NVM_BOOL allow_adjustment)
{
	// check if the namespace exists
	struct nvm_namespace_details nvm_details;
	memset(&nvm_details, 0, sizeof (nvm_details));
	int rc = get_namespace_details(namespace_uid, &nvm_details);
	if (rc == NVM_SUCCESS)
	{
		NVM_UID pool_uid;
		rc = get_pool_uid_from_namespace_details(&nvm_details, &pool_uid);
		if (rc == NVM_SUCCESS)
		{
			struct pool *p_pool = (struct pool *)calloc(1, sizeof (struct pool));
			if (p_pool)
			{
				rc = nvm_get_pool(pool_uid, p_pool);
				if (rc == NVM_SUCCESS)
				{
					rc = validate_namespace_size_for_modification(p_pool, p_block_count,
							&nvm_details, allow_adjustment);
				}
				free(p_pool);
			}
			else
			{
				rc = NVM_ERR_NOMEMORY;
			}
		}
	}
	else
	{
		COMMON_LOG_ERROR("Failed to retrieve namespace details.");
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Modify the namespace specified.
 */
int nvm_modify_namespace_name(const NVM_UID namespace_uid,
		const NVM_NAMESPACE_NAME name)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(rename_namespace)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Renaming a namespace is not supported.");
	}
	else if (namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, namespace_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (name == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, name is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		struct namespace_details details;
		memset(&details, 0, sizeof (details));
		rc = nvm_get_namespace_details(namespace_uid, &details);
		if (rc == NVM_SUCCESS)
		{
			rc = modify_namespace_name(namespace_uid, name);
			if (rc == NVM_SUCCESS)
			{
				// the namespace context is no longer valid
				invalidate_namespaces();

				// Log an event indicating we successfully modified a namespace
				NVM_EVENT_ARG ns_uid_arg;
				uid_to_event_arg(namespace_uid, ns_uid_arg);
				NVM_EVENT_ARG ns_name_arg;
				s_strncpy(ns_name_arg, NVM_EVENT_ARG_LEN,
						name, NVM_NAMESPACE_NAME_LEN);
				log_mgmt_event(EVENT_SEVERITY_INFO,
						EVENT_CODE_MGMT_NAMESPACE_MODIFIED,
						namespace_uid,
						0, // no action required
						ns_name_arg, ns_uid_arg, NULL);
			}
			else
			{
				COMMON_LOG_ERROR("Could not modify namespace name");
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Modify the namespace specified.
 */
int nvm_modify_namespace_block_count(const NVM_UID namespace_uid,
		NVM_UINT64 block_count, NVM_BOOL allow_adjustment)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if (namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, namespace_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (block_count == 0)
	{
		COMMON_LOG_ERROR("Invalid block count to create a namespace.");
		rc = NVM_ERR_BADSIZE;
	}
	else
	{
		struct namespace_details details;
		memset(&details, 0, sizeof (details));
		rc = nvm_get_namespace_details(namespace_uid, &details);
		if (rc == NVM_SUCCESS)
		{
			if (details.block_count < block_count &&
				(rc = IS_NVM_FEATURE_LICENSED(grow_namespace)) != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR("Increasing namespace size not supported.");
			}
			else if (details.block_count > block_count &&
					(rc = IS_NVM_FEATURE_LICENSED(shrink_namespace)) != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR("Decreasing namespace size not supported.");
			}
			else
			{
				rc = validate_namespace_block_count(namespace_uid,
						&block_count, allow_adjustment);
				if (rc == NVM_SUCCESS)
				{
					rc = modify_namespace_block_count(namespace_uid, block_count);
					if (rc == NVM_SUCCESS)
					{
						// the namespace context is no longer valid
						invalidate_namespaces();

						// Log an event indicating we successfully modified a namespace
						NVM_EVENT_ARG ns_uid_arg;
						uid_to_event_arg(namespace_uid, ns_uid_arg);
						NVM_EVENT_ARG ns_name_arg;
						s_strncpy(ns_name_arg, NVM_EVENT_ARG_LEN,
								details.discovery.friendly_name, NVM_NAMESPACE_NAME_LEN);
						log_mgmt_event(EVENT_SEVERITY_INFO,
								EVENT_CODE_MGMT_NAMESPACE_MODIFIED,
								namespace_uid,
								0, // no action required
								ns_name_arg, ns_uid_arg, NULL);
					}
					else
					{
						COMMON_LOG_ERROR("Could not modify namespace block count");
					}
				}
				else
				{
					COMMON_LOG_ERROR("Bad block count for nvm_modify_namespace");
				}
			}
		}
		else
		{
			COMMON_LOG_ERROR("Could not retrieve namespace details.");
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Modify the namespace specified.
 */
int nvm_modify_namespace_enabled(const NVM_UID namespace_uid,
		const enum namespace_enable_state enabled)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if (namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, namespace_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (enabled == NAMESPACE_ENABLE_STATE_DISABLED ||
			enabled == NAMESPACE_ENABLE_STATE_ENABLED)
	{
		if (enabled  == NAMESPACE_ENABLE_STATE_DISABLED &&
			(rc = IS_NVM_FEATURE_LICENSED(disable_namespace)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR("Disabling a namespace is not supported.");
		}
		else if (enabled  == NAMESPACE_ENABLE_STATE_ENABLED &&
				(rc = IS_NVM_FEATURE_LICENSED(enable_namespace)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR("Enabling a namespace is not supported.");
		}
		else
		{
			struct namespace_details details;
			memset(&details, 0, sizeof (details));
			rc = nvm_get_namespace_details(namespace_uid, &details);
			if (rc == NVM_SUCCESS && details.enabled != enabled)
			{
				rc = modify_namespace_enabled(namespace_uid, enabled);
				if (rc == NVM_SUCCESS)
				{
					// the namespace context is no longer valid
					invalidate_namespaces();

					// Log an event indicating we successfully modified a namespace
					NVM_EVENT_ARG ns_uid_arg;
					uid_to_event_arg(namespace_uid, ns_uid_arg);
					NVM_EVENT_ARG ns_name_arg;
					s_strncpy(ns_name_arg, NVM_EVENT_ARG_LEN,
							details.discovery.friendly_name, NVM_NAMESPACE_NAME_LEN);
					log_mgmt_event(EVENT_SEVERITY_INFO,
							EVENT_CODE_MGMT_NAMESPACE_MODIFIED,
							namespace_uid,
							0, // no action required
							ns_name_arg, ns_uid_arg, NULL);
				}
				else
				{
					COMMON_LOG_ERROR("Could not modify namespace block count");
				}
			}
		}
	}
	else
	{
		COMMON_LOG_ERROR("Unrecognized enable state.");
		rc = NVM_ERR_BADNAMESPACEENABLESTATE;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
/*
 * Delete an existing namespace.
 */
int nvm_delete_namespace(const NVM_UID namespace_uid)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(delete_namespace)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Deleting a namespace is not supported.");
	}
	else if (namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, namespace_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		struct namespace_details details;
		memset(&details, 0, sizeof (details));
		rc = nvm_get_namespace_details(namespace_uid, &details);
		if (rc == NVM_SUCCESS)
		{
			rc = delete_namespace(namespace_uid);
			if (rc == NVM_SUCCESS)
			{
				// the namespace context is no longer valid
				invalidate_namespaces();

				// Log an event indicating we successfully deleted a namespace
				NVM_EVENT_ARG ns_uid_arg;
				uid_to_event_arg(namespace_uid, ns_uid_arg);
				NVM_EVENT_ARG ns_name_arg;
				s_strncpy(ns_name_arg, NVM_EVENT_ARG_LEN,
						details.discovery.friendly_name, NVM_NAMESPACE_NAME_LEN);
				log_mgmt_event(EVENT_SEVERITY_INFO,
						EVENT_CODE_MGMT_NAMESPACE_DELETED,
						namespace_uid,
						0, // no action required
						ns_name_arg, ns_uid_arg, NULL);

				// clear any action required events for the deleted namespace
				struct event_filter filter;
				memset(&filter, 0, sizeof (filter));
				filter.filter_mask = NVM_FILTER_ON_UID | NVM_FILTER_ON_AR;
				memmove(filter.uid, namespace_uid, NVM_MAX_UID_LEN);
				filter.action_required = 1;
				acknowledge_events(&filter);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Return the size of the largest App Direct namespace that can be created.
 * Expect pool_uid and p_size to not be NULL
 */
void get_largest_app_direct_namespace(const struct pool *p_pool, NVM_UINT64 *p_size)
{
	COMMON_LOG_ENTRY();

	*p_size = 0;

	// find the largest available space on any interleave sets in the pool
	for (int i = 0; i < p_pool->ilset_count; i++)
	{
		if (p_pool->ilsets[i].available_size > *p_size &&
			!interleave_set_has_namespace(p_pool->ilsets[i].driver_id))
		{
			*p_size = p_pool->ilsets[i].available_size;
		}
	}

	COMMON_LOG_EXIT();
}

/*
 * Return the size of the smallest App Direct namespace that can be created.
 */
void get_smallest_app_direct_namespace(const struct pool *p_pool,
		const NVM_UINT64 interleave_alignment_size, NVM_UINT64 *p_size)
{
	COMMON_LOG_ENTRY();

	// find the smallest interleave way
	int min_way = 0;
	if (p_pool->ilset_count)
	{
		min_way = p_pool->dimm_count;
		for (int i = 0; i < p_pool->ilset_count; i++)
		{
			if (p_pool->ilsets[i].dimm_count < min_way)
			{
				min_way = p_pool->ilsets[i].dimm_count;
			}
		}
	}

	// smallest = interleave_alignment_size * way
	*p_size = interleave_alignment_size * min_way;
	COMMON_LOG_EXIT();
}

/*
 * Return the size of the largest storage namespace that can be created
 */
int get_largest_storage_namespace(const struct pool *p_pool, NVM_UINT64 *p_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_pool->type == POOL_TYPE_PERSISTENT_MIRROR ||
		p_pool->type == POOL_TYPE_VOLATILE)
	{
		*p_size = 0;
	}
	else
	{
		*p_size = 0;

		for (int i = 0; (i < p_pool->dimm_count) && (rc == NVM_SUCCESS); i++)
		{
			NVM_UINT64 dimm_largest_storage_namespace;

			if ((rc = get_largest_storage_namespace_on_a_dimm(p_pool, p_pool->dimms[i],
				&dimm_largest_storage_namespace)) == NVM_SUCCESS)
			{
				if (dimm_largest_storage_namespace > *p_size)
				{
					*p_size = dimm_largest_storage_namespace;
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_UINT64 get_smallest_block_size(const struct nvm_capabilities *p_caps)
{
	COMMON_LOG_ENTRY();
	NVM_UINT64 smallest_block_size = 0;
	if (p_caps->sw_capabilities.block_size_count > 0)
	{
		smallest_block_size = p_caps->sw_capabilities.block_sizes[0];
		for (int i = 1; i < p_caps->sw_capabilities.block_size_count; i++)
		{
			if (p_caps->sw_capabilities.block_sizes[i] < smallest_block_size)
			{
				smallest_block_size = p_caps->sw_capabilities.block_sizes[0];
			}
		}
	}
	COMMON_LOG_EXIT();
	return smallest_block_size;
}

int get_pool_supported_size_ranges(const struct pool *p_pool,
		const struct nvm_capabilities *p_caps,
		struct possible_namespace_ranges *p_range)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	get_largest_app_direct_namespace(p_pool, &(p_range->largest_possible_app_direct_ns));
	if (p_range->largest_possible_app_direct_ns > 0)
	{
		NVM_UINT64 interleave_alignment_size =
				(NVM_UINT64)pow(2,
				p_caps->platform_capabilities.app_direct_mode.interleave_alignment_size);
		p_range->app_direct_increment = interleave_alignment_size;
		get_smallest_app_direct_namespace(p_pool, interleave_alignment_size,
				&(p_range->smallest_possible_app_direct_ns));
	}
	rc = get_largest_storage_namespace(p_pool, &(p_range->largest_possible_storage_ns));
	if (p_range->largest_possible_storage_ns > 0)
	{
		p_range->smallest_possible_storage_ns =
				get_minimum_ns_size(1, p_caps->sw_capabilities.min_namespace_size);
		p_range->storage_increment = get_smallest_block_size(p_caps);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * Given a namespace setting instance, return the largest and smallest
 * namespaces that can be created with that setting, along with the increment
 * that must be used to determine the valid capacities between the smallest and
 * largest namespaces.
 */
int nvm_get_available_persistent_size_range(const NVM_UID pool_uid,
		struct possible_namespace_ranges *p_range)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (pool_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, pool_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_range == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_range is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else
	{
		memset(p_range, 0, sizeof (struct possible_namespace_ranges));

		// retrieve the pool
		struct pool *p_pool = (struct pool *)calloc(1, sizeof (struct pool));
		if (p_pool)
		{
			if ((rc = nvm_get_pool(pool_uid, p_pool)) == NVM_SUCCESS)
			{
				// retrieve the capabilities
				struct nvm_capabilities nvm_caps;
				memset(&nvm_caps, 0, sizeof (nvm_caps));
				if ((rc = nvm_get_nvm_capabilities(&nvm_caps)) == NVM_SUCCESS)
				{
					rc = get_pool_supported_size_ranges(p_pool, &nvm_caps, p_range);
				}
			}
			free(p_pool);
		}
		else
		{
			rc = NVM_ERR_NOMEMORY;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Determine if any existing namespaces of the specified type
 * utilize capacity from the specified device.
 */
int nvm_get_device_namespace_count(const NVM_UID device_uid,
		const enum namespace_type type)
{
	int rc = 0; // return namespace count
	struct nvm_capabilities capabilities;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = nvm_get_nvm_capabilities(&capabilities)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the system capabilities.");
	}
	else if (!capabilities.nvm_features.get_namespaces) // also confirms pass through
	{
		COMMON_LOG_ERROR("Retrieving namespaces is not supported.");
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		rc = dimm_has_namespaces_of_type(discovery.device_handle, type);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
