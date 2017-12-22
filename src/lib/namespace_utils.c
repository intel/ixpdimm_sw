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
#include "device_utilities.h"

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

int get_nvm_namespaces_details_alloc(struct nvm_namespace_details **pp_namespaces)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	int ns_count = get_namespace_count();
	if (ns_count > 0)
	{
		struct nvm_namespace_discovery *namespaces = malloc(ns_count * sizeof(struct nvm_namespace_discovery));
		memset(namespaces, 0, sizeof (struct nvm_namespace_discovery) * ns_count);
		if (get_namespaces(ns_count, namespaces) == ns_count)
		{
			*pp_namespaces = calloc(1, sizeof (struct nvm_namespace_details) * ns_count);
			if (!(*pp_namespaces))
			{
				COMMON_LOG_ERROR("Failed to allocate memory to gather namespaces");
				rc = NVM_ERR_NOMEMORY;
			}
			for (int i = 0; i < ns_count; i++)
			{
				int tmp_rc = get_namespace_details(namespaces[i].namespace_uid,
						&(*pp_namespaces)[i]);
				if (tmp_rc != NVM_SUCCESS)
				{
					COMMON_LOG_ERROR("Failed to retrieve current namespaces");
					free(*pp_namespaces);
					rc = tmp_rc;
					break;
				}
			}
		}

        free(namespaces);
	}

	if (rc == NVM_SUCCESS)
	{
		rc = ns_count;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
