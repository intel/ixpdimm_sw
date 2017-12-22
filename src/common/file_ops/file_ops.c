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
 * This file contains the OS independent implementation of the file_ops_adapter.h
 * system call wrappers.
 */


#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "file_ops_adapter.h"
#include <string/s_str.h>
#include <os/os_adapter.h>

/*
 * Determine if a file exists
 */
int file_exists(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	// safe file name
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributes(file_path);

    if (!((dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))))
    {
        return FALSE;
    }
    return TRUE;
#else
	return (access(file_path, F_OK) != -1);
#endif
}

/*
 * Copy a buffer to a file.
 */
int copy_buffer_to_file(void *p_buf, unsigned int buf_size,
		const COMMON_PATH path, const COMMON_SIZE path_len, int oflags)
{
	int rc = COMMON_SUCCESS;
    FILE *fd = NULL;

	if (path == NULL || (p_buf == NULL))
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		// safe file name
		COMMON_PATH file_path;
		s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

		// check if the file exists
		if (NULL == (fd = fopen(file_path, "wb+")))
		{
			rc = COMMON_ERR_BADFILE;
		}
		else if (buf_size != fwrite(p_buf, 1, buf_size, fd))
		{
			rc = COMMON_ERR_BADFILE;
		}
	}

	if (NULL != fd)
	{
		fclose(fd);
		fd = NULL;
	}
	return rc;
}

/*
 * Copy a file to a buffer.
 */
int copy_file_to_buffer(const COMMON_PATH path, const COMMON_SIZE path_len,
		void **pp_buf, unsigned int *p_buf_len)
{
	int rc = COMMON_SUCCESS;
	FILE *fd = NULL;

	if (path == NULL || (pp_buf == NULL) || (*pp_buf != NULL))
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		// safe file name
		COMMON_PATH file_path;
        size_t file_size = 0;
		s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

		// check if the file exists
        if (NULL == (fd = fopen(file_path, "rb")))
		{
			rc = COMMON_ERR_BADFILE;
		}
		// get the file information
		else if (get_filesize(file_path, &file_size) != 0)
		{
			rc = COMMON_ERR_BADFILE;
		}
		// make sure it's not empty
		else if (file_size <= 0)
		{
			rc = COMMON_ERR_BADFILE;
		}
		else if ((*pp_buf = malloc(file_size)) == NULL)
		{
			rc = COMMON_ERR_NOMEMORY;
		}
		else if (file_size != fread(*pp_buf, 1, file_size, fd))
		{
			rc = COMMON_ERR_BADFILE;
		}
		else
		{
			*p_buf_len = file_size;
		}
	}

	if (NULL != fd)
	{
		fclose(fd);
		fd = NULL;
	}
	return rc;
}
