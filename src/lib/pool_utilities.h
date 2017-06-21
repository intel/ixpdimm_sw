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
 * This file defines the interface to shared helper functions for
 * converting interleave sets to pools.
 *
 * The drivers don't provide pools so they must be constructed.
 * The following understanding of pools is used.
 * Pools:
 * 		All platform memory capacity forms a single memory pool.
 * 		All mirrored interleave sets on a socket are included in one pool.
 * 		All remaining PM space on a socket is included in one pool. This can include non-mirrored
 * 		interleave sets as well as storage only capacity.
 */

#ifndef POOL_UTILITIES_H_
#define	POOL_UTILITIES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "adapter_types.h"
#include "nvm_types.h"
#include "platform_config_data.h"
#include "nfit_tables.h"

#define	MIRRORED_INTERLEAVE(attributes) \
		(attributes & NFIT_MAPPING_ATTRIBUTE_EFI_MEMORY_MORE_RELIABLE) \
		== NFIT_MAPPING_ATTRIBUTE_EFI_MEMORY_MORE_RELIABLE

#define	REDUCE_CAPACITY(current, less) \
	if (less >= current) \
		current = 0; \
	else \
		current -= less;

/*
 * Describes the features pertaining to the unique construction and usage of a
 * pool of memory.
 */
struct nvm_pool
{
	NVM_UID pool_uid; // Unique identifier of the pool.
	enum pool_type type; // The type of pool.
	NVM_UINT64 capacity; // Size of the pool in bytes.
	NVM_UINT64 free_capacity; // Available size of the pool in bytes.
	NVM_INT16 socket_id; // The processor socket identifier.  -1 for system level pool.
	NVM_UINT8 dimm_count; // number of dimms in pool
	NVM_UINT8 ilset_count; // the number of interleave sets in the pool
	// The memory capacities of the dimms in the pool
	NVM_UINT64 memory_capacities[NVM_MAX_DEVICES_PER_POOL];
	// The raw capacities of the dimms in the pool in bytes
	NVM_UINT64 raw_capacities[NVM_MAX_DEVICES_PER_POOL];
	NVM_NFIT_DEVICE_HANDLE dimms[NVM_MAX_DEVICES_PER_POOL]; // Unique ID's of underlying NVM-DIMMs.
	// The interleave sets in this pool
	struct nvm_interleave_set ilsets[NVM_MAX_DEVICES_PER_SOCKET * 2];
	enum pool_health health; // The overall health of the pool
};

/*
 * Device persistent capacities that are available for namespace creation.
 * Calculated from free storage capacity and interleave information.
 */
struct device_free_capacities
{
	NVM_NFIT_DEVICE_HANDLE device_handle; // The unique device handle of the memory module
	NVM_UID uid;
	NVM_UINT64 app_direct_byone_capacity; // total free x1 AD capacity
	NVM_UINT64 app_direct_interleaved_capacity; // total free (not mirrored) AD capacity
	NVM_UINT64 app_direct_mirrored_capacity; // total free mirrored AD capacity
};

/*
 * Get the Memory Mode capacity of a DIMM
 */
int get_dimm_memory_capacity(NVM_NFIT_DEVICE_HANDLE handle, NVM_UINT64 *p_size);

/*
 * Initialize a new pool with its UUID and type
 */
int init_pool(struct pool *p_pool, const char *host_name,
		const enum pool_type type, const int socket);

/*
 * Get an nvm_interleave set by the driver identifier
 */
int get_interleaveset_by_driver_id(NVM_UINT32 driver_id,
		struct nvm_interleave_set *p_interleave);

int get_pool_from_namespace_details(
		const struct nvm_namespace_details *p_details, struct pool *p_pool);

/*
 * Converts DPA offset (from start of DIMM) to offset from the start of the PM partition.
 * Drivers typically have offset as DPA, Platform Config Data
 * has it as offset from PM partition start.
 */
int get_interleave_set_offset_from_pm_partition_start(
		const NVM_NFIT_DEVICE_HANDLE device_handle,
		const NVM_UINT64 offset_from_dimm_start,
		NVM_UINT64* p_offset_from_partition_start);

/*
 * Fills out the interleave_format, mirroring, and PCD interleave set ID with data queried
 * from a DIMM in the interleave set. The DPA offset is used to
 * identify the specific interleave set on the DIMM.
 */
int fill_interleave_set_settings_and_id_from_dimm(
		struct interleave_set *p_interleave_set,
		const NVM_NFIT_DEVICE_HANDLE handle,
		const NVM_UINT64 interleave_dpa_offset);

/*
 * Determines if a DIMM with a given UID is in a given interleave set.
 */
NVM_BOOL dimm_is_in_interleave_set(const NVM_UID device_uid,
		const struct interleave_set *p_ilset);

int get_dimm_free_capacities(
		const struct nvm_capabilities *p_nvm_caps,
		const struct device_discovery *p_discovery,
		const struct pool *p_pool,
		const struct nvm_namespace_details *p_namespaces,
		const int ns_count,
		struct device_capacities *p_capacity,
		struct device_free_capacities *p_free_capacity);

NVM_BOOL interleave_set_has_namespace(const NVM_UINT32 interleave_set_driver_id,
	const struct nvm_namespace_details *p_namespaces, int ns_count);

#ifdef __cplusplus
}
#endif

#endif /* POOL_UTILITIES_H_ */
