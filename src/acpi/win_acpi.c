/*
 * Copyright (c) 2016 2017, Intel Corporation
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
 * Implementations of ACPI helper functions for Windows
 */

#include "acpi.h"
#include <windows.h>
#include <stdio.h>

DWORD string_to_dword(const char *str)
{
	union
	{
		DWORD dword;
		char string[4];
	} fw_table_signature;

	memmove(fw_table_signature.string, str, sizeof (fw_table_signature.string));

	return fw_table_signature.dword;
}

DWORD get_acpi_provider_signature()
{
	// Endian-flipped "ACPI"
	static const char *ACPI_PROVIDER_SIGNATURE = "IPCA";
	return string_to_dword(ACPI_PROVIDER_SIGNATURE);
}

/*!
 * Return the specified ACPI table or the size
 * required
 */
int get_acpi_table(const char *table_signature, struct acpi_table *p_table,
	const unsigned int table_size)
{
	int rc = 0;

	DWORD acpi_table_sig = string_to_dword(table_signature);
	UINT size_fetched = GetSystemFirmwareTable(get_acpi_provider_signature(), acpi_table_sig,
		p_table, table_size);

	if (size_fetched == 0)
	{
		rc = ACPI_ERR_TABLENOTFOUND;
	}
	else
	{
		rc = size_fetched;
	}

	return rc;
}
