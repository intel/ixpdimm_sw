/*
 * Copyright (c) 2015 2017, Intel Corporation
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <common_types.h>

/*
 * Verify the ACPI table size, checksum and signature
 */
int check_acpi_table(const char *signature,
		struct acpi_table *p_table)
{
	int rc = ACPI_SUCCESS;

	if (!p_table)
	{
		rc = ACPI_ERR_BADINPUT;
	}
	else if (!verify_checksum((COMMON_UINT8 *)p_table, p_table->header.length))
	{
		rc = ACPI_ERR_CHECKSUMFAIL;
	}
	// check overall table length is at least as big as the header
	else if (p_table->header.length < sizeof (struct acpi_table))
	{
		rc = ACPI_ERR_BADTABLE;
	}
	// check signature
	else if (strncmp(signature, p_table->header.signature, ACPI_SIGNATURE_LEN) != 0)
	{
		rc = ACPI_ERR_BADTABLESIGNATURE;
	}
	return rc;
}

/*
 * Print the ACPI table header
 */
void print_acpi_table_header(struct acpi_table_header *p_header)
{
	if (p_header)
	{
		printf("Signature: %.4s", p_header->signature);
		printf("Length: %u", p_header->length);
		printf("Revision: %hhu", p_header->revision);
		printf("Checksum: %hhu", p_header->checksum);
		printf("OEM ID: %s", p_header->oem_id);
		printf("OEM Table ID: %s", p_header->oem_table_id);
		printf("OEM Revision: %u", p_header->oem_revision);
		printf("Creator ID: %u", p_header->creator_id);
		printf("Creator Revision: %u", p_header->creator_revision);
	}
	else
	{
		printf("ACPI table is NULL");
	}
}
