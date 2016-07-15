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
 * This file contains the implementation of the helper functions for
 * reading/writing the BIOS platform capabilities table (PCAT).
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvm_types.h"
#include "platform_capabilities.h"
#include "platform_config_data.h"
#include <persistence/logging.h>

// Helper macro for database functions to update
// the platform configuration data.
#define	DB_PCAT_ERROR(db_rc, new_db_rc, msg) \
	if (db_rc >= NVM_SUCCESS && new_db_rc != DB_SUCCESS) \
	{ \
		COMMON_LOG_ERROR(msg); \
		db_rc = NVM_ERR_DRIVERFAILED; \
	}


int clear_pcat_from_db(PersistentStore *p_db)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// clear out and existing pcat header
	DB_PCAT_ERROR(rc, db_delete_all_platform_capabilitiess(p_db),
		"Failed to remove the existing pcat header");

	// clear existing PCAT platform info extension tables
	DB_PCAT_ERROR(rc, db_delete_all_platform_info_capabilitys(p_db),
			"Failed to remove existing PCAT platform info extension tables");


	// clear existing PCAT interleave info extension tables
	DB_PCAT_ERROR(rc, db_delete_all_interleave_capabilitys(p_db),
			"Failed to remove existing PCAT interleave info extension tables");

	// clear existing PCAT runtime config extension tables
	DB_PCAT_ERROR(rc, db_delete_all_runtime_config_validations(p_db),
		"Failed to remove existing PCAT runtime config validation extension tables");

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Update the platform capabilities data stored in the db
 * Tries to store as much data as possible while propagating any errors.
 */
int update_pcat_in_db(PersistentStore *p_db,
		const struct bios_capabilities *p_capabilities,
		const int history_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!p_db)
	{
		COMMON_LOG_ERROR("Database is invalid");
		rc = NVM_ERR_UNKNOWN;
	}
	else if (!p_capabilities)
	{
		COMMON_LOG_ERROR("p_capabilities is invalid");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	// make sure we have good data
	else if ((rc = check_pcat(p_capabilities)) == NVM_SUCCESS)
	{
		// clear existing platform config data for this dimm
		rc = clear_pcat_from_db(p_db);
		if (rc == NVM_SUCCESS)
		{
			// write new pcat data
			// write header
			struct db_platform_capabilities db_cap;
			memset(&db_cap, 0, sizeof (db_cap));
			memmove(db_cap.signature, p_capabilities->header.signature,
					PLATFORM_CAPABILITIES_SIGNATURE_LEN);
			db_cap.length = p_capabilities->header.length;
			db_cap.revision = p_capabilities->header.revision;
			db_cap.checksum = p_capabilities->header.checksum;
			memmove(db_cap.oem_id, p_capabilities->header.oem_id,
					PLATFORM_CAPABILITIES_OEM_ID_LEN);
			memmove(db_cap.oem_table_id, p_capabilities->header.oem_table_id,
					PLATFORM_CAPABILITIES_OEM_TABLE_ID_LEN);
			db_cap.oem_revision = p_capabilities->header.oem_revision;
			memmove(db_cap.creator_id, p_capabilities->header.creator_id,
					PLATFORM_CAPABILITIES_CREATOR_ID_COUNT);
			db_cap.creator_revision = p_capabilities->header.creator_revision;

			int db_rc;
			if (history_id)
			{
				db_rc = db_save_platform_capabilities_state(p_db, history_id, &db_cap);
			}
			else
			{
				db_rc = db_add_platform_capabilities(p_db, &db_cap);
			}

			// don't continue if we can't store header because it won't be valid
			if (db_rc != DB_SUCCESS)
			{
				COMMON_LOG_ERROR("Unable to store the PCAT header in the db");
				rc = NVM_ERR_DRIVERFAILED;
			}
			else
			{
				NVM_UINT32 offset = PCAT_TABLE_SIZE;
				// write extension tables
				while (offset < p_capabilities->header.length)
				{
					struct pcat_extension_table_header *p_header =
							(struct pcat_extension_table_header *)
							((NVM_UINT8 *)p_capabilities + offset);

					// check the length for validity
					if (p_header->length == 0 ||
							(p_header->length + offset) > p_capabilities->header.length)
					{
						COMMON_LOG_ERROR_F("Extension table length %d invalid", p_header->length);
						rc = NVM_ERR_BADPCAT;
						break;
					}

					// store pcat platform info extension table
					if (p_header->type == PCAT_TABLE_PLATFORM_INFO)
					{
						struct platform_capabilities_ext_table *p_plat_info =
								(struct platform_capabilities_ext_table *)p_header;

						struct db_platform_info_capability db_platform;
						memset(&db_platform, 0, sizeof (db_platform));
						db_get_next_platform_info_capability_id(p_db, &db_platform.id);
						db_platform.type = p_plat_info->header.type;
						db_platform.length = p_plat_info->header.length;
						db_platform.current_mem_mode = p_plat_info->current_mem_mode;
						db_platform.mem_mode_capabilities = p_plat_info->mem_mode_capabilities;
						db_platform.mgmt_sw_config_support = p_plat_info->mgmt_sw_config_support;
						db_platform.pmem_ras_capabilities = p_plat_info->pmem_ras_capabilities;

						// store history
						if (history_id)
						{
							db_rc = db_save_platform_info_capability_state(p_db,
									history_id, &db_platform);
						}
						else
						{
							db_rc = db_add_platform_info_capability(p_db, &db_platform);
						}
						// store error but continue to save as much data as possible
						DB_PCAT_ERROR(rc, db_rc,
							"Unable to store the PCAT platform info extension table in the db");
					}
					// store pcat memory interleave extension table
					else if (p_header->type == PCAT_TABLE_MEMORY_INTERLEAVE_INFO)
					{
						struct memory_interleave_capabilities_ext_table *p_interleave =
								(struct memory_interleave_capabilities_ext_table *)p_header;
						struct db_interleave_capability db_interleave;
						memset(&db_interleave, 0, sizeof (db_interleave));
						db_get_next_interleave_capability_id(p_db, &db_interleave.id);
						db_interleave.type = p_interleave->header.type;
						db_interleave.length = p_interleave->header.length;
						db_interleave.memory_mode = p_interleave->memory_mode;
						db_interleave.interleave_alignment_size =
								p_interleave->interleave_alignment_size;
						db_interleave.supported_interleave_count =
								p_interleave->supported_interleave_count;
						for (int i = 0; i < db_interleave.supported_interleave_count; i++)
						{
							db_interleave.interleave_format_list[i] =
									p_interleave->interleave_format_list[i];
						}
						// store history
						if (history_id)
						{
							db_rc = db_save_interleave_capability_state(p_db,
									history_id, &db_interleave);
						}
						else
						{
							db_rc = db_add_interleave_capability(p_db, &db_interleave);
						}
						// store error but continue to save as much data as possible
						DB_PCAT_ERROR(rc, db_rc,
							"Unable to store the PCAT memory interleave capabilities "
							"extension table in the db");
					}
					// store reconfig input validation extension table
					else if (p_header->type == PCAT_TABLE_RECONFIG_INPUT_VALIDATION)
					{
						struct reconfig_input_validation_ext_table *p_rt_config =
								(struct reconfig_input_validation_ext_table *)p_header;
						struct db_runtime_config_validation db_rt_config;
						memset(&db_rt_config, 0, sizeof (db_rt_config));
						db_get_next_runtime_config_validation_id(p_db, &db_rt_config.id);
						db_rt_config.type = p_rt_config->header.type;
						db_rt_config.length = p_rt_config->header.length;
						db_rt_config.access_size = p_rt_config->access_size;
						db_rt_config.address = p_rt_config->address;
						db_rt_config.address_space_id = p_rt_config->address_space_id;
						db_rt_config.bit_offset = p_rt_config->bit_offset;
						db_rt_config.bit_width = p_rt_config->bit_width;
						memmove(db_rt_config.gas_structure, p_rt_config->gas_structure,
								RUNTIME_CONFIG_VALIDATION_GAS_STRUCTURE_COUNT);
						db_rt_config.mask_1 = p_rt_config->mask_1;
						db_rt_config.mask_2 = p_rt_config->mask_2;
						db_rt_config.operation_type_1 = p_rt_config->operation_type_1;
						db_rt_config.operation_type_2 = p_rt_config->operation_type_2;
						db_rt_config.value = p_rt_config->value;

						// store history
						if (history_id)
						{
							db_rc = db_save_runtime_config_validation_state(p_db,
									history_id, &db_rt_config);
						}
						else
						{
							db_rc = db_add_runtime_config_validation(p_db, &db_rt_config);
						}
						// store error but continue to save as much data as possible
						DB_PCAT_ERROR(rc, db_rc,
								"Unable to store the PCAT runtime config validation "
								"extension table in the db");
					}
					// else, just ignore it other table types
					offset += p_header->length;
				} // end while extension tables
			} // end if save header
		} // end if clear existing
	} // end check input

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to get the PCAT runtime configuration extension table
 * from the database and copy the data to the bios_capabilities struct
 */
int get_pcat_runtime_validation_table_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		NVM_UINT32 *p_offset, const NVM_UINT32 cap_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!p_db)
	{
		COMMON_LOG_ERROR("Database is invalid");
		rc =  NVM_ERR_UNKNOWN;
	}
	else
	{
		struct db_runtime_config_validation db_runtime;
		memset(&db_runtime, 0, sizeof (db_runtime));

		if (db_get_runtime_config_validations(p_db, &db_runtime, 1) == 1)
		{
			// make sure there is space to copy
			NVM_UINT32 space_needed = *p_offset + RT_CONFIG_TABLE_SIZE;
			if (space_needed > cap_len)
			{
				COMMON_LOG_ERROR(
					"p_capabilities buffer is too small to add the runtime validation ext table");
				rc = NVM_ERR_INVALIDPARAMETER;
			}
			else
			{
				struct reconfig_input_validation_ext_table *p_rt_config =
						(struct reconfig_input_validation_ext_table *)
						((NVM_UINT8 *)p_capabilities + *p_offset);
				p_rt_config->header.type = PCAT_TABLE_RECONFIG_INPUT_VALIDATION;
				p_rt_config->header.length = RT_CONFIG_TABLE_SIZE;
				p_rt_config->address_space_id = db_runtime.address_space_id;
				p_rt_config->bit_width = db_runtime.bit_width;
				p_rt_config->bit_offset = db_runtime.bit_offset;
				p_rt_config->access_size = db_runtime.access_size;
				p_rt_config->address = db_runtime.address;
				p_rt_config->operation_type_1 = db_runtime.operation_type_1;
				p_rt_config->value = db_runtime.value;
				p_rt_config->mask_1 = db_runtime.mask_1;
				memmove(p_rt_config->gas_structure, db_runtime.gas_structure,
					RUNTIME_CONFIG_VALIDATION_GAS_STRUCTURE_COUNT);
				p_rt_config->operation_type_2 = db_runtime.operation_type_2;
				p_rt_config->mask_2 = db_runtime.mask_2;

				*p_offset += p_rt_config->header.length;
			}
		}
		// else ignore any db read failures - table is optional
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
/*
 * Helper function to get the PCAT memory interleave extension tables
 * from the database and copy the data to the bios_capabilities structure
 */
int get_pcat_interleave_tables_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		NVM_UINT32 *p_offset, const NVM_UINT32 cap_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!p_db)
	{
		COMMON_LOG_ERROR("Database is invalid");
		rc =  NVM_ERR_UNKNOWN;
	}
	else
	{
		int db_interleave_count = 0;
		if (db_get_interleave_capability_count(p_db, &db_interleave_count) == DB_SUCCESS)
		{
			struct db_interleave_capability db_interleaves[db_interleave_count];
			memset(&db_interleaves, 0,
					sizeof (struct db_interleave_capability) * db_interleave_count);

			db_interleave_count = db_get_interleave_capabilitys(p_db,
					db_interleaves, db_interleave_count);
			for (int i = 0; i < db_interleave_count; i++)
			{
				// make sure there is enough buffer space to copy
				NVM_UINT32 space_needed = *p_offset +
						MEMORY_INTERLEAVE_TABLE_SIZE +
						(db_interleaves[i].supported_interleave_count * sizeof (NVM_UINT32));
				if (space_needed > cap_len)
				{
					COMMON_LOG_ERROR(
						"p_capabilities buffer is too small to add the interleave info ext table");
					rc = NVM_ERR_INVALIDPARAMETER;
					break;
				}
				else
				{
					struct memory_interleave_capabilities_ext_table *p_interleave_info =
						(struct memory_interleave_capabilities_ext_table *)
						((NVM_UINT8 *)p_capabilities + *p_offset);
					p_interleave_info->header.type = PCAT_TABLE_MEMORY_INTERLEAVE_INFO;
					p_interleave_info->header.length = MEMORY_INTERLEAVE_TABLE_SIZE;
					p_interleave_info->memory_mode = db_interleaves[i].memory_mode;
					p_interleave_info->interleave_alignment_size =
							db_interleaves[i].interleave_alignment_size;
					p_interleave_info->supported_interleave_count =
							db_interleaves[i].supported_interleave_count;
					for (int j = 0; j < p_interleave_info->supported_interleave_count; j++)
					{
						p_interleave_info->interleave_format_list[j] =
								db_interleaves[i].interleave_format_list[j];
						p_interleave_info->header.length += sizeof (NVM_UINT32);
					}

					*p_offset += p_interleave_info->header.length;
				}
			}
		}
		// else ignore any db read failures - tables are optional
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to get the PCAT platform capabilities extension table
 * from the database and copy the data to the bios_capabilities struct
 */
int get_pcat_platform_info_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		NVM_UINT32 *p_offset, const NVM_UINT32 cap_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!p_db)
	{
		COMMON_LOG_ERROR("Database is invalid");
		rc =  NVM_ERR_UNKNOWN;
	}
	else
	{
		struct db_platform_info_capability db_platform_info;
		memset(&db_platform_info, 0, sizeof (db_platform_info));

		if (db_get_platform_info_capabilitys(p_db, &db_platform_info, 1) != 1)
		{
			COMMON_LOG_INFO("Unable to retrieve the PCAT platform info ext table from the db");
			rc = 0; // no error, just doesn't exist
		}
		else
		{
			// make sure there is enough buffer space
			NVM_UINT32 space_needed = *p_offset + PLATFORM_INFO_TABLE_SIZE;
			if (space_needed > cap_len)
			{
				COMMON_LOG_ERROR(
					"p_capabilities buffer is too small to add the platform info ext table");
				rc = NVM_ERR_INVALIDPARAMETER;
			}
			else
			{
				struct platform_capabilities_ext_table *p_pcat_info =
					(struct platform_capabilities_ext_table *)
					((NVM_UINT8 *)p_capabilities + *p_offset);
				p_pcat_info->header.type = PCAT_TABLE_PLATFORM_INFO;
				p_pcat_info->header.length = PLATFORM_INFO_TABLE_SIZE;
				p_pcat_info->current_mem_mode = db_platform_info.current_mem_mode;
				p_pcat_info->mem_mode_capabilities = db_platform_info.mem_mode_capabilities;
				p_pcat_info->mgmt_sw_config_support = db_platform_info.mgmt_sw_config_support;
				p_pcat_info->pmem_ras_capabilities = db_platform_info.pmem_ras_capabilities;
				*p_offset += p_pcat_info->header.length;
			}
		}
		// else ignore any db read failures - table is optional
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the capabilities of the host platform
 */
int get_pcat_from_db(PersistentStore *p_db,
		struct bios_capabilities *p_capabilities,
		const NVM_UINT32 cap_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!p_db)
	{
		COMMON_LOG_ERROR("Invalid database.");
		rc = NVM_ERR_UNKNOWN;
	}
	else if (p_capabilities == NULL)
	{
		COMMON_LOG_ERROR("p_capabilities cannot be NULL.");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (cap_len < PCAT_TABLE_SIZE)
	{
		COMMON_LOG_ERROR("p_capabilities buffer too small");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		// get header from db
		struct db_platform_capabilities db_cap;
		memset(&db_cap, 0, sizeof (struct db_platform_capabilities));
		if (db_get_platform_capabilitiess(p_db, &db_cap, 1) != 1)
		{
			COMMON_LOG_ERROR("db_get_platform_capabilities failed");
			rc = NVM_ERR_DRIVERFAILED;
		}
		else
		{
			memmove(p_capabilities->header.signature, db_cap.signature,
					PLATFORM_CAPABILITIES_SIGNATURE_LEN);
			p_capabilities->header.length = db_cap.length; // calculated
			p_capabilities->header.revision = db_cap.revision;
			p_capabilities->header.checksum = db_cap.checksum; // calculated
			memmove(p_capabilities->header.oem_id, db_cap.oem_id, PLATFORM_CAPABILITIES_OEM_ID_LEN);
			memmove(p_capabilities->header.oem_table_id, db_cap.oem_table_id,
					PLATFORM_CAPABILITIES_OEM_TABLE_ID_LEN);
			p_capabilities->header.oem_revision = db_cap.oem_revision;
			memmove(p_capabilities->header.creator_id, db_cap.creator_id,
					PLATFORM_CAPABILITIES_CREATOR_ID_COUNT);
			p_capabilities->header.creator_revision = db_cap.creator_revision;


			// variable length extension tables
			// get the platform info tables
			NVM_UINT32 offset = PCAT_TABLE_SIZE;
			rc = get_pcat_platform_info_from_db(p_db, p_capabilities, &offset, cap_len);
			if (rc == NVM_SUCCESS)
			{
				// get memory interleave tables
				rc = get_pcat_interleave_tables_from_db(p_db, p_capabilities, &offset, cap_len);
				if (rc == NVM_SUCCESS)
				{
					// get runtime validation tables
					rc = get_pcat_runtime_validation_table_from_db(p_db,
							p_capabilities, &offset, cap_len);

					// set the length
					p_capabilities->header.length = offset;

					// generate a valid checksum
					generate_checksum((NVM_UINT8*)p_capabilities,
							p_capabilities->header.length,
							PCAT_CHECKSUM_OFFSET);
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int check_pcat(const struct bios_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// check overall table checksum
	rc = verify_checksum((NVM_UINT8*)p_capabilities, p_capabilities->header.length);
	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("PCAT checksum failed");
		rc = NVM_ERR_BADPCAT;
	}
	// check overall table length is at least as big as the header
	else if (p_capabilities->header.length < PCAT_TABLE_SIZE)
	{
		COMMON_LOG_ERROR("PCAT size is too small");
		rc = NVM_ERR_BADPCAT;
	}
	// check signature
	else if (strncmp(PCAT_TABLE_SIGNATURE,
					p_capabilities->header.signature, PCAT_SIGNATURE_LEN) != 0)
	{
		COMMON_LOG_ERROR_F("PCAT signature mismatch. Expected: %s, actual: %s",
				PCAT_TABLE_SIGNATURE, p_capabilities->header.signature);
		rc = NVM_ERR_BADPCAT;
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Given a starting offset of the extension tables, get the offset of the specfied table
 */
NVM_UINT32 get_offset_of_ext_table(const struct bios_capabilities *p_capabilities,
		enum pcat_ext_table_type type, NVM_UINT32 offset)
{
	COMMON_LOG_ENTRY();
	int rc = offset;
	int found = 0;

	if (p_capabilities)
	{
		// write extension tables
		while (offset < p_capabilities->header.length && !found)
		{
			struct pcat_extension_table_header *p_header =
					(struct pcat_extension_table_header *)
					((NVM_UINT8 *)p_capabilities + offset);

			// check the length for validity
			if (p_header->length == 0 ||
					(p_header->length + offset) > p_capabilities->header.length)
			{
				COMMON_LOG_ERROR_F("Extension table length %d invalid", p_header->length);
				rc = NVM_ERR_BADPCAT;
				break;
			}

			// store pcat platform info extension table
			if (p_header->type == type)
			{
				rc = offset;
				found = 1;
			}
			offset += p_header->length;
		}
	}

	if (!found)
	{
		rc = -1;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
