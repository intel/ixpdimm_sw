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

void add_ifc_to_list(const unsigned short ifc, unsigned short *p_ifc_list,
		const size_t ifc_list_length)
{
	for (size_t i = 0; i < ifc_list_length; i++)
	{
		if (p_ifc_list[i] == ifc) // already in list
		{
			break;
		}
		else if (p_ifc_list[i] == 0)
		{
			p_ifc_list[i] = ifc;
			break;
		}
	}
}

void add_all_dimm_format_codes_from_nfit(const struct parsed_nfit *p_parsed_nfit,
		struct nfit_dimm *p_dimm)
{
	for (int i = 0; i < p_parsed_nfit->control_region_count; i++)
	{
		if (p_parsed_nfit->control_region_list[i].serial_number == p_dimm->serial_number)
		{
			add_ifc_to_list(p_parsed_nfit->control_region_list[i].ifc,
					p_dimm->ifc, NFIT_MAX_IFC_COUNT);
		}
	}
}

int add_dimm_control_region_info_from_index(const struct parsed_nfit *p_parsed_nfit,
		const unsigned short index, struct nfit_dimm *p_dimm)
{
	int result = NFIT_ERR_BADNFIT;
	for (int i = 0; i < p_parsed_nfit->control_region_count; i++)
	{
		if (p_parsed_nfit->control_region_list[i].index == index)
		{
			p_dimm->serial_number = p_parsed_nfit->control_region_list[i].serial_number;
			p_dimm->vendor_id = p_parsed_nfit->control_region_list[i].vendor_id;
			p_dimm->device_id = p_parsed_nfit->control_region_list[i].device_id;
			p_dimm->revision_id = p_parsed_nfit->control_region_list[i].revision_id;
			p_dimm->subsystem_vendor_id =
				p_parsed_nfit->control_region_list[i].subsystem_vendor_id;
			p_dimm->subsystem_device_id =
				p_parsed_nfit->control_region_list[i].subsystem_device_id;
			p_dimm->subsystem_revision_id =
				p_parsed_nfit->control_region_list[i].subsystem_revision_id;
			p_dimm->valid_fields = p_parsed_nfit->control_region_list[i].valid_fields;
			p_dimm->manufacturing_location =
				p_parsed_nfit->control_region_list[i].manufacturing_location;
			p_dimm->manufacturing_date =
				p_parsed_nfit->control_region_list[i].manufacturing_date;

			add_all_dimm_format_codes_from_nfit(p_parsed_nfit, p_dimm);

			result = NFIT_SUCCESS;
		}
	}
	return result;

}

int add_dimm_info_from_handle(const struct parsed_nfit *p_parsed_nfit,
		const unsigned int handle, struct nfit_dimm *p_dimm)
{
	int result = NFIT_ERR_BADNFIT;
	for (int i = 0; i < p_parsed_nfit->region_mapping_count; i++)
	{
		if (p_parsed_nfit->region_mapping_list[i].handle == handle)
		{
			p_dimm->handle = handle;
			p_dimm->physical_id = p_parsed_nfit->region_mapping_list[i].physical_id;
			p_dimm->state_flags = p_parsed_nfit->region_mapping_list[i].state_flag;
			result = add_dimm_control_region_info_from_index(p_parsed_nfit,
					p_parsed_nfit->region_mapping_list[i].control_region_index,
					p_dimm);
			break;
		}
	}
	return result;
}

unsigned char has_matching_control_region_table(const struct parsed_nfit *p_parsed_nfit,
		const unsigned short index)
{
	int found = 0;
	for (int i = 0; i < p_parsed_nfit->control_region_count; i++)
	{
		if (p_parsed_nfit->control_region_list[i].index == index)
		{
			found = 1;
			break;
		}
	}
	return found;
}


int get_unique_dimm_region_handles(const struct parsed_nfit *p_parsed_nfit,
		unsigned int *p_unique_handles)
{
	int dimm_count = 0;
	for (int i = 0; i < p_parsed_nfit->region_mapping_count; i++)
	{
		unsigned char is_unique = 1;
		for (int j = 0; j < dimm_count && is_unique; j++)
		{
			if (p_parsed_nfit->region_mapping_list[i].handle == p_unique_handles[j])
			{
				is_unique = 0;
			}
		}
		if (is_unique)
		{
			// make sure it has a matching control region
			if (!has_matching_control_region_table(p_parsed_nfit,
					p_parsed_nfit->region_mapping_list[i].control_region_index))
			{
				dimm_count = NFIT_ERR_BADNFIT;
				break;
			}
			else
			{
				p_unique_handles[dimm_count] = p_parsed_nfit->region_mapping_list[i].handle;
				dimm_count++;
			}
		}
	}
	return dimm_count;
}

int nfit_get_dimms_from_parsed_nfit(const int count,
		struct nfit_dimm *p_nfit_dimms,
		const struct parsed_nfit *p_parsed_nfit)
{
	int result = 0;
	if (p_parsed_nfit->region_mapping_count)
	{
		unsigned int unique_handles[p_parsed_nfit->region_mapping_count];
		memset(unique_handles, 0, (sizeof (unsigned int) * p_parsed_nfit->region_mapping_count));
		result = get_unique_dimm_region_handles(p_parsed_nfit, unique_handles);
		if (count) // retrieve dimms structs
		{
			if (count < result || !p_nfit_dimms)
			{
				result = NFIT_ERR_BADINPUT;
			}
			else
			{
				memset(p_nfit_dimms, 0, sizeof (struct nfit_dimm) * count);
				for (int i = 0; i < result; i++)
				{
					int add_result = add_dimm_info_from_handle(p_parsed_nfit,
							unique_handles[i], &p_nfit_dimms[i]);
					if (add_result != NFIT_SUCCESS)
					{
						result = add_result;
						break;
					}
				}
			}
		}
	}
	return result;
}

/*
 * Retrieves a list of DIMMs from the NFIT and returns the count.
 * If input count is 0, simply retrieve the count from the NFIT.
 */
int nfit_get_dimms(const int count, struct nfit_dimm *p_nfit_dimms)
{
	// get the parsed nfit
	struct parsed_nfit *p_parsed_nfit;
	int result = nfit_get_parsed_nfit(&p_parsed_nfit);
	if (result == NFIT_SUCCESS)
	{
		result = nfit_get_dimms_from_parsed_nfit(count, p_nfit_dimms, p_parsed_nfit);
		free_parsed_nfit(p_parsed_nfit);
	}
	return result;
}

void nfit_print_dimm(const struct nfit_dimm *p_dimm)
{
	if (p_dimm)
	{
		printf("handle: 0x%x\n", p_dimm->handle);
		printf("physical_id: 0x%x\n", p_dimm->physical_id);
		printf("device_id: 0x%x\n", p_dimm->device_id);
		printf("vendor_id: 0x%x\n", p_dimm->vendor_id);
		printf("revision_id: 0x%x\n", p_dimm->revision_id);
		printf("subsystem_device_id: 0x%x\n", p_dimm->subsystem_device_id);
		printf("subsystem_vendor_id: 0x%x\n", p_dimm->subsystem_vendor_id);
		printf("subsystem_revision_id: 0x%x\n", p_dimm->subsystem_revision_id);
		printf("valid_fields: 0x%x\n", p_dimm->valid_fields);
		printf("manufacturing_date: 0x%x\n", p_dimm->manufacturing_date);
		printf("manufacturing_location: 0x%x\n", p_dimm->manufacturing_location);
		for (int i = 0; i < NFIT_MAX_IFC_COUNT; i++)
		{
			printf("ifc[%d]: 0x%x\n", i, p_dimm->ifc[i]);
		}
		printf("\n");
	}
}
