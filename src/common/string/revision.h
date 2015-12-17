/*
 * Copyright (c) 2015, Intel Corporation
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
 * This file contains the definition of common revision helper functions.
 */

#ifndef REVISION_H_
#define	REVISION_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <common_types.h>
#include "s_str.h"

/*!
 * Correctly formats the version numbers in a revision string
 * @remarks
 * 		An example of the correct format is @a 01.02.03.4567
 * @param[in] rev
 * 		The incorrectly formatted revision string
 * @param[in] rev_len
 * 		The size of the buffer allocated for @c rev
 * @param[out] fmt_rev
 * 		The correctly formatted revision string
 * @param[in] fmt_rev_len
 * 		The size of the buffer allocated for @c fmt_rev
 * @return
 * 		1 if successfully parsed, 0 if failed
 */
extern int format_revision(const char *rev, size_t rev_len, char *fmt_rev, size_t fmt_rev_len);

/*!
 * Correctly formats the version numbers in a firmware revision string
 * @param[in] p_firmware_rev
 *		A string containing an improperly formatted fw revision
 * @param[in] fw_rev_len
 * 		The size of the buffer allocated for @c p_firmware_rev
 * @param[out] fmt_rev
 * 		The formatted firmware revision number.
 * @param[in] fmt_rev_len
 * 		The size of the buffer allocated for @c fmt_rev
 * @return
 * 		1 if successfully parsed, 0 if failed
 */
extern int format_fw_revision(const char *p_firmware_rev, size_t fw_rev_len, char *fmt_rev,
		size_t fmt_rev_len);

/*!
 * Parse a revision string into it's appropriate parts (major, minor, and build numbers)
 * @remarks
 * 		Even on failure, this function will parse what it can.
 * 		If a part is not found than that part will be 0.
 * @param[out] p_major
 * 		Major version number
 * @param[out] p_minor
 * 		Minor version number
 * @param[out] p_hotfix
 * 		Hotfix number
 * @param[out] p_build
 * 		Build number
 * @param[in] revision
 * 		The revision string to be parsed.
 * @param[in] revision_len
 * 		The size of the buffer allocated for @c revision
 * @return
 * 		1 if successfully parsed, 0 if failed
 */
extern int parse_main_revision(unsigned short int *p_major, unsigned short int *p_minor,
		unsigned short int *p_hotfix, unsigned short int *p_build, const char *const revision,
		size_t revision_len);

/*!
 * Parse a firmware revision string into it's appropriate parts (major & minor numbers)
 * @remarks
 * 		Even on failure, this function will parse what it can.
 * 		If a part is not found than that part will be 0.
 * @param[out] p_major
 * 		Major version number
 * @param[out] p_minor
 * 		Minor version number
 * @param[in] fw_revision
 * 		The firmware revision string to be parsed.
 * @param[in] fw_revision_len
 * 		The size of the buffer allocated for @c fw_revision
 * @return
 * 		1 if successfully parsed, 0 if failed
 */
extern int parse_fw_revision(unsigned short int *p_major, unsigned short int *p_minor,
		const char *fw_revision, size_t fw_revision_len);

/*!
 * Utility method to create a version string from its parts
 * @param[out] revision
 * 		The revision string to be generated.
 * @param[in] revision_len
 * 		The size of the buffer allocated for @c revision
 * @param[in] major
 * 		Major version number
 * @param[in] minor
 * 		Minor version number
 * @param[in] hotfix
 * 		Hotfix number
 * @param[in] build
 * 		Build number
 */
extern void build_revision(char *revision, size_t revision_len,
		unsigned short int major, unsigned short int minor,
		unsigned short int hotfix, unsigned short int build);

/*!
 * Utility method to create firmware version string from parts
 * @param[out] fw_revision
 * 		The firmware revision string to be generated.
 * @param[in] fw_revision_len
 * 		The size of the buffer allocated for @c fw_revision
 * @param[in] major
 * 		Major version number
 * @param[in] minor
 * 		Minor version number
 */
extern void build_fw_revision(char *fw_revision, size_t fw_revision_len,
		unsigned short int major, unsigned short int minor);


#ifdef __cplusplus
}
#endif

#endif /* REVISION_H_ */
