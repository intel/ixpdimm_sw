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
#include <unistd.h>

#include "file_ops_adapter.h"
#include <string/s_str.h>

/*
 * Determine if a file exists
 */
int file_exists(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	// safe file name
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);
	return (access(file_path, F_OK) != -1);
}

/*
 * Copy a file to a buffer.
 */
int copy_file_to_buffer(const COMMON_PATH path, const COMMON_SIZE path_len,
		void **pp_buf, unsigned int *p_buf_len)
{
	int rc = COMMON_SUCCESS;
	int fd = -1;
	struct stat sb;
#ifdef __WINDOWS__
	int OS_flags = O_BINARY;
#else
	int OS_flags = 0;
#endif


	if (path == NULL || (pp_buf == NULL) || (*pp_buf != NULL))
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		// safe file name
		COMMON_PATH file_path;
		s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

		// check if the file exists
		if ((fd = open(file_path, O_RDWR|OS_flags, 0)) == -1)
		{
			rc = COMMON_ERR_BADFILE;
		}
		// get the file information
		else if (stat(file_path, &sb) != 0)
		{
			rc = COMMON_ERR_BADFILE;
		}
		// make sure it's not empty
		else if (sb.st_size <= 0)
		{
			rc = COMMON_ERR_BADFILE;
		}
		else if ((*pp_buf = malloc(sb.st_size)) == NULL)
		{
			rc = COMMON_ERR_NOMEMORY;
		}
		else if (read(fd, *pp_buf, sb.st_size) != (ssize_t)sb.st_size)
		{
			rc = COMMON_ERR_BADFILE;
		}
		else
		{
			*p_buf_len = sb.st_size;
		}
	}

	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
	return rc;
}
