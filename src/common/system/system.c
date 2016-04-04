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
 * This file contains the implementation of common functions to retrieve system
 * processor information.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "system.h"
#include <common_types.h>
#include <string/s_str.h>
#include <os/os_adapter.h>

/*
 * Retrieves the processor brand string.
 */
int get_processor_brand(char *brand, COMMON_SIZE brand_len)
{
	int rc = COMMON_ERR_UNKNOWN;

	// check that the caller has supplied a buffer of at least 48 bytes
	if (!brand || brand_len < 48)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		COMMON_UINT32 eax;
		COMMON_UINT32 ebx;
		COMMON_UINT32 ecx;
		COMMON_UINT32 edx;

		// make sure brand string feature is supported
		if (get_cpuid(0x80000000, &eax, &ebx, &ecx, &edx) && (eax >= 0x80000004))
		{
			// the processor string is returned 16 bytes at a time via 3
			// calls to cpuid
			if (get_cpuid(0x80000002, &eax, &ebx, &ecx, &edx))
			{
				memmove(brand, &eax, 4);
				memmove(brand + 4, &ebx, 4);
				memmove(brand + 8, &ecx, 4);
				memmove(brand + 12, &edx, 4);

				if (get_cpuid(0x80000003, &eax, &ebx, &ecx, &edx))
				{
					memmove(brand + 16, &eax, 4);
					memmove(brand + 20, &ebx, 4);
					memmove(brand + 24, &ecx, 4);
					memmove(brand + 28, &edx, 4);

					if (get_cpuid(0x80000004, &eax, &ebx, &ecx, &edx))
					{
						memmove(brand + 32, &eax, 4);
						memmove(brand + 36, &ebx, 4);
						memmove(brand + 40, &ecx, 4);
						memmove(brand + 44, &edx, 4);

						if (brand_len > 48)
						{
							brand[48] = '\0';
						}

						rc = COMMON_SUCCESS;
					}
				}
			}
		}
	}
	s_strtrim(brand, brand_len);
	return rc;
}

/*
 * Retrieve manufacturer information from the processor.
 */
int get_processor_manufacturer(char *manufacturer, COMMON_SIZE manufacturer_len)
{
	int rc = COMMON_ERR_UNKNOWN;

	// check that the caller has supplied a buffer of at least 12 bytes
	if (!manufacturer || manufacturer_len < 12)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		COMMON_UINT32 eax;
		COMMON_UINT32 ebx;
		COMMON_UINT32 ecx;
		COMMON_UINT32 edx;

		if (get_cpuid(0, &eax, &ebx, &ecx, &edx))
		{
			memmove(manufacturer, &ebx, 4);
			memmove(manufacturer + 4, &edx, 4);
			memmove(manufacturer + 8, &ecx, 4);

			if (manufacturer_len > 12)
			{
				manufacturer[12] = '\0';
			}

			rc = COMMON_SUCCESS;
		}
	}

	return rc;
}

int get_processor_info(processor_info *p_info)
{
	int rc = COMMON_ERR_UNKNOWN;

	if (p_info == NULL)
	{
		rc = COMMON_ERR_INVALIDPARAMETER;
	}
	else
	{
		// define temporary structure to organize EAX register
		typedef union
		{
			struct
			{
				unsigned stepping :4;
				unsigned model :4;
				unsigned family :4;
				unsigned type :2;
				unsigned reserved1 :2;
				unsigned extended_model :4;
				unsigned extended_family :8;
				unsigned reserved2 :4;
			} bits;
			COMMON_UINT32 num;
		} eax_register;

		// define temporary structure to organize EBX register
		typedef union
		{
			struct
			{
				unsigned brand_index :8;
				unsigned clflush_size :8;
				unsigned max_logical_cpus :8;
				unsigned default_apic_id :8;
			} bits;
			COMMON_UINT32 num;
		} ebx_register;

		eax_register eax;
		ebx_register ebx;
		COMMON_UINT32 ecx;
		COMMON_UINT32 edx;

		if (get_cpuid(1, &eax.num, &ebx.num, &ecx, &edx))
		{
			// clear the output
			memset(p_info, 0, sizeof (processor_info));

			// parse out the EAX fields
			p_info->stepping = eax.bits.stepping;
			p_info->type = eax.bits.type;
			p_info->family = eax.bits.family + eax.bits.extended_family;
			p_info->model = (eax.bits.extended_model << 4) + eax.bits.model;

			// parse out the EBX fields
			p_info->brand_index = ebx.bits.brand_index;

			rc = COMMON_SUCCESS;
		}
	}

	return rc;
}
