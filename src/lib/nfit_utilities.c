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

void swap_bytes(unsigned char *p_dest, unsigned char *p_src, size_t len)
{
	for (unsigned int i = 0; i < len; i++)
	{
		p_dest[(len - 1) - i] = p_src[i];
	}
}

/*
 * Get the number of DIMMs in the system's memory topology
 * directly from the NFIT table
 */
int get_topology_count_from_nfit()
{
	COMMON_LOG_ENTRY();
	int topo_count = nfit_get_dimms(0, NULL);
	if (topo_count < 0)
	{
		COMMON_LOG_ERROR("The NFIT could not be read");
		topo_count = NVM_ERR_BADNFIT;
	}
	COMMON_LOG_EXIT_RETURN_I(topo_count);
	return topo_count;
}

/*
 * Get the system's memory topology directly from the NFIT
 */
int get_topology_from_nfit(const NVM_UINT8 count, struct nvm_topology *p_dimm_topo)
{
	COMMON_LOG_ENTRY();
	int topo_count = 0;

	if (p_dimm_topo == NULL || count == 0)
	{
		topo_count = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((topo_count = get_topology_count_from_nfit()) > 0)
	{
		struct nfit_dimm nfit_dimms[topo_count];
		memset(nfit_dimms, 0, sizeof (struct nfit_dimm) * topo_count);
		topo_count = nfit_get_dimms(topo_count, nfit_dimms);
		if (topo_count < 0)
		{
			COMMON_LOG_ERROR("The NFIT could not be read");
			topo_count = NVM_ERR_BADNFIT;
		}
		else if (count < topo_count)
		{
			COMMON_LOG_ERROR("Array is too small for all the dimms");
			topo_count = NVM_ERR_ARRAYTOOSMALL;
		}
		else if (topo_count > 0)
		{
			// convert nfit dimm struct to topology struct
			memset(p_dimm_topo, 0, sizeof (struct nvm_topology) * count);
			for (int i = 0; i < topo_count; i++)
			{
				p_dimm_topo[i].device_handle.handle = nfit_dimms[i].handle;
				p_dimm_topo[i].id = nfit_dimms[i].physical_id;
				memmove(&p_dimm_topo[i].serial_number, &nfit_dimms[i].serial_number,
						sizeof (NVM_SERIAL_NUMBER));
				// swap bytes for JEDEC compatibility
				swap_bytes((unsigned char *)&p_dimm_topo[i].vendor_id,
						(unsigned char *)&nfit_dimms[i].vendor_id,
						sizeof (p_dimm_topo[i].vendor_id));
				p_dimm_topo[i].device_id = nfit_dimms[i].device_id;
				p_dimm_topo[i].revision_id = nfit_dimms[i].revision_id;
				swap_bytes((unsigned char *)&p_dimm_topo[i].subsystem_vendor_id,
						(unsigned char *)&nfit_dimms[i].subsystem_vendor_id,
						sizeof (p_dimm_topo[i].subsystem_vendor_id));
				p_dimm_topo[i].subsystem_device_id = nfit_dimms[i].subsystem_device_id;
				p_dimm_topo[i].subsystem_revision_id = nfit_dimms[i].subsystem_revision_id;
				p_dimm_topo[i].manufacturing_info_valid = nfit_dimms[i].valid_fields;
				swap_bytes((unsigned char *)&p_dimm_topo[i].manufacturing_location,
						(unsigned char *)&nfit_dimms[i].manufacturing_location,
					sizeof (p_dimm_topo[i].manufacturing_location));
				swap_bytes((unsigned char *)&p_dimm_topo[i].manufacturing_date,
						(unsigned char *)&nfit_dimms[i].manufacturing_date,
						sizeof (p_dimm_topo[i].manufacturing_date));
				p_dimm_topo[i].fmt_interface_codes[0] = nfit_dimms[i].ifc;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(topo_count);
	return topo_count;
}
