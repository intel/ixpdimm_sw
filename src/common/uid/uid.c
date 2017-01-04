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

#include <stdlib.h>
#include <string.h>
#include <guid/guid.h>
#include <string/s_str.h>

#include "uid.h"


void uid_copy(const char *src, COMMON_UID dst)
{
	if (src != NULL && dst != NULL)
	{
		s_strcpy(dst, src, COMMON_UID_LEN);
	}
}

/*
 * Convert binary guid to a string.
 * Note requirement that caller supply buffer of appropriate size.
 */
void guid_to_uid(const COMMON_GUID guid, COMMON_UID uid)
{
	guid_to_str(guid, uid);
}

/*
 * Check if two guids are equal
 */
int uid_cmp(const COMMON_UID guid1, const COMMON_UID guid2)
{
	if (strncmp(guid1, guid2, COMMON_UID_LEN) == 0)
		return 1;
	else
		return 0;
}

int get_uid_index(const COMMON_UID uid, const COMMON_UID *uid_list,
		const COMMON_UINT16 uid_list_len)
{
	int rc = -1;
	for (int i = 0; i < uid_list_len; i++)
	{
		if (uid_cmp(uid, uid_list[i]))
		{
			rc = i;
			break;
		}
	}
	return rc;
}

COMMON_BOOL is_uid_in_list(const COMMON_UID uid, const COMMON_UID *uid_list,
		const COMMON_UINT16 uid_list_len)
{
	return (get_uid_index(uid, uid_list, uid_list_len) >= 0);
}
