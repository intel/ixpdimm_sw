/*
 * Copyright (c) 2017, Intel Corporation
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
 * This file defines helper functions for
 * reading/writing the BIOS platform capabilities table (PCAT).
 */

#ifndef PLATFORM_CAPABILITIES_DB_H_
#define	PLATFORM_CAPABILITIES_DB_H_

#include <persistence/schema.h>
#include "nvm_types.h"
#include <acpi/acpi.h>
#include "platform_capabilities.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Helper function to retrieve pcat from db
 */
int get_pcat_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		const NVM_UINT32 cap_len);

/*
 * Helper function to get the PCAT memory interleave extension tables
 * from the database and copy the data to the bios_capabilities structure
 */
int get_pcat_interleave_tables_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		NVM_UINT32 *p_offset, const NVM_UINT32 cap_len);

/*
 * Helper function to get the PCAT platform capabilities extension table
 * from the database and copy the data to the bios_capabilities struct
 */
int get_pcat_platform_info_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		NVM_UINT32 *p_offset, const NVM_UINT32 cap_len);

/*
 * Helper function to get the PCAT runtime configuration extension table
 * from the database and copy the data to the bios_capabilities struct
 */
int get_pcat_runtime_validation_table_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		NVM_UINT32 *p_offset, const NVM_UINT32 cap_len);

/*
 * Update the platform capabilities data stored in the db
 * Tries to store as much data as possible while propagating any errors.
 */
int update_pcat_in_db(PersistentStore *p_db,
		const struct bios_capabilities *p_capabilities,
		const int history_id);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_CAPABILITIES_DB_H_ */
