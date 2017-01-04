/*
 * Copyright (c) 2016, Intel Corporation
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

#include <uid/uid.h>
#include "utility.h"
#include "namespace_utils.h"

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

NVM_UINT32 get_alignment_size(NVM_UINT32 block_size, NVM_UINT32 ways)
{
	return get_lowest_common_multiple(block_size, PAGE_SIZE_x86 * ways);
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

int interleave_matches_byone_request(const struct interleave_set *ilset, int by_one)
{
	int rc = 0;
	if (by_one && ilset->settings.ways == INTERLEAVE_WAYS_1)
	{
		rc = 1;
	}
	else if (!by_one && ilset->settings.ways != INTERLEAVE_WAYS_1)
	{
		rc = 1;
	}
	return rc;
}

int get_ad_dimms_from_pool(const struct pool *p_pool,
		struct device_free_capacities *ad_caps,
		NVM_UINT16 *ad_cap_count, int by_one)
{
	int rc = NVM_SUCCESS;
	NVM_UINT16 valid_caps = 0;
	struct device_free_capacities *tmp_caps = calloc(*ad_cap_count,
			sizeof (struct device_free_capacities));
	if (!tmp_caps)
	{
		rc = NVM_ERR_NOMEMORY;
	}
	else
	{
		for (int i = 0; i < *ad_cap_count; i++)
		{
			for (int j = 0; j < p_pool->ilset_count; j++)
			{
				if (interleave_matches_byone_request(&p_pool->ilsets[j], by_one) &&
					is_uid_in_list(ad_caps[i].uid, p_pool->ilsets[j].dimms,
							p_pool->ilsets[j].dimm_count))
				{
					memmove(&tmp_caps[valid_caps++], &ad_caps[i],
						sizeof (struct device_free_capacities));
				}
			}
		}
	}
	memmove(ad_caps, tmp_caps, sizeof (struct device_free_capacities) * valid_caps);
	*ad_cap_count = valid_caps;
	free(tmp_caps);
	return rc;
}

NVM_UINT64 capacity_to_real_block_count(NVM_UINT64 capacity, NVM_UINT32 block_size)
{
	COMMON_LOG_ENTRY();
	int ways = 1;
	NVM_BOOL allow_adjustment = 1;
	NVM_UINT32 real_block_size = get_real_block_size(block_size);
	NVM_UINT64 block_count = capacity / real_block_size;
	if (!check_namespace_alignment(real_block_size * block_count, real_block_size, ways))
	{
		NVM_UINT64 alignment_size = get_alignment_size(real_block_size, ways);
		block_count = (capacity - alignment_size) / real_block_size;
		adjust_namespace_block_count_if_allowed(&block_count,
			real_block_size, ways, allow_adjustment);
	}
	COMMON_LOG_EXIT();
	return block_count;
}

void set_block_count_in_settings(const struct device_free_capacities *p_cap, NVM_BOOL use_hi_cap,
		NVM_UINT64 hi_cap, NVM_UINT64 low_cap,
		struct namespace_create_settings *p_settings)
{
	if (use_hi_cap)
	{
		p_settings->block_count =
			capacity_to_real_block_count(hi_cap, p_settings->block_size);
	}
	else
	{
		p_settings->block_count =
			capacity_to_real_block_count(low_cap, p_settings->block_size);
	}

}

NVM_BOOL is_capacity_larger(NVM_UINT64 new_total, NVM_UINT64 old_total,
		NVM_UINT64 new_preferred, NVM_UINT64 old_preferred)
{
	int rc = 0;
	if (new_total > old_total)
	{
		rc = 1;
	}
	else if (new_total == old_total &&
		new_preferred > old_preferred)
	{
		rc = 1;
	}
	return rc;
}

int select_largest_storage_only_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	const struct device_free_capacities *p_best_cap = NULL;
	for (int i = 0; i < candidate_dimm_count; i++)
	{
		if (p_free_capacities[i].storage_only_capacity >= minimum_ns_size)
		{
			if (!p_best_cap || p_free_capacities[i].storage_only_capacity >
				p_best_cap->storage_only_capacity)
			{
				p_best_cap = &p_free_capacities[i];
			}
		}
	}

	if (!p_best_cap)
	{
		COMMON_LOG_ERROR("No storage-only regions that meet minimum namespace size.");
		rc = NVM_ERR_BADNAMESPACESETTINGS;
	}
	else
	{
		*p_namespace_creation_id = p_best_cap->device_handle.handle;
		set_block_count_in_settings(p_best_cap, 1, p_best_cap->storage_only_capacity, 0,
			p_settings);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int select_largest_adx1_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	const struct device_free_capacities *p_best_cap = NULL;
	NVM_UINT64 best_adx1_ns_cap = 0;
	for (int i = 0; i < candidate_dimm_count; i++)
	{
		NVM_UINT64 adx1_ns_cap = p_free_capacities[i].storage_only_capacity +
			p_free_capacities[i].app_direct_byone_capacity;
		if (adx1_ns_cap >= minimum_ns_size)
		{
			if (!p_best_cap || adx1_ns_cap > best_adx1_ns_cap)
			{
				p_best_cap = &p_free_capacities[i];
				best_adx1_ns_cap = adx1_ns_cap;
			}
		}
	}
	if (!p_best_cap)
	{
		COMMON_LOG_ERROR("No ADx1 regions that meet minimum namespace size.");
		rc = NVM_ERR_BADNAMESPACESETTINGS;
	}
	else
	{
		*p_namespace_creation_id = p_best_cap->device_handle.handle;
		set_block_count_in_settings(p_best_cap, 1, best_adx1_ns_cap, 0,
			p_settings);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int select_largest_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	const struct device_free_capacities *p_best_cap = NULL;
	for (int i = 0; i < candidate_dimm_count; i++)
	{
		if (p_free_capacities[i].total_capacity >= minimum_ns_size)
		{
			if (!p_best_cap ||
				is_capacity_larger(p_free_capacities[i].total_capacity,
						p_best_cap->total_capacity,
						p_free_capacities[i].storage_only_capacity,
						p_best_cap->storage_only_capacity))
			{
				p_best_cap = &p_free_capacities[i];
			}
		}
	}
	if (!p_best_cap)
	{
		COMMON_LOG_ERROR("No available regions that meet minimum namespace size.");
		rc = NVM_ERR_BADNAMESPACESETTINGS;
	}
	else
	{
		*p_namespace_creation_id = p_best_cap->device_handle.handle;
		set_block_count_in_settings(p_best_cap, 1, p_best_cap->total_capacity, 0,
			p_settings);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int fit_namespace_to_largest_matching_region(const struct pool *p_pool,
		struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		const struct interleave_format *p_format, NVM_UINT64 minimum_ns_size,
		NVM_UINT32 *p_namespace_creation_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if (p_format == NULL ||
		(p_format->ways != INTERLEAVE_WAYS_1 && p_format->imc != INTERLEAVE_SIZE_NONE))
	{
		if (p_format)
		{
			rc = get_ad_dimms_from_pool(p_pool, p_free_capacities,
					&candidate_dimm_count, 0);
			if (!candidate_dimm_count)
			{
				rc = NVM_ERR_BADNAMESPACESETTINGS;
			}
		}
		if (rc == NVM_SUCCESS)
		{
			rc = select_largest_region(p_free_capacities, candidate_dimm_count,
			p_settings, minimum_ns_size, p_namespace_creation_id);
		}
	}
	else if (p_format->ways == INTERLEAVE_WAYS_1)
	{
		if ((rc = get_ad_dimms_from_pool(p_pool, p_free_capacities,
					&candidate_dimm_count, 1)) == NVM_SUCCESS)
		{
			if (!candidate_dimm_count)
			{
				rc = NVM_ERR_BADNAMESPACESETTINGS;
			}
			else
			{
				rc = select_largest_adx1_region(p_free_capacities,
					candidate_dimm_count, p_settings, minimum_ns_size,
					p_namespace_creation_id);
			}
		}
	}
	else
	{
		rc = select_largest_storage_only_region(p_free_capacities,
			candidate_dimm_count, p_settings, minimum_ns_size,
			p_namespace_creation_id);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int calculate_aligned_storage_capacity(struct namespace_create_settings *p_settings,
		NVM_BOOL allow_adj, NVM_UINT64 *hi_ns_cap, NVM_UINT64 *low_ns_cap)
{
	int rc = NVM_SUCCESS;
	int ways = 1;
	NVM_UINT32 real_block_size = get_real_block_size(p_settings->block_size);
	NVM_UINT64 block_count = p_settings->block_count;
	*hi_ns_cap = p_settings->block_count * real_block_size;
	*low_ns_cap = *hi_ns_cap;

	if (!check_namespace_alignment(*hi_ns_cap, real_block_size, ways))
	{
		NVM_UINT64 alignment_size = get_alignment_size(real_block_size, ways);
		adjust_namespace_block_count_if_allowed(&block_count,
			real_block_size, ways, allow_adj);
		*hi_ns_cap = block_count * real_block_size;
		*low_ns_cap = *hi_ns_cap - alignment_size;
		if (!check_namespace_alignment(*hi_ns_cap, real_block_size, ways))
		{
			COMMON_LOG_ERROR("Requested capacity cannot be aligned.");
			rc = NVM_ERR_BADALIGNMENT;
		}
	}
	return rc;
}

int select_smallest_storage_only_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id,
		NVM_BOOL allow_adj)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	NVM_UINT64 hi_ns_cap = 0;
	NVM_UINT64 low_ns_cap = 0;
	const struct device_free_capacities *p_best_cap = NULL;
	int best_cap_is_hi = 0;

	rc = calculate_aligned_storage_capacity(p_settings, allow_adj, &hi_ns_cap, &low_ns_cap);

	if (rc == NVM_SUCCESS)
	{
		for (int i = 0; i < candidate_dimm_count; i++)
		{
			if (hi_ns_cap >= minimum_ns_size)
			{
				if (p_free_capacities[i].storage_only_capacity >= hi_ns_cap)
				{
					if (!p_best_cap ||
						p_free_capacities[i].storage_only_capacity <
						p_best_cap->storage_only_capacity)
					{
						p_best_cap = &p_free_capacities[i];
						best_cap_is_hi = 1;
					}
				}
				else if (!best_cap_is_hi && low_ns_cap >= minimum_ns_size &&
					p_free_capacities[i].storage_only_capacity >= low_ns_cap)
				{
					if (!p_best_cap ||
						p_free_capacities[i].storage_only_capacity <
						p_best_cap->storage_only_capacity)
					{
						p_best_cap = &p_free_capacities[i];
					}
				}
			}
		}
		if (!p_best_cap)
		{
			rc = NVM_ERR_BADSIZE;
		}
		else
		{
			*p_namespace_creation_id = p_best_cap->device_handle.handle;
			set_block_count_in_settings(p_best_cap, best_cap_is_hi, hi_ns_cap, low_ns_cap,
				p_settings);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int select_smallest_adx1_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id,
		NVM_BOOL allow_adj)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	NVM_UINT64 hi_ns_cap = 0;
	NVM_UINT64 low_ns_cap = 0;
	const struct device_free_capacities *p_best_cap = NULL;
	NVM_UINT64 best_adx1_ns_cap = 0;
	int best_cap_is_hi = 0;

	rc = calculate_aligned_storage_capacity(p_settings, allow_adj, &hi_ns_cap, &low_ns_cap);

	if (rc == NVM_SUCCESS)
	{
		for (int i = 0; i < candidate_dimm_count; i++)
		{
			NVM_UINT64 adx1_ns_cap = p_free_capacities[i].storage_only_capacity +
				p_free_capacities[i].app_direct_byone_capacity;
			if (hi_ns_cap >= minimum_ns_size)
			{
				if (adx1_ns_cap >= hi_ns_cap)
				{
					if (!p_best_cap || adx1_ns_cap < best_adx1_ns_cap)
					{
						p_best_cap = &p_free_capacities[i];
						best_cap_is_hi = 1;
						best_adx1_ns_cap = adx1_ns_cap;
					}
				}
				else if (!best_cap_is_hi && low_ns_cap >= minimum_ns_size &&
					adx1_ns_cap >= low_ns_cap)
				{
					if (!p_best_cap || adx1_ns_cap < best_adx1_ns_cap)
					{
						p_best_cap = &p_free_capacities[i];
						best_adx1_ns_cap = adx1_ns_cap;
					}
				}
			}
		}
		if (!p_best_cap)
		{
			rc = NVM_ERR_BADSIZE;
		}
		else
		{
			*p_namespace_creation_id = p_best_cap->device_handle.handle;
			set_block_count_in_settings(p_best_cap, best_cap_is_hi, hi_ns_cap, low_ns_cap,
				p_settings);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int select_smallest_combined_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id,
		NVM_BOOL allow_adj)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	NVM_UINT64 hi_ns_cap = 0;
	NVM_UINT64 low_ns_cap = 0;
	const struct device_free_capacities *p_best_cap = NULL;
	int best_cap_is_hi = 0;

	rc = calculate_aligned_storage_capacity(p_settings, allow_adj, &hi_ns_cap, &low_ns_cap);

	if (rc == NVM_SUCCESS)
	{
		for (int i = 0; i < candidate_dimm_count; i++)
		{
			if (hi_ns_cap >= minimum_ns_size)
			{
				if (p_free_capacities[i].total_capacity >= hi_ns_cap)
				{
					if (!p_best_cap ||
						p_free_capacities[i].total_capacity <
						p_best_cap->total_capacity)
					{
						p_best_cap = &p_free_capacities[i];
						best_cap_is_hi = 1;
					}
				}
				else if (!best_cap_is_hi && low_ns_cap >= minimum_ns_size &&
					p_free_capacities[i].total_capacity >= low_ns_cap)
				{
					if (!p_best_cap ||
						p_free_capacities[i].total_capacity <
						p_best_cap->total_capacity)
					{
						p_best_cap = &p_free_capacities[i];
					}
				}
			}
		}
		if (!p_best_cap)
		{
			rc = NVM_ERR_BADSIZE;
		}
		else
		{
			*p_namespace_creation_id = p_best_cap->device_handle.handle;
			set_block_count_in_settings(p_best_cap, best_cap_is_hi, hi_ns_cap, low_ns_cap,
				p_settings);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;

}

int select_smallest_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id,
		NVM_BOOL allow_adj)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if ((rc = select_smallest_storage_only_region(p_free_capacities, candidate_dimm_count,
			p_settings, minimum_ns_size, p_namespace_creation_id, allow_adj)) ==
			NVM_ERR_BADSIZE)
	{
		if ((rc = select_smallest_adx1_region(p_free_capacities, candidate_dimm_count,
			p_settings, minimum_ns_size, p_namespace_creation_id, allow_adj)) ==
			NVM_ERR_BADSIZE)
		{
			rc = select_smallest_combined_region(p_free_capacities,
				candidate_dimm_count, p_settings, minimum_ns_size,
				p_namespace_creation_id, allow_adj);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int select_smallest_matching_region(const struct pool *p_pool,
		struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		const struct interleave_format *p_format, NVM_UINT64 minimum_ns_size,
		NVM_UINT32 *p_namespace_creation_id, NVM_BOOL allow_adj)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if (p_format == NULL ||
		(p_format->ways != INTERLEAVE_WAYS_1 && p_format->imc != INTERLEAVE_SIZE_NONE))
	{
		if (p_format)
		{
			rc = get_ad_dimms_from_pool(p_pool, p_free_capacities,
					&candidate_dimm_count, 0);
			if (!candidate_dimm_count)
			{
				rc = NVM_ERR_BADNAMESPACESETTINGS;
			}
		}
		if (rc == NVM_SUCCESS)
		{
			rc = select_smallest_region(p_free_capacities, candidate_dimm_count,
				p_settings, minimum_ns_size, p_namespace_creation_id, allow_adj);
		}
	}
	else if (p_format->ways == INTERLEAVE_WAYS_1)
	{
		if ((rc = get_ad_dimms_from_pool(p_pool, p_free_capacities,
					&candidate_dimm_count, 1)) == NVM_SUCCESS)
		{
			if (!candidate_dimm_count)
			{
				rc = NVM_ERR_BADNAMESPACESETTINGS;
			}
			else
			{
				rc = select_smallest_adx1_region(p_free_capacities,
					candidate_dimm_count, p_settings, minimum_ns_size,
					p_namespace_creation_id, allow_adj);
			}
		}
	}
	else
	{
		rc = select_smallest_storage_only_region(p_free_capacities,
			candidate_dimm_count, p_settings, minimum_ns_size,
			p_namespace_creation_id, allow_adj);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_UINT64 get_dimm_free_ad_byone_capacity(const struct pool *p_pool, const NVM_UID dimm)
{
	NVM_UINT64 capacity = 0;
	for (int i = 0; i < p_pool->ilset_count; i++)
	{
		if ((p_pool->ilsets[i].settings.ways == INTERLEAVE_WAYS_1) &&
			(is_uid_in_list(dimm, p_pool->ilsets[i].dimms, p_pool->ilsets[i].dimm_count)))
		{
			capacity += p_pool->ilsets[i].available_size;
		}
	}
	return capacity;
}

NVM_UINT64 get_dimm_free_ad_capacity(const struct pool *p_pool, const NVM_UID dimm)
{
	NVM_UINT64 capacity = 0;
	for (int i = 0; i < p_pool->ilset_count; i++)
	{
		if ((p_pool->ilsets[i].settings.ways != INTERLEAVE_WAYS_1) &&
			(is_uid_in_list(dimm, p_pool->ilsets[i].dimms, p_pool->ilsets[i].dimm_count)))
		{
			capacity += p_pool->ilsets[i].available_size / p_pool->ilsets[i].dimm_count;
		}
	}
	return capacity;
}

void get_free_capacities_for_devices(const struct pool *p_pool,
		const struct device_discovery *p_discoveries,
		const struct nvm_storage_capacities *p_capacities, NVM_UINT16 dimm_count,
		struct device_free_capacities *p_free_capacities)
{
	COMMON_LOG_ENTRY();
	struct device_free_capacities free_caps;
	for (int i = 0; i < dimm_count; i++)
	{
		memset(&free_caps, 0, sizeof (free_caps));
		free_caps.device_handle = p_discoveries[i].device_handle;
		uid_copy(p_discoveries[i].uid, free_caps.uid);
		for (int j = 0; j < dimm_count; j++)
		{
			if (p_capacities[j].device_handle.handle ==
					p_discoveries[i].device_handle.handle)
			{
				free_caps.app_direct_byone_capacity =
					get_dimm_free_ad_byone_capacity(p_pool,
									p_discoveries[i].uid);
				free_caps.app_direct_interleaved_capacity =
					get_dimm_free_ad_capacity(p_pool, p_discoveries[i].uid);
				free_caps.total_capacity = p_capacities[j].free_storage_capacity;
				if (p_capacities[j].free_storage_capacity >
						free_caps.app_direct_byone_capacity +
						free_caps.app_direct_interleaved_capacity)
				{
					free_caps.storage_only_capacity =
						p_capacities[j].free_storage_capacity -
						free_caps.app_direct_byone_capacity -
						free_caps.app_direct_interleaved_capacity;
				}
				break;
			}
		}
		memmove(&p_free_capacities[i], &free_caps, sizeof (free_caps));
	}
	COMMON_LOG_EXIT();
}
