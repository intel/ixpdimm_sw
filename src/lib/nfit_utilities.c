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
 * This file contains the implementation of the diagnostic entry point and common
 * diagnostic helper functions for the native API.
 */

#include "nfit_utilities.h"
#include <persistence/logging.h>
#include <string.h>
#include "device_utilities.h"
#include <uid/uid.h>
#include "nvm_context.h"
#include "nvm_types.h"

/*
 * Convert NFIT library error the NVM library error
 */
int nfit_err_to_lib_err(const enum nfit_error err)
{
	int rc = NVM_SUCCESS;
	switch (err)
	{
		case NFIT_SUCCESS:
			rc = NVM_SUCCESS;
			break;
		case NFIT_ERR_TABLENOTFOUND:
			COMMON_LOG_ERROR("The NFIT table was not found.");
			rc = NVM_ERR_BADNFIT;
			break;
		case NFIT_ERR_CHECKSUMFAIL:
			COMMON_LOG_ERROR("The NFIT table checksum failed.");
			rc = NVM_ERR_BADNFIT;
			break;
		case NFIT_ERR_BADNFIT:
			COMMON_LOG_ERROR("The NFIT table was corrupted.");
			rc = NVM_ERR_BADNFIT;
			break;
		case NFIT_ERR_NOMEMORY:
			COMMON_LOG_ERROR("No memory to retrieve the NFIT table.");
			rc = NVM_ERR_NOMEMORY;
			break;
		case NFIT_ERR_BADINPUT:
			COMMON_LOG_ERROR("Bad input to the NFIT library.");
			rc = NVM_ERR_BADNFIT;
			break;
		default:
			COMMON_LOG_ERROR("An unknown NFIT error occurred.");
			rc = NVM_ERR_BADNFIT;
			break;
	}
	return rc;
}

/*
 * Retrieve the NFIT table from ACPI and parse it.
 * The caller is responsible for freeing the parsed_nfit structure.
 */
int get_parsed_nfit(struct parsed_nfit **pp_parsed_nfit)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_BADNFIT;
	int nfit_size = get_nvm_context_nfit_size();
	if (nfit_size > 0)
	{
		*pp_parsed_nfit = calloc(1, nfit_size);
		if (!(*pp_parsed_nfit))
		{
			COMMON_LOG_ERROR("Not enough memory to retrieve the NFIT");
			rc = NVM_ERR_NOMEMORY;
		}
		else
		{
			rc = get_nvm_context_nfit(nfit_size, *pp_parsed_nfit);
		}
	}
	else
	{
		nfit_size = nfit_get_parsed_nfit(pp_parsed_nfit);
		if (nfit_size && *pp_parsed_nfit)
		{
			rc = set_nvm_context_nfit(nfit_size, *pp_parsed_nfit);
		}
		else
		{
			rc = nfit_err_to_lib_err(nfit_size);
		}
	}
	return rc;
}

/*
 * Convert NFIT dimm to NVM topology struct
 */
void nfit_dimm_to_nvm_topology(const struct nfit_dimm *p_nfit_dimm,
		struct nvm_topology *p_dimm_topo)
{
	p_dimm_topo->device_handle.handle = p_nfit_dimm->handle;
	p_dimm_topo->id = p_nfit_dimm->physical_id;
	// swap bytes for JEDEC compatibility
	memmove(&p_dimm_topo->serial_number, &p_nfit_dimm->serial_number,
			sizeof (NVM_SERIAL_NUMBER));
	swap_bytes((unsigned char *)&p_dimm_topo->vendor_id,
			(unsigned char *)&p_nfit_dimm->vendor_id,
			sizeof (p_dimm_topo->vendor_id));
	swap_bytes((unsigned char *)&p_dimm_topo->device_id,
			(unsigned char *)&p_nfit_dimm->device_id,
			sizeof (p_nfit_dimm->device_id));
	swap_bytes((unsigned char *)&p_dimm_topo->subsystem_vendor_id,
			(unsigned char *)&p_nfit_dimm->subsystem_vendor_id,
			sizeof (p_dimm_topo->subsystem_vendor_id));
	swap_bytes((unsigned char *)&p_dimm_topo->subsystem_device_id,
			(unsigned char *)&p_nfit_dimm->subsystem_device_id,
			sizeof (p_dimm_topo->subsystem_device_id));
	swap_bytes((unsigned char *)&p_dimm_topo->manufacturing_location,
			(unsigned char *)&p_nfit_dimm->manufacturing_location,
		sizeof (p_dimm_topo->manufacturing_location));
	swap_bytes((unsigned char *)&p_dimm_topo->manufacturing_date,
			(unsigned char *)&p_nfit_dimm->manufacturing_date,
			sizeof (p_dimm_topo->manufacturing_date));
	p_dimm_topo->revision_id = p_nfit_dimm->revision_id;
	p_dimm_topo->subsystem_revision_id = p_nfit_dimm->subsystem_revision_id;
	p_dimm_topo->manufacturing_info_valid = p_nfit_dimm->valid_fields;

	for (int j = 0; j < NFIT_MAX_IFC_COUNT && j < NVM_MAX_IFCS_PER_DIMM; j++)
	{
		p_dimm_topo->fmt_interface_codes[j] = p_nfit_dimm->ifc[j];
	}
}



/*
 * Get the number of DIMMs in the system's memory topology
 * directly from the NFIT table
 */
int get_topology_count_from_nfit()
{
	COMMON_LOG_ENTRY();
	struct parsed_nfit *p_nfit = NULL;
	int rc = get_parsed_nfit(&p_nfit);
	if (rc == NVM_SUCCESS && p_nfit)
	{
		int topo_count = nfit_get_dimms_from_parsed_nfit(0, NULL, p_nfit);
		if (topo_count < 0)
		{
			rc = nfit_err_to_lib_err(topo_count);
		}
		else
		{
			rc = topo_count;
		}
		free(p_nfit);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the system's memory topology directly from the NFIT
 */
int get_topology_from_nfit(const NVM_UINT8 count, struct nvm_topology *p_dimm_topo)
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	if (count == 0 || p_dimm_topo == NULL)
	{
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		struct parsed_nfit *p_nfit = NULL;
		rc = get_parsed_nfit(&p_nfit);
		if (rc == NVM_SUCCESS && p_nfit)
		{
			rc = get_topology_count_from_nfit();
			if (rc > 0)
			{
				int topo_count = rc;
				struct nfit_dimm nfit_dimms[topo_count];
				memset(nfit_dimms, 0, sizeof (struct nfit_dimm) * topo_count);
				topo_count = nfit_get_dimms_from_parsed_nfit(topo_count, nfit_dimms, p_nfit);
				if (topo_count < 0)
				{
					rc = nfit_err_to_lib_err(topo_count);
				}
				else if (count < topo_count)
				{
					COMMON_LOG_ERROR("Array is too small for all the dimms");
					rc = NVM_ERR_ARRAYTOOSMALL;
				}
				else if (topo_count > 0)
				{
					rc = topo_count;
					// convert nfit dimm struct to topology struct
					memset(p_dimm_topo, 0, sizeof (struct nvm_topology) * count);
					for (int i = 0; i < topo_count; i++)
					{
						nfit_dimm_to_nvm_topology(&nfit_dimms[i], &p_dimm_topo[i]);
					}
				}
			}
			free(p_nfit);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * Get the number of interleave sets from the NFIT
 */
int get_interleave_set_count_from_nfit()
{
	COMMON_LOG_ENTRY();
	struct parsed_nfit *p_nfit = NULL;
	int rc = get_parsed_nfit(&p_nfit);
	if (rc == NVM_SUCCESS && p_nfit)
	{
		int count = nfit_get_interleave_sets_from_parsed_nfit(0, NULL, p_nfit);
		if (count < 0)
		{
			rc = nfit_err_to_lib_err(count);
		}
		else
		{
			rc = count;
		}
		free(p_nfit);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
/*
 * Copies all of the data from the NFIT interleaved set into the corresponding
 * parameters in the NVM interleaved set.
 *
 * Returns: NVM_SUCCESS if success
 *          NVM_ERR_INTERLEAVESET if dimms from more than one socket are on
 *                                the same interleaved set
 */
int nfit_iset_to_nvm_iset(const struct nfit_interleave_set *p_nfit_iset,
		struct nvm_interleave_set *p_nvm_iset)
{
	int rc = NVM_SUCCESS;
	p_nvm_iset->id = p_nfit_iset->id;
	p_nvm_iset->size = p_nfit_iset->size;
	p_nvm_iset->attributes = p_nfit_iset->attributes;
	p_nvm_iset->dimm_count = p_nfit_iset->dimm_count;

	// Set socket id based on the first dimm in the interleaved set.
	// Check that the other dimms in the set are in the same socket in the
	// loop below
	if (p_nvm_iset->dimm_count > 0)
	{
		p_nvm_iset->socket_id = ((NVM_NFIT_DEVICE_HANDLE)p_nfit_iset->dimms[0]).parts.socket_id;
	}

	for (int i = 0; i < p_nvm_iset->dimm_count; i++)
	{
		p_nvm_iset->dimms[i] = p_nfit_iset->dimms[i];
		p_nvm_iset->dimm_region_pdas[i] = p_nfit_iset->dimm_region_pdas[i];
		p_nvm_iset->dimm_region_offsets[i] = p_nfit_iset->dimm_region_offsets[i];
		p_nvm_iset->dimm_sizes[i] = p_nfit_iset->dimm_sizes[i];
		if (p_nvm_iset->socket_id != ((NVM_NFIT_DEVICE_HANDLE)p_nfit_iset->dimms[i]).parts.socket_id)
		{
			// Interleaved sets should be only over one socket
			rc = NVM_ERR_INTERLEAVESET;
		}
	}
	return rc;
}

/*
 * Get the interleave sets from the NFIT
 */
int get_interleave_sets_from_nfit(const NVM_UINT8 count,
		struct nvm_interleave_set *p_interleaves)
{
	COMMON_LOG_ENTRY();
	int rc = 0;
	if (count == 0 || p_interleaves == NULL)
	{
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		struct parsed_nfit *p_nfit = NULL;
		rc = get_parsed_nfit(&p_nfit);
		if (rc == NVM_SUCCESS && p_nfit)
		{
			rc = get_interleave_set_count_from_nfit();
			if (rc > 0)
			{
				int set_count = rc;
				struct nfit_interleave_set nfit_isets[set_count];
				set_count = nfit_get_interleave_sets_from_parsed_nfit(set_count,
						nfit_isets, p_nfit);
				if (set_count < 0)
				{
					rc = nfit_err_to_lib_err(set_count);
				}
				else if (count < set_count)
				{
					COMMON_LOG_ERROR("Array is too small for all the dimms");
					rc = NVM_ERR_ARRAYTOOSMALL;
				}
				else if (set_count > 0)
				{
					rc = set_count;
					// convert nfit dimm struct to topology struct
					memset(p_interleaves, 0, sizeof (struct nvm_interleave_set) * count);
					for (int i = 0; i < set_count; i++)
					{
						if ((rc = nfit_iset_to_nvm_iset(&nfit_isets[i], &p_interleaves[i])) != NVM_SUCCESS)
						{
							break;
						}
					}
					if (rc == NVM_SUCCESS)
					{
						rc = set_count;
					}
				}
			}
			free(p_nfit);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
