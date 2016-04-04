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
 * This file describes methods used to translate BIOS Platform Configuration Data
 * to a database that may then be loaded as a simulator.
 */

#ifndef PLATFORM_CONFIG_DATA_DB_H_
#define	PLATFORM_CONFIG_DATA_DB_H_

#include <persistence/schema.h>
#include "platform_config_data.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Get the size of the platform config data by counting the tables in the database
 */
int get_dimm_platform_config_size_from_db(PersistentStore *p_db, const unsigned int device_handle);

/*
 * Get the the platform config data from the database
 */
int get_dimm_platform_config_data_from_db(PersistentStore *p_db, const unsigned int device_handle,
		struct platform_config_data *p_config_data, const NVM_UINT32 size);

/*
 * Write the platform config data back to the specified database
 */
int update_dimm_platform_config_in_db(PersistentStore *p_db,
		const unsigned int device_handle, struct platform_config_data *p_config_data);

/*
 * Clear out existing platform config data in the database for this dimm
 */
int clear_dimm_platform_config_from_db(PersistentStore *p_db, const unsigned int device_handle);



#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_CONFIG_DATA_DB_H_ */
