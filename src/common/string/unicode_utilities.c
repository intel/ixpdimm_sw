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
 */

#ifdef __WINDOWS__
#include <string/unicode_utilities.h>
#include <errno.h>
#include <windows.h>

/*
 *  Convert a UTF-16 encoded wchar_t array to a UTF-8 encoded char array.
 */
int wchar_to_utf8(char *dst, size_t dst_bytes, const wchar_t *src, int src_wchars)
{
	int ret = -1;

	// src must exist, as well as a valid value indicating the number of characters to process
	// dst must exist if we wish to write data to it
	if ((src != NULL) && ((src_wchars == -1) || (src_wchars > 0)) &&
			((dst_bytes == 0) || (dst != NULL)))
	{
		ret = WideCharToMultiByte(
				(UINT)CP_UTF8,	// convert to UTF-8
				0,				// use default conversion type (fastest)
				(LPCWSTR)src,	// source UTF-16 encoded wchar_t array
				src_wchars,		// number of src characters to convert
				(LPSTR)dst,		// destination UTF-8 encoded char array
				(int)dst_bytes,	// size (in bytes) of the destination array
				NULL,			// do not use default replacement char
				NULL);			// do not use default replacement char
	}

	return ret;
}

/*
 *  Convert a UTF-8 encoded char array to a UTF-16 encoded wchar_t array.
 */
int utf8_to_wchar(wchar_t *dst, size_t dst_wchars, const char *src, int src_bytes)
{
	int ret = -1;

	// src must exist, as well as a valid value indicating the number of bytes to process
	// dst must exist if we wish to write data to it
	if ((src != NULL) && ((src_bytes == -1) || (src_bytes > 0)) &&
			((dst_wchars == 0) || (dst != NULL)))
	{
		ret = MultiByteToWideChar(
				(UINT)CP_UTF8,		// convert from UTF-8
				0,					// use default conversion type (fastest)
				(LPCSTR)src,		// source UTF-8 encoded char array
				src_bytes,			// number of src bytes to convert
				(LPWSTR)dst,		// destination UTF-16 encoded wchar_t array
				(int)dst_wchars);	// size (in wchar_t characters) of the destination array
	}

	return ret;
}
#endif
