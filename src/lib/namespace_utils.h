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

#include "nvm_management.h"
#include "adapter_types.h"

/*
 * Device persistent capacities that are available for namespace creation.
 * Calculated from free storage capacity and interleave information.
 */
struct device_free_capacities
{
	NVM_NFIT_DEVICE_HANDLE device_handle; // The unique device handle of the memory module
	NVM_UID uid;
	NVM_UINT64 total_capacity;
	NVM_UINT64 app_direct_byone_capacity;
	NVM_UINT64 app_direct_interleaved_capacity;
	NVM_UINT64 storage_only_capacity;
};

void adjust_namespace_block_count_if_allowed(NVM_UINT64 *p_block_count,
		const NVM_UINT16 block_size,
		NVM_UINT8 ways, const NVM_BOOL allow_adjustment);

NVM_UINT32 get_alignment_size(NVM_UINT32 block_size, NVM_UINT32 ways);

NVM_BOOL check_namespace_alignment(NVM_UINT64 capacity, NVM_UINT32 block_size, NVM_UINT8 ways);

int get_ad_dimms_from_pool(const struct pool *p_pool,
		struct device_free_capacities *ad_caps,
		NVM_UINT16 *ad_cap_count, int by_one);

NVM_UINT64 capacity_to_real_block_count(NVM_UINT64 capacity, NVM_UINT32 block_size);

int select_largest_storage_only_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id);

int select_largest_adx1_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id);

int select_largest_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id);

int fit_namespace_to_largest_matching_region(const struct pool *p_pool,
		struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		const struct interleave_format *p_format, NVM_UINT64 minimum_ns_size,
		NVM_UINT32 *p_namespace_creation_id);

int calculate_aligned_storage_capacity(struct namespace_create_settings *p_settings,
		NVM_BOOL allow_adj, NVM_UINT64 *hi_ns_cap, NVM_UINT64 *low_ns_cap);

int select_smallest_storage_only_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id,
		NVM_BOOL allow_adj);

int select_smallest_adx1_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id,
		NVM_BOOL allow_adj);

int select_smallest_combined_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id,
		NVM_BOOL allow_adj);

int select_smallest_region(const struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		NVM_UINT64 minimum_ns_size, NVM_UINT32 *p_namespace_creation_id,
		NVM_BOOL allow_adj);

int select_smallest_matching_region(const struct pool *p_pool,
		struct device_free_capacities *p_free_capacities,
		NVM_UINT16 candidate_dimm_count, struct namespace_create_settings *p_settings,
		const struct interleave_format *p_format, NVM_UINT64 minimum_ns_size,
		NVM_UINT32 *p_namespace_creation_id, NVM_BOOL allow_adj);

NVM_UINT64 get_dimm_free_ad_byone_capacity(const struct pool *p_pool, const NVM_UID dimm);

NVM_UINT64 get_dimm_free_ad_capacity(const struct pool *p_pool, const NVM_UID dimm);

void get_free_capacities_for_devices(const struct pool *p_pool,
		const struct device_discovery *p_discoveries,
		const struct nvm_storage_capacities *p_capacities, NVM_UINT16 dimm_count,
		struct device_free_capacities *p_free_capacities);