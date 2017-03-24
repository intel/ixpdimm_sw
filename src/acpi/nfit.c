/*
 * Copyright (c) 2017, Intel Corporation
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
 * This file defines an interface to retrieve parsed information from the
 * NFIT tables
 */

#include "nfit.h"
#include "acpi.h"
#include <stdlib.h>
#include <stdio.h>

enum nfit_error acpi_error_to_nfit_error(enum acpi_error acpi_err)
{
	enum nfit_error nfit_err;
	switch (acpi_err)
	{
		case ACPI_ERR_CHECKSUMFAIL:
			nfit_err = NFIT_ERR_CHECKSUMFAIL;
			break;
		case ACPI_ERR_BADINPUT:
		case ACPI_ERR_BADTABLE:
		case ACPI_ERR_BADTABLESIGNATURE:
			nfit_err = NFIT_ERR_BADNFIT;
			break;
		case ACPI_ERR_TABLENOTFOUND:
			nfit_err = NFIT_ERR_TABLENOTFOUND;
			break;
		default:
			nfit_err = NFIT_SUCCESS;
			break;
	}
	return nfit_err;
}

/*
 * Retrieve the NFIT table from ACPI and parse it
 */
int nfit_get_parsed_nfit(struct parsed_nfit **pp_parsed_nfit)
{
	int result = NFIT_SUCCESS;

	int buffer_size = get_acpi_table("NFIT", NULL, 0);
	if (buffer_size <= 0)
	{
		result = acpi_error_to_nfit_error(buffer_size);
	}
	else
	{
		unsigned char *buffer = (unsigned char *) calloc(1, buffer_size);
		if (!buffer)
		{
			result = NFIT_ERR_NOMEMORY;
		}
		else
		{
			result = acpi_error_to_nfit_error(
					get_acpi_table("NFIT", (struct acpi_table *)buffer,
						(int)buffer_size));
			if (result == NFIT_SUCCESS)
			{
				result = nfit_parse_raw_nfit(buffer, buffer_size, pp_parsed_nfit);
			}
			free(buffer);
		}
	}
	return result;
}

/*
 * Print a descriptive version of an NFIT error
 */
void nfit_print_error(const int err)
{
	switch ((enum nfit_error) err)
	{
		case NFIT_SUCCESS:
			printf("Success.\n");
			break;
		case NFIT_ERR_TABLENOTFOUND:
			printf("NFIT table not found.\n");
			break;
		case NFIT_ERR_CHECKSUMFAIL:
			printf("NFIT table checksum failed.\n");
			break;
		case NFIT_ERR_BADNFIT:
			printf("NFIT table is corrupted.\n");
			break;
		case NFIT_ERR_NOMEMORY:
			printf("Out of memory.\n");
			break;
		default:
			printf("Internal error.\n");
			break;
	}
}
