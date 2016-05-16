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
 * This file contains the implementation of native API functions to get pools
 * of capacity, dump configuration and load configuration.
 */

#include "nvm_management.h"
#include "device_adapter.h"
#include "pool_utilities.h"
#include "config_goal.h"
#include "utility.h"
#include "platform_config_data.h"
#include <uid/uid.h>
#include <persistence/lib_persistence.h>
#include <string/s_str.h>
#include <string/x_str.h>
#include <file_ops/file_ops_adapter.h>
#include "device_utilities.h"
#include "capabilities.h"
#include "nvm_context.h"
#include "system.h"

/*
 * Use the device handles in struct nvm_pool to get uids for struct pool
 */
int fill_in_device_uids(struct pool *p_pool, struct nvm_pool *p_nvm_pool)
{
	int rc = NVM_SUCCESS;
	int tmp_rc = NVM_SUCCESS;

	for (int i = 0; i < p_pool->dimm_count; i++)
	{
		struct device_discovery discovery;
		tmp_rc = lookup_dev_handle(p_nvm_pool->dimms[i], &discovery);
		if (tmp_rc != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F(
				"Failed to find the device from the device handle %u",
				p_nvm_pool->dimms[i].handle);
			rc = tmp_rc;
			break; // don't continue on error
		}
		else
		{
			memmove(p_pool->dimms[i], discovery.uid, NVM_MAX_UID_LEN);
		}
	}

	for (int i = 0; i < p_pool->ilset_count; i++)
	{
		for (int j = 0; j < p_pool->ilsets[i].dimm_count; j++)
		{
			struct device_discovery discovery;
			tmp_rc = lookup_dev_handle(p_nvm_pool->dimms[j], &discovery);
			if (tmp_rc != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F(
					"Failed to find the device from the device handle %u",
					p_nvm_pool->dimms[j].handle);
				rc = tmp_rc;
				break; // don't continue on error
			}
			else
			{
				memmove(p_pool->ilsets[i].dimms[j],
						discovery.uid, NVM_MAX_UID_LEN);
			}
		}
	}

	return rc;
}

// Macro used in interleave calculations - hold onto more severe health
#define	KEEP_INTERLEAVE_HEALTH(health, new_health)	\
{ \
	if ((new_health > health) || (health == INTERLEAVE_HEALTH_NORMAL)) \
	{ \
		health = new_health; \
	} \
}

/*
 * Calculate the health of an interleave set based on DIMM info.
 * Assumes the underlying DIMMs have already been populated in the struct.
 */
int calculate_interleave_health(struct interleave_set *p_interleave)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	enum interleave_set_health health = INTERLEAVE_HEALTH_NORMAL;
	for (int i = 0; i < p_interleave->dimm_count; i++)
	{
		struct device_status status;
		memset(&status, 0, sizeof (status));
		rc = nvm_get_device_status(p_interleave->dimms[i], &status);
		if (rc != NVM_SUCCESS)
		{
			if (rc == NVM_ERR_BADDEVICE) // DIMM is gone
			{
				health = INTERLEAVE_HEALTH_FAILED;
			}
			else
			{
				health = INTERLEAVE_HEALTH_UNKNOWN;
			}

			NVM_UID uid_str;
			uid_copy(p_interleave->dimms[i], uid_str);
			COMMON_LOG_ERROR_F("couldn't get status of underlying DIMM %s",
					uid_str);
			break;
		}

		// Interleave health should correlate with underlying DIMM health
		switch (status.health)
		{
		case DEVICE_HEALTH_NORMAL:
			// Ignore
			break;
		case DEVICE_HEALTH_NONCRITICAL:
		case DEVICE_HEALTH_CRITICAL:
			KEEP_INTERLEAVE_HEALTH(health, INTERLEAVE_HEALTH_DEGRADED);
			break;
		case DEVICE_HEALTH_FATAL:
			KEEP_INTERLEAVE_HEALTH(health, INTERLEAVE_HEALTH_FAILED);
			break;
		case DEVICE_HEALTH_UNKNOWN:
		default:
			KEEP_INTERLEAVE_HEALTH(health, INTERLEAVE_HEALTH_UNKNOWN);
		}

		// Check platform config info
		if (status.config_status == CONFIG_STATUS_ERR_BROKEN_INTERLEAVE)
		{
			KEEP_INTERLEAVE_HEALTH(health, INTERLEAVE_HEALTH_FAILED);
		}
	}

	p_interleave->health = health;

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int calculate_all_interleave_health(struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	for (int i = 0; i < p_pool->ilset_count; i++)
	{
		int tmp_rc = calculate_interleave_health(&(p_pool->ilsets[i]));
		if (tmp_rc != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("couldn't calculate interleave health for set %u",
					i);
			rc = tmp_rc;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int calculate_pool_security(struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	/*
	 * EncryptionCapable is true if its possible to create a namespace that is wholly
	 * contained on NVM-DIMMs that support encryption.
	 * EncryptionEnabled is true if its possible to create a namespace that is wholly
	 * contained on NVM-DIMMs that have encryption enabled.
	 * EraseCapable is true if its possible to create namespace that would be
	 * wholly contained on eraseable NVDIMMs.
	 */

	// check if a storage namespace can be created
	NVM_BOOL create_storage_encryption_enabled_ns = 0;
	NVM_BOOL create_storage_encryption_capable_ns = 0;
	NVM_BOOL create_storage_erase_capable_ns = 0;
	if (p_pool->type == POOL_TYPE_PERSISTENT)
	{
		struct nvm_capabilities nvm_caps;
		rc = nvm_get_nvm_capabilities(&nvm_caps);
		if (rc == NVM_SUCCESS)
		{
			for (int i = 0; i < p_pool->dimm_count; i++)
			{
				struct device_discovery discovery;
				if ((nvm_get_device_discovery(
					p_pool->dimms[i],
					&discovery)) == NVM_SUCCESS)
				{
					if ((p_pool->storage_capacities[i] >
						nvm_caps.sw_capabilities.min_namespace_size))
					{
						if (device_is_encryption_enabled(discovery.lock_state))
						{
							create_storage_encryption_enabled_ns = 1;
						}
						if (discovery.security_capabilities.passphrase_capable)
						{
							create_storage_encryption_capable_ns = 1;
						}
						if (device_is_erase_capable(discovery.security_capabilities))
						{
							create_storage_erase_capable_ns = 1;
						}
					}
				}
			}
		}
		else
		{
			COMMON_LOG_ERROR("Failed to get capabilities.");
		}
	}

	// check if a app direct namespace can be created if storage ns can't be created
	NVM_BOOL create_ad_encryption_enabled_ns = 0;
	NVM_BOOL create_ad_encryption_capable_ns = 0;
	NVM_BOOL create_ad_erase_capable_ns = 0;
	if (!create_storage_encryption_enabled_ns)
	{
		if ((p_pool->type == POOL_TYPE_PERSISTENT_MIRROR) ||
			(p_pool->type == POOL_TYPE_PERSISTENT))
		{
			enum encryption_status encryption_enabled;
			enum encryption_status encryption_capable;
			enum erase_capable_status erase_capable;
			for (int i = 0; i < p_pool->ilset_count; i++)
			{
				rc = calculate_app_direct_interleave_security(p_pool->ilsets[i].driver_id,
						&encryption_enabled, &erase_capable, &encryption_capable);
				if (rc == NVM_SUCCESS)
				{
					if (encryption_capable == NVM_ENCRYPTION_ON)
					{
						create_ad_encryption_capable_ns = 1;

						if (encryption_enabled == NVM_ENCRYPTION_ON)
						{
							create_ad_encryption_enabled_ns = 1;
						}
					}
					if (erase_capable)
					{
						create_ad_erase_capable_ns = 1;
					}
				}
			}
		}
	}

	// if a storage or app direct namespace cannot be created, security attributes are on
	if (create_storage_encryption_enabled_ns || create_ad_encryption_enabled_ns)
	{
		p_pool->encryption_enabled = 1;
	}
	else
	{
		p_pool->encryption_enabled = 0;
	}

	if (create_storage_encryption_capable_ns || create_ad_encryption_capable_ns)
	{
		p_pool->encryption_capable = 1;
	}
	else
	{
		p_pool->encryption_capable = 0;
	}

	if (create_storage_erase_capable_ns || create_ad_erase_capable_ns)
	{
		p_pool->erase_capable = 1;
	}
	else
	{
		p_pool->erase_capable = 0;

	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to convert an nvm_pool structure into a pool structure
 */
int convert_nvm_pool_to_pool(struct nvm_pool *p_nvm_pool, struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	memmove(p_pool->pool_uid, p_nvm_pool->pool_uid, NVM_MAX_UID_LEN);
	p_pool->type = p_nvm_pool->type;
	p_pool->capacity = p_nvm_pool->capacity;
	p_pool->free_capacity = p_nvm_pool->free_capacity;
	p_pool->health = p_nvm_pool->health;
	p_pool->socket_id = p_nvm_pool->socket_id;
	p_pool->dimm_count = p_nvm_pool->dimm_count;
	p_pool->ilset_count = p_nvm_pool->ilset_count;

	for (int j = 0; j < p_pool->dimm_count; j++)
	{
		p_pool->memory_capacities[j] =
				p_nvm_pool->memory_capacities[j];
		p_pool->storage_capacities[j] =
				p_nvm_pool->storage_capacities[j];
		p_pool->raw_capacities[j] =
				p_nvm_pool->raw_capacities[j];
	}

	for (int j = 0; j < p_pool->ilset_count; j++)
	{
		p_pool->ilsets[j].size = p_nvm_pool->ilsets[j].size;
		memmove(&(p_pool->ilsets[j].settings),
			&(p_nvm_pool->ilsets[j].settings),
			sizeof (struct interleave_format));
		p_pool->ilsets[j].dimm_count = p_nvm_pool->ilsets[j].dimm_count;
		p_pool->ilsets[j].set_index = p_nvm_pool->ilsets[j].set_index;
		p_pool->ilsets[j].driver_id = p_nvm_pool->ilsets[j].driver_id;
		p_pool->ilsets[j].mirrored = p_nvm_pool->ilsets[j].mirrored;
		p_pool->ilsets[j].available_size = p_nvm_pool->ilsets[j].available_size;
		p_pool->ilsets[j].socket_id = p_nvm_pool->ilsets[j].socket_id;
	}

	rc = fill_in_device_uids(p_pool, p_nvm_pool);
	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("The device is not in the device table.");
		rc = NVM_ERR_BADDEVICE;
	}

	KEEP_ERROR(rc, calculate_all_interleave_health(p_pool));
	KEEP_ERROR(rc, calculate_pool_security(p_pool));
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to retrieve the nvm_pool directly from the driver
 */
int get_nvm_pool(const NVM_UID pool_uid, struct nvm_pool *p_nvm_pool)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_ERR_BADPOOL;
	int pool_count = get_pool_count();
	if (pool_count < 0)
	{
		rc = pool_count;
	}
	else if (pool_count > 0)
	{
		struct nvm_pool nvm_pools[pool_count];
		memset(nvm_pools, 0, sizeof (nvm_pools));
		pool_count = get_pools(pool_count, nvm_pools);
		if (pool_count < 0)
		{
			rc = pool_count;
		}
		else if (pool_count > 0)
		{
			rc = NVM_ERR_BADPOOL;
			for (int i = 0; i < pool_count; i++)
			{
				if (uid_cmp(nvm_pools[i].pool_uid, pool_uid))
				{
					rc = NVM_SUCCESS;
					memmove(p_nvm_pool, &nvm_pools[i], sizeof (struct nvm_pool));
					break;
				}

			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * Retrieve the number of configured pools of NVM-DIMM capacity in the host server.
 */
int nvm_get_pool_count()
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(get_pools)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving pools is not supported.");
	}
	else if ((rc = get_nvm_context_pool_count()) < 0)
	{
		rc = get_pool_count();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 *
 * Retrieve a list of the configured pools of NVM-DIMM capacity in host server.
 */
int nvm_get_pools(struct pool *p_pools, const NVM_UINT8 count)
{
	COMMON_LOG_ENTRY();
	int rc = 0; // 0 pools

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(get_pools)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving pools is not supported.");
	}
	else if (p_pools == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_pools is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (count == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, count is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = get_nvm_context_pools(p_pools, count)) < 0 &&
			rc != NVM_ERR_ARRAYTOOSMALL)
	{
		memset(p_pools, 0, (sizeof (struct pool) * count));

		int pool_count = get_pool_count();
		if (pool_count < 0)
		{
			rc = pool_count;
		}
		else
		{
			struct nvm_pool nvm_pools[pool_count];
			pool_count = get_pools(pool_count, nvm_pools);
			if (pool_count < 0)
			{
				rc = pool_count;
			}
			else
			{
				int copy_count = 0;
				for (int i = 0; i < pool_count; i++)
				{
					// check array size
					if (i >= count)
					{
						COMMON_LOG_ERROR("The provided array is too small.");
						rc = NVM_ERR_ARRAYTOOSMALL;
						break;
					}
					if ((rc = convert_nvm_pool_to_pool(&nvm_pools[i], &p_pools[i])) != NVM_SUCCESS)
					{
						break;
					}
					else
					{
						copy_count++;
					}
				}
				if (rc == NVM_SUCCESS)
				{
					rc = copy_count;
				}

				// update the context
				set_nvm_context_pools(p_pools, copy_count);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve a list of the configured pools of NVM-DIMM capacity in host server.
 */
int nvm_get_pool(const NVM_UID pool_uid, struct pool *p_pool)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_BADPOOL;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_LICENSED(get_pools)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving pools is not supported.");
	}
	else if (pool_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, pool_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_pool == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_pool is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = get_nvm_context_pool(pool_uid, p_pool)) != NVM_SUCCESS)
	{
		memset(p_pool, 0, sizeof (struct pool));

		struct nvm_pool nvm_pool;
		memset(&nvm_pool, 0, sizeof (nvm_pool));
		if ((rc = get_nvm_pool(pool_uid, &nvm_pool)) == NVM_SUCCESS)
		{
			rc = convert_nvm_pool_to_pool(&nvm_pool, p_pool);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Store the configuration settings from the specified NVM-DIMM
 * to a file in order to duplicate the configuration elsewhere.
 */
int nvm_dump_config(const NVM_UID device_uid,
		const NVM_PATH path, const NVM_SIZE path_len,
		const NVM_BOOL append)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(get_device_capacity)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving device capacity is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path == NULL)
	{
		COMMON_LOG_ERROR("File path is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR_F(
				"Invalid parameter, path length is too big: %d; <= %d",
				path_len, NVM_PATH_LEN);
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, path length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		// get the platform config data for this dimm
		struct platform_config_data *p_cfg_data = NULL;
		int tmp_rc = get_dimm_platform_config(discovery.device_handle, &p_cfg_data);
		if (tmp_rc != NVM_SUCCESS)
		{
			rc = tmp_rc; // propagate the error
		}
		else if (!p_cfg_data) // this should never happen, but just in case.
		{
			COMMON_LOG_ERROR("Config data structure returned is NULL");
			rc = NVM_ERR_BADDEVICECONFIG;
		}
		else
		{
			struct current_config_table *p_current_config =
					cast_current_config(p_cfg_data);
			// no current config is not valid
			if (!p_current_config)
			{
				COMMON_LOG_ERROR("No current config data");
				rc = NVM_ERR_NOTFOUND;
			}
			else
			{
				// Unconfigured dimms and unapplied memory allocation goals are not included
				if (p_current_config->config_status != CURRENT_CONFIG_STATUS_UNCONFIGURED)
				{
					// convert current config into config_goal struct
					struct config_goal goal;
					memset(&goal, 0, sizeof (goal));
					// default memory size to the current in case there
					// is no partition size change table
					goal.memory_size =
							(p_current_config->mapped_memory_capacity / BYTES_PER_GB);
					rc = config_input_table_to_config_goal(device_uid,
							(unsigned char *)p_current_config, sizeof (struct current_config_table),
							p_current_config->header.length, &goal);
					if (rc == NVM_SUCCESS)
					{
						// write it to the path
						rc = write_dimm_config(&discovery, &goal, path, path_len, append);
					}
				}
				else
				{
					COMMON_LOG_ERROR("dimm is not configured.");
					rc = NVM_ERR_NOTFOUND;
				}
			}
		}
		free(p_cfg_data);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Read from the dimm config file until we find a valid line
 */
int read_dimm_config(FILE *p_file, struct config_goal *p_goal,
		NVM_UINT16 *p_socket, NVM_UINT32 *p_dimm_handle,
		NVM_UINT64 *p_dimm_size_gb)
{
	COMMON_LOG_ENTRY();
	int rc = 0; // end of file

	char line[NVM_MAX_CONFIG_LINE_LEN];
	while (fgets(line, NVM_MAX_CONFIG_LINE_LEN, p_file) != NULL)
	{
		// trim any leading white space
		s_strtrim_left(line, NVM_MAX_CONFIG_LINE_LEN);
		if (line[0] != '#')
		{
			NVM_UINT32 p1_format = 0;
			NVM_UINT32 p2_format = 0;

			NVM_BOOL bad_format = 0;
			const char *delim = ",\n";
			char *str;
			const char *p_end;
			char *pLine = line;

			str = x_strtok(&pLine, delim);
			if (s_strtous(str, strlen(str),
					&p_end, p_socket) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_strtoui(str, strlen(str),
					&p_end, p_dimm_handle) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_strtoull(str, strlen(str),
					&p_end, p_dimm_size_gb) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_strtoull(str, strlen(str),
					&p_end, &p_goal->memory_size) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_strtoull(str, strlen(str),
					&p_end, &p_goal->app_direct_1_size) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_strtoui(str, strlen(str),
					&p_end, &p1_format) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_digitstrtouc(str, strlen(str),
					&p_end, &p_goal->app_direct_1_settings.mirrored) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_strtous(str, strlen(str),
					&p_end, &p_goal->app_direct_1_set_id) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_strtoull(str, strlen(str),
					&p_end, &p_goal->app_direct_2_size) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_strtoui(str, strlen(str),
					&p_end, &p2_format) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_digitstrtouc(str, strlen(str),
					&p_end, &p_goal->app_direct_2_settings.mirrored) != strlen(str))
			{
				bad_format = 1;
			}

			str = x_strtok(&pLine, delim);
			if (str == NULL || s_strtous(str, strlen(str),
					&p_end, &p_goal->app_direct_2_set_id) != strlen(str))
			{
				bad_format = 1;
			}

			// make the load command compatible with future file formats that include extra data
			// in the csv list by ignoring extra data at the end of line if encountered
			while (str != NULL)
			{
				NVM_UINT16 extraField = 0;
				str = x_strtok(&pLine, delim);
				if (str == NULL || s_strtous(str, strlen(str),
						&p_end, &extraField) != strlen(str))
				{
					COMMON_LOG_INFO("Config file has extra data that will be ignored.");
					break;
				}
			}

			if (!bad_format && (pLine == NULL || strlen(pLine) == 0))
			{
				// convert the format to the goal
				interleave_format_to_struct(p1_format,
						&p_goal->app_direct_1_settings.interleave);
				interleave_format_to_struct(p2_format,
						&p_goal->app_direct_2_settings.interleave);
				rc = 1;
			}
			else
			{
				COMMON_LOG_ERROR_F("Found a badly formatted line in the config file '%s'",
					line);
				rc = NVM_ERR_BADFILE;
			}
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int fill_goal_dimm_uids(FILE *p_file, struct config_goal *p_goal)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	int p1_count = 0;
	int p2_count = 0;

	rewind(p_file); // reset the file to the beginning
	NVM_UINT16 socket = 0;
	NVM_UINT32 dimm_handle = 0;
	NVM_UINT64 dimm_size_gb = 0;
	struct config_goal tmp;
	memset(&tmp, 0, sizeof (tmp));
	while ((rc = read_dimm_config(p_file, &tmp, &socket, &dimm_handle, &dimm_size_gb)) > 0)
	{
		// if our goal has a P1, check for a matching index
		if (p_goal->app_direct_count > 0 &&
			p_goal->app_direct_1_set_id == tmp.app_direct_1_set_id)
		{
			// convert the dimm_handle to a uid
			NVM_NFIT_DEVICE_HANDLE handle;
			handle.handle = dimm_handle;
			struct device_discovery discovery;
			if (lookup_dev_handle(handle, &discovery) == NVM_SUCCESS)
			{
				memmove(p_goal->app_direct_1_settings.dimms[p1_count],
						discovery.uid, NVM_MAX_UID_LEN);
				p1_count++;
			}
		}
		// if our goal has a P2 index and it matches this one
		if (p_goal->app_direct_count > 1 &&
				p_goal->app_direct_2_set_id == tmp.app_direct_2_set_id)
		{
			NVM_NFIT_DEVICE_HANDLE handle;
			handle.handle = dimm_handle;
			struct device_discovery discovery;
			if (lookup_dev_handle(handle, &discovery) == NVM_SUCCESS)
			{
				memmove(p_goal->app_direct_2_settings.dimms[p2_count],
						discovery.uid, NVM_MAX_UID_LEN);
				p2_count++;
			}
		}
	}

	// make sure we found all dimms in the interleave set for P1
	if (p_goal->app_direct_count > 0)
	{
		if (p1_count != p_goal->app_direct_1_settings.interleave.ways)
		{
			COMMON_LOG_ERROR(
					"Not all the DIMMs in the p1 interleave set were found in the file");
			rc = NVM_ERR_BADDEVICECONFIG;
		}
	}
	// make sure we found all dimms in the interleave set for P2
	if (p_goal->app_direct_count > 1)
	{
		if (p2_count != p_goal->app_direct_2_settings.interleave.ways)
		{
			COMMON_LOG_ERROR(
					"Not all the DIMMs in the p2 interleave set were found in the file");
			rc = NVM_ERR_BADDEVICECONFIG;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Apply a previously stored configuration goal from a file onto the specified NVM-DIMM.
 */
int nvm_load_config(const NVM_UID device_uid,
		const NVM_PATH path, const NVM_SIZE path_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != COMMON_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_capacity)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying device capacity is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path == NULL)
	{
		COMMON_LOG_ERROR("File path is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR_F(
				"Invalid parameter, path length is too big: %d; <= %d",
				path_len, NVM_PATH_LEN);
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, path length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (!file_exists(path, path_len))
	{
		COMMON_LOG_ERROR_F("File %s does not exist", path);
		rc = NVM_ERR_BADFILE;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		// read the file to get the desired config goal for this dimm
		FILE *p_file = NULL;
		p_file = open_file(path, path_len, "r");
		if (!p_file)
		{
			COMMON_LOG_ERROR_F("Failed to open file %s", path);
			rc = NVM_ERR_BADFILE;
		}
		else
		{
			if (lock_file(p_file, FILE_LOCK_MODE_READ) != COMMON_SUCCESS)
			{
				COMMON_LOG_ERROR_F("Failed to lock file %s for reading", path);
				rc = NVM_ERR_BADFILE;
			}
			else
			{
				NVM_BOOL dimm_found = 0;
				NVM_UINT16 socket_id = 0;
				NVM_UINT32 dimm_handle = 0;
				NVM_UINT64 dimm_size_gb = 0;
				struct config_goal goal;
				memset(&goal, 0, sizeof (goal));
				while ((rc = read_dimm_config(p_file, &goal, &socket_id,
						&dimm_handle, &dimm_size_gb)) > 0)
				{
					// is this the dimm we're looking for?
					if (dimm_handle == discovery.device_handle.handle &&
						socket_id == discovery.device_handle.parts.socket_id)
					{
						dimm_found = 1;
						struct device_discovery discovery;
						if ((rc = nvm_get_device_discovery(device_uid, &discovery))
								== NVM_SUCCESS)
						{
							// make sure dimm is big enough for config
							if ((discovery.capacity / BYTES_PER_GB) >= dimm_size_gb)
							{
								// set App Direct count
								if (goal.app_direct_1_size)
								{
									goal.app_direct_count = 1;
								}
								if (goal.app_direct_2_size)
								{
									goal.app_direct_count = 2;
								}
								// find matching dimm uids for the interleave sets
								if ((rc = fill_goal_dimm_uids(p_file, &goal)) == NVM_SUCCESS)
								{
									rc = nvm_create_config_goal(discovery.uid, &goal);
								}
							} // end dimm big enough
						} // end get discovery
						break;
					} // end match dimm/socket
				} // end getline
				if (rc == NVM_SUCCESS && !dimm_found)
				{
					COMMON_LOG_ERROR_F("The dimm config was not found in the file %s", path);
					rc = NVM_ERR_BADDEVICECONFIG;
				}
				// clean up the file
				lock_file(p_file, FILE_LOCK_MODE_UNLOCK);
			}
			fclose(p_file);
		} // end open file
	} // end exists and manageable

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
