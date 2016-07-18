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
 * This file contains the Linux implementation of the file_ops_adapter.h
 * system call wrappers.
 */


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// file I/O
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file_ops_adapter.h"
#include <string/s_str.h>

extern char *realpath(__const char *__restrict __name,
	char *__restrict __resolved) __THROW __wur;

/*
 * Create a file that is empty, this function will fail is the file already exits
 */
int create_trunc_file(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	int rc = 0;

	// safe file name
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

	// open the file and get the file descriptor
	int oflag = O_RDWR | O_CREAT | O_EXCL | O_TRUNC;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int file_des = open(file_path, oflag, mode);
	if (file_des >= 0)
	{
		close(file_des);
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

	// open the file and get the file descriptor
	int oflag = O_RDWR | O_CREAT | O_EXCL;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int file_des = open(file_path, oflag, mode);
	if (file_des >= 0)
	{
		close(file_des);
		rc = 1;
	}

	return rc;
}

/*
 * Create a directory
 */
int create_dir(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	int rc = 0;

	// safe file name
	COMMON_PATH dir_path;
	s_strncpy(dir_path, COMMON_PATH_LEN, path, path_len);

	struct stat st = {0};
	if (stat(dir_path, &st) == -1)
	{
		if (mkdir(dir_path, 0700) == 0)
		{
			rc = 1;
		}
	}

	return rc;
}

/*
 * Copies a file. If the destination file does not exist it is created.
 * If the destination file exists the function will fail
 * Returns 1 on success, 0 if the operation failed.
 */
int copy_file(const COMMON_PATH source, const COMMON_SIZE source_len,
		const COMMON_PATH destination, const COMMON_SIZE destination_len)
{
	// safe file names
	COMMON_PATH source_path;
	COMMON_PATH destination_path;
	s_strncpy(source_path, COMMON_PATH_LEN, source, source_len);
	s_strncpy(destination_path, COMMON_PATH_LEN, destination, destination_len);

	// open both files
	int fd_in = open(source_path, O_RDONLY);
	int fd_out = open(destination_path, O_WRONLY|O_CREAT|O_EXCL, 0x0664);
	char buf[512];
	size_t copied = 0;
	size_t bytestocopy;

	// assume that we are successful until proven otherwise
	int rc = 1;
	if ((fd_in != -1) && (fd_out != -1))
	{
		struct stat stat_buf;
		if (fstat(fd_in, &stat_buf) == 0)
		{
			while (copied < stat_buf.st_size && rc != -1)
			{
				bytestocopy = (stat_buf.st_size > 512) ? 512 : stat_buf.st_size;
				rc = (read(fd_in, buf, bytestocopy) != -1);
				if (rc)
				{
					rc = (write(fd_out, buf, bytestocopy) != -1);
				}
				if (rc)
				{
					copied += bytestocopy;
				}
			}

			if (copied != stat_buf.st_size)
			{
				rc = 0;
			}
		}
		else
		{
			rc = 0;
		}
	}
	else
	{
		rc = 0;
	}

	if (fd_in >= 0)
	{
		close(fd_in);
	}

	if (fd_out >= 0)
	{
		// set the permissions on the copy
		fchmod(fd_out, S_IROTH | S_IRGRP | S_IWUSR | S_IRUSR);
		close(fd_out);
	}

	return rc;
}

/*
 * linux version of open_file.
 */
FILE *open_file(const COMMON_PATH path, const COMMON_SIZE path_len, const char *args)
{
	// safe file name
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

	return fopen(file_path, args);
}

/*
 * Delete a file, return 1 if deleted, 0 if not
 */
int delete_file(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	// safe file name
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

	return (remove(file_path) == 0);
}

/*
 * delete an existing & empty directory
 */
int delete_dir(const COMMON_PATH path, const COMMON_SIZE path_len)
{
	// safe file name
	COMMON_PATH dir_path;
	s_strncpy(dir_path, COMMON_PATH_LEN, path, path_len);

	return (rmdir(dir_path) == 0);
}

/*
 * Convert a relative or absolute path to an absolute path
 */
int get_absolute_path(const COMMON_PATH path, const COMMON_SIZE path_len,
		COMMON_PATH abs_path)
{
	int rc = COMMON_SUCCESS;

	// safe file names
	COMMON_PATH file_path;
	s_strncpy(file_path, COMMON_PATH_LEN, path, path_len);

	// remove file://
	COMMON_PATH non_uri_path;
	const char *uri_prefix = "file://";
	if (s_strncmpi(file_path, uri_prefix, strlen(uri_prefix)) == 0)
	{
		s_strcpy(non_uri_path, file_path + strlen(uri_prefix), COMMON_PATH_LEN);
	}
	else
	{
		s_strcpy(non_uri_path, file_path, COMMON_PATH_LEN);
	}

	if (realpath(non_uri_path, abs_path) == NULL)
	{
		// ignore file doesn't exist errors
		if (errno != ENOENT)
		{
			rc = COMMON_ERR_INVALIDPARAMETER;
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
		switch (lock_mode)
		{
			case FILE_LOCK_MODE_UNLOCK:
				funlockfile(p_file);
				break;
			case FILE_LOCK_MODE_WRITE:
			case FILE_LOCK_MODE_READ:
				if (ftrylockfile(p_file) != 0)
				{
					rc = COMMON_ERR_FAILED;
				}
				break;
			default:
				rc = COMMON_ERR_INVALIDPARAMETER;
				break;
		}
	}

	return rc;
}
