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
 * This file contains the definition of common functions to crawl raw
 * SMBIOS tables. Structures and values come from the DMTF SMBIOS specification.
 */

#ifndef SRC_COMMON_SMBIOS_H_
#define SRC_COMMON_SMBIOS_H_

#include <common_types.h>
#include "smbios_types.h"

/*
 * Returns a pointer to the next structure in the table after p_current, NULL if none.
 * p_remaining_length must contain the length of the SMBIOS table from p_current to the end of the buffer.
 * Updates p_remaining_length with length from the returned pointer to the end of the buffer.
 */
const struct smbios_structure_header *smbios_get_next_structure(const struct smbios_structure_header* p_current,
		size_t* p_remaining_length);

/*
 * Returns a pointer to the first structure of type in the table starting with p_current, NULL if not found.
 * p_remaining_length must contain the length of the SMBIOS table from p_current to the end of the buffer.
 * Updates p_remaining_length with length from the returned pointer to the end of the buffer.
 */
const struct smbios_structure_header *smbios_get_first_structure_of_type(const enum smbios_structure_type type,
		const struct smbios_structure_header *p_current, size_t *p_remaining_length);

/*
 * Returns a pointer to the next structure of type in the table after p_current, NULL if not found.
 * p_remaining_length must contain the length of the SMBIOS table from p_current to the end of the buffer.
 * Updates p_remaining_length with length from the returned pointer to the end of the buffer.
 */
const struct smbios_structure_header *smbios_get_next_structure_of_type(const enum smbios_structure_type type,
		const struct smbios_structure_header *p_current, size_t *p_remaining_length);

/*
 * Returns a pointer to the structure with a given handle in the table (starting with the structure at p_top),
 * NULL if not found.
 * p_remaining_length must contain the length of the SMBIOS table from p_top to the end of the buffer.
 * Updates p_remaining_length with length from the returned pointer to the end of the buffer.
 */
const struct smbios_structure_header *smbios_get_structure_with_handle(const COMMON_UINT16 handle,
		const struct smbios_structure_header *p_top, size_t *p_remaining_length);

/*
 * Returns a pointer to the string with a given number associated with the SMBIOS structure, NULL if not found.
 * Per the SMBIOS spec, the string number is expected to be non-zero.
 */
const char *smbios_get_structure_string(const struct smbios_structure_header *p_structure, const size_t data_length,
		const COMMON_UINT32 string_number);

/*
 * Fetch an SMBIOS structure string by number and copy it into a buffer
 */
void smbios_copy_structure_string_to_buffer(const struct smbios_structure_header *p_structure, const size_t data_length,
		const COMMON_UINT32 string_number,
		char *p_buf, const size_t buf_length);

/*
 * Returns the number of structures of a given type in the table
 */
int smbios_get_structure_count_of_type(const enum smbios_structure_type type,
		const struct smbios_structure_header *p_top, const size_t data_length);

#endif /* SRC_COMMON_SMBIOS_H_ */
