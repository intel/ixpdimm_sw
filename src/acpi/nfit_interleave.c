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
 * This file contains helper functions to retrieve NVDIMM info from the NFIT
 */

#include "nfit.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <guid/guid.h>

int is_dimm_already_added(unsigned int handle, struct nfit_interleave_set *p_interleave_set)
{
	int added = 0;
	for (int i = 0; i < p_interleave_set->dimm_count; i++)
	{
		if (handle == p_interleave_set->dimms[i])
		{
			added = 1;
			break;
		}
	}
	return added;
}

int populate_interleave_set_dimm_info(struct nfit_interleave_set *p_interleave_set,
		const struct parsed_nfit *p_parsed_nfit)
{
	int result = NFIT_SUCCESS;
	for (int i = 0; i < p_parsed_nfit->region_mapping_count; i++)
	{
		if (p_parsed_nfit->region_mapping_list[i].spa_index == p_interleave_set->id)
		{
			if (!is_dimm_already_added(
					p_parsed_nfit->region_mapping_list[i].handle,
					p_interleave_set))
			{
				p_interleave_set->dimms[p_interleave_set->dimm_count] =
					p_parsed_nfit->region_mapping_list[i].handle;
				p_interleave_set->dimm_region_pdas[p_interleave_set->dimm_count] =
					p_parsed_nfit->region_mapping_list[i].physical_address_region_base;
				p_interleave_set->dimm_region_offsets[p_interleave_set->dimm_count] =
					p_parsed_nfit->region_mapping_list[i].region_offset;
				p_interleave_set->dimm_sizes[p_interleave_set->dimm_count] =
					p_parsed_nfit->region_mapping_list[i].region_size;
				p_interleave_set->dimm_count++;
			}
		}
	}
	return result;
}

/*
 * Retrieves a list of interleave sets from the NFIT and returns the count.
 * If input count is 0, simply retrieve the count from the NFIT.
 */
int nfit_get_interleave_sets(const int count, struct nfit_interleave_set *p_interleave_sets)
{
	// get the parsed nfit
	struct parsed_nfit *p_parsed_nfit;
	int result = nfit_get_parsed_nfit(&p_parsed_nfit);
	if (result == NFIT_SUCCESS && p_parsed_nfit)
	{
		result = nfit_get_interleave_sets_from_parsed_nfit(
				count, p_interleave_sets, p_parsed_nfit);
		free_parsed_nfit(p_parsed_nfit);
	}
	return result;

}

/*
 * Retrieves a list of interleave sets from a pre-parsed NFIT and returns the count.
 * If input count is 0, simply retrieve the count from the NFIT.
 */
int nfit_get_interleave_sets_from_parsed_nfit(const int count,
		struct nfit_interleave_set *p_interleave_sets,
		const struct parsed_nfit *p_parsed_nfit)
{
	int set_count = 0;
	if (count && p_interleave_sets)
	{
		memset(p_interleave_sets, 0, sizeof (struct nfit_interleave_set) * count);
	}

	for (int i = 0; i < p_parsed_nfit->spa_count; i++)
	{
		COMMON_GUID pm_guid;
		str_to_guid(SPA_RANGE_PM_REGION_GUID_STR, pm_guid);
		if (guid_cmp(pm_guid, p_parsed_nfit->spa_list[i].address_range_type_guid))
		{
			if (count && p_interleave_sets)
			{
				if (count <= set_count) // array is not big enough
				{
					set_count = NFIT_ERR_BADINPUT;
					break;
				}
				else
				{
					p_interleave_sets[set_count].id =
						p_parsed_nfit->spa_list[i].spa_range_index;
					p_interleave_sets[set_count].proximity_domain =
						p_parsed_nfit->spa_list[i].proximity_domain;
					p_interleave_sets[set_count].address =
						p_parsed_nfit->spa_list[i].spa_range_base;
					p_interleave_sets[set_count].size =
						p_parsed_nfit->spa_list[i].spa_range_length;
					p_interleave_sets[set_count].attributes =
						p_parsed_nfit->spa_list[i].address_range_memory_mapping_attribute;
					// populate dimm info
					populate_interleave_set_dimm_info(&p_interleave_sets[set_count],
							p_parsed_nfit);
				}
			}
			set_count++;
		}
	}

	return set_count;
}

/*
 * Print an NFIT interleave set
 */
void nfit_print_interleave_set(const struct nfit_interleave_set *p_interleave)
{
	if (p_interleave)
	{
		printf("id: 0x%x\n", p_interleave->id);
		printf("proximity_domain: 0x%x\n", p_interleave->proximity_domain);
		printf("address: 0x%llx\n", p_interleave->address);
		printf("size: 0x%llx\n", p_interleave->size);
		printf("attributes: 0x%llx\n", p_interleave->attributes);
		printf("dimm_count: 0x%x\n", p_interleave->dimm_count);
		for (int i = 0; i < p_interleave->dimm_count; i++)
		{
			printf("\tdimm handle: 0x%x\n", p_interleave->dimms[i]);
			printf("\tdimm region address: 0x%llx\n", p_interleave->dimm_region_pdas[i]);
			printf("\tdimm region offset: 0x%llx\n", p_interleave->dimm_region_offsets[i]);
			printf("\tdimm size: 0x%llx\n", p_interleave->dimm_sizes[i]);
			printf("\n");
		}
	}
}
