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
 * This file contains the implementation of support functions of the native API.
 */

#include <file_ops/file_ops_adapter.h>
#include <encrypt/encrypt.h>
#include "nvm_management.h"
#include <persistence/logging.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include <persistence/event.h>
#include <string/s_str.h>
#include <uid/uid.h>
#include "device_adapter.h"
#include "cleanup_support_events.h"
#include "support.h"

#include "device_utilities.h"
#include "system.h"
#include "capabilities.h"
#include "nvm_context.h"
#include "system.h"
#include <unistd.h>
#include <fcntl.h>
#include "utility.h"
#include "namespace_labels.h"

#define	COMMON_INT_LENGTH	11
/*
 * Declare Internal Helper functions
 */

int support_filter_data(const NVM_PATH support_file, NVM_UINT16 filter_mask);

/*
 * Generate a support database
 * NOTE: ignore errors in data collection and gather as much data as possible
 */
int nvm_gather_support(const NVM_PATH support_file, const NVM_SIZE support_file_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	int max_no_support_snapshots = atoi(MAX_SUPPORT_SNAPSHOTS_DEFAULT);

	PersistentStore *p_store;
	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if ((p_store = get_lib_store()) == NULL)
	{
		rc = NVM_ERR_UNKNOWN;
	}
	else if (get_bounded_config_value_int(SQL_KEY_SUPPORT_SNAPSHOT_MAX, &max_no_support_snapshots)
				!= COMMON_SUCCESS)
	{
		// should never get here
		COMMON_LOG_ERROR_F("Failed to retrieve key %s.  Defaulting dump support enabled.",
				SQL_KEY_SUPPORT_SNAPSHOT_MAX);
	}
	else if (!max_no_support_snapshots)
	{
		COMMON_LOG_WARN("Gather support is disabled");
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else if (support_file == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, support file buffer is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (support_file_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, support file buffer length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (support_file_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR_F(
				"Invalid parameter, path length is too big: %d; <= %d",
				support_file_len, NVM_PATH_LEN);
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		// store the current state of the system to the database, ignore error
		const char *name = "nvm_gather_support";
		int temp_rc = 0;
		if ((temp_rc = nvm_save_state(name, strlen(name))) != NVM_SUCCESS)
		{
			COMMON_LOG_WARN_F("Save state failed or is incomplete. rc=%d", temp_rc);
		}

		// store the logs
		log_gather();

		struct stat statbuf;
		if (stat(support_file, &statbuf) != -1)
		{
			unlink(support_file);
		}

		// Copy the database to the path specified in p_support_file
		COMMON_PATH config_file;
		if (get_lib_store_path(config_file) != COMMON_SUCCESS ||
				!copy_file(config_file, COMMON_PATH_LEN, support_file, support_file_len))
		{
			COMMON_LOG_ERROR_F("Unable to copy %s to: %s", CONFIG_FILE, support_file);
			rc = NVM_ERR_BADFILE;
		}
		else
		{
			int filter_mask = (int)(GSF_HOST_DATA | GSF_NAMESPACE_DATA
							| GSF_SERIAL_NUMS | GSF_SYSTEM_LOG);
			if (get_config_value_int(SQL_KEY_GATHER_SUPPORT_FILTER, &filter_mask) != COMMON_SUCCESS)
			{
				// should never get here
				COMMON_LOG_ERROR_F("Failed to retrieve key %s. Default all filtering except logs.",
						SQL_KEY_GATHER_SUPPORT_FILTER);
				// security prevails upon failed attempts to read SQL DB
				// however, keep logs available because they are likely relevant
				filter_mask = (int)(GSF_HOST_DATA | GSF_NAMESPACE_DATA
								| GSF_SERIAL_NUMS | GSF_SYSTEM_LOG);
			}

			if ((temp_rc = support_filter_data(support_file, (NVM_UINT16)filter_mask))
					!= DB_SUCCESS)
			{
				// should never get in here
				COMMON_LOG_ERROR_F("Error filtering support file: rc=%d. Deleting: %s",
						temp_rc, support_file);
				delete_file(support_file, support_file_len);
				rc = NVM_ERR_BADFILE;
			}
			else
			{
				int encrypt = -1;
				if (get_config_value_int(SQL_KEY_ENCRYPT_GATHER_SUPPORT, &encrypt)
						!= COMMON_SUCCESS)
				{
					// should never get here
					COMMON_LOG_ERROR_F("Failed to retrieve key %s.  Defaulting encryption enabled.",
							SQL_KEY_ENCRYPT_GATHER_SUPPORT);
					// security prevails upon failed attempts to read SQL DB
					encrypt = 1;
				}

				if (encrypt)
				{
					COMMON_PATH compressed_file;
					COMMON_PATH encrypted_file;

					// This entire process adds a CRYPTO_FILE_EXT file extension to the filename
					if ((temp_rc = compress_file(support_file, compressed_file)) != COMMON_SUCCESS)
					{
						// should never get in here
						COMMON_LOG_ERROR_F("Support file compression failed. rc=%d", temp_rc);
						rc = NVM_ERR_BADFILE;
					}
					else if ((temp_rc = rsa_encrypt(compressed_file, encrypted_file))
							!= COMMON_SUCCESS)
					{

						// should never get in here
						COMMON_LOG_ERROR_F("Support file encryption failed. rc=%d", temp_rc);
						rc = NVM_ERR_BADFILE;

					}
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

unsigned int get_size_of_fa_blob_including_header(const NVM_NFIT_DEVICE_HANDLE device_handle,
		unsigned int current_token_id)
{
	COMMON_LOG_ENTRY();

	unsigned int size = 0;
	struct pt_input_payload_fa_data_register_values input_register;
	struct pt_output_payload_get_fa_blob_header blob_header;

	memset(&input_register, 0, sizeof (input_register));
	memset(&blob_header, 0, sizeof (blob_header));

	input_register.action = GET_FA_BLOB_HEADER;
	input_register.id = current_token_id;

	int rc = fw_get_fa_data(device_handle, &input_register, &blob_header);

	if (rc == NVM_SUCCESS)
	{
		if (blob_header.size != 0)
		{
			size = blob_header.size +
					sizeof (struct pt_output_payload_get_fa_blob_header);
		}
	}

	COMMON_LOG_EXIT();
	return size;
}

int retrieve_fa_blob_header(const NVM_NFIT_DEVICE_HANDLE device_handle,
		struct pt_output_payload_get_fa_blob_header *p_blob_header,
		unsigned int current_token_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_input_payload_fa_data_register_values input_register;

	memset(&input_register, 0, sizeof (input_register));

	/* Get the inventory to get the max token ID */

	input_register.action = GET_FA_BLOB_HEADER;
	input_register.id = current_token_id;

	rc = fw_get_fa_data(device_handle, &input_register, p_blob_header);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int retrieve_fa_blob_small_payload(const NVM_NFIT_DEVICE_HANDLE device_handle,
		unsigned char *p_blob, unsigned int current_token_id)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;
	unsigned int blob_data_size = 0;
	unsigned int bytes_remaining = 0;
	unsigned int bytes_retrieved = 0;
	unsigned int bytes_written = 0;
	unsigned char small_payload_buffer[DEV_FA_SMALL_PAYLOAD_BLOB_DATA_SIZE];

	struct pt_output_payload_get_fa_blob_header blob_header;

	memset(&blob_header, 0, sizeof (blob_header));

	rc = retrieve_fa_blob_header(device_handle, &blob_header, current_token_id);

	memmove(p_blob, &blob_header, sizeof (blob_header));

	bytes_written += sizeof (blob_header);

	if (rc == NVM_SUCCESS)
	{
		blob_data_size = blob_header.size;

		struct pt_input_payload_fa_data_register_values input_register;
		memset(&input_register, 0, sizeof (input_register));

		input_register.action = GET_FA_BLOB_SMALL_PAYLOAD;
		input_register.id = current_token_id;

		for (input_register.offset = 0;
				input_register.offset < blob_data_size;
				input_register.offset += DEV_FA_SMALL_PAYLOAD_BLOB_DATA_SIZE)
		{
			memset(small_payload_buffer, 0, DEV_FA_SMALL_PAYLOAD_BLOB_DATA_SIZE);
			rc = fw_get_fa_data(device_handle, &input_register, small_payload_buffer);

			if (rc == NVM_SUCCESS)
			{
				bytes_remaining = blob_data_size - input_register.offset;

				if (bytes_remaining < DEV_FA_SMALL_PAYLOAD_BLOB_DATA_SIZE)
				{
					bytes_retrieved = bytes_remaining;
				}
				else
				{
					bytes_retrieved = DEV_FA_SMALL_PAYLOAD_BLOB_DATA_SIZE;
				}

				memmove((p_blob + bytes_written), small_payload_buffer, bytes_retrieved);
				bytes_written += bytes_retrieved;
			}
			else
			{
				break;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int retrieve_all_fa_data_blobs(const NVM_NFIT_DEVICE_HANDLE device_handle,
		unsigned int max_token_id, const NVM_PATH support_file,
		const NVM_SIZE support_file_len,
		NVM_PATH support_files[NVM_MAX_EAFD_FILES])
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;
	unsigned char *p_blob = NULL;
	unsigned int blob_file_name_size = support_file_len + COMMON_INT_LENGTH;
	char blob_file[blob_file_name_size];
	unsigned int blob_count = 0;

	for (unsigned int current_token_id = 1; current_token_id <= max_token_id; current_token_id++)
	{
		unsigned int blob_size =
				get_size_of_fa_blob_including_header(device_handle,
						current_token_id);

		if (blob_size != 0)
		{
			p_blob = (unsigned char *)malloc(blob_size);

			if (p_blob)
			{
				memset(p_blob, 0, blob_size);

				retrieve_fa_blob_small_payload(device_handle, (unsigned char *)(p_blob),
						current_token_id);

				snprintf(blob_file, blob_file_name_size, "%s_%d", support_file,
						current_token_id);

				int tmprc = copy_buffer_to_file((unsigned char *)p_blob, blob_size, blob_file,
						blob_file_name_size, O_CREAT);

				if (tmprc == COMMON_SUCCESS && support_files != NULL)
				{
					snprintf(support_files[blob_count],
							blob_file_name_size, "%s", blob_file);
				}
				else if (tmprc == COMMON_ERR_BADFILE)
				{
					rc = NVM_ERR_BADFILE;
				}
				else if (tmprc == COMMON_ERR_INVALIDPARAMETER)
				{
					rc = NVM_ERR_INVALIDPARAMETER;
				}

				free(p_blob);
			}
			else
			{
				rc = NVM_ERR_NOMEMORY;
				break;
			}

			blob_count++;
		}
	}

	if (blob_count == 0)
	{
		rc = NVM_ERR_NOFADATAAVAILABLE;
	}
	else
	{
		rc = blob_count;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_maximum_token_id(const NVM_NFIT_DEVICE_HANDLE device_handle,
		unsigned int *p_max_token_id)
{
	int rc = NVM_SUCCESS;
	struct pt_input_payload_fa_data_register_values input_register;
	struct pt_output_payload_get_fa_inventory fa_inventory;

	memset(&input_register, 0, sizeof (input_register));

	/* Get the inventory to get the max token ID */

	input_register.action = GET_FA_INVENTORY;
	if ((rc = fw_get_fa_data(device_handle, &input_register,
			&fa_inventory)) == NVM_SUCCESS)
	{
		*p_max_token_id = fa_inventory.max_fa_token_id;
	}
	return rc;
}

int nvm_dump_device_support(const NVM_UID device_uid, const NVM_PATH support_file,
		const NVM_SIZE support_file_len,
		NVM_PATH support_files[NVM_MAX_EAFD_FILES])
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (support_file == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, support file buffer is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (support_file_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, support file buffer length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (support_file_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR_F(
				"Invalid parameter, path length is too big: %d; <= %d",
				support_file_len, NVM_PATH_LEN);
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		unsigned int max_token_id = 0;

		if ((rc = get_maximum_token_id(discovery.device_handle, &max_token_id))
				== NVM_SUCCESS)
		{
			unsigned int fa_file_name_size = support_file_len + NVM_MAX_UID_LEN;
			char fa_file_name[fa_file_name_size];

			snprintf(fa_file_name, fa_file_name_size, "%s_%s", support_file, discovery.uid);

			rc = retrieve_all_fa_data_blobs(discovery.device_handle,
					max_token_id, fa_file_name, fa_file_name_size, support_files);
		}
		else
		{
			COMMON_LOG_ERROR("Could not retrieve inventory");
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Clear all history tables
 */
int nvm_purge_state_data()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	PersistentStore *p_store = NULL;
	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if ((p_store = get_lib_store()) == NULL)
	{
		rc = NVM_ERR_UNKNOWN;
	}
	else if (DB_SUCCESS != db_clear_history(p_store))
	{
		rc = NVM_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Load a simulator file.
 */
int nvm_add_simulator(const NVM_PATH simulator, const NVM_SIZE simulator_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (simulator == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, simulator is NULL.");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (simulator_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR("Invalid parameter, simulator_len is too big.");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (simulator_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, simulator_len is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		// check if file exists
		if (file_exists(simulator, simulator_len))
		{
			// call device adapter implementation
			rc = add_simulator(simulator, simulator_len);
		}
		else
		{
			COMMON_LOG_ERROR_F("Simulator file %s does not exist", simulator);
			rc = NVM_ERR_BADFILE;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Remove a simulator file.
 */
int nvm_remove_simulator()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		// call device adapter implementation
		rc = remove_simulator();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Set NVM DIMM FW logging level.
 */
int nvm_set_fw_log_level(const NVM_UID device_uid, const enum fw_log_level log_level)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;
	struct device_discovery discovery;

	// verify log_level is within range
  if ((log_level > FW_LOG_LEVEL_DEBUG) || (log_level < FW_LOG_LEVEL_ERROR))
  {
		rc = NVM_ERR_INVALIDPARAMETER;
		COMMON_LOG_ERROR_F("Invalid parameter, log_level is %d", log_level);
	}
	else if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_settings)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying device settings is not supported.");
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		struct fw_cmd cmd = {0};
		cmd.device_handle = discovery.device_handle.handle;
		cmd.opcode = PT_SET_ADMIN_FEATURES;
		cmd.sub_opcode = SUBOP_FW_DBG_LOG_LEVEL;
		cmd.input_payload = (unsigned char *)&log_level;
		cmd.input_payload_size = sizeof (unsigned char);
		if ((rc = ioctl_passthrough_cmd(&cmd)) == NVM_SUCCESS)
		{
			rc = NVM_SUCCESS;
		}

		// clear any device context
		invalidate_devices();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * get current NVM DIMM FW logging level
 */
int nvm_get_fw_log_level(const NVM_UID device_uid, enum fw_log_level *p_log_level)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;
	struct device_discovery discovery;

	if (p_log_level == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_log_level is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(update_device_firmware)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Updating device firmware is not supported.");
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		unsigned char log_level;
		struct fw_cmd cmd = {0};
		cmd.device_handle = discovery.device_handle.handle;
		cmd.opcode = PT_GET_ADMIN_FEATURES;
		cmd.sub_opcode = SUBOP_FW_DBG_LOG_LEVEL;
		cmd.output_payload = &log_level;
		cmd.output_payload_size = sizeof (unsigned char);

		if ((rc = ioctl_passthrough_cmd(&cmd)) == NVM_SUCCESS)
		{
			rc = NVM_SUCCESS;
			*p_log_level = log_level;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int update_db_table_property(PersistentStore *p_ps,
		const char *table_name,
		const char *prop_name, const char *prop_value)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_ps == NULL)
	{
		COMMON_LOG_ERROR("Database pointer is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		char custom_update_sql[512];
		s_snprintf(custom_update_sql, sizeof (custom_update_sql),
				"UPDATE %s SET %s=%s",
				table_name, prop_name, prop_value);
		if (DB_SUCCESS !=
				db_run_custom_sql(p_ps, custom_update_sql))
		{
			COMMON_LOG_ERROR_F("update %s failed.", table_name);
			rc = NVM_ERR_DEVICEERROR;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int change_serial_num_in_db_table(PersistentStore *p_ps,
		const char *table_name)
{
	COMMON_LOG_ENTRY();
	int rc = update_db_table_property(p_ps, table_name,
			"serial_num", "device_handle");

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns DEVICEERROR if the database access fails otherwise return SUCCESS
 */
int change_serial_num_in_identify_dimm(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = change_serial_num_in_db_table(p_ps, "identify_dimm");

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns DEVICEERROR if the database access fails otherwise return SUCCESS
 */
int change_serial_num_in_identify_dimm_history(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = change_serial_num_in_db_table(p_ps, "identify_dimm_history");

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns DEVICEERROR if the database access fails otherwise return SUCCESS
 */
int change_serial_num_in_interleave_set_dimm_info(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = change_serial_num_in_db_table(p_ps, "interleave_set_dimm_info_v1");

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns DEVICEERROR if the database access fails otherwise return SUCCESS
 */
int change_serial_num_in_interleave_set_dimm_info_history(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = change_serial_num_in_db_table(p_ps, "interleave_set_dimm_info_v1_history");

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns DEVICEERROR if the database access fails otherwise return SUCCESS
 */
int change_serial_num_in_topology_state(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = change_serial_num_in_db_table(p_ps, "topology_state");

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int change_host_name_in_db_table(PersistentStore *p_ps,
		const char *table_name)
{
	COMMON_LOG_ENTRY();
	int rc = update_db_table_property(p_ps,
			table_name, "name", "'NVMDIMMHOST'");

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns DEVICEERROR if the database access fails otherwise return SUCCESS
 */
int change_hostname_in_host(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = change_host_name_in_db_table(p_ps, "host");
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns DEVICEERROR if the database access fails otherwise return SUCCESS
 */
int change_hostname_in_host_history(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = change_host_name_in_db_table(p_ps, "host_history");

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns DEVICEERROR if the database access fails otherwise return SUCCESS
 */
int change_hostname_in_sw_inventory(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = change_host_name_in_db_table(p_ps, "sw_inventory");
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns DEVICEERROR if the database access fails otherwise return SUCCESS
 */
int change_hostname_in_sw_inventory_history(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = change_host_name_in_db_table(p_ps, "sw_inventory_history");
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_filter_data(const NVM_PATH support_file, NVM_UINT16 filter_mask)
{
	int db_rc = DB_SUCCESS;

	int db_tbl_rc = DB_SUCCESS;
	int db_tbl_hist_rc = DB_SUCCESS;

	// open the copied database
	PersistentStore *p_support = open_PersistentStore(support_file);

	if (NULL == p_support)
	{
		return DB_ERR_FAILURE;
	}
	else
	{
		// filter host tables
		if (filter_mask & GSF_HOST_DATA)
		{
			db_tbl_rc = change_hostname_in_host(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			db_tbl_rc = change_hostname_in_host_history(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			db_tbl_rc = change_hostname_in_sw_inventory(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			db_tbl_rc = change_hostname_in_sw_inventory_history(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
		}

		// filter namespace tables
		if (filter_mask & GSF_NAMESPACE_DATA)
		{
			db_tbl_rc = db_delete_all_namespaces(p_support); // probably pointless
			db_tbl_hist_rc = db_delete_namespace_history(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			KEEP_ERROR(db_rc, db_tbl_hist_rc);
		}

		// filter system logs
		if (filter_mask & GSF_SYSTEM_LOG)
		{
			db_tbl_rc = db_delete_all_logs(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			KEEP_ERROR(db_rc, db_tbl_hist_rc);
		}

		// filter serial numbers
		if (filter_mask & GSF_SERIAL_NUMS)
		{
			// note that entities with attributes that specify '.isClearable()'
			// clear that attribute in the entities table *and* associated history table
			db_tbl_rc = change_serial_num_in_identify_dimm(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			db_tbl_rc = change_serial_num_in_identify_dimm_history(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			db_tbl_rc = change_serial_num_in_interleave_set_dimm_info(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			db_tbl_rc = change_serial_num_in_interleave_set_dimm_info_history(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			db_tbl_rc = change_serial_num_in_topology_state(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
			// platform configuration check diagnostic stores serial number when an interleave set
			// is incomplete.
			db_tbl_rc = change_serial_num_in_events(p_support,
					EVENT_TYPE_DIAG_PLATFORM_CONFIG,
					EVENT_CODE_DIAG_PCONFIG_BROKEN_ISET);
			KEEP_ERROR(db_rc, db_tbl_rc);
			// Device UIDs contain serial numbers
			db_tbl_rc = change_dimm_uid_in_events(p_support);
			KEEP_ERROR(db_rc, db_tbl_rc);
		}

		// TODO: filter performance data
		// TODO: filter events

		// releases the db
		free_PersistentStore(&p_support);
	}

	return db_rc;
}

int nvm_set_user_preference(const NVM_PREFERENCE_KEY key,
		const NVM_PREFERENCE_VALUE value)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (key == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, preference key is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (value == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, preference value is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		int tempRc = add_config_value(key, value);
		if (tempRc != COMMON_SUCCESS)
		{
			rc = CommonErrorToLibError(tempRc);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * clear NVM dimm LSA (namespace label storage area in PCD)
 */
int nvm_clear_dimm_lsa(const NVM_UID device_uid)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) != NVM_SUCCESS)
	{
		rc = NVM_ERR_NOTMANAGEABLE;
	}
	// Check that no namespaces exist on this DIMM
	else if ((rc = dimm_has_namespaces_of_type(discovery.device_handle,
			NAMESPACE_TYPE_UNKNOWN)) != 0)
	{
		if (rc > 0)
		{
			rc = NVM_ERR_NAMESPACESEXIST;
		}
	}
	// check security permission
	else if ((rc = nvm_get_security_permission(&discovery)) == NVM_SUCCESS)
	{
		// clear out namespace label storage area
		zero_dimm_namespace_labels(discovery.device_handle.handle);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
