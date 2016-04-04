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
 * This file contains the implementation of common functions to crawl raw
 * SMBIOS tables. Structures and values come from the DMTF SMBIOS specification.
 */

#include "smbios.h"
#include <stddef.h>
#include <string.h>
#include <string/s_str.h>

unsigned char SMBIOS_32BIT_ANCHOR_STR[] = {0x5f, 0x53, 0x4d, 0x5f};
unsigned char SMBIOS_64BIT_ANCHOR_STR[] = {0x5f, 0x53, 0x4d, 0x33, 0x5f};

COMMON_UINT8 *smbios_get_byte_after_end_of_string_list(COMMON_UINT8 *p_strings,
		size_t *p_remaining_length)
{
	size_t remaining_length = *p_remaining_length;
	COMMON_UINT8 *p_ptr = NULL;

	// 0000h marks the end of the string list
	COMMON_UINT8 nulls_seen = 0;
	COMMON_UINT8 *p_byte = p_strings;
	while (!p_ptr && (remaining_length > 0))
	{
		if (nulls_seen == 2)
		{
			p_ptr = p_byte;
		}
		else
		{
			if (*p_byte == 0x00)
			{
				nulls_seen++;
			}
			else
			{
				nulls_seen = 0;
			}

			p_byte++;
			remaining_length--;
		}
	}

	*p_remaining_length = remaining_length;
	return p_ptr;
}

const struct smbios_structure_header *smbios_get_next_structure(
		const struct smbios_structure_header *p_current,
		size_t *p_remaining_length)
{
	const struct smbios_structure_header *p_next = NULL;

	if (p_current && p_remaining_length && (*p_remaining_length > 0))
	{
		if (*p_remaining_length >= (size_t)p_current->length)
		{
			// Start looking at the end of the formatted section
			COMMON_UINT8 *p_strings = (COMMON_UINT8 *)p_current + p_current->length;
			*p_remaining_length -= p_current->length;

			p_next = (const struct smbios_structure_header *)
					smbios_get_byte_after_end_of_string_list(p_strings, p_remaining_length);
		}
		else
		{
			*p_remaining_length = 0;
		}
	}

	return p_next;
}

const struct smbios_structure_header *smbios_get_first_structure_of_type(
		const enum smbios_structure_type type,
		const struct smbios_structure_header *p_current, size_t *p_remaining_length)
{
	const struct smbios_structure_header *p_first = NULL;

	if (p_current && p_remaining_length &&
			*p_remaining_length > sizeof (struct smbios_structure_header))
	{
		if (p_current->type == type)
		{
			p_first = p_current;
		}
		else
		{
			p_first = smbios_get_next_structure_of_type(type, p_current, p_remaining_length);
		}
	}

	return p_first;
}

const struct smbios_structure_header *smbios_get_next_structure_of_type(
		const enum smbios_structure_type type,
		const struct smbios_structure_header *p_current,
		size_t *p_remaining_length)
{
	const struct smbios_structure_header *p_next = NULL;

	if (p_current && p_remaining_length && *p_remaining_length > 0)
	{
		COMMON_BOOL found = 0;

		p_next = p_current;
		while (!found && (*p_remaining_length > 0))
		{
			p_next = smbios_get_next_structure(p_next, p_remaining_length);
			if (p_next && p_next->type == type)
			{
				found = 1;
			}
		}
	}

	return p_next;
}

const struct smbios_structure_header *smbios_get_structure_with_handle(const COMMON_UINT16 handle,
		const struct smbios_structure_header *p_top, size_t *p_remaining_length)
{
	const struct smbios_structure_header *p_struct = NULL;

	if (p_top && p_remaining_length &&
			(*p_remaining_length >= sizeof (struct smbios_structure_header)))
	{
		p_struct = p_top;
		do
		{
			if (p_struct->handle == handle)
			{
				break;
			}

			p_struct = smbios_get_next_structure(p_struct, p_remaining_length);
		}
		while (p_struct && (*p_remaining_length >= sizeof (struct smbios_structure_header)));
	}

	return p_struct;
}

const char *smbios_get_next_string(const char *p_current_str, size_t *p_remaining_length)
{
	const char *p_next_str = NULL;

	const char *p_cursor = p_current_str;
	COMMON_BOOL found_terminator = 0;
	for (size_t i = 0; i < *p_remaining_length; i++)
	{
		if (found_terminator)
		{
			// Second null => no more strings for structure
			if (*p_cursor != '\0')
			{
				p_next_str = p_cursor;
			}

			break;
		}
		else if (*p_cursor == '\0')
		{
			found_terminator = 1;
		}

		p_cursor++;
	}

	return p_next_str;
}

const char *smbios_get_structure_string(const struct smbios_structure_header *p_structure,
		const size_t data_length,
		const COMMON_UINT32 string_number)
{
	const char *p_str = NULL;

	if (p_structure && (data_length > p_structure->length) && (string_number > 0))
	{
		size_t remaining_length = data_length;

		// First string starts at the end of the formatted section
		p_str = (const char *)p_structure + p_structure->length;
		remaining_length -= p_structure->length;
		if (*p_str == '\0') // likely end of the list
		{
			// End of the buffer cuts off too early OR explicitly end of list
			if ((remaining_length < 2) || (*(p_str + 1) == '\0'))
			{
				p_str = NULL;
			}
		}

		for (COMMON_UINT32 i = 1; i < string_number; i++)
		{
			if (p_str == NULL) // no more - we won't find it
			{
				break;
			}

			p_str = smbios_get_next_string(p_str, &remaining_length);
		}
	}

	return p_str;
}

void smbios_copy_structure_string_to_buffer(const struct smbios_structure_header *p_structure,
		const size_t data_length,
		const COMMON_UINT32 string_number,
		char *p_buf, const size_t buf_length)
{
	if (p_buf && buf_length > 0)
	{
		memset(p_buf, 0, buf_length);

		const char *p_str = smbios_get_structure_string(p_structure, data_length, string_number);
		if (p_str)
		{
			s_strcpy(p_buf, p_str, buf_length);
		}
	}
}

int smbios_get_structure_count_of_type(const enum smbios_structure_type type,
		const struct smbios_structure_header *p_top, const size_t data_length)
{
	int count = 0;

	if (p_top && data_length >= sizeof (struct smbios_structure_header))
	{
		size_t remaining_length = data_length;
		const struct smbios_structure_header *p_current =
				smbios_get_first_structure_of_type(type, p_top, &remaining_length);
		while (p_current)
		{
			count++;
			p_current = smbios_get_next_structure_of_type(type, p_current, &remaining_length);
		}
	}
	else
	{
		count = COMMON_ERR_INVALIDPARAMETER;
	}

	return count;
}
