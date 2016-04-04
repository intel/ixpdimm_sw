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
 * This file contains the Windows implementation of the file_ops_adapter.h
 * system call wrappers.
 */


#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <sys/locking.h>
#include "file_ops_adapter.h"
#include <string/s_str.h>
#include <string/unicode_utilities.h>
#include "Shlwapi.h"

/*
 * Create an empty file, this function will fail is the file already exits
 */
int create_trunc_file(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	int rc = 0;

	// safe file name
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

	COMMON_WPATH w_file_path;
	utf8_to_wchar(w_file_path, (size_t)COMMON_PATH_LEN, file_path, (int)COMMON_PATH_LEN);

	HANDLE file_handle = CreateFileW(
			w_file_path,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	// simple implementation says that we are creating only, but not leaving it open
	if (file_handle)
	{
		CloseHandle(file_handle);
		rc = 1;
	}

	return rc;
}

/*
 * Create a file
 */
int create_file(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	int rc = 0;

	// safe file name
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

	COMMON_WPATH w_file_path;
	utf8_to_wchar(w_file_path, (size_t)COMMON_PATH_LEN, file_path, (int)COMMON_PATH_LEN);

	HANDLE file_handle = CreateFileW(
			w_file_path,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	// simple implementation says that we are creating only, but not leaving it open
	if (file_handle)
	{
		CloseHandle(file_handle);
		rc = 1;
	}

	return rc;
}

/*
 * Create a directory
 */
int create_dir(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	// safe file name
	COMMON_PATH dir_path;
	s_strncpy(dir_path, COMMON_PATH_LEN, path, path_len);

	COMMON_WPATH w_file_path;
	utf8_to_wchar(w_file_path, (size_t)COMMON_PATH_LEN, dir_path, (int)COMMON_PATH_LEN);

	return (CreateDirectoryW(w_file_path, NULL) != 0);
}

/*
 * Copies a file. If the destination file does not exist it is created.
 * If the destination file exists the function will fail
 * Returns 1 on success, 0 if the operation failed.
 * Note: Windows version must convert paths to UTF-16 first
 */
int copy_file(const COMMON_PATH source, const COMMON_SIZE source_len,
		const COMMON_PATH destination, const COMMON_SIZE destination_len)
{
	// safe file names
	COMMON_PATH source_path;
	COMMON_PATH destination_path;
	s_strncpy(source_path, COMMON_PATH_LEN, source, source_len);
	s_strncpy(destination_path, COMMON_PATH_LEN, destination, destination_len);

	COMMON_WPATH w_source;
	COMMON_WPATH w_dest;
	utf8_to_wchar(w_source, (size_t)COMMON_PATH_LEN, source_path, (int)COMMON_PATH_LEN);
	utf8_to_wchar(w_dest, (size_t)COMMON_PATH_LEN, destination_path, (int)COMMON_PATH_LEN);

	return (CopyFileW(w_source, w_dest, 1) != 0);
}

/*
 * Windows version of open file.  Windows expects unicode to be in UTF-16 so will convert first and
 * use wide char version of "open"
 */
FILE *open_file(const COMMON_PATH path, const COMMON_SIZE path_len, const char *args)
{
	// safe file name
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

	COMMON_WPATH w_source;
	size_t args_size = 32;
	wchar_t w_args[args_size];
	utf8_to_wchar(w_source, (size_t)COMMON_PATH_LEN, file_path, (int)COMMON_PATH_LEN);
	utf8_to_wchar(w_args, args_size, args, (int)args_size);
	return _wfopen(w_source, w_args);
}

/*
 * delete a file, return 1 if deleted, 0 if not
 */
int delete_file(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	// safe file name
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

	COMMON_WPATH w_file_path;
	utf8_to_wchar(w_file_path, (size_t)COMMON_PATH_LEN, file_path, (int)COMMON_PATH_LEN);

	return (DeleteFileW(w_file_path) != 0);
}

/*
 * delete an existing & empty directory
 */
int delete_dir(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	// safe file name
	COMMON_PATH dir_path;
	s_strncpy(dir_path, COMMON_PATH_LEN, path, path_len);

	COMMON_WPATH w_dir_path;
	utf8_to_wchar(w_dir_path, (size_t)COMMON_PATH_LEN, dir_path, (int)COMMON_PATH_LEN);

	return (RemoveDirectoryW(w_dir_path) != 0);
}

/*
 * Convert a relative or absolute path to an absolute path
 */
int get_absolute_path(const COMMON_PATH path, const COMMON_SIZE path_len,
		COMMON_PATH abs_path)
{
	int rc = COMMON_ERR_INVALIDPARAMETER;

	// safe file names
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

	// remove file:///
	if (UrlIsFileUrl(file_path))
	{
		DWORD len = strlen(file_path);
		PathCreateFromUrl(file_path, file_path, &len, 0);
	}
	// or just copy
	else
	{
		s_strcpy(file_path, file_path, COMMON_PATH_LEN);
	}

	// clean up and convert
	COMMON_PATH clean_path;
	if (PathCanonicalize(clean_path, file_path))
	{
		// convert relative to absolute
		if (PathIsRelative(clean_path))
		{
			if (GetFullPathName(clean_path, COMMON_PATH_LEN, abs_path, NULL))
			{
				rc = COMMON_SUCCESS;
			}
		}
		// or just copy it
		else
		{
			s_strcpy(abs_path, clean_path, COMMON_PATH_LEN);
			rc = COMMON_SUCCESS;
		}
	}

	return rc;
}

/*
 * Lock or unlock a file
 */
int lock_file(FILE *p_file, const enum file_lock_mode lock_mode)
{
	int rc = COMMON_SUCCESS;

	if (!p_file)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		int mode = 0;
		switch (lock_mode)
		{
			case FILE_LOCK_MODE_UNLOCK:
				mode = LK_UNLCK;
				break;
			case FILE_LOCK_MODE_WRITE:
				mode = _LK_LOCK;
				break;
			case FILE_LOCK_MODE_READ:
				mode = _LK_RLCK;
				break;
			default:
				rc = COMMON_ERR_INVALIDPARAMETER;
				break;
		}
		if (rc == COMMON_SUCCESS)
		{
			if (_locking(p_file->_file, mode, MAXDWORD) != 0)
			{
				rc = COMMON_ERR_FAILED;
			}
		}
	}
	return rc;
}
