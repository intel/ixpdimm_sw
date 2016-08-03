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
 * This file implements methods used to translate BIOS Platform Configuration Data
 * to a database that may then be loaded as a simulator.
 */

#include "platform_config_data_db.h"
#include "utility.h"
#include <string/s_str.h>

// Helper macro for database functions to update
// the platform configuration data.
#define	DB_DIMM_ERROR(db_rc, new_db_rc, msg, id) \
	if (db_rc >= NVM_SUCCESS && new_db_rc != DB_SUCCESS) \
	{ \
		COMMON_LOG_ERROR_F(msg, id); \
		db_rc = NVM_ERR_DRIVERFAILED; \
	}

int clear_dimm_platform_config_from_db(PersistentStore *p_db,
		const unsigned int device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// clear out and existing platform config data in the db for this dimm
	// clear existing header
	DB_DIMM_ERROR(rc, db_delete_dimm_platform_config_by_device_handle(p_db, device_handle),
		"Failed to remove the existing platform configuration data for " NVM_DIMM_NAME " %d",
		device_handle);

	// clear existing input
	DB_DIMM_ERROR(rc, db_delete_dimm_config_input_by_device_handle(p_db, device_handle),
		"Failed to remove the existing configuration input data for " NVM_DIMM_NAME " %d",
		device_handle);

	// clear existing output
	DB_DIMM_ERROR(rc, db_delete_dimm_config_output_by_device_handle(p_db, device_handle),
		"Failed to remove the existing configuration output data for " NVM_DIMM_NAME " %d",
		device_handle);

	// clear existing current config
	DB_DIMM_ERROR(rc, db_delete_dimm_current_config_by_device_handle(p_db, device_handle),
		"Failed to remove the existing current configuration data for " NVM_DIMM_NAME " %d",
		device_handle);

	// clear interleave tables (current, input and output) for this dimm
	DB_DIMM_ERROR(rc,
		db_delete_dimm_interleave_set_by_dimm_topology_device_handle(p_db, device_handle),
		"Failed to remove the existing interleave set data for " NVM_DIMM_NAME " %d",
		device_handle);

	// clear interleave set dimm info tables (current, input and output) for this dimm
	DB_DIMM_ERROR(rc,
		db_delete_interleave_set_dimm_info_by_dimm_topology_device_handle(p_db, device_handle),
		"Failed to remove the existing interleave set dimm info data for " NVM_DIMM_NAME " %d",
		device_handle);

	// clear partition size change tables (input and output)
	DB_DIMM_ERROR(rc,
		db_delete_dimm_partition_change_by_dimm_topology_device_handle(p_db, device_handle),
		"Failed to remove the existing partition size change data for " NVM_DIMM_NAME " %d",
		device_handle);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int update_dimm_partition_size_in_db(PersistentStore *p_db,
		const unsigned int device_handle,
		const enum config_table_type table_type,
		struct partition_size_change_extension_table *p_partition)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct db_dimm_partition_change db_partition;
	db_get_next_dimm_partition_change_id(p_db, &db_partition.id);
	db_partition.device_handle = device_handle;
	db_partition.config_table_type = table_type;
	db_partition.extension_table_type = PARTITION_CHANGE_TABLE;
	db_partition.length = p_partition->header.length;
	db_partition.partition_size = p_partition->partition_size;
	db_partition.status = p_partition->status;
	rc = db_add_dimm_partition_change(p_db, &db_partition);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int update_dimm_interleave_set_in_db(PersistentStore *p_db,
		const unsigned int device_handle,
		const enum config_table_type table_type,
		struct interleave_info_extension_table *p_interleave_set)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct db_dimm_interleave_set db_set;
	db_get_next_dimm_interleave_set_id(p_db, &db_set.id);
	db_set.device_handle = device_handle;
	db_set.config_table_type = table_type;
	db_set.extension_table_type = INTERLEAVE_TABLE;
	db_set.length = p_interleave_set->header.length;
	db_set.index_id = p_interleave_set->index;
	db_set.dimm_count = p_interleave_set->dimm_count;
	db_set.memory_type = p_interleave_set->memory_type;
	db_set.interleave_format = p_interleave_set->interleave_format;
	db_set.mirror_enable = p_interleave_set->mirror_enable;
	db_set.status = p_interleave_set->status;
	rc = db_add_dimm_interleave_set(p_db, &db_set);

	// add the interleave table
	int offset = sizeof (struct interleave_info_extension_table);
	for (int i = 0; i < p_interleave_set->dimm_count && rc == NVM_SUCCESS; i++)
	{
		// check the length
		if (p_interleave_set->header.length <
				(offset + sizeof (struct dimm_info_extension_table)))
		{
			COMMON_LOG_ERROR_F("Interleave set table length %d invalid",
					p_interleave_set->header.length);
			rc = NVM_ERR_BADDEVICECONFIG;
			break;
		}

		struct dimm_info_extension_table *p_dimm_info =
				(struct dimm_info_extension_table *)((NVM_UINT8 *)p_interleave_set + offset);
		struct db_interleave_set_dimm_info db_dimm;
		db_get_next_interleave_set_dimm_info_id(p_db, &db_dimm.id);
		db_dimm.config_table_type = table_type;
		db_dimm.index_id = p_interleave_set->index;
		db_dimm.device_handle = device_handle;
		// convert manufacturer to uint16
		db_dimm.manufacturer = MANUFACTURER_TO_UINT(p_dimm_info->manufacturer);
		// convert serial number to uint32
		db_dimm.serial_num = SERIAL_NUMBER_TO_UINT(p_dimm_info->serial_number);
		s_strcpy(db_dimm.model_num, p_dimm_info->model_number, NVM_MODEL_LEN);
		db_dimm.offset = p_dimm_info->offset;
		db_dimm.size = p_dimm_info->size;
		rc = db_add_interleave_set_dimm_info(p_db, &db_dimm);
		offset += sizeof (struct dimm_info_extension_table);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int update_dimm_extension_tables_in_db(PersistentStore *p_db,
		const unsigned int device_handle,
		const enum config_table_type table_type,
		struct extension_table_header *p_top,
		const NVM_UINT32 size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// iterate over all the tables
	NVM_UINT32 offset = 0;
	while (offset < size && rc == NVM_SUCCESS)
	{
		struct extension_table_header *p_header =
				(struct extension_table_header *)((NVM_UINT8 *)p_top + offset);
		// check the length for validity
		if ((p_header->length + offset) > size)
		{
			COMMON_LOG_ERROR_F("Extension table length %d invalid", p_header->length);
			rc = NVM_ERR_BADDEVICECONFIG;
			break;
		}

		// is it a partition size change table
		if (p_header->type == PARTITION_CHANGE_TABLE)
		{
			// store it in the db
			rc = update_dimm_partition_size_in_db(p_db, device_handle, table_type,
					(struct partition_size_change_extension_table *)p_header);
		}
		// is it a interleave table
		else if (p_header->type == INTERLEAVE_TABLE)
		{
			// store it in the db
			rc = update_dimm_interleave_set_in_db(p_db, device_handle, table_type,
					(struct interleave_info_extension_table *)p_header);
		}
		// else unrecognized table, go to next
		offset += p_header->length;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Update the platform config data stored in the db.
 * Tries to store as much data as possible while propagating any errors.
 */
int update_dimm_platform_config_in_db(PersistentStore *p_db,
		const unsigned int device_handle, struct platform_config_data *p_config_data)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!p_db)
	{
		COMMON_LOG_ERROR("Database is invalid");
		rc = NVM_ERR_UNKNOWN;
	}
	else if (!p_config_data)
	{
		COMMON_LOG_ERROR("Config data invalid");
		rc = NVM_ERR_UNKNOWN;
	}
	// make sure we have good data
	else if ((rc = check_platform_config(p_config_data)) == NVM_SUCCESS)
	{
		// make this function more efficient by doing it as one transaction
		db_begin_transaction(p_db);

		// clear existing platform config data for this dimm
		rc = clear_dimm_platform_config_from_db(p_db, device_handle);

		// write new platform config data for this dimm
		// write header
		struct db_dimm_platform_config db_config;
		db_config.device_handle = device_handle;
		memmove(db_config.signature, p_config_data->header.signature, SIGNATURE_LEN);
		db_config.length = p_config_data->header.length;
		db_config.revision = p_config_data->header.revision;
		db_config.checksum = p_config_data->header.checksum;
		memmove(db_config.oem_id, p_config_data->header.oem_id, OEM_ID_LEN);
		memmove(db_config.oem_table_id, p_config_data->header.oem_table_id, OEM_TABLE_ID_LEN);
		db_config.oem_revision = p_config_data->header.oem_revision;
		db_config.creator_id = p_config_data->header.creator_id;
		db_config.creator_revision = p_config_data->header.creator_revision;
		db_config.current_config_size = p_config_data->current_config_size;
		db_config.current_config_offset = p_config_data->current_config_offset;
		db_config.config_input_size = p_config_data->config_input_size;
		db_config.config_input_offset = p_config_data->config_input_offset;
		db_config.config_output_size = p_config_data->config_output_size;
		db_config.config_output_offset = p_config_data->config_output_offset;
		DB_DIMM_ERROR(rc, db_add_dimm_platform_config(p_db, &db_config),
			"Failed to add the platform configuration header data for " NVM_DIMM_NAME " %d",
			device_handle);

		// write config input
		if (p_config_data->config_input_size)
		{
			struct config_input_table *p_config_input = cast_config_input(p_config_data);
			if (p_config_input)
			{
				struct db_dimm_config_input db_config_input;
				db_config_input.device_handle = device_handle;
				memmove(db_config_input.signature,
						p_config_input->header.signature, SIGNATURE_LEN);
				db_config_input.length = p_config_input->header.length;
				db_config_input.revision = p_config_input->header.revision;
				db_config_input.checksum = p_config_input->header.checksum;
				memmove(db_config_input.oem_id,
						p_config_input->header.oem_id, OEM_ID_LEN);
				memmove(db_config_input.oem_table_id,
						p_config_input->header.oem_table_id, OEM_TABLE_ID_LEN);
				db_config_input.oem_revision = p_config_input->header.oem_revision;
				db_config_input.creator_id = p_config_input->header.creator_id;
				db_config_input.creator_revision =
						p_config_input->header.creator_revision;
				db_config_input.sequence_number = p_config_input->sequence_number;
				DB_DIMM_ERROR(rc, db_add_dimm_config_input(p_db, &db_config_input),
					"Failed to add the configuration input data for " NVM_DIMM_NAME " %d",
					device_handle);

				// write the extension tables
				if (p_config_input->header.length > sizeof (struct config_input_table))
				{
					KEEP_ERROR(rc, update_dimm_extension_tables_in_db(p_db, device_handle,
						TABLE_TYPE_CONFIG_INPUT,
						(struct extension_table_header *)(NVM_UINT8 *)&p_config_input->p_ext_tables,
						p_config_input->header.length - sizeof (struct config_input_table)));
				}
			}
		}

		// write output
		if (p_config_data->config_output_size)
		{
			struct config_output_table *p_config_output = cast_config_output(p_config_data);
			if (p_config_output)
			{
				struct db_dimm_config_output db_config_output;
				db_config_output.device_handle = device_handle;
				memmove(db_config_output.signature,
						p_config_output->header.signature, SIGNATURE_LEN);
				db_config_output.length = p_config_output->header.length;
				db_config_output.revision = p_config_output->header.revision;
				db_config_output.checksum = p_config_output->header.checksum;
				memmove(db_config_output.oem_id,
						p_config_output->header.oem_id, OEM_ID_LEN);
				memmove(db_config_output.oem_table_id,
						p_config_output->header.oem_table_id, OEM_TABLE_ID_LEN);
				db_config_output.oem_revision = p_config_output->header.oem_revision;
				db_config_output.creator_id = p_config_output->header.creator_id;
				db_config_output.creator_revision =
						p_config_output->header.creator_revision;
				db_config_output.sequence_number = p_config_output->sequence_number;
				db_config_output.validation_status = p_config_output->validation_status;
				DB_DIMM_ERROR(rc, db_add_dimm_config_output(p_db, &db_config_output),
					"Failed to add the configuration output data for " NVM_DIMM_NAME " %d",
					device_handle);

				// write the extension tables
				if (p_config_output->header.length > sizeof (struct config_output_table))
				{
					KEEP_ERROR(rc, update_dimm_extension_tables_in_db(p_db, device_handle,
						TABLE_TYPE_CONFIG_OUTPUT,
						(struct extension_table_header *)
								(NVM_UINT8 *)&p_config_output->p_ext_tables,
						p_config_output->header.length - sizeof (struct config_output_table)));
				}
			}
		}

		// write current config
		if (p_config_data->current_config_size)
		{
			struct current_config_table *p_current_config = cast_current_config(p_config_data);

			if (p_current_config)
			{
				struct db_dimm_current_config db_current_config;
				db_current_config.device_handle = device_handle;
				memmove(db_current_config.signature,
						p_current_config->header.signature, SIGNATURE_LEN);
				db_current_config.length = p_current_config->header.length;
				db_current_config.revision = p_current_config->header.revision;
				db_current_config.checksum = p_current_config->header.checksum;
				memmove(db_current_config.oem_id, p_current_config->header.oem_id, OEM_ID_LEN);
				memmove(db_current_config.oem_table_id,
						p_current_config->header.oem_table_id, OEM_TABLE_ID_LEN);
				db_current_config.oem_revision = p_current_config->header.oem_revision;
				db_current_config.creator_id = p_current_config->header.creator_id;
				db_current_config.creator_revision =
						p_current_config->header.creator_revision;
				db_current_config.config_status = p_current_config->config_status;
				db_current_config.mapped_memory_capacity =
						p_current_config->mapped_memory_capacity;
				db_current_config.mapped_app_direct_capacity =
						p_current_config->mapped_app_direct_capacity;
				DB_DIMM_ERROR(rc, db_add_dimm_current_config(p_db, &db_current_config),
					"Failed to add the current configuration data for " NVM_DIMM_NAME " %d",
					device_handle);

				// write the extension tables
				if (p_current_config->header.length > sizeof (struct current_config_table))
				{
					KEEP_ERROR(rc, update_dimm_extension_tables_in_db(p_db, device_handle,
							TABLE_TYPE_CURRENT_CONFIG,
							(struct extension_table_header *)
									(NVM_UINT8 *)&p_current_config->p_ext_tables,
							p_current_config->header.length -
								sizeof (struct current_config_table)));
				}
			}
		}
		db_end_transaction(p_db);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the interleave set dimm info structure from the
 * database and return the size. Data is only copied
 * if p_data is not null.
 */
int get_dimm_interleave_dimms_from_db(PersistentStore *p_db,
		const unsigned int device_handle, const int table_type, const int index,
		struct platform_config_data *p_data, const NVM_UINT32 offset,
		const NVM_UINT32 size)
{
	COMMON_LOG_ENTRY();
	int rc = 0; // 0 dimm info structs

	// get the total number of dimm info table
	int count = 0;
	int db_rc = db_get_interleave_set_dimm_info_count(p_db, &count);
	if (db_rc != DB_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the count of interleave set dimms");
		rc = NVM_ERR_DRIVERFAILED;
	}
	else if (count > 0)
	{
		// get all the dimms for this interleave set, then filter out the ones we want
		struct db_interleave_set_dimm_info db_dimms[count];
		memset(db_dimms, 0, count * sizeof (struct db_interleave_set_dimm_info));
		count = db_get_interleave_set_dimm_infos(p_db, db_dimms, count);
		if (count < DB_SUCCESS)
		{
			COMMON_LOG_ERROR("Failed to retrieve the interleave set dimms");
			rc = NVM_ERR_DRIVERFAILED;
		}
		else
		{
			for (int i = 0; i < count; i++)
			{
				if (db_dimms[i].device_handle == device_handle &&
						db_dimms[i].index_id == index &&
						db_dimms[i].config_table_type == table_type)
				{
					if (p_data)
					{
						// make sure we have enough space for the dimm info struct
						if (size < p_data->header.length + rc +
								sizeof (struct dimm_info_extension_table))
						{
							COMMON_LOG_ERROR(
								"Platform config data buffer size is too small");
							rc = NVM_ERR_BADDEVICECONFIG;
							break;
						}

						// copy the data
						struct dimm_info_extension_table *p_dimm =
							(struct dimm_info_extension_table *)
							((NVM_UINT8 *)p_data + offset + rc);
						// copy the dimm info data over
						p_dimm->size = db_dimms[i].size;
						p_dimm->offset = db_dimms[i].offset;

						// convert db storage to unsigned char array
						UINT_TO_MANUFACTURER(db_dimms[i].manufacturer, p_dimm->manufacturer);
						UINT_TO_SERIAL_NUMBER(db_dimms[i].serial_num, p_dimm->serial_number);
						memmove(p_dimm->model_number, db_dimms[i].model_num, NVM_MODEL_LEN - 1);
					}
					rc += sizeof (struct dimm_info_extension_table);
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
/*
 * Get the interleave tables from the
 * database and return the size. Data is only copied
 * if p_data is not null.
 */
int get_dimm_interleave_tables_from_db(PersistentStore *p_db,
		const unsigned int device_handle, const int table_type,
		struct platform_config_data *p_data, const NVM_UINT32 offset,
		const NVM_UINT32 size)
{
	COMMON_LOG_ENTRY();
	int rc = 0; // 0 interleave set tables

	// get the total number of interleave sets
	int count = 0;
	int db_rc = db_get_dimm_interleave_set_count(p_db, &count);
	// 0 is ok, failure is not
	if (db_rc != DB_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the count of interleave sets");
		rc = NVM_ERR_DRIVERFAILED;
	}
	else if (count > 0)
	{
		// get all the interleave sets, then filter out the ones we want
		struct db_dimm_interleave_set db_sets[count];
		memset(db_sets, 0, count * sizeof (struct db_dimm_interleave_set));
		count = db_get_dimm_interleave_sets(p_db, db_sets, count);
		if (count < DB_SUCCESS)
		{
			COMMON_LOG_ERROR("Failed to retrieve the interleave sets");
			rc = NVM_ERR_DRIVERFAILED;
		}
		else
		{
			for (int i = 0; i < count; i++)
			{
				if (db_sets[i].device_handle == device_handle &&
						db_sets[i].config_table_type == table_type)
				{
					if (p_data)
					{
						// make sure we have enough space for the table
						if (size
							< p_data->header.length + rc
								+ sizeof (struct interleave_info_extension_table))
						{
							COMMON_LOG_ERROR(
								"Platform config data buffer size is too small");
							rc = NVM_ERR_BADDEVICECONFIG;
							break;
						}

						// copy the data
						struct interleave_info_extension_table *p_set =
							(struct interleave_info_extension_table *)
							((NVM_UINT8 *)p_data + offset + rc);
						p_set->header.type = db_sets[i].extension_table_type;
						p_set->header.length = sizeof (struct interleave_info_extension_table);
						p_set->index = db_sets[i].index_id;
						p_set->interleave_format = db_sets[i].interleave_format;
						p_set->memory_type = db_sets[i].memory_type;
						p_set->mirror_enable = db_sets[i].mirror_enable;
						p_set->status = db_sets[i].status;
						p_set->dimm_count = (NVM_UINT8) db_sets[i].dimm_count;
					}

					int dimm_info_size = get_dimm_interleave_dimms_from_db(p_db, device_handle,
							table_type, db_sets[i].index_id, p_data,
							offset + rc + sizeof (struct interleave_info_extension_table), size);
					if (dimm_info_size < 0)
					{
						rc = dimm_info_size;
						break;
					}
					else if ((dimm_info_size / sizeof (struct dimm_info_extension_table))
							!= db_sets[i].dimm_count)
					{
						COMMON_LOG_ERROR_F(
							"Dimm count %d is incorrect for interleave set %d, expect %d",
								(int)(dimm_info_size / sizeof (struct dimm_info_extension_table)),
								db_sets[i].index_id,
								db_sets[i].dimm_count);
						rc = NVM_ERR_BADDEVICECONFIG;
						break;
					}

					// add the size of the dimm info structs
					if (p_data)
					{
						struct interleave_info_extension_table *p_set =
							(struct interleave_info_extension_table *)
							((NVM_UINT8 *)p_data + offset + rc);
						p_set->header.length += dimm_info_size;
					}
					rc += sizeof (struct interleave_info_extension_table);
					rc += dimm_info_size;
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the partition size change table from the
 * database and return the size. Data is only copied
 * if p_data is not null.
 */
int get_dimm_partition_table_from_db(PersistentStore *p_db,
		const unsigned int device_handle, const int table_type,
		struct platform_config_data *p_data, const NVM_UINT32 offset,
		const NVM_UINT32 size)
{
	COMMON_LOG_ENTRY();
	int rc = 0;  // size of table found

	// get the total number of partition change tables
	int count = 0;
	int db_rc = db_get_dimm_partition_change_count(p_db, &count);
	if (db_rc != DB_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the count of partition size change tables");
		rc = NVM_ERR_DRIVERFAILED;
	}
	else if (count > 0)
	{
		// get all the tables and then filter
		struct db_dimm_partition_change db_tables[count];
		memset(db_tables, 0, count * sizeof (struct db_dimm_partition_change));
		db_rc = db_get_dimm_partition_changes(p_db, db_tables, count);
		if (db_rc < DB_SUCCESS)
		{
			COMMON_LOG_ERROR("Failed to retrieve the partition size change tables");
			rc = NVM_ERR_DRIVERFAILED;
		}
		else
		{
			// find ours - break after first one
			for (int i = 0; i < db_rc; i++)
			{
				if (db_tables[i].device_handle == device_handle	&&
					db_tables[i].config_table_type == table_type)
				{
					if (p_data)
					{
						// make sure we have enough space for the table
						if (size < p_data->header.length +
								sizeof (struct partition_size_change_extension_table *))
						{
							COMMON_LOG_ERROR(
								"Platform config data buffer size is too small");
							rc = NVM_ERR_BADDEVICECONFIG;
							break;
						}

						// copy the data
						struct partition_size_change_extension_table *p_partition =
							(struct partition_size_change_extension_table *)
							((NVM_UINT8 *)p_data + offset);
						p_partition->header.type = db_tables[i].extension_table_type;
						p_partition->header.length =
								sizeof (struct partition_size_change_extension_table);
						p_partition->partition_size = db_tables[i].partition_size;
						p_partition->status = db_tables[i].status;
					}
					rc = sizeof (struct partition_size_change_extension_table);
					break;
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the configuration input table and extension tables from the db and
 * store in the data structure
 */
int get_dimm_config_input_from_db(PersistentStore *p_db,
		const unsigned int device_handle, struct platform_config_data *p_data,
		const NVM_UINT32 size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct db_dimm_config_input db_config_input;
	memset(&db_config_input, 0, sizeof (db_config_input));
	db_config_input.device_handle = device_handle;

	// no current config data is valid - set the offset/size to 0
	if (db_get_dimm_config_input_by_device_handle(p_db,
			(unsigned int)device_handle, &db_config_input) != DB_SUCCESS)
	{
		p_data->config_input_size = 0;
		p_data->config_input_offset = 0;
	}
	// make sure we're not over-running the data buffer
	else if (size < p_data->header.length + sizeof (struct config_input_table))
	{
		COMMON_LOG_ERROR(
			"Platform config data buffer size is too samll");
		rc = NVM_ERR_BADDEVICECONFIG;
	}
	else
	{
		// update the main table sizes
		p_data->header.length += sizeof (struct config_input_table);
		// config input comes after current config and config output
		p_data->config_input_offset =
				sizeof (struct platform_config_data) +
				p_data->current_config_size +
				p_data->config_output_size;
		p_data->config_input_size = sizeof (struct config_input_table);

		struct config_input_table *p_config_input =
			(struct config_input_table *)((NVM_UINT8 *)p_data + p_data->config_input_offset);
		// copy the config input table data
		memmove(p_config_input->header.signature,
				db_config_input.signature, SIGNATURE_LEN);
		p_config_input->header.length = sizeof (struct config_input_table);
		p_config_input->header.revision = db_config_input.revision;
		memmove(p_config_input->header.oem_id, db_config_input.oem_id, OEM_ID_LEN);
		memmove(p_config_input->header.oem_table_id,
				db_config_input.oem_table_id, OEM_TABLE_ID_LEN);
		p_config_input->header.oem_revision = db_config_input.oem_revision;
		p_config_input->header.creator_id = db_config_input.creator_id;
		p_config_input->header.creator_revision = db_config_input.creator_revision;
		p_config_input->sequence_number = db_config_input.sequence_number;

		// get the partition size change table
		int partition_table_size = get_dimm_partition_table_from_db(p_db, device_handle,
				TABLE_TYPE_CONFIG_INPUT, p_data,
				p_data->config_input_offset + p_data->config_input_size, size);
		// failed to retrieve the partition size change table - error in db
		if (partition_table_size < 0)
		{
			rc = partition_table_size;
		}
		else
		{
			// add the size of the partition size change table
			p_config_input->header.length += partition_table_size;
			p_data->header.length += partition_table_size;
			p_data->config_input_size += partition_table_size;

			// get the interleave extension tables
			int interleave_tables_size = get_dimm_interleave_tables_from_db(
					p_db, device_handle, TABLE_TYPE_CONFIG_INPUT, p_data,
					p_data->config_input_offset + p_data->config_input_size, size);
			if (interleave_tables_size < 0)
			{
				rc = interleave_tables_size;
			}
			else
			{
				// add the size of the interleave tables
				p_config_input->header.length += interleave_tables_size;
				p_data->header.length += interleave_tables_size;
				p_data->config_input_size += interleave_tables_size;

				// calculate checksum
				generate_checksum((NVM_UINT8*)p_data + p_data->config_input_offset,
						p_data->config_input_size,
						CHECKSUM_OFFSET);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the configuration input table and extension tables from the db and
 * store in the data structure
 */
int get_dimm_config_output_from_db(PersistentStore *p_db,
		const unsigned int device_handle, struct platform_config_data *p_data,
		const NVM_UINT32 size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct db_dimm_config_output db_config_output;
	memset(&db_config_output, 0, sizeof (db_config_output));
	db_config_output.device_handle = device_handle;

	// no current config data is valid - set the offset/size to 0
	if (db_get_dimm_config_output_by_device_handle(p_db,
			(unsigned int)device_handle, &db_config_output) != DB_SUCCESS)
	{
		p_data->config_output_size = 0;
		p_data->config_output_offset = 0;
	}
	// make sure we're not over-running the data buffer
	else if (size < (p_data->header.length + sizeof (struct config_output_table)))
	{
		COMMON_LOG_ERROR(
			"Platform config data buffer size is too small");
		rc = NVM_ERR_BADDEVICECONFIG;
	}
	else
	{
		// update the main table sizes
		p_data->header.length += sizeof (struct config_output_table);
		// config output comes after current config
		p_data->config_output_offset = sizeof (struct platform_config_data) +
				p_data->current_config_size;
		p_data->config_output_size = sizeof (struct config_output_table);
		struct config_output_table *p_config_output =
				(struct config_output_table *)((NVM_UINT8 *)p_data + p_data->config_output_offset);

		memmove(p_config_output->header.signature,
				db_config_output.signature, SIGNATURE_LEN);
		p_config_output->header.length = sizeof (struct config_output_table);
		p_config_output->header.revision = db_config_output.revision;
		memmove(p_config_output->header.oem_id, db_config_output.oem_id, OEM_ID_LEN);
		memmove(p_config_output->header.oem_table_id,
				db_config_output.oem_table_id, OEM_TABLE_ID_LEN);
		p_config_output->header.oem_revision = db_config_output.oem_revision;
		p_config_output->header.creator_id = db_config_output.creator_id;
		p_config_output->header.creator_revision = db_config_output.creator_revision;
		p_config_output->sequence_number = db_config_output.sequence_number;
		p_config_output->validation_status = db_config_output.validation_status;

		// get the size of the partition size change table
		int partition_table_size = get_dimm_partition_table_from_db(p_db, device_handle,
				TABLE_TYPE_CONFIG_OUTPUT, p_data,
				p_data->config_output_offset + p_data->config_output_size, size);
		// failed to retrieve the partition size change table - error in db
		if (partition_table_size < 0)
		{
			rc = partition_table_size;
		}
		else
		{
			// add the size of the partition table
			p_config_output->header.length += partition_table_size;
			p_data->header.length += partition_table_size;
			p_data->config_output_size += partition_table_size;

			// get the interleave extension tables
			int interleave_tables_size = get_dimm_interleave_tables_from_db(
					p_db, device_handle, TABLE_TYPE_CONFIG_OUTPUT, p_data,
					p_data->config_output_offset + p_data->config_output_size, size);
			if (interleave_tables_size < 0)
			{
				rc = interleave_tables_size;
			}
			else
			{
				// add the size of the interleave tables
				p_config_output->header.length += interleave_tables_size;
				p_data->header.length += interleave_tables_size;
				p_data->config_output_size += interleave_tables_size;

				// calculate checksum
				generate_checksum((NVM_UINT8*)p_data + p_data->config_output_offset,
						p_data->config_output_size,
						CHECKSUM_OFFSET);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the current configuration table and extension tables from the db and
 * store in the data structure
 */
int get_dimm_current_config_from_db(PersistentStore *p_db,
		const unsigned int device_handle, struct platform_config_data *p_data,
		const NVM_UINT32 size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct db_dimm_current_config db_current_config;
	memset(&db_current_config, 0, sizeof (db_current_config));
	db_current_config.device_handle = device_handle;

	// no current config data is valid - set the offset/size to 0
	if (db_get_dimm_current_config_by_device_handle(p_db,
			(unsigned int)device_handle, &db_current_config) != DB_SUCCESS)
	{
		p_data->current_config_size = 0;
		p_data->current_config_offset = 0;
	}
	// make sure we're not over-running the data buffer
	else if (size < (p_data->header.length + sizeof (struct current_config_table)))
	{
		COMMON_LOG_ERROR(
			"Platform config data buffer size is too small");
		rc = NVM_ERR_BADDEVICECONFIG;
	}
	else
	{
		// current config is right after header
		p_data->header.length += sizeof (struct current_config_table);
		p_data->current_config_offset = sizeof (struct platform_config_data);
		p_data->current_config_size = sizeof (struct current_config_table);
		struct current_config_table *p_current_config =
			(struct current_config_table *)((NVM_UINT8 *)p_data + p_data->current_config_offset);

		// copy the table body
		memmove(p_current_config->header.signature,
				db_current_config.signature, SIGNATURE_LEN);
		p_current_config->header.length = sizeof (struct current_config_table);
		p_current_config->header.revision = db_current_config.revision;
		memmove(p_current_config->header.oem_id,
				db_current_config.oem_id, OEM_ID_LEN);
		memmove(p_current_config->header.oem_table_id,
				db_current_config.oem_table_id, OEM_TABLE_ID_LEN);
		p_current_config->header.oem_revision =
				db_current_config.oem_revision;
		p_current_config->header.creator_id =
				db_current_config.creator_id;
		p_current_config->header.creator_revision =
				db_current_config.creator_revision;
		p_current_config->config_status =
				db_current_config.config_status;
		p_current_config->mapped_memory_capacity =
				db_current_config.mapped_memory_capacity;
		p_current_config->mapped_app_direct_capacity =
				db_current_config.mapped_app_direct_capacity;

		// get the interleave extension tables
		int interleave_tables_size = get_dimm_interleave_tables_from_db(
				p_db, device_handle, TABLE_TYPE_CURRENT_CONFIG, p_data,
				p_data->current_config_offset + p_data->current_config_size, size);
		if (interleave_tables_size < 0)
		{
			rc = interleave_tables_size;
		}
		else
		{
			// add the size of the interleave tables
			p_current_config->header.length += interleave_tables_size;
			p_data->header.length += interleave_tables_size;
			p_data->current_config_size += interleave_tables_size;

			// calculate checksum
			generate_checksum((NVM_UINT8*)p_data + p_data->current_config_offset,
					p_data->current_config_size,
					CHECKSUM_OFFSET);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_dimm_platform_config_data_from_db(PersistentStore *p_db,
		const unsigned int device_handle, struct platform_config_data *p_data,
		const NVM_UINT32 size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// check input
	if (!p_db)
	{
		COMMON_LOG_ERROR("Database is invalid");
		rc = NVM_ERR_UNKNOWN;
	}
	else if (p_data == NULL)
	{
		COMMON_LOG_ERROR("p_data is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else if (size < sizeof (struct platform_config_data))
	{
		COMMON_LOG_ERROR("size is too small");
		rc = NVM_ERR_BADSIZE;
	}
	else
	{
		// get the header from the database
		struct db_dimm_platform_config db_config_header;
		memset(&db_config_header, 0, sizeof (db_config_header));
		db_config_header.device_handle = device_handle;
		if (db_get_dimm_platform_config_by_device_handle(p_db,
				(unsigned int)device_handle, &db_config_header) != DB_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to get simulated platform config for id: %i.",
				device_handle);
			rc = NVM_ERR_BADDEVICECONFIG;
		}
		else
		{
			// copy the table header
			memmove(p_data->header.signature,
					db_config_header.signature, SIGNATURE_LEN);
			p_data->header.revision =
					db_config_header.revision;
			memmove(p_data->header.oem_id,
					db_config_header.oem_id, OEM_ID_LEN);
			memmove(p_data->header.oem_table_id,
					db_config_header.oem_table_id, OEM_TABLE_ID_LEN);
			p_data->header.oem_revision =
					db_config_header.oem_revision;
			p_data->header.creator_id =
					db_config_header.creator_id;
			p_data->header.creator_revision =
					db_config_header.creator_revision;

			// manually set the lengths - ignore whatever is in the database
			p_data->header.length = sizeof (struct platform_config_data);

			// get the current config data
			rc = get_dimm_current_config_from_db(p_db, device_handle, p_data, size);

			// get the configuration output extension table
			if (rc == NVM_SUCCESS)
			{
				rc = get_dimm_config_output_from_db(p_db, device_handle, p_data, size);

				// get the configuration input extension table
				if (rc == NVM_SUCCESS)
				{
					rc = get_dimm_config_input_from_db(p_db, device_handle, p_data, size);

					// calculate the checksum
					if (rc == NVM_SUCCESS)
					{
						// calculate the platform config table checksum
						generate_checksum((NVM_UINT8*)p_data,
								p_data->header.length, CHECKSUM_OFFSET);
					}
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_dimm_config_output_size_from_db(PersistentStore *p_db, const unsigned int device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = 0;
	struct db_dimm_config_output db_config_output;
	memset(&db_config_output, 0, sizeof (db_config_output));
	db_config_output.device_handle = device_handle;

	// found a config output table for this dimm
	if (db_get_dimm_config_output_by_device_handle(p_db,
			(unsigned int)device_handle, &db_config_output) == DB_SUCCESS)
	{
		rc = sizeof (struct config_input_table);

		// get the partition size table size
		int partition_table_size =
				get_dimm_partition_table_from_db(p_db, device_handle,
				TABLE_TYPE_CONFIG_OUTPUT, NULL, 0, 0);
		if (partition_table_size < 0)
		{
			rc = partition_table_size; // Propagate the error
		}
		else
		{
			rc += partition_table_size;

			// get the size fo the interleave tables
			int interleave_table_size = get_dimm_interleave_tables_from_db(
					p_db, device_handle, TABLE_TYPE_CONFIG_OUTPUT, NULL, 0, 0);
			if (interleave_table_size < 0)
			{
				rc = interleave_table_size;
			}
			else
			{
				rc += interleave_table_size;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_dimm_config_input_size_from_db(PersistentStore *p_db, const unsigned int device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	struct db_dimm_config_input db_config_input;
	memset(&db_config_input, 0, sizeof (db_config_input));
	db_config_input.device_handle = device_handle;

	// found a config input table for this dimm
	if (db_get_dimm_config_input_by_device_handle(p_db,
			(unsigned int)device_handle, &db_config_input) == DB_SUCCESS)
	{
		rc = sizeof (struct config_input_table);

		int partition_table_size =
				get_dimm_partition_table_from_db(p_db, device_handle,
				TABLE_TYPE_CONFIG_INPUT, NULL, 0, 0);
		if (partition_table_size < 0)
		{
			rc = partition_table_size; // Propagate the error
		}
		else
		{
			rc += partition_table_size;

			// get the size of the interleave tables
			int interleave_table_size = get_dimm_interleave_tables_from_db(
					p_db, device_handle, TABLE_TYPE_CONFIG_INPUT, NULL, 0, 0);
			if (interleave_table_size < 0)
			{
				rc = interleave_table_size;
			}
			else
			{
				rc += interleave_table_size;
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_dimm_current_config_size_from_db(PersistentStore *p_db, const unsigned int device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	struct db_dimm_current_config db_current_config;
	memset(&db_current_config, 0, sizeof (db_current_config));
	db_current_config.device_handle = device_handle;

	// found a current config table for this dimm
	if (db_get_dimm_current_config_by_device_handle(p_db,
			(unsigned int)device_handle, &db_current_config) == DB_SUCCESS)
	{
		rc = sizeof (struct current_config_table);

		int interleave_table_size = get_dimm_interleave_tables_from_db(
				p_db, device_handle, TABLE_TYPE_CURRENT_CONFIG, NULL, 0, 0);
		if (interleave_table_size < 0)
		{
			rc = interleave_table_size;
		}
		else
		{
			rc += interleave_table_size;
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the size of the platform config data by counting the tables in the database
 */
int get_dimm_platform_config_size_from_db(PersistentStore *p_db, const unsigned int device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	if (!p_db)
	{
		COMMON_LOG_ERROR("Database is invalid");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		// retrieve the header table from the db
		struct db_dimm_platform_config db_config_header;
		memset(&db_config_header, 0, sizeof (db_config_header));
		db_config_header.device_handle = device_handle;
		if (db_get_dimm_platform_config_by_device_handle(p_db,
				(unsigned int)device_handle, &db_config_header) != DB_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to get simulated platform config for id: %i.",
				device_handle);
			rc = NVM_ERR_BADDEVICECONFIG;
		}
		else
		{
			rc = sizeof (struct platform_config_data);

			// get current config table size
			int current_config_size = get_dimm_current_config_size_from_db(p_db, device_handle);
			if (current_config_size < 0)
			{
				rc = current_config_size;
			}
			else
			{
				rc += current_config_size;
				// get config input table size
				int config_input_size = get_dimm_config_input_size_from_db(p_db, device_handle);
				if (config_input_size < 0)
				{
					rc = config_input_size;
				}
				else
				{
					rc += config_input_size;
					// get config output table size
					int config_output_size = get_dimm_config_output_size_from_db(p_db,
						device_handle);
					if (config_output_size < 0)
					{
						rc = config_output_size;
					}
					else
					{
						rc += config_output_size;
					}
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
