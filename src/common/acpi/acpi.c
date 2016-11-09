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
 * Implementations of common ACPI helper functions
 */

#include "acpi.h"
#include <string.h>
#include <stdio.h>



#define	COMMON_LOG_ENTRY()
#define	COMMON_LOG_ERROR(error)
#define	COMMON_LOG_EXIT_RETURN_I(rc)
#define	COMMON_LOG_DEBUG_F(fmt, ...) \
	printf(fmt "\n", __VA_ARGS__)

#define	COMMON_LOG_ERROR_F(fmt, ...)
#define	COMMON_LOG_DEBUG(str)
#define	COMMON_LOG_EXIT()

/*
 * Verify the ACPI table size, checksum and signature
 */
int check_acpi_table(const char *signature,
		struct acpi_table *p_table)
{
	COMMON_LOG_ENTRY();
	int rc = COMMON_SUCCESS;

	if (!p_table)
	{
		COMMON_LOG_ERROR("ACPI table is NULL");
		rc = COMMON_ERR_FAILED;
	}
	else if (!verify_checksum((COMMON_UINT8 *)p_table, p_table->header.length))
	{
		COMMON_LOG_ERROR_F("ACPI table '%.4s' failed checksum", signature);
		rc = COMMON_ERR_FAILED;
	}

	// check overall table length is at least as big as the header
	else if (p_table->header.length < sizeof (struct acpi_table))
	{
		COMMON_LOG_ERROR_F("ACPI table '%.4s' length is invalid", signature);
		rc = COMMON_ERR_FAILED;
	}
	// check signature
	else if (strncmp(signature, p_table->header.signature, ACPI_SIGNATURE_LEN) != 0)
	{
		COMMON_LOG_ERROR_F("ACPI table '%.4s' signature mismatch", signature);
		rc = COMMON_ERR_FAILED;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Print the ACPI table header
 */
void print_acpi_table_header(struct acpi_table_header *p_header)
{
	COMMON_LOG_ENTRY();

	if (p_header)
	{
		COMMON_LOG_DEBUG_F("Signature: %.4s", p_header->signature);
		COMMON_LOG_DEBUG_F("Length: %u", p_header->length);
		COMMON_LOG_DEBUG_F("Revision: %hhu", p_header->revision);
		COMMON_LOG_DEBUG_F("Checksum: %hhu", p_header->checksum);
		COMMON_LOG_DEBUG_F("OEM ID: %s", p_header->oem_id);
		COMMON_LOG_DEBUG_F("OEM Table ID: %s", p_header->oem_table_id);
		COMMON_LOG_DEBUG_F("OEM Revision: %u", p_header->oem_revision);
		COMMON_LOG_DEBUG_F("Creator ID: %u", p_header->creator_id);
		COMMON_LOG_DEBUG_F("Creator Revision: %u", p_header->creator_revision);
	}
	else
	{
		COMMON_LOG_DEBUG("ACPI table is NULL");
	}

	COMMON_LOG_EXIT();
}
