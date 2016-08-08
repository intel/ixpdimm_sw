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
 * This file contains the implementation of the platform configuration diagnostic
 * for the native API.
 */

#include "nvm_management.h"
#include "diagnostic.h"
#include <persistence/logging.h>
#include <persistence/event.h>
#include "device_adapter.h"
#include <string/s_str.h>
#include "platform_config_data.h"
#include "utility.h"
#include "config_goal.h"
#include "capabilities.h"
#include "pool_utilities.h"

int get_nvm_capabilities_from_pcat(NVM_UINT32 *p_results, struct nvm_capabilities *nvm_caps);
int verify_pcd(int dev_count, const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results);
void check_bios_config_support(NVM_UINT32 *p_results, const struct platform_capabilities *p_caps);

#define	MEMORY_EVENT_ARG	"memory"
#define	APP_DIRECT_EVENT_ARG	"App Direct"
#define	MEMORY_CAP_STR		"Memory Mode"
#define	APP_DIRECT_CAP_STR	"App Direct"

/*
 * Run the platform configuration check diagnostic
 */
int diag_platform_config_check(const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	*p_results = 0;

	// clear previous results
	diag_clear_results(EVENT_TYPE_DIAG_PLATFORM_CONFIG, 0, NULL);

	if ((rc = IS_NVM_FEATURE_SUPPORTED(platform_config_diagnostic)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("The platform configuration diagnostic is not supported.");
	}
	else
	{
		// verify existence and format of nfit table
		int dev_count = 0;
		struct nvm_capabilities nvm_caps;
		if (!(p_diagnostic->excludes & DIAG_THRESHOLD_PCONFIG_NFIT))
		{
			KEEP_ERROR(rc, verify_nfit(&dev_count, p_results));
		}

		if (*p_results == 0) // NFIT is OK
		{
			if (!(p_diagnostic->excludes & DIAG_THRESHOLD_PCONFIG_PCAT))
			{
				//  verify contents of PCAT and get nvm capabilities
				if (get_nvm_capabilities_from_pcat(p_results, &nvm_caps) == NVM_SUCCESS)
				{
					//	check if BIOS is set to provisioning using mgmt sw
					check_bios_config_support(p_results, &nvm_caps.platform_capabilities);
				}
			}

			// verify the contents of PCD of all DIMM
			if (!(p_diagnostic->excludes & DIAG_THRESHOLD_PCONFIG_PCD))
			{
				KEEP_ERROR(rc, verify_pcd(dev_count, p_diagnostic, p_results));
			}

			// check best practices
			if (!(p_diagnostic->excludes & DIAG_THRESHOLD_PCONFIG_BEST_PRACTICES))
			{
				KEEP_ERROR(rc, check_platform_config_best_practices(p_results));
			}
		}

		if ((rc == NVM_SUCCESS) && (*p_results == 0)) // No errors/warnings
		{
			store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
					EVENT_SEVERITY_INFO, EVENT_CODE_DIAG_PCONFIG_SUCCESS, NULL, 0,
					NULL, NULL, NULL, DIAGNOSTIC_RESULT_OK);
			(*p_results)++;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * get nvm capabilities from the CR platform capability table
 */
int get_nvm_capabilities_from_pcat(NVM_UINT32 *p_results, struct nvm_capabilities *p_nvm_caps)
{
	COMMON_LOG_ENTRY();

	int rc = nvm_get_nvm_capabilities(p_nvm_caps);
	if (rc != NVM_SUCCESS)
	{
		// returns success only if the platform capability table is retrieved and
		// if the signature, revision and table checksum are verified.
		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN, EVENT_CODE_DIAG_PCONFIG_INVALID_PCAT, NULL, 0, NULL,
				NULL, NULL, DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * check if BIOS is set up to provisioning using management software
 */
void check_bios_config_support(NVM_UINT32 *p_results, const struct platform_capabilities *p_caps)
{
	COMMON_LOG_ENTRY();

	if (!p_caps->bios_config_support)
	{
		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG, EVENT_SEVERITY_INFO,
				EVENT_CODE_DIAG_PCONFIG_NO_BIOS_CONFIG_SUPPORT, NULL, 0, NULL,
				NULL, NULL, DIAGNOSTIC_RESULT_WARNING);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

/*
 * verify that all interleave sets described by current config data are complete
 */
void check_interleave_sets(struct current_config_table *p_current_config,
		NVM_UID dimm_uid, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();

	// check if current config table has extension table(s)
	if (p_current_config->header.length
			> sizeof (struct current_config_table))
	{
		NVM_UINT64 offset = 0;
		NVM_SIZE total_length =
				p_current_config->header.length
				- sizeof (struct current_config_table);

		// walk through the extension tables
		while (offset < total_length)
		{
			struct extension_table_header *p_header =
					(struct extension_table_header *)((NVM_UINT8 *)
							&p_current_config->p_ext_tables + offset);
			if (p_header->length == 0)
			{
				NVM_UID uid_str;
				uid_copy(dimm_uid, uid_str);
				COMMON_LOG_ERROR(
						"Invalid extension table, length is 0.");
				store_event_by_parts(
						EVENT_TYPE_DIAG_PLATFORM_CONFIG,
						EVENT_SEVERITY_WARN,
						EVENT_CODE_DIAG_PCONFIG_INVALID_CURRENT_PCD,
						dimm_uid, 0,
						uid_str, NULL, NULL,
						DIAGNOSTIC_RESULT_FAILED);
				(*p_results)++;
				break; // can't progress any further
			}
			else // valid extension table
			{
				if (p_header->type == INTERLEAVE_TABLE)
				{
					struct interleave_info_extension_table
					*p_interleave_info_tbl;
					struct dimm_info_extension_table *p_dimms;
					p_interleave_info_tbl =
							(struct interleave_info_extension_table *)
							p_header;
					p_dimms = (struct dimm_info_extension_table *)
										&p_interleave_info_tbl->p_dimms;
					for (int i = 0; i
					< p_interleave_info_tbl->dimm_count; i++)
					{
						struct device_discovery discovery;
						if (lookup_dev_manufacturer_serial_model(
								p_dimms[i].manufacturer,
								p_dimms[i].serial_number,
								p_dimms[i].model_number,
								&discovery) != NVM_SUCCESS)
						{
							char serial_str[NVM_SERIALSTR_LEN];
							SERIAL_NUMBER_TO_STRING(
									p_dimms[i].serial_number, serial_str);
							char set_index_str[10];
							s_snprintf(set_index_str, 10, "%hu",
									p_interleave_info_tbl->index);
							store_event_by_parts(
									EVENT_TYPE_DIAG_PLATFORM_CONFIG,
									EVENT_SEVERITY_WARN,
									EVENT_CODE_DIAG_PCONFIG_BROKEN_ISET,
									NULL, 0,
									set_index_str,
									serial_str, NULL,
									DIAGNOSTIC_RESULT_FAILED);
							(*p_results)++;
						}
					}
				}
				// go to next extension table
				offset += p_header->length;
			}
		}
	}

	COMMON_LOG_EXIT();
}

/*
 * Return the index where the uid is found in the pool's dimms array.
 * Return NVM_ERR_NOTFOUND if the uid is not in the array.
 */
int get_dimm_index_in_pool(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int result = NVM_ERR_NOTFOUND;

	for (int dimm_index = 0; dimm_index < p_pool->dimm_count &&
			dimm_index < NVM_MAX_DEVICES_PER_POOL; dimm_index++)
	{
		if (device_handle.handle == p_pool->dimms[dimm_index].handle)
		{
			result = dimm_index;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

/*
 * return true/false depending on whether or not the dimm belongs to the interleave set
 */
NVM_BOOL dimm_is_in_interleave_set(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_interleave_set *p_ilset)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	for (int dimm_index = 0; dimm_index < p_ilset->dimm_count; dimm_index++)
	{
		if (device_handle.handle == p_ilset->dimms[dimm_index].handle)
		{
			result = 1;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

/*
 * find mapped memory capacity of this dimm
 */
void get_mapped_memory_capacity(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_pool *pools, const int pool_count, NVM_UINT64 *p_memory_capacity)
{
	COMMON_LOG_ENTRY();

	int index = 0;

	for (int pool_index = 0; pool_index < pool_count; pool_index++)
	{
		if (pools[pool_index].type == POOL_TYPE_VOLATILE &&
				(index = get_dimm_index_in_pool(device_handle,
						&(pools[pool_index]))) != NVM_ERR_NOTFOUND)
		{
			*p_memory_capacity += pools[pool_index].memory_capacities[index];
			break;
		}
	}

	COMMON_LOG_EXIT();
}

/*
 * find mapped app direct capacity of this dimm
 */
void get_mapped_app_direct_capacity(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_pool *pools, const int pool_count, NVM_UINT64 *p_app_direct_capacity)
{
	COMMON_LOG_ENTRY();

	for (int pool_index = 0; pool_index < pool_count; pool_index++)
	{
		struct nvm_pool pool = pools[pool_index];
		if (pool.type != POOL_TYPE_VOLATILE)
		{
			if (get_dimm_index_in_pool(device_handle, &pool) != NVM_ERR_NOTFOUND)
			{
				for (int iset_index = 0; iset_index < pool.ilset_count; iset_index++)
				{
					struct nvm_interleave_set iset = pool.ilsets[iset_index];
					if (dimm_is_in_interleave_set(device_handle, &iset))
					{
						// individual dimms in an interleave set contribute the
						// same amount to the interleave set
						NVM_UINT64 ilset_capacity = iset.size / (NVM_UINT64)iset.dimm_count;

						// the size of a mirrored interleave set on a dimm is reported
						// as the size requested by the user. Its actual size is twice
						// as large because it is mirrored.
						if (iset.mirrored)
						{
							ilset_capacity *= (NVM_UINT64)2;
						}

						*p_app_direct_capacity += ilset_capacity;
					}
				}
			}
		}
	}

	COMMON_LOG_EXIT();
}

void check_device_mapped_memory_capacity(NVM_UINT32 *p_results,
		const struct device_discovery *p_device,
		const struct current_config_table *p_current_config,
		const struct nvm_pool *p_pools, const NVM_UINT32 pool_count)
{
	COMMON_LOG_ENTRY();

	NVM_UINT64 memory_capacity = 0;
	get_mapped_memory_capacity(p_device->device_handle, p_pools, pool_count, &memory_capacity);

	// compare mapped memory capacity reported by
	// the bios to that by the driver
	if (p_current_config->mapped_memory_capacity != memory_capacity)
	{
		char mapped_vcap_str[NVM_EVENT_ARG_LEN];
		char vcap_str[NVM_EVENT_ARG_LEN];
		s_snprintf(mapped_vcap_str,
				NVM_EVENT_ARG_LEN, "%llu", p_current_config->mapped_memory_capacity);
		s_snprintf(vcap_str,
		NVM_EVENT_ARG_LEN, "%llu", memory_capacity);

		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_PCONFIG_MAPPED_CAPACITY,
				p_device->uid,
				0,
				MEMORY_CAP_STR,
				mapped_vcap_str,
				vcap_str,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

void check_device_mapped_app_direct_capacity(NVM_UINT32 *p_results,
		const struct device_discovery *p_device,
		const struct current_config_table *p_current_config,
		const struct nvm_pool *p_pools, const NVM_UINT32 pool_count)
{
	COMMON_LOG_ENTRY();

	NVM_UINT64 app_direct_capacity = 0;
	get_mapped_app_direct_capacity(p_device->device_handle, p_pools, pool_count,
			&app_direct_capacity);

	// compare mapped app direct capacity reported
	// by the bios to that by the driver
	if (p_current_config->mapped_app_direct_capacity != app_direct_capacity)
	{
		char mapped_app_direct_str[NVM_EVENT_ARG_LEN];
		char app_direct_str[NVM_EVENT_ARG_LEN];
		s_snprintf(mapped_app_direct_str,
				NVM_EVENT_ARG_LEN, "%llu", p_current_config->mapped_app_direct_capacity);
		s_snprintf(app_direct_str,
				NVM_EVENT_ARG_LEN, "%llu", app_direct_capacity);


		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_PCONFIG_MAPPED_CAPACITY,
				p_device->uid,
				0,
				APP_DIRECT_CAP_STR,
				mapped_app_direct_str,
				app_direct_str,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

int check_mapped_capacities_for_device(NVM_UINT32 *p_results,
		const struct device_discovery *p_device,
		const struct current_config_table *p_current_config)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;
	int pool_count = get_pool_count();
	if (pool_count > 0)
	{
		struct nvm_pool *pools = (struct nvm_pool *)calloc(pool_count, sizeof (struct nvm_pool));
		if (pools == NULL)
		{
			COMMON_LOG_ERROR("Could not allocate memory for pools.");
			rc = NVM_ERR_NOMEMORY;
		}
		else
		{
			pool_count = get_pools(pool_count, pools);
			if (pool_count > 0)
			{
				check_device_mapped_memory_capacity(p_results, p_device,
						p_current_config, pools, pool_count);

				check_device_mapped_app_direct_capacity(p_results, p_device,
						p_current_config, pools, pool_count);
			}

			free(pools);
		}
	}

	if (pool_count < 0)
	{
		COMMON_LOG_ERROR_F(
			"Failed to retrieve the pool information, error %d",
			pool_count);

		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_PCONFIG_POOLS_FAILED,
				NULL,
				0,
				NULL,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_FAILED);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void check_unmapped_device_config(NVM_UINT32 *p_results,
		const NVM_UID uid, const struct current_config_table *p_current_config)
{
	COMMON_LOG_ENTRY();

	NVM_UID uid_str;
	uid_copy(uid, uid_str);

	if ((p_current_config->config_status == CURRENT_CONFIG_STATUS_UNCONFIGURED) ||
			(p_current_config->config_status == CURRENT_CONFIG_STATUS_ERROR_UNMAPPED))
	{
		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_WARN,
				EVENT_CODE_DIAG_PCONFIG_UNCONFIGURED,
				uid,
				0,
				uid_str, NULL, NULL,
				DIAGNOSTIC_RESULT_FAILED);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

int check_pcd_current_config_for_device(NVM_UINT32 *p_results,
		const struct diagnostic *p_diagnostic,
		struct platform_config_data *p_config,
		struct device_discovery *p_device)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;
	struct current_config_table *p_current_config = cast_current_config(p_config);
	if (p_current_config)
	{
		// Config error
		if (p_current_config->config_status != CURRENT_CONFIG_STATUS_SUCCESS)
		{
			if (!(p_diagnostic->excludes & DIAG_THRESHOLD_PCONFIG_UNCONFIGURED))
			{
				check_unmapped_device_config(p_results, p_device->uid, p_current_config);
			}
		}
		else
		{
			if (!(p_diagnostic->excludes & DIAG_THRESHOLD_PCONFIG_MAPPED_CAPACITY))
			{
				rc = check_mapped_capacities_for_device(p_results, p_device, p_current_config);
			}
		}

		if (!(p_diagnostic->excludes & DIAG_THRESHOLD_PCONFIG_BROKEN_ISET))
		{
			// check for complete interleave sets
			check_interleave_sets(p_current_config, p_device->uid, p_results);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL config_goal_is_unapplied(const struct config_input_table *p_config_input,
		const struct config_output_table *p_config_output)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL unapplied = 0;

	// Config input table => There's a config goal
	if (p_config_input)
	{
		// Config output table => last applied config
		if (p_config_output)
		{
			if (p_config_input->sequence_number != p_config_output->sequence_number)
			{
				unapplied = 1;
			}
		}
		else // no output table => any config input is unapplied
		{
			unapplied = 1;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(unapplied);
	return unapplied;
}

/*
 * Generate events for new config goals that have not yet been applied to create pools.
 */
void check_for_unapplied_config_goal(NVM_UINT32 *p_results,
		const struct device_discovery *p_device,
		struct platform_config_data *p_config)
{
	COMMON_LOG_ENTRY();

	struct config_input_table *p_config_input = cast_config_input(p_config);
	struct config_output_table *p_config_output = cast_config_output(p_config);

	if (config_goal_is_unapplied(p_config_input, p_config_output))
	{
		NVM_UID uid_str;
		uid_copy(p_device->uid, uid_str);

		store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
				EVENT_SEVERITY_INFO,
				EVENT_CODE_DIAG_PCONFIG_REBOOT_NEEDED_TO_APPLY_GOAL,
				p_device->uid,
				0,
				uid_str,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_WARNING);
		(*p_results)++;
	}

	COMMON_LOG_EXIT();
}

/*
 * Generate events for a dimm current configuration that violate the SKU
 */
void check_current_sku_violations(NVM_UINT32 *p_results,
		const struct device_discovery *p_device,
		struct platform_config_data *p_config,
		const struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();

	// check current config for SKU violations
	struct current_config_table *p_current_config = cast_current_config(p_config);
	if (p_current_config)
	{
		NVM_UID uid_str;
		uid_copy(p_device->uid, uid_str);

		// DIMM is configured with memory but memory mode is not supported
		if (p_current_config->mapped_memory_capacity > 0 &&
			(!p_capabilities->nvm_features.memory_mode ||
			!p_device->device_capabilities.memory_mode_capable))
		{
			COMMON_LOG_DEBUG_F(NVM_DIMM_NAME" %s has mapped memory capacity "
					"but memory mode is not supported.", uid_str);
			store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
					EVENT_SEVERITY_CRITICAL,
					EVENT_CODE_DIAG_PCONFIG_DIMM_SKU_VIOLATION,
					p_device->uid,
					0,
					uid_str,
					MEMORY_EVENT_ARG,
					NULL,
					DIAGNOSTIC_RESULT_FAILED);
			(*p_results)++;
		}

		// DIMM is configured with app direct but app direct mode is not supported
		if (p_current_config->mapped_app_direct_capacity > 0 &&
			(!p_capabilities->nvm_features.app_direct_mode ||
			!p_device->device_capabilities.app_direct_mode_capable))
		{
			COMMON_LOG_DEBUG_F(NVM_DIMM_NAME" %s has mapped app direct memory "
					"but app direct mode is not supported.", uid_str);
			store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
					EVENT_SEVERITY_CRITICAL,
					EVENT_CODE_DIAG_PCONFIG_DIMM_SKU_VIOLATION,
					p_device->uid,
					0,
					uid_str,
					APP_DIRECT_EVENT_ARG,
					NULL,
					DIAGNOSTIC_RESULT_FAILED);
			(*p_results)++;
		}
	}

	COMMON_LOG_EXIT();
}

/*
 * Generate events for a dimm goal that violates the SKU
 */
void check_goal_sku_violations(NVM_UINT32 *p_results,
		const struct device_discovery *p_device,
		struct platform_config_data *p_config,
		const struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();

	struct config_input_table *p_config_input = cast_config_input(p_config);
	if (p_config_input)
	{
		// convert config input table into config_goal
		struct config_goal goal;
		memset(&goal, 0, sizeof (goal));
		// bad config goals are captured elsewhere in this diagnostic,
		// just proceed with SKU violation check if valid
		if (config_input_table_to_config_goal(p_device->uid,
				(unsigned char *)p_config_input, sizeof (struct config_input_table),
				p_config_input->header.length, &goal) == NVM_SUCCESS)
		{
			NVM_UID uid_str;
			uid_copy(p_device->uid, uid_str);

			// DIMM goal contains memory mode capacity but memory mode is not supported
			if (goal.memory_size > 0 &&
				(!p_capabilities->nvm_features.memory_mode ||
				!p_device->device_capabilities.memory_mode_capable))
			{
				COMMON_LOG_DEBUG_F(NVM_DIMM_NAME" %s has a goal with requested memory capacity "
						"but memory mode is not supported.", uid_str);
				store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
						EVENT_SEVERITY_CRITICAL,
						EVENT_CODE_DIAG_PCONFIG_DIMM_GOAL_SKU_VIOLATION,
						p_device->uid,
						0,
						uid_str,
						MEMORY_EVENT_ARG,
						NULL,
						DIAGNOSTIC_RESULT_FAILED);
				(*p_results)++;
			}

			// DIMM goal contains mapped app direct but mode is not supported
			if (goal.app_direct_count > 0 &&
				(!p_capabilities->nvm_features.app_direct_mode ||
				!p_device->device_capabilities.app_direct_mode_capable))
			{
				COMMON_LOG_DEBUG_F(NVM_DIMM_NAME" %s has a goal with mapped app direct memory "
						"but app direct mode is not supported.", uid_str);
				store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
						EVENT_SEVERITY_CRITICAL,
						EVENT_CODE_DIAG_PCONFIG_DIMM_GOAL_SKU_VIOLATION,
						p_device->uid,
						0,
						uid_str,
						APP_DIRECT_EVENT_ARG,
						NULL,
						DIAGNOSTIC_RESULT_FAILED);
				(*p_results)++;
			}
		}
	}

	COMMON_LOG_EXIT();
}

int check_platform_config_data_for_device(NVM_UINT32* p_results,
		struct device_discovery *p_device,
		const struct diagnostic *p_diagnostic)
{
	COMMON_LOG_ENTRY();

	NVM_UID uid_str;
	uid_copy(p_device->uid, uid_str);

	// fetch the platform config tables for the DIMM
	struct platform_config_data *p_config = NULL;
	int rc = get_dimm_platform_config(p_device->device_handle, &p_config);
	if (rc != NVM_SUCCESS)
	{
		if (!(p_diagnostic->excludes & DIAG_THRESHOLD_PCONFIG_PCD))
		{
			// fails if platform config data can't be retrieved or if any of platform
			// config header, current config , config input & output tables are invalid
			store_event_by_parts(EVENT_TYPE_DIAG_PLATFORM_CONFIG,
					EVENT_SEVERITY_WARN,
					EVENT_CODE_DIAG_PCONFIG_INVALID_PCD,
					p_device->uid,
					0,
					uid_str,
					NULL,
					NULL, DIAGNOSTIC_RESULT_FAILED);
			(*p_results)++;
		}
		// Since the error was processed as a diagnostic result
		rc = NVM_SUCCESS;
	}
	else
	{
		if (!(p_diagnostic->excludes & DIAG_THRESHOLD_PCONFIG_CURRENT_PCD))
		{
			KEEP_ERROR(rc,
					check_pcd_current_config_for_device(p_results,
							p_diagnostic, p_config, p_device));
		}

		check_for_unapplied_config_goal(p_results, p_device, p_config);

		struct nvm_capabilities capabilities;
		int tmprc = nvm_get_nvm_capabilities(&capabilities);
		if (tmprc != NVM_SUCCESS)
		{
			KEEP_ERROR(rc, tmprc);
		}
		else
		{
			// check for SKU violations
			check_current_sku_violations(p_results, p_device, p_config, &capabilities);
			check_goal_sku_violations(p_results, p_device, p_config, &capabilities);
		}

	}

	if (p_config)
	{
		free(p_config);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * verify the platform config data
 */
int verify_pcd(int dev_count, const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (dev_count > 0)
	{
		// get device_discovery information of all dimms
		struct device_discovery dimms[dev_count];
		dev_count = nvm_get_devices(dimms, dev_count);
		if (dev_count > 0)
		{
			rc = NVM_SUCCESS;
			for (int current_dev = 0; current_dev < dev_count; current_dev++)
			{
				// don't bother with unmanageable DIMMs
				if (dimms[current_dev].manageability == MANAGEMENT_VALIDCONFIG)
				{
					KEEP_ERROR(rc,
							check_platform_config_data_for_device(p_results,
								&(dimms[current_dev]),
								p_diagnostic));
				}
			}
		}
		else // nvm_get_devices failed
		{
			rc = dev_count;
		}
	}
	else // nvm_get_device_count failed
	{
		rc = dev_count;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
