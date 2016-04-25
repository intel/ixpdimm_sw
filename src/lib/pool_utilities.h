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

#include "device_adapter.h"
#include "nvm_types.h"


/*
 * Retrieve the number of pools.
 */
int get_pool_count();

/*
 * Retrieve a list of pools.
 */
int get_pools(const NVM_UINT32 count, struct nvm_pool *p_pools);

/*
 * Get if the system has a memory pool configured
 * @return
 * 		1 if has memory pool, 0 if not, or an error code
 */
int has_memory_pool();

/*
 * If a system is configured to have memory memory, setup the memory pool.
 * @return
 * 		NVM_SUCCESS or appropriate error code
 */
int get_memory_pool(struct nvm_pool *p_pool);

/*
 * Get the number of mirrored pools on the system
 * @return
 * 		Number of mirrored pools or an error code
 */
int get_mirrored_pools_count();

/*
 * Get the mirrored pools on the system
 * @return
 * 		Number of mirrored pools or an error code
 */
int get_mirrored_pools(struct nvm_pool pools[], const NVM_UINT32 count);

/*
 * Get the number of persistent pools on the system
 * @return
 * 		Number of persistent pools or an error code
 */
int get_persistent_pools_count();

/*
 * Get the persistent pools on the system
 * @return
 * 		Number of persistent pools or an error code
 */
int get_persistent_pools(struct nvm_pool pools[], const NVM_UINT32 count);

/*
 * Helper function to copy one set of pools to another. Handles the case when
 * caller doesn't want to copy the pools, but just wants to know the number of source
 * pools. Also handles the case if the caller passes an invalid destination array size
 *
 * Return the number of source pools, or an error code.
 */
int copy_pools(struct nvm_pool dst_pools[], const NVM_UINT32 dst_count,
		struct nvm_pool src_pools[], const NVM_UINT32 src_count);

/*
 * Add a DIMM to the pool structure.
 */
int add_dimm_to_pool(struct nvm_pool *p_pool, NVM_NFIT_DEVICE_HANDLE handle);

/*
 * Add a DIMM to the relevant pool. If it doesn't exist, create it.
 */
int add_dimm_to_pools(struct nvm_pool *p_pools, int *p_pools_count, NVM_NFIT_DEVICE_HANDLE handle,
		enum pool_type type, char *uuid_src_prefix);

/*
 * Add an interleave set to the pool structure.
 */
int add_ilset_to_pool(struct nvm_pool *p_pool, struct nvm_interleave_set *p_ilset);

/*
 * Add an interleave set to the relevant pool. If it doesn't exist, create it.
 */
int add_ilset_to_pools(struct nvm_pool *p_pools, int *p_pools_count,
	struct nvm_interleave_set *p_ilset,	enum pool_type type, char *uuid_src_prefix);

/*
 * Get the capacity of a DIMM occupied by interleave sets
 */
int get_dimm_ilset_capacity(NVM_NFIT_DEVICE_HANDLE handle, NVM_UINT64 *p_mirrored_size,
		NVM_UINT64 *p_unmirrored_size);

/*
 * Get the Memory Mode capacity of a DIMM
 */
int get_dimm_memory_capacity(NVM_NFIT_DEVICE_HANDLE handle, NVM_UINT64 *p_size);

/*
 * Initialize a new pool with its UUID and type
 */
int init_pool(struct nvm_pool *p_pool, const char *uuid_src, const enum pool_type type,
		const int socket);

/*
 * Determine the security status for an interleave set.
 */
int calculate_app_direct_interleave_security(NVM_UINT32 interleave_setid,
	enum encryption_status *p_encryption,
	enum erase_capable_status *p_erase_capable,
	enum encryption_status *p_encryption_capable /* could be NULL */);

/*
 * Get an nvm_interleave set by the driver identifier
 */
int get_interleaveset_by_driver_id(NVM_UINT32 driver_id,
		struct nvm_interleave_set *p_interleave);

int get_pool_uid_from_namespace_details(
		const struct nvm_namespace_details *p_details, NVM_UID *p_pool_uid);

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
		struct nvm_interleave_set *p_interleave_set,
		const NVM_NFIT_DEVICE_HANDLE handle,
		const NVM_UINT64 interleave_dpa_offset);

#ifdef __cplusplus
}
#endif

#endif /* POOL_UTILITIES_H_ */
