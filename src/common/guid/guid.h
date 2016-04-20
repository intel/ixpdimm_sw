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
 * This file contains the definitions of helper functions for
 * GUID creation and generation.
 */

#ifndef GUID_H_
#define	GUID_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <common_types.h>

/*!
 * A helper function for guid_to_str to convert an integer
 * to a 2 char hex string.
 * @param[in] value
 * 		The unsigned @b char to be converted.
 * @param[out] destination
 * 		The character array indicating the equivalent hex value.
 */
void int_to_hex_str(unsigned char value, char *destination);

/*!
 * Convert a GUID to a string
 * @param[in] guid
 * 		The GUID to be converted into a string
 * @param[out] guid_str
 * 		The converted GUID string
 */
extern void guid_to_str(const COMMON_GUID guid, COMMON_GUID_STR guid_str);

/*!
 * Convert a string to a GUID
 * @param[in] guid_str
 * 		The GUID string to convert to a GUID
 * @param[out] guid
 * 		The converted GUID
 */
extern void str_to_guid(const COMMON_GUID_STR guid_str, COMMON_GUID guid);

/*!
 * Compare two GUIDs for equality
 * @param[in] guid1
 *		The first GUID to be compared
 * @param[in] guid2
 *		The second GUID to be compared
 * @return
 * 		1 if equivalent, @n
 * 		0 if different, @n
 * 		-1 if either argument is @b NULL
 */
extern int guid_cmp(const COMMON_GUID guid1, const COMMON_GUID guid2);

/*!
 * Generate a random GUID
 * @param[out] guid
 * 		The GUID to be generated
 */
extern void generate_guid(COMMON_GUID guid);

/*!
 * Generate a random GUID string
 * @param[out] guid_str
 * 		The GUID string to be generated
 */
extern void generate_guid_str(COMMON_GUID_STR guid_str);

/*!
 * Create hash guid from specified input string
 * @param[in] source
 * 		string to use for guid hash creation
 * @param[in] source_len
 * 		length of input source string
 * @param[out]
 * 		newly created guid
 * @return
 * 		1 if succeeded in creating guid
 * 		0 if failed to create guid
 */
int guid_hash(const unsigned char *source,
		const size_t source_len, unsigned char *p_guid);

/*
 * Create hash guid represented by char array
 */
extern int guid_hash_str(const unsigned char *source, const size_t source_len, char *p_guid);

#ifdef __cplusplus
}
#endif

#endif /* GUID_H_ */
