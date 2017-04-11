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
 * This file contains the implementation of the helper functions for
 * reading/writing the BIOS platform capabilities table (PCAT).
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvm_types.h"
#include "platform_capabilities.h"
#include "platform_config_data.h"
#include <persistence/logging.h>

/*
 * Given a starting offset of the extension tables, get the offset of the specfied table
 */
NVM_UINT32 get_offset_of_ext_table(const struct bios_capabilities *p_capabilities,
		enum pcat_ext_table_type type, NVM_UINT32 offset)
{
	COMMON_LOG_ENTRY();
	int rc = offset;
	int found = 0;

	if (p_capabilities)
	{
		// write extension tables
		while (offset < p_capabilities->header.length && !found)
		{
			struct pcat_extension_table_header *p_header =
					(struct pcat_extension_table_header *)
					((NVM_UINT8 *)p_capabilities + offset);

			// check the length for validity
			if (p_header->length == 0 ||
					(p_header->length + offset) > p_capabilities->header.length)
			{
				COMMON_LOG_ERROR_F("Extension table length %d invalid", p_header->length);
				rc = NVM_ERR_BADPCAT;
				break;
			}

			// store pcat platform info extension table
			if (p_header->type == type)
			{
				rc = offset;
				found = 1;
			}
			offset += p_header->length;
		}
	}

	if (!found)
	{
		rc = -1;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the capabilities of the host platform
 */
int get_pcat(struct bios_capabilities *p_pcat)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_pcat == NULL)
	{
		COMMON_LOG_ERROR("PCAT structure is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		memset(p_pcat, 0, sizeof (*p_pcat));
		int size = get_acpi_table(PCAT_TABLE_SIGNATURE, (struct acpi_table *)p_pcat,
			sizeof (*p_pcat));
		if (size < 0)
		{
			rc = size;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
