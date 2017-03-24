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

#include "nfit.h"
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

/*
 * Allocate space for and copy the spa table
 * to the parsed_nfit structure.
 */
int add_spa_to_parsed_nfit(
	struct parsed_nfit *p_nfit,
	const struct spa *p_spa_table)
{
	int result = NFIT_SUCCESS;

	// allocate space for the extension table
	p_nfit->spa_list =
		(struct spa *)
		realloc(p_nfit->spa_list,
		sizeof (struct spa) *
		(p_nfit->spa_count + 1));
	if (!p_nfit->spa_list)
	{
		result = NFIT_ERR_NOMEMORY;
	}
	else
	{
		// copy the extension table
		memmove(&p_nfit->spa_list[p_nfit->spa_count],
			p_spa_table,
			sizeof (struct spa));
		p_nfit->spa_count++;
	}

	return result;
}
/*
 * Allocate space for and copy the region_mapping table
 * to the parsed_nfit structure.
 */
int add_region_mapping_to_parsed_nfit(
	struct parsed_nfit *p_nfit,
	const struct region_mapping *p_region_mapping_table)
{
	int result = NFIT_SUCCESS;

	// allocate space for the extension table
	p_nfit->region_mapping_list =
		(struct region_mapping *)
		realloc(p_nfit->region_mapping_list,
		sizeof (struct region_mapping) *
		(p_nfit->region_mapping_count + 1));
	if (!p_nfit->region_mapping_list)
	{
		result = NFIT_ERR_NOMEMORY;
	}
	else
	{
		// copy the extension table
		memmove(&p_nfit->region_mapping_list[p_nfit->region_mapping_count],
			p_region_mapping_table,
			sizeof (struct region_mapping));
		p_nfit->region_mapping_count++;
	}

	return result;
}
/*
 * Allocate space for and copy the interleave table
 * to the parsed_nfit structure.
 */
int add_interleave_to_parsed_nfit(
	struct parsed_nfit *p_nfit,
	const struct interleave *p_interleave_table)
{
	int result = NFIT_SUCCESS;

	// allocate space for the extension table
	p_nfit->interleave_list =
		(struct interleave *)
		realloc(p_nfit->interleave_list,
		sizeof (struct interleave) *
		(p_nfit->interleave_count + 1));
	if (!p_nfit->interleave_list)
	{
		result = NFIT_ERR_NOMEMORY;
	}
	else
	{
		// copy the extension table
		memmove(&p_nfit->interleave_list[p_nfit->interleave_count],
			p_interleave_table,
			sizeof (struct interleave));
		p_nfit->interleave_count++;
	}

	return result;
}
/*
 * Allocate space for and copy the smbios_management_info table
 * to the parsed_nfit structure.
 */
int add_smbios_management_info_to_parsed_nfit(
	struct parsed_nfit *p_nfit,
	const struct smbios_management_info *p_smbios_management_info_table)
{
	int result = NFIT_SUCCESS;

	// allocate space for the extension table
	p_nfit->smbios_management_info_list =
		(struct smbios_management_info *)
		realloc(p_nfit->smbios_management_info_list,
		sizeof (struct smbios_management_info) *
		(p_nfit->smbios_management_info_count + 1));
	if (!p_nfit->smbios_management_info_list)
	{
		result = NFIT_ERR_NOMEMORY;
	}
	else
	{
		// copy the extension table
		memmove(&p_nfit->smbios_management_info_list[p_nfit->smbios_management_info_count],
			p_smbios_management_info_table,
			sizeof (struct smbios_management_info));
		p_nfit->smbios_management_info_count++;
	}

	return result;
}
/*
 * Allocate space for and copy the control_region table
 * to the parsed_nfit structure.
 */
int add_control_region_to_parsed_nfit(
	struct parsed_nfit *p_nfit,
	const struct control_region *p_control_region_table)
{
	int result = NFIT_SUCCESS;

	// allocate space for the extension table
	p_nfit->control_region_list =
		(struct control_region *)
		realloc(p_nfit->control_region_list,
		sizeof (struct control_region) *
		(p_nfit->control_region_count + 1));
	if (!p_nfit->control_region_list)
	{
		result = NFIT_ERR_NOMEMORY;
	}
	else
	{
		// copy the extension table
		memmove(&p_nfit->control_region_list[p_nfit->control_region_count],
			p_control_region_table,
			sizeof (struct control_region));
		p_nfit->control_region_count++;
	}

	return result;
}
/*
 * Allocate space for and copy the block_data_window_region table
 * to the parsed_nfit structure.
 */
int add_block_data_window_region_to_parsed_nfit(
	struct parsed_nfit *p_nfit,
	const struct block_data_window_region *p_block_data_window_region_table)
{
	int result = NFIT_SUCCESS;

	// allocate space for the extension table
	p_nfit->block_data_window_region_list =
		(struct block_data_window_region *)
		realloc(p_nfit->block_data_window_region_list,
		sizeof (struct block_data_window_region) *
		(p_nfit->block_data_window_region_count + 1));
	if (!p_nfit->block_data_window_region_list)
	{
		result = NFIT_ERR_NOMEMORY;
	}
	else
	{
		// copy the extension table
		memmove(&p_nfit->block_data_window_region_list[p_nfit->block_data_window_region_count],
			p_block_data_window_region_table,
			sizeof (struct block_data_window_region));
		p_nfit->block_data_window_region_count++;
	}

	return result;
}
/*
 * Allocate space for and copy the flush_hint_address table
 * to the parsed_nfit structure.
 */
int add_flush_hint_address_to_parsed_nfit(
	struct parsed_nfit *p_nfit,
	const struct flush_hint_address *p_flush_hint_address_table)
{
	int result = NFIT_SUCCESS;

	// allocate space for the extension table
	p_nfit->flush_hint_address_list =
		(struct flush_hint_address *)
		realloc(p_nfit->flush_hint_address_list,
		sizeof (struct flush_hint_address) *
		(p_nfit->flush_hint_address_count + 1));
	if (!p_nfit->flush_hint_address_list)
	{
		result = NFIT_ERR_NOMEMORY;
	}
	else
	{
		// copy the extension table
		memmove(&p_nfit->flush_hint_address_list[p_nfit->flush_hint_address_count],
			p_flush_hint_address_table,
			sizeof (struct flush_hint_address));
		p_nfit->flush_hint_address_count++;
	}

	return result;
}
/*
 * Parse raw NFIT data into a parsed_nfit structure.
 * The caller is responsible for freeing the parsed_nfit structure.
 */
int nfit_parse_raw_nfit(unsigned char *buffer, size_t buffer_size,
	struct parsed_nfit ** pp_parsed_nfit)
{
	int result = NFIT_SUCCESS;

	if (buffer_size == 0 || buffer == NULL)
	{
		return NFIT_ERR_BADINPUT;
	}

	*pp_parsed_nfit = calloc(1, sizeof (struct parsed_nfit));
	if (!(*pp_parsed_nfit))
	{
		return NFIT_ERR_NOMEMORY;
	}

	// copy the NFIT header
	memmove(&(*pp_parsed_nfit)->nfit, buffer, sizeof (struct nfit));
	size_t offset = sizeof (struct nfit);

	// copy the NFIT extension tables
	while (offset < buffer_size && result == NFIT_SUCCESS)
	{
		unsigned short type = *(buffer + offset);
		unsigned short length = *(buffer + offset + 2);

		// check the length for validity
		if (length == 0 || (length + offset) > buffer_size)
		{
			result = NFIT_ERR_BADNFIT;
			break;
		}

		// create the extension table
		switch (type)
		{
			case 0: // spa extension table
			{
				result = add_spa_to_parsed_nfit(
					*pp_parsed_nfit, (struct spa *)(buffer + offset));
				break;
			}
			case 1: // region_mapping extension table
			{
				result = add_region_mapping_to_parsed_nfit(
					*pp_parsed_nfit, (struct region_mapping *)(buffer + offset));
				break;
			}
			case 2: // interleave extension table
			{
				result = add_interleave_to_parsed_nfit(
					*pp_parsed_nfit, (struct interleave *)(buffer + offset));
				break;
			}
			case 3: // smbios_management_info extension table
			{
				result = add_smbios_management_info_to_parsed_nfit(
					*pp_parsed_nfit, (struct smbios_management_info *)(buffer + offset));
				break;
			}
			case 4: // control_region extension table
			{
				result = add_control_region_to_parsed_nfit(
					*pp_parsed_nfit, (struct control_region *)(buffer + offset));
				break;
			}
			case 5: // block_data_window_region extension table
			{
				result = add_block_data_window_region_to_parsed_nfit(
					*pp_parsed_nfit, (struct block_data_window_region *)(buffer + offset));
				break;
			}
			case 6: // flush_hint_address extension table
			{
				result = add_flush_hint_address_to_parsed_nfit(
					*pp_parsed_nfit, (struct flush_hint_address *)(buffer + offset));
				break;
			}
			default:
				break;
		}
		offset += length;
	} // end while extension tables

	// on error, free the parsed_nfit struct
	if (result != NFIT_SUCCESS && *pp_parsed_nfit)
	{
		free_parsed_nfit(*pp_parsed_nfit);
	}

	return result;
}

/*
 * Clean up a parsed nfit structure
 */
void free_parsed_nfit(struct parsed_nfit *p_parsed_nfit)
{
	if (p_parsed_nfit)
	{
		if (p_parsed_nfit->spa_count &&
			p_parsed_nfit->spa_list)
		{
			free(p_parsed_nfit->spa_list);
		}
		if (p_parsed_nfit->region_mapping_count &&
			p_parsed_nfit->region_mapping_list)
		{
			free(p_parsed_nfit->region_mapping_list);
		}
		if (p_parsed_nfit->interleave_count &&
			p_parsed_nfit->interleave_list)
		{
			free(p_parsed_nfit->interleave_list);
		}
		if (p_parsed_nfit->smbios_management_info_count &&
			p_parsed_nfit->smbios_management_info_list)
		{
			free(p_parsed_nfit->smbios_management_info_list);
		}
		if (p_parsed_nfit->control_region_count &&
			p_parsed_nfit->control_region_list)
		{
			free(p_parsed_nfit->control_region_list);
		}
		if (p_parsed_nfit->block_data_window_region_count &&
			p_parsed_nfit->block_data_window_region_list)
		{
			free(p_parsed_nfit->block_data_window_region_list);
		}
		if (p_parsed_nfit->flush_hint_address_count &&
			p_parsed_nfit->flush_hint_address_list)
		{
			free(p_parsed_nfit->flush_hint_address_list);
		}
	}
	free(p_parsed_nfit);
}