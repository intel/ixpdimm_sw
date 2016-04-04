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
 * Common functions to translate data from the SMBIOS table into NVM
 * structures and values.
 */

#ifndef SRC_LIB_SMBIOS_UTILITIES_H_
#define	SRC_LIB_SMBIOS_UTILITIES_H_

#include <smbios/smbios_types.h>
#include <common_types.h>
#include "device_adapter.h"

/*
 * Get the SMBIOS physical ID of a DIMM with a given device handle.
 */
int get_dimm_physical_id_from_handle(const NVM_NFIT_DEVICE_HANDLE device_handle);

/*
 * Get DIMM details for a specific DIMM physical ID from the SMBIOS table.
 */
int get_dimm_details_for_physical_id(const NVM_UINT16 physical_id,
		struct nvm_details *p_dimm_details);

/*
 * Get DIMM details for a specific DIMM physical ID from the SMBIOS table.
 */
int get_dimm_details_from_smbios_table(const NVM_UINT8 *p_smbios_table,
		const size_t smbios_data_length,
		const NVM_UINT16 physical_id, struct nvm_details *p_dimm_details);

/*
 * Get Type 17 memory type for a specific DIMM physical ID from an SMBIOS table.
 * Returns the type or error if not found.
 */
int get_device_memory_type_from_smbios_table(const NVM_UINT8 *p_smbios_table,
		const size_t smbios_data_length,
		const NVM_UINT16 physical_id);

/*
 * Copy from SMBIOS table to an nvm_details array.
 * Caller is responsible for null-checking inputs.
 * Returns count if successful, NVM error code if not.
 */
int smbios_table_to_nvm_details_array(const NVM_UINT8 *p_smbios_table,
		const size_t smbios_data_length,
		struct nvm_details *p_details, const size_t num_details);

/*
 * Copy from SMBIOS Type 17 structure to nvm_details.
 * Caller is responsible for null-checking inputs.
 */
void smbios_memory_device_to_nvm_details(const struct smbios_memory_device *p_smbios_dev,
		const size_t smbios_data_length,
		struct nvm_details *p_details);

/*
 * Returns the number of Memory Device structures in the table that are populated
 * with DIMMs (not empty slots)
 */
int smbios_get_populated_memory_device_count(const NVM_UINT8 *p_smbios_table,
		const size_t smbios_data_length);

/*
 * Get the size out of the SMBIOS Type 17 structure and convert it to bytes.
 * Caller is responsible for null-checking input.
 */
COMMON_UINT64 smbios_get_memory_device_size_in_bytes(
		const struct smbios_memory_device *p_smbios_dev);

#endif /* SRC_LIB_SMBIOS_UTILITIES_H_ */
