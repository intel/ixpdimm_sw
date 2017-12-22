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
#include <utility.h>

// Used when checking for duplicate serial numbers
struct serial_number_count_list_item
{
	NVM_SERIAL_NUMBER serial_number;
	size_t count;
	struct serial_number_count_list_item *p_next;
};

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
		struct nvm_details *smbios_dimms = malloc(smbios_dimm_count * sizeof(struct nvm_details));
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

        free(smbios_dimms);
	}
	else if (rc < 0)
	{
		COMMON_LOG_ERROR_F("error getting SMBIOS table 17 count, rc = %d", rc);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void generate_event_for_duplicate_serial_numbers(const size_t duplicate_count,
		const NVM_SERIAL_NUMBER duplicate_serial)
{
	COMMON_LOG_ENTRY();

	char duplicate_count_str[NVM_EVENT_ARG_LEN];
	s_snprintf(duplicate_count_str, sizeof (duplicate_count_str),
			"%llu", duplicate_count);

	char duplicate_serial_str[NVM_SERIALSTR_LEN];
	SERIAL_NUMBER_TO_STRING(duplicate_serial, duplicate_serial_str);

	store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
			EVENT_SEVERITY_CRITICAL,
			EVENT_CODE_DIAG_PCONFIG_DUPLICATE_SERIAL_NUMBERS,
			"",
			0,
			duplicate_count_str,
			duplicate_serial_str,
			"",
			DIAGNOSTIC_RESULT_FAILED);

	COMMON_LOG_EXIT();
}

NVM_UINT32 add_events_for_all_duplicate_serial_numbers_in_list(
		struct serial_number_count_list_item *p_list)
{
	COMMON_LOG_ENTRY();

	NVM_UINT32 events_added = 0;
	struct serial_number_count_list_item *p_item = p_list;
	while (p_item)
	{
		if (p_item->count > 1)
		{
			generate_event_for_duplicate_serial_numbers(p_item->count,
					p_item->serial_number);
			events_added++;
		}

		p_item = p_item->p_next;
	}

	COMMON_LOG_EXIT_RETURN_I(events_added);
	return events_added;
}

struct serial_number_count_list_item *get_serial_number_item_from_list(
		const NVM_SERIAL_NUMBER serial_number,
		struct serial_number_count_list_item *p_list)
{
	COMMON_LOG_ENTRY();

	struct serial_number_count_list_item *p_item = p_list;
	while (p_item != NULL)
	{
		if (memcmp(serial_number, p_item->serial_number,
				sizeof (NVM_SERIAL_NUMBER)) == 0)
		{
			break;
		}
		p_item = p_item->p_next;
	}

	COMMON_LOG_EXIT();
	return p_item;
}

void add_serial_number_to_list(const NVM_SERIAL_NUMBER serial_number,
		struct serial_number_count_list_item **pp_list)
{
	COMMON_LOG_ENTRY();

	struct serial_number_count_list_item *p_item =
			get_serial_number_item_from_list(serial_number, *pp_list);
	if (p_item)
	{
		p_item->count++;
	}
	else
	{
		p_item = calloc(1, sizeof (struct serial_number_count_list_item));
		memmove(p_item->serial_number, serial_number, NVM_SERIAL_LEN);
		p_item->count = 1;

		// Inject at the beginning
		p_item->p_next = *pp_list;
		*pp_list = p_item;
	}

	COMMON_LOG_EXIT();
}

void add_device_serial_numbers_to_list(struct serial_number_count_list_item **pp_list,
		const struct nvm_topology *devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();

	for (NVM_UINT32 i = 0; i < device_count; i++)
	{
		add_serial_number_to_list(devices[i].serial_number, pp_list);
	}

	COMMON_LOG_EXIT();
}

// Deletes all items and sets *pp_list to NULL
void delete_serial_number_count_list(struct serial_number_count_list_item **pp_list)
{
	COMMON_LOG_ENTRY();

	while (*pp_list)
	{
		struct serial_number_count_list_item *p_item = *pp_list;
		*pp_list = p_item->p_next;
		free(p_item);
	}

	COMMON_LOG_EXIT();
}

void check_for_duplicate_serial_numbers(NVM_UINT32 *p_results,
		const struct nvm_topology *devices, const NVM_UINT32 device_count)
{
	COMMON_LOG_ENTRY();

	struct serial_number_count_list_item *p_list = NULL;
	add_device_serial_numbers_to_list(&p_list, devices, device_count);

	(*p_results) += add_events_for_all_duplicate_serial_numbers_in_list(p_list);

	delete_serial_number_count_list(&p_list);

	COMMON_LOG_EXIT();
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
				EVENT_SEVERITY_INFO, EVENT_CODE_DIAG_PCONFIG_NO_DIMMS, NULL, 0, NULL,
				NULL, NULL, DIAGNOSTIC_RESULT_ABORTED);
		(*p_results)++;
	}
	else
	{
		// get_topology verifies NFIT table
		struct nvm_topology *topol = malloc(*p_dev_count * sizeof(struct nvm_topology));
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
			check_for_duplicate_serial_numbers(p_results, topol, *p_dev_count);
		}

        free(topol);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
