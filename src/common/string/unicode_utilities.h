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
 * This file contains the implementation of various functions to manipulate
 * unicode strings (primarily used for Windows).
 * Windows expects wchar_t* (UTF-16) while Linux (and internally to the code)
 * UTF8 is used and stored in char*
 */

#ifndef	_UNICODE_UTILITIES_H_
#define	_UNICODE_UTILITIES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __WINDOWS__
#include <string.h>

/*!
 * Convert a UTF-16 encoded @b wchar_t array to a UTF-8 encoded @b char array.
 * @remarks
 *		Standard use is a two-step process.  First, use this function to determine
 *		the @b char array size required for UTF-8 conversion, without performing the
 *		conversion. (set @c dst_bytes = 0)  The second step is then to perform the
 *		actual conversion, with the correctly sized destination array.
 * @param[out] dst
 * 		Pointer to the target UTF-8 encoded @b char array.
 * @param[in] dst_bytes
 * 		Size (in bytes) of @c dst.  If set to 0, no conversion will be performed
 *		while the required buffer size (in bytes) will be returned.
 * @param[in] src
 * 		Pointer to the target UTF-16 encoded @b wchar_t array, which need not be null
 * 		terminated.
 * @param[in] src_wchars
 * 		The number of @b wchar_t characters to be converted in the UTF-16 @c src
 * 		string.  This may optionally include a null terminator.  If set to -1, @c src
 * 		is assumed to be a null terminated string, and will convert all characters
 * 		including the null terminator.  If set to 0, this function will fail with
 * 		no conversion attempted.
 * @return
 * 		The number of bytes to be written to @c dst if successful. @n
 * 		-1 if an invalid combination of input arguments is detected. @n
 * 		0 if the conversion failed.  Use GetLastError() to get extended error info.
 */
extern int wchar_to_utf8(char *dst, size_t dst_bytes, const wchar_t *src, int src_wchars);

/*!
 * Convert a UTF-8 encoded @b char array to a UTF-16 encoded @b wchar_t array.
 * @remarks
 * 		Standard use is a two-step process.  First, use this function to determine
 * 		the @b wchar_t array size required for UTF-16 conversion, without performing
 * 		the conversion. (set @c dst_bytes = 0)  The second step is then to perform
 * 		the actual conversion, with the correctly sized destination array.
 * @param[out] dst
 *		Pointer to the target UTF-16 encoded @b wchar_t array.  All non UTF-8 characters
 *		will get converted to xFFFD
 * @param[in] dst_wchars
 * 		Size (in @b wchar_t) of @c dst. If set to 0, no conversion will be performed
 *			while the required buffer size (in @b wchar_t) will be returned.
 * @param[in] src
 * 		Pointer to the target UTF-8 encoded @b char array, which need not be null
 * 		terminated.
 * @param[in] src_bytes
 * 		The number of bytes to be converted in the UTF-8 @c src string.  This may
 * 		optionally include a null terminator.  If set to -1, @c src is assumed to be a
 *		null terminated string, and will convert all characters including the null
 *		terminator.  If set to 0, this function will fail with no conversion attempted.
 * @return
 * 		The number of @b wchar_t characters to be written to @c dst if successful. @n
 * 		-1 if an invalid combination of input arguments is detected. @n
 * 		0 if the conversion failed.  Use GetLastError() to get extended error info.
 */
extern int utf8_to_wchar(wchar_t *dst, size_t dst_wchars, const char *src, int src_bytes);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _UNICODE_UTILITIES_H_ */
