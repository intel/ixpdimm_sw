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
 * Implementations of ACPI helper functions for Linux
 */

#include "acpi.h"
#include <string/s_str.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>

#define	SYSFS_ACPI_PATH	"/sys/firmware/acpi/tables/"

#define	COMMON_LOG_ENTRY()
#define	COMMON_LOG_ERROR(error)
#define	COMMON_LOG_EXIT_RETURN_I(rc)
#define	COMMON_LOG_DEBUG_F(fmt, ...) \
	printf(fmt "\n", __VA_ARGS__)

#define	COMMON_LOG_ERROR_F(fmt, ...)
#define	COMMON_LOG_DEBUG(str)
#define	COMMON_LOG_EXIT()

/*!
 * Return the specified ACPI table or the size
 * required
 */
int get_acpi_table(
		const char *signature,
		struct acpi_table *p_table,
		const COMMON_UINT32 size)
{
	COMMON_LOG_ENTRY();
	int rc = 0; // table size

	COMMON_PATH table_path;
	s_strncpy(table_path, COMMON_PATH_LEN, SYSFS_ACPI_PATH,
		strnlen(SYSFS_ACPI_PATH, COMMON_PATH_LEN));
	s_strncat(table_path, COMMON_PATH_LEN, signature, strnlen(signature, COMMON_PATH_LEN));

	int fd = open(table_path, O_RDONLY|O_CLOEXEC);
	if (fd < 0)
	{
		COMMON_LOG_ERROR_F("ACPI table '%.4s' not found", signature);
		rc = COMMON_ERR_FAILED;
	}
	else
	{
		struct acpi_table_header header;
		size_t header_size = sizeof (header);


		ssize_t hdr_bytes_read = read(fd, &header, header_size);
		if (hdr_bytes_read != header_size)
		{
			COMMON_LOG_ERROR_F("ACPI table '%.4s' is invalid", signature);
			rc = COMMON_ERR_FAILED;
		}
		else
		{
			size_t total_table_size = header.length;
			rc = (int)total_table_size;
			if (p_table)
			{
				memset(p_table, 0, size);
				memcpy(&(p_table->header), &header, header_size);
				if (size < total_table_size)
				{
					COMMON_LOG_ERROR_F(
							"buffer length of %u is not large enough for ACPI table of size %u",
							size, total_table_size);
					rc = COMMON_ERR_FAILED;
				}
				else
				{
					ssize_t tbl_bytes_read = read(fd, p_table->p_ext_tables,
						total_table_size - header_size);

					if (tbl_bytes_read + header_size != total_table_size)
					{
						COMMON_LOG_ERROR_F("Failed to read ACPI table '%.4s'", signature);
						rc = COMMON_ERR_FAILED;
					}
					else
					{
						rc = check_acpi_table(signature, p_table);
					}
				}
			}
		}
		close(fd);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
