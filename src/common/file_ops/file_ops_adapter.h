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
 * This file contains definition of the OS adapter interface
 * for general file I/O operations.
 */

#ifndef _FILE_OPERATIONS_H_
#define _FILE_OPERATIONS_H_

#include <common_types.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum file_lock_mode
{
	FILE_LOCK_MODE_UNLOCK = 0,
	FILE_LOCK_MODE_WRITE = 1,
	FILE_LOCK_MODE_READ = 2
};

extern int create_trunc_file(const COMMON_PATH path, const COMMON_SIZE path_len);

/*!
 * Creates a file with default file properties
 * @remarks
 * 		The file is closed after its successful creation
 * @param[in] path
 * 		The path of the file to be created
 * @param[in] path_len
 * 		The length of the path buffer
 * @return
 * 		1 if success, @n
 * 		0 if failure
 */
extern int create_file(const COMMON_PATH path, const COMMON_SIZE path_len);

/*!
 * Creates a directory with default properties
 * @param[in] path
 * 		The path of the directory to be created
 * @param[in] path_len
 * 		The length of the path buffer
 * @return
 * 		1 if success, @n
 * 		0 if failure
 */
extern int create_dir(const COMMON_PATH path, const COMMON_SIZE path_len);

/*!
 * Copy a file
 * @param[in] source
 * 		The full path of the source file to be copied
 * @param[in] source_len
 * 		The length of the source buffer
 * @param[in] destination
 * 		The full path that the source file should be copied to.
 * @param[in] destination_len
 * 		The lengt of the destination buffer
 * @return
 * 		Non-zero if success, @n
 * 		0 if failure
 */
extern int copy_file(const COMMON_PATH source, const COMMON_SIZE source_len,
		const COMMON_PATH destination, const COMMON_SIZE destination_len);

/*!
 * Copy a file to a buffer
 * @note The buffer needs to be freed by the caller
 * @param[in] path
 * 		The full path of the source file to be copied
 * @param[in] path
 * 		The length of the path buffer
 * @param[in,out] pp_buf
 * 		A pointer to the buffer to copy the file into
 * @param[in,out] p_buf_len
 * 		The length of data copied to the buffer
 * @return
 * 		Non-zero if success, @n
 * 		0 if failure
 */
extern int copy_file_to_buffer(const COMMON_PATH path, const COMMON_SIZE path_len,
		void **pp_buf, unsigned int *p_buf_len);

/*!
 * Open a file
 * @param[in] path
 * 		The name of the file
 * @param[in] path_len
 * 		The length of the path buffer
 * @param[in] args
 * 		A @b char array specifying the kind of access requested to the file
 * @return
 * 		A pointer to the open file. @n
 * 		NULL if an error occurred.
 */
extern FILE *open_file(const COMMON_PATH path, const COMMON_SIZE path_len, const char *args);

/*!
 * Delete a file
 * @param[in] path
 * 		The name of the file to be deleted
 * @param[in] path_len
 * 		The length of the path buffer
 * @return
 * 		1 if deleted, @n
 * 		0 if not deleted
 */
extern int delete_file(const COMMON_PATH path, const COMMON_SIZE path_len);

/*!
 * Delete a existing & empty directory
 * @param path
 * 		A string containing the path of the directory to be deleted
 * @param[in] path_len
 * 		The length of the path buffer
 * @return
 * 		1 if deleted, @n
 * 		0 if not deleted
 */
extern int delete_dir(const COMMON_PATH dir_path, const COMMON_SIZE path_len);

/*!
 * Determines if a given file exists
 * @param path
 * 		The path to the file in question
 * @param[in] path_len
 * 		The length of the path buffer
 * @return
 * 		1 if the file exists, @n
 * 		0 if it does not exist
 */
extern int file_exists(const COMMON_PATH path, const COMMON_SIZE path_len);

/*!
 * Convert a relative or absolute path to an absolute path
 * @param path
 * 		The path to convert
 * @param[in] path_len
 * 		The length of the path buffer
 * @param abs_path
 * 		The returned absolute path
 * @return
 *  	COMMON_SUCCESS
 *		COMMON_ERR_INVALIDPARAMETER
 */
extern int get_absolute_path(const COMMON_PATH path, const COMMON_SIZE path_len,
		COMMON_PATH abs_path);

/*!
 * Lock or unlock a file
 * @param[in] p_file
 * 		A pointer to the file to lock or unlock.
 * @param[in] lock_mode
 *		Read/write or unlock
 * @return
 *  	COMMON_SUCCESS
 *  	COMMON_ERR_INVALIDPARAMETER
 *  	COMMON_ERR_FAILED
 */
extern int lock_file(FILE *p_file, const enum file_lock_mode mode);

#ifdef __cplusplus
}
#endif

#endif /* _FILE_OPERATIONS_H_ */
