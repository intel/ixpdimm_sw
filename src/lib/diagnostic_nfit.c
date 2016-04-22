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
 * This file contains the implementation of the platform configuration diagnostic's
 * NFIT/SMBIOS checks for the native API.
 */

#include "nvm_management.h"
#include "device_adapter.h"
#include <string/s_str.h>
#include <persistence/event.h>

/*
 * SMBIOS includes all memory devices in the system - not just NVM-DIMMs
 */
NVM_BOOL smbios_entry_is_nvmdimm(const struct nvm_details *p_smbios_inventory)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	if ((p_smbios_inventory->type == SMBIOS_MEMORY_TYPE_DDR4) &&
			(p_smbios_inventory->type_detail_bits & SMBIOS_MEMORY_TYPE_DETAIL_NONVOLATILE))
	{
		result = 1;
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

NVM_BOOL smbios_entry_is_in_topology(const struct nvm_details *p_smbios_inventory,
		const struct nvm_topology *p_devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL found = 0;

	NVM_UINT16 smbios_physical_id = p_smbios_inventory->id;
	for (NVM_UINT32 i = 0; i < device_count; i++)
	{
		if (p_devices[i].id == smbios_physical_id)
		{
			found = 1;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(found);
	return found;
}

void check_if_all_dimms_from_smbios_are_in_topology(NVM_UINT32 *p_results,
		const struct nvm_details *p_smbios_inventory, const NVM_UINT32 smbios_inv_count,
		const struct nvm_topology *p_devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();

	for (NVM_UINT32 i = 0; i < smbios_inv_count; i++)
	{
		if (smbios_entry_is_nvmdimm(&(p_smbios_inventory[i])) &&
				!smbios_entry_is_in_topology(&(p_smbios_inventory[i]), p_devices, device_count))
		{
			NVM_EVENT_ARG physical_id_str;
			s_snprintf(physical_id_str, NVM_EVENT_ARG_LEN, "%hu", p_smbios_inventory[i].id);

			store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
					EVENT_SEVERITY_CRITICAL,
					EVENT_CODE_DIAG_PCONFIG_DIMM_INIT_FAILED,
					NULL, // no way to generate a uid without NFIT info
					0,
					physical_id_str, NULL, NULL,
					DIAGNOSTIC_RESULT_FAILED);

			(*p_results)++;
		}
	}

	COMMON_LOG_EXIT();
}

int check_smbios_table_for_uninitialized_dimms(NVM_UINT32 *p_results,
		const struct nvm_topology *p_devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();
	int rc = get_smbios_inventory_count();
	if (rc > 0)
	{
		NVM_UINT8 smbios_dimm_count = rc;
		struct nvm_details smbios_dimms[smbios_dimm_count];
		rc = get_smbios_inventory(smbios_dimm_count, smbios_dimms);
		if (rc > 0)
		{
			rc = NVM_SUCCESS;

			check_if_all_dimms_from_smbios_are_in_topology(p_results,
					smbios_dimms, smbios_dimm_count, p_devices, device_count);
		}
		else if (rc < 0)
		{
			COMMON_LOG_ERROR_F("error getting SMBIOS table 17 inventory, rc = %d",
					rc);
		}
	}
	else if (rc < 0)
	{
		COMMON_LOG_ERROR_F("error getting SMBIOS table 17 count, rc = %d", rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * verify the existence and format of NFIT table
 */
int verify_nfit(int *p_dev_count, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	*p_dev_count = get_topology_count();
	if (*p_dev_count < 0)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN, EVENT_CODE_DIAG_PCONFIG_INVALID_NFIT, NULL, 0, NULL,
				NULL, NULL, DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}
	else if (*p_dev_count == 0)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN, EVENT_CODE_DIAG_PCONFIG_NO_DIMMS, NULL, 0, NULL,
				NULL, NULL, DIAGNOSTIC_RESULT_ABORTED);
		(*p_results)++;
	}
	else
	{
		// get_topology verifies NFIT table
		struct nvm_topology topol[*p_dev_count];
		int temprc = get_topology(*p_dev_count, topol);
		if (temprc < NVM_SUCCESS)
		{
			// can't retrieve topology if NFIT table is invalid
			store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
					EVENT_SEVERITY_WARN, EVENT_CODE_DIAG_PCONFIG_INVALID_NFIT, NULL, 0, NULL,
					NULL, NULL, DIAGNOSTIC_RESULT_FAILED);
			(*p_results)++;
		}
		else
		{
			if (temprc == 0)
			{
				store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
						EVENT_SEVERITY_WARN, EVENT_CODE_DIAG_PCONFIG_NO_DIMMS, NULL, 0, NULL,
						NULL, NULL, DIAGNOSTIC_RESULT_ABORTED);
				(*p_results)++;
			}

			rc = check_smbios_table_for_uninitialized_dimms(p_results, topol, *p_dev_count);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
