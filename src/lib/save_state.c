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
 * This file contains the implementation of save state functions of the native API.
 */

#include "nvm_management.h"
#include "device_adapter.h"
#include "utility.h"
#include <persistence/logging.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include "platform_config_data.h"
#include <string/s_str.h>
#include <string/revision.h>
#include <uid/uid.h>
#include "device_utilities.h"

int support_store_host(PersistentStore *p_store, int history_id);
int support_store_sockets(PersistentStore *p_store, int history_id);
int support_store_platform_capabilities(PersistentStore *p_store, int history_id);
int support_store_dimm_topology(PersistentStore *p_store,
		int history_id, struct nvm_topology topol);
int support_store_identify_dimm(PersistentStore *p_store,
		int history_id, NVM_NFIT_DEVICE_HANDLE device_handle);
int support_store_smart(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);
int support_store_memory(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);
int support_store_fw_image(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE uid);
int support_store_dimm_details(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);
int support_store_dimm_partition_info(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);
int support_store_dimm_security_state(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);
int support_store_namespaces(PersistentStore *p_store, int history_id);
int support_store_fw_error_logs(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);
int support_store_fw_debug_logs(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);

int support_store_driver_capabilities(PersistentStore *p_store, int history_id);
extern int get_fw_die_spare_policy(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct pt_get_die_spare_policy *payload);
int support_store_optional_config_data(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);
int support_store_die_sparing(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);
int support_store_platform_config_data(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle);

int db_get_history_count(const PersistentStore *p_ps, int *p_count)
{
	return table_row_count(p_ps, "history", p_count);
}

/*
 * Capture a snapshot of the current state of the system in the configuration database
 * with the current date/time and optionally a user supplied name and description.
 */
int nvm_save_state(const char *name, const NVM_SIZE name_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	PersistentStore *p_store = NULL;
	int max_no_support_snapshots;
	if (get_bounded_config_value_int(SQL_KEY_SUPPORT_SNAPSHOT_MAX, &max_no_support_snapshots)
			!= COMMON_SUCCESS)
	{ // should never get here
		COMMON_LOG_ERROR_F("Failed to retrieve key %s.", SQL_KEY_SUPPORT_SNAPSHOT_MAX);
	}

	int history_count = 0;
	if (!max_no_support_snapshots)
	{
		COMMON_LOG_WARN("Gather support is disabled");
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else if ((p_store = get_lib_store()) == NULL)
	{
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		if ((rc = table_row_count(p_store, "history", &history_count)) != DB_SUCCESS)
		{
			COMMON_LOG_ERROR("Failed to get number of entries in history table.");
		}
		else
		{
			// add a new row to the history table
			int history_id;
			if ((rc = db_add_history(p_store, name, &history_id)) != DB_SUCCESS)
			{
				COMMON_LOG_ERROR("Failed creating a history table row.");
			}

			if (history_count++ >= max_no_support_snapshots)
			{
				COMMON_LOG_INFO_F(
				"Roll the history tables to user specified maximum number of support snapshots %d",
				max_no_support_snapshots);
				db_roll_history(p_store, max_no_support_snapshots);
			}

			KEEP_ERROR(rc, support_store_host(p_store, history_id));

			// clear interleave tables from store file
			db_delete_all_interleave_set_dimm_infos(p_store);
			db_delete_all_dimm_interleave_sets(p_store);

			KEEP_ERROR(rc, support_store_sockets(p_store, history_id));
			KEEP_ERROR(rc, support_store_platform_capabilities(p_store, history_id));
			KEEP_ERROR(rc, support_store_namespaces(p_store, history_id));
			KEEP_ERROR(rc, support_store_driver_capabilities(p_store, history_id));

			// TODO:  add dimm_long_op_status when implemented

			// iterate through each device (for all adapters)
			int dev_count = get_topology_count();
			if (dev_count > 0)
			{
				// get topology, aka discovery info
				struct nvm_topology topol[dev_count];
				int temprc = get_topology(dev_count, topol);
				if (temprc < NVM_SUCCESS)
				{
					COMMON_LOG_ERROR("Failed getting topology information");
					KEEP_ERROR(rc, temprc);
				}
				else
				{
					dev_count = temprc;
					for (int i = 0; i < dev_count; i++)
					{
						KEEP_ERROR(rc, support_store_dimm_topology(p_store,
								history_id, topol[i]));
						KEEP_ERROR(rc, support_store_identify_dimm(p_store,
								history_id,
								topol[i].device_handle));
						KEEP_ERROR(rc, support_store_smart(p_store,
								history_id, topol[i].device_handle));
						KEEP_ERROR(rc, support_store_memory(p_store,
								history_id, topol[i].device_handle));
						KEEP_ERROR(rc, support_store_fw_image(p_store,
								history_id, topol[i].device_handle));
						KEEP_ERROR(rc,
							support_store_dimm_details(p_store,
							history_id, topol[i].device_handle));
						KEEP_ERROR(rc,
							support_store_dimm_partition_info(p_store,
							history_id, topol[i].device_handle));
						KEEP_ERROR(rc,
							support_store_dimm_security_state(p_store,
							history_id, topol[i].device_handle));
						KEEP_ERROR(rc,
							support_store_fw_error_logs(p_store,
							history_id, topol[i].device_handle));
						KEEP_ERROR(rc,
							support_store_fw_debug_logs(p_store,
							history_id, topol[i].device_handle));
						KEEP_ERROR(rc,
							support_store_die_sparing(p_store,
							history_id, topol[i].device_handle));
						KEEP_ERROR(rc,
							support_store_optional_config_data(p_store,
							history_id, topol[i].device_handle));
						KEEP_ERROR(rc,
							support_store_platform_config_data(p_store,
							history_id, topol[i].device_handle));
					} // for each device
				} // get topology success
			} // if dev count > 0
		} // added history entry ok
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper functions
 */

/*
 * Store the host server information in the database specified
 */
int support_store_host(PersistentStore *p_store, int history_id)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// get the host server information
	struct host host_server;
	if ((rc = nvm_get_host(&host_server)) == NVM_SUCCESS)
	{
		// convert host struct to db_host struct
		struct db_host db_host;
		db_host.os_type = (int)host_server.os_type;
		s_strncpy(db_host.name, HOST_NAME_LEN, host_server.name, NVM_COMPUTERNAME_LEN);
		s_strncpy(db_host.os_name, HOST_OS_NAME_LEN, host_server.os_name, NVM_OSNAME_LEN);
		s_strncpy(db_host.os_version, HOST_OS_VERSION_LEN,
				host_server.os_version, NVM_OSVERSION_LEN);
		if (db_save_host_state(p_store, history_id, &db_host) != DB_SUCCESS)
		{
			COMMON_LOG_ERROR("Failed storing host %s history information");
		}

		struct sw_inventory inventory;
		if ((rc = nvm_get_sw_inventory(&inventory)) == NVM_SUCCESS)
		{
			struct db_sw_inventory db_inventory;
			s_strncpy(db_inventory.name, SW_INVENTORY_NAME_LEN, host_server.name,
					NVM_COMPUTERNAME_LEN);
			s_strncpy(db_inventory.mgmt_sw_rev, SW_INVENTORY_MGMT_SW_REV_LEN,
					inventory.mgmt_sw_revision, NVM_VERSION_LEN);
			s_strncpy(db_inventory.vendor_driver_rev, SW_INVENTORY_VENDOR_DRIVER_REV_LEN,
					inventory.vendor_driver_revision, NVM_VERSION_LEN);
			db_inventory.supported_driver_available = inventory.vendor_driver_compatible;
			if (db_save_sw_inventory_state(p_store, history_id, &db_inventory) != DB_SUCCESS)
			{
				COMMON_LOG_ERROR("Failed storing software inventory history information");
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_sockets(PersistentStore *p_store, int history_id)
{
	int rc = NVM_SUCCESS;
	// get numa_nodes
	int socket_count = nvm_get_socket_count();
	if (socket_count > 0)
	{
		struct socket sockets[socket_count];
		if (socket_count != nvm_get_sockets(sockets, socket_count))
		{
			COMMON_LOG_ERROR("Failed getting socket information");
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			struct db_socket db_socket;

			for (int i = 0; i < socket_count; i++)
			{
				memset(&db_socket, 0, sizeof (struct db_socket));

				db_socket.socket_id = sockets[i].id;
				db_socket.type = sockets[i].type;
				db_socket.model = sockets[i].model;
				db_socket.family = sockets[i].family;
				db_socket.brand = sockets[i].brand;
				db_socket.stepping = sockets[i].stepping;
				db_socket.logical_processor_count = sockets[i].logical_processor_count;
				s_strncpy(db_socket.manufacturer, SOCKET_MANUFACTURER_LEN, sockets[i].manufacturer,
						NVM_SOCKET_MANUFACTURER_LEN);
				db_socket.rapl_limited = get_dimm_power_limited(sockets[i].id);

				// save the numa nodes to the history tables
				if (DB_SUCCESS != db_save_socket_state(p_store, history_id, &db_socket))
				{
					COMMON_LOG_ERROR("Failed storing socket history information");
					rc = NVM_ERR_UNKNOWN;
				}
			}
		}
	}
	return rc;
}

int support_store_platform_capabilities(PersistentStore *p_store, int history_id)
{
	int rc = NVM_SUCCESS;

	// retrieve current pcat table
	struct bios_capabilities *p_pcat = calloc(1, sizeof (struct bios_capabilities));
	if (!p_pcat)
	{
		COMMON_LOG_ERROR("Unable to allocate memory for the PCAT structure");
		rc = NVM_ERR_NOMEMORY;
	}
	else
	{
		rc = get_platform_capabilities(p_pcat, PCAT_MAX_LEN);
		if (rc == NVM_SUCCESS)
		{
			rc = update_pcat_in_db(p_store, p_pcat, history_id);
		}
		free(p_pcat);
	}
	return rc;
}

int support_store_namespaces(PersistentStore *p_store, int history_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	// get pools
	int ns_count = nvm_get_namespace_count();
	if (ns_count > 0)
	{
		struct namespace_discovery namespaces[ns_count];
		ns_count = nvm_get_namespaces(namespaces, ns_count);
		if (ns_count < 0)
		{
			COMMON_LOG_ERROR_F(
				"Failed to retrieve namespace list, error %d", ns_count);
			rc = ns_count;
		}
		else if (ns_count > 0) // at least one namespace
		{
			for (int i = 0; i < ns_count; i++)
			{
				// store as much info as we can get - start with discovery info
				struct db_namespace db_namespace;
				memset(&db_namespace, 0, sizeof (db_namespace));
				uid_copy(namespaces[i].namespace_uid, db_namespace.namespace_uid);
				s_strcpy(db_namespace.friendly_name, namespaces[i].friendly_name,
						NVM_NAMESPACE_NAME_LEN);

				// get details
				struct namespace_details details;
				int tmp_rc = nvm_get_namespace_details(namespaces[i].namespace_uid, &details);
				if (tmp_rc != NVM_SUCCESS)
				{
					NVM_UID uid_str;
					uid_copy(namespaces[i].namespace_uid, uid_str);
					COMMON_LOG_ERROR_F(
						"Failed to retrieve namespace details for namespace %s", uid_str);
					KEEP_ERROR(rc, tmp_rc);
				}
				else
				{
					db_namespace.block_count = details.block_count;
					db_namespace.block_size = details.block_size;
					db_namespace.btt = details.btt;
					db_namespace.enabled = details.enabled;
					db_namespace.health = details.health;
					db_namespace.type = details.type;
					db_namespace.memory_page_allocation = details.memory_page_allocation;
				}

				// save the namespace to the history table
				if (db_save_namespace_state(p_store,
						history_id, &db_namespace) != DB_SUCCESS)
				{
					COMMON_LOG_ERROR_F("Failed storing namespace %s history information",
							db_namespace.friendly_name);
					KEEP_ERROR(rc, NVM_ERR_UNKNOWN);
				}
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


int support_store_dimm_topology(PersistentStore *p_store, int history_id,
		struct nvm_topology topol)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// ... for each device ...
	// add topology table
	struct db_dimm_topology db_dimm_topo;
	memset(&db_dimm_topo, 0, sizeof (struct db_dimm_topology));

	db_dimm_topo.device_handle = topol.device_handle.handle;
	db_dimm_topo.id = topol.id;
	db_dimm_topo.vendor_id = topol.vendor_id;
	db_dimm_topo.device_id = topol.device_id;
	db_dimm_topo.revision_id = topol.revision_id;
	db_dimm_topo.subsystem_vendor_id = topol.subsystem_vendor_id;
	db_dimm_topo.subsystem_device_id = topol.subsystem_device_id;
	db_dimm_topo.subsystem_revision_id = topol.subsystem_revision_id;
	db_dimm_topo.manufacturing_info_valid = topol.manufacturing_info_valid;
	db_dimm_topo.manufacturing_location = topol.manufacturing_location;
	db_dimm_topo.manufacturing_date = topol.manufacturing_date;
	db_dimm_topo.type = topol.type;

	// Copy all IFCs
	int max_ifcs = (NVM_MAX_IFCS_PER_DIMM <= DIMM_TOPOLOGY_INTERFACE_FORMAT_CODES_COUNT) ?
			NVM_MAX_IFCS_PER_DIMM : DIMM_TOPOLOGY_INTERFACE_FORMAT_CODES_COUNT;
	for (int i = 0; i < max_ifcs; i++)
	{
		db_dimm_topo.interface_format_codes[i] = topol.fmt_interface_codes[i];
	}

	if (DB_SUCCESS != db_save_dimm_topology_state(p_store, history_id, &db_dimm_topo))
	{
		COMMON_LOG_ERROR("Failed storing topology history information");
		rc = NVM_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_identify_dimm(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// add identify dimm table
	struct pt_payload_identify_dimm id_dimm;

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle.handle;
	cmd.opcode = PT_IDENTIFY_DIMM;
	cmd.sub_opcode = 0;
	cmd.output_payload_size = sizeof (id_dimm);
	cmd.output_payload = &id_dimm;
	int temprc = ioctl_passthrough_cmd(&cmd);
	if (NVM_SUCCESS != temprc)
	{
		COMMON_LOG_ERROR("Failed getting identify dimm information");
		rc = temprc;
	}
	else
	{
		// add identify dimm table
		struct db_identify_dimm db_idimm;
		memset(&db_idimm, 0, sizeof (struct db_identify_dimm));

		db_idimm.device_handle = device_handle.handle;
		db_idimm.vendor_id = id_dimm.vendor_id;
		db_idimm.device_id = id_dimm.device_id;
		db_idimm.revision_id = id_dimm.revision_id;
		db_idimm.block_control_region_offset = id_dimm.obmcr;
		db_idimm.dimm_sku = id_dimm.dimm_sku;
		db_idimm.block_windows = id_dimm.nbw;
		db_idimm.fw_api_version = id_dimm.api_ver;
		db_idimm.fw_sw_mask = id_dimm.fswr;
		db_idimm.interface_format_code = id_dimm.ifc;
		db_idimm.raw_cap = MULTIPLES_TO_BYTES(id_dimm.rc);
		db_idimm.write_flush_addresses = id_dimm.nwfa;
		db_idimm.write_flush_address_start = id_dimm.wfas;
		// convert fw version to string
		build_revision(db_idimm.fw_revision, IDENTIFY_DIMM_FW_REVISION_LEN,
				id_dimm.fwr[4], id_dimm.fwr[3], id_dimm.fwr[2],
				((id_dimm.fwr[1] * 100) + id_dimm.fwr[0]));

		// convert unsigned char array to number for storage in db
		db_idimm.manufacturer = MANUFACTURER_TO_UINT(id_dimm.mf);
		db_idimm.serial_num = SERIAL_NUMBER_TO_UINT(id_dimm.sn);

		s_strncpy(db_idimm.model_num, IDENTIFY_DIMM_MODEL_NUM_LEN,
				(char *)id_dimm.mn, DEV_MODELNUM_LEN);

		if (DB_SUCCESS != db_save_identify_dimm_state(p_store, history_id, &db_idimm))
		{
			COMMON_LOG_ERROR("Failed storing identify dimm history information");
			rc = NVM_ERR_UNKNOWN;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_smart(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// add smart table
	struct pt_payload_smart_health dimm_smart;
	int temprc = fw_get_smart_health(device_handle.handle, &dimm_smart);
	if (NVM_SUCCESS != temprc)
	{
		COMMON_LOG_ERROR("Failed getting dimm smart information");
		rc = temprc;
	}
	else
	{
		struct db_dimm_smart db_smart;
		db_smart.device_handle = device_handle.handle;
		db_smart.validation_flags = dimm_smart.validation_flags.flags;
		db_smart.health_status = dimm_smart.health_status;
		db_smart.media_temperature = dimm_smart.media_temperature;
		db_smart.controller_temperature = dimm_smart.controller_temperature;
		db_smart.spare = dimm_smart.spare;
		db_smart.alarm_trips = dimm_smart.alarm_trips;
		db_smart.percentage_used = dimm_smart.percentage_used;
		db_smart.lss = dimm_smart.lss;
		db_smart.vendor_specific_data_size = dimm_smart.vendor_specific_data_size;
		db_smart.power_cycles = dimm_smart.vendor_data.power_cycles;
		db_smart.power_on_seconds = dimm_smart.vendor_data.power_on_seconds;
		db_smart.uptime = dimm_smart.vendor_data.uptime;
		db_smart.unsafe_shutdowns = dimm_smart.vendor_data.unsafe_shutdowns;
		db_smart.lss_details = dimm_smart.vendor_data.lss_details;
		db_smart.last_shutdown_time = dimm_smart.vendor_data.last_shutdown_time;

		if (DB_SUCCESS != db_save_dimm_smart_state(p_store, history_id, &db_smart))
		{
			COMMON_LOG_ERROR("Failed storing dimm smart history information");
			rc = NVM_ERR_UNKNOWN;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_memory(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();
	int temp_rc;

	// Page 0
	{
		struct pt_payload_memory_info_page0 page;
		temp_rc = fw_get_memory_info_page(device_handle.handle, 0,
			&page, sizeof (page));
		KEEP_ERROR(rc, temp_rc);
		if (temp_rc == NVM_SUCCESS)
		{
			struct db_dimm_memory_info_page0 db_page = { 0 };
			db_page.device_handle = device_handle.handle;
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.bytes_read, db_page.bytes_read);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.bytes_written, db_page.bytes_written);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.read_reqs, db_page.read_reqs);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.write_reqs, db_page.write_reqs);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.block_read_reqs, db_page.block_read_reqs);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.block_write_reqs, db_page.block_write_reqs);

			db_save_dimm_memory_info_page0_state(p_store, history_id, &db_page);
		}
	}

	// Page 1
	{
		struct pt_payload_memory_info_page1 page;
		temp_rc = fw_get_memory_info_page(device_handle.handle, 1,
			&page, sizeof (page));
		KEEP_ERROR(rc, temp_rc);
		if (temp_rc == NVM_SUCCESS)
		{
			struct db_dimm_memory_info_page1 db_page = { 0 };
			db_page.device_handle = device_handle.handle;
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.total_bytes_read, db_page.total_bytes_read);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.total_bytes_written, db_page.total_bytes_written);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.total_read_reqs, db_page.total_read_reqs);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.total_write_reqs, db_page.total_write_reqs);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.total_block_read_reqs,
				db_page.total_block_read_reqs);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.total_block_write_reqs,
				db_page.total_block_write_reqs);

			db_save_dimm_memory_info_page1_state(p_store, history_id, &db_page);
		}
	}

	// Page 2
	{
		struct pt_payload_memory_info_page2 page;
		temp_rc = fw_get_memory_info_page(device_handle.handle, 2,
			&page, sizeof (page));
		KEEP_ERROR(rc, temp_rc);

		if (temp_rc == NVM_SUCCESS)
		{
			struct db_dimm_memory_info_page2 db_page = {
				.device_handle = device_handle.handle,
				.write_count_max = page.write_count_max,
				.write_count_average = page.write_count_average,
				.uncorrectable_host = page.uncorrectable_host,
				.uncorrectable_non_host = page.uncorrectable_non_host,
				.media_errors_uc = page.media_errors_uc
			};
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.media_errors_ce,
				db_page.media_errors_ce);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(page.media_errors_ecc,
				db_page.media_errors_ecc);

			db_save_dimm_memory_info_page2_state(p_store, history_id, &db_page);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_fw_image(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// add fw image table
	struct pt_payload_fw_image_info fw_image_info;
	int temprc = fw_get_fw_image_info(device_handle.handle, &fw_image_info);
	if (NVM_SUCCESS != temprc)
	{
		COMMON_LOG_ERROR("Failed getting firmware image information");
		rc = temprc;
	}
	else
	{
		struct db_dimm_fw_image db_dimm_fw;

		db_dimm_fw.device_handle = device_handle.handle;
		// convert fw version to string
		FW_VER_ARR_TO_STR(fw_image_info.fw_rev, db_dimm_fw.fw_rev, DIMM_FW_IMAGE_FW_REV_LEN);
		FW_VER_ARR_TO_STR(
			fw_image_info.staged_fw_rev, db_dimm_fw.staged_fw_rev, DIMM_FW_IMAGE_FW_REV_LEN);
		db_dimm_fw.fw_type = fw_image_info.fw_type;
		db_dimm_fw.staged_fw_type = fw_image_info.staged_fw_type;
		db_dimm_fw.staged_fw_status = fw_image_info.staged_fw_status;
		memmove(db_dimm_fw.commit_id, fw_image_info.commit_id, DEV_FW_COMMIT_ID_LEN);
		memmove(db_dimm_fw.build_configuration, fw_image_info.build_configuration,
			DEV_FW_BUILD_CONFIGURATION_LEN);

		// make sure the string is NULL terminated
		db_dimm_fw.commit_id[DEV_FW_COMMIT_ID_LEN] = 0;
		db_dimm_fw.build_configuration[DEV_FW_BUILD_CONFIGURATION_LEN] = 0;

		if (DB_SUCCESS != db_save_dimm_fw_image_state(p_store, history_id, &db_dimm_fw))
		{
			COMMON_LOG_ERROR("Failed storing firmware image history information");
			rc = NVM_ERR_UNKNOWN;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_dimm_details(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// add details
	struct nvm_details dimm_details;
	int temprc = get_dimm_details(device_handle, &dimm_details);
	if (NVM_SUCCESS != temprc)
	{
		COMMON_LOG_ERROR("Failed getting dimm details information");
		rc = temprc;
	}
	else
	{
		struct db_dimm_details db_details;

		db_details.device_handle = device_handle.handle;
		db_details.form_factor = dimm_details.form_factor;
		db_details.data_width = dimm_details.data_width;
		db_details.total_width = dimm_details.total_width;
		db_details.speed = dimm_details.speed;
		db_details.size = dimm_details.size;
		db_details.type = dimm_details.type;
		db_details.type_detail = dimm_details.type_detail_bits;
		db_details.id = dimm_details.id;
		s_strncpy(db_details.part_number, DIMM_DETAILS_PART_NUMBER_LEN,
				dimm_details.part_number, NVM_PART_NUM_LEN);
		s_strncpy(db_details.device_locator, DIMM_DETAILS_DEVICE_LOCATOR_LEN,
						dimm_details.device_locator, NVM_DEVICE_LOCATOR_LEN);
		s_strncpy(db_details.bank_label, DIMM_DETAILS_BANK_LABEL_LEN,
				dimm_details.bank_label, NVM_BANK_LABEL_LEN);
		s_strncpy(db_details.manufacturer, DIMM_DETAILS_MANUFACTURER_LEN,
				dimm_details.manufacturer, NVM_MANUFACTURERSTR_LEN);
		if (DB_SUCCESS
				!= db_save_dimm_details_state(p_store, history_id, &db_details))
		{
			COMMON_LOG_ERROR("Failed storing dimm "
					"details history information");
			rc = NVM_ERR_UNKNOWN;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_dimm_partition_info(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// get total capacities from the dimm partition info
	struct pt_payload_get_dimm_partition_info pi;
	memset(&pi, 0, sizeof (pi));
	struct fw_cmd partition_cmd;
	memset(&partition_cmd, 0, sizeof (partition_cmd));
	partition_cmd.device_handle = device_handle.handle;
	partition_cmd.opcode = PT_GET_ADMIN_FEATURES;
	partition_cmd.sub_opcode = SUBOP_DIMM_PARTITION_INFO;
	partition_cmd.output_payload_size = sizeof (pi);
	partition_cmd.output_payload = &pi;
	if ((rc = ioctl_passthrough_cmd(&partition_cmd)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed getting dimm %u partition information",
				device_handle.handle);
	}
	else
	{
		// store dimm partition info in support db
		struct db_dimm_partition db_partition;
		memset(&db_partition, 0, sizeof (db_partition));
		db_partition.device_handle = device_handle.handle;
		db_partition.pm_start = pi.start_pmem;
		db_partition.pmem_capacity = pi.pmem_capacity;
		db_partition.raw_capacity = pi.raw_capacity;
		db_partition.volatile_capacity = pi.volatile_capacity;
		db_partition.volatile_start = pi.start_volatile;
		if (db_save_dimm_partition_state(p_store,
				history_id, &db_partition) != DB_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to store dimm %u partition history information",
					device_handle.handle);
			rc = NVM_ERR_UNKNOWN;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_dimm_security_state(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// get the current security state
	struct pt_payload_get_security_state security_state;
	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle.handle;
	cmd.opcode = PT_GET_SEC_INFO;
	cmd.sub_opcode = 0;
	cmd.output_payload_size = sizeof (security_state);
	cmd.output_payload = &security_state;
	int temprc = ioctl_passthrough_cmd(&cmd);
	if (temprc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to get the security state for dimm %d", device_handle.handle);
		rc = temprc;
	}
	// add it to the database
	else
	{
		struct db_dimm_security_info db_security;
		db_security.device_handle = device_handle.handle;
		db_security.security_state = security_state.security_status;
		if (db_save_dimm_security_info_state(
				p_store, history_id, &db_security) != DB_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to store the security state for dimm %d",
				device_handle.handle);
			rc = NVM_ERR_UNKNOWN;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_low_priority_media_logs(PersistentStore *p_store, int history_id,
	NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// get the total number of low priority media log entries
	unsigned int error_count = fw_get_fw_error_log_count(device_handle.handle,
			DEV_FW_ERR_LOG_LOW, DEV_FW_ERR_LOG_MEDIA);
	if (error_count <= 0)
	{
		rc = error_count;
	}
	else
	{
		// get low priority media error log entries
		NVM_UINT8 *large_buffer = calloc(1, error_count * sizeof (struct pt_fw_media_log_entry));
		if (large_buffer != NULL)
		{
			int temprc = fw_get_fw_error_logs(device_handle.handle,
					error_count, large_buffer,
					DEV_FW_ERR_LOG_LOW, DEV_FW_ERR_LOG_MEDIA);
			if (temprc != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F(
				"Failed to get low priority firmware media error logs for dimm %d",
				device_handle.handle);
				KEEP_ERROR(rc, temprc);
			}
			else
			{
				struct pt_fw_media_log_entry *p_low_media_logs =
					(struct pt_fw_media_log_entry *)large_buffer;
				struct db_fw_media_low_log_entry media_low_log;
				for (int i = 0; i < error_count; i++)
				{
					memset(&media_low_log, 0, sizeof (media_low_log));
					media_low_log.device_handle = device_handle.handle;
					media_low_log.system_timestamp = p_low_media_logs[i].system_timestamp;
					media_low_log.dpa = p_low_media_logs[i].dpa;
					media_low_log.pda = p_low_media_logs[i].pda;
					media_low_log.transaction_type = p_low_media_logs[i].transaction_type;
					media_low_log.error_flags = p_low_media_logs[i].error_flags;
					media_low_log.error_type = p_low_media_logs[i].error_type;
					media_low_log.range = p_low_media_logs[i].range;
					db_save_fw_media_low_log_entry_state(p_store, history_id, &media_low_log);
				}
			}
			free(large_buffer);
			large_buffer = NULL;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_high_priority_media_logs(PersistentStore *p_store, int history_id,
	NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// get the total number of high priority media log entries
	unsigned int error_count = fw_get_fw_error_log_count(device_handle.handle,
			DEV_FW_ERR_LOG_HIGH, DEV_FW_ERR_LOG_MEDIA);
	if (error_count <= 0)
	{
		rc = error_count;
	}
	else
	{
		// get high priority media error log entries
		NVM_UINT8 *large_buffer = calloc(1, error_count * sizeof (struct pt_fw_media_log_entry));
		if (large_buffer != NULL)
		{
			int temprc = fw_get_fw_error_logs(device_handle.handle,
								error_count, large_buffer,
								DEV_FW_ERR_LOG_HIGH, DEV_FW_ERR_LOG_MEDIA);
			if (temprc != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F(
				"Failed to get high priority firmware media error logs for dimm %d",
				device_handle.handle);
				KEEP_ERROR(rc, temprc);
			}
			else
			{
				struct pt_fw_media_log_entry *p_high_media_logs =
					(struct pt_fw_media_log_entry *)large_buffer;
				struct db_fw_media_high_log_entry media_high_log;
				for (int i = 0; i < error_count; i++)
				{
					memset(&media_high_log, 0, sizeof (media_high_log));
					media_high_log.device_handle = device_handle.handle;
					media_high_log.system_timestamp = p_high_media_logs[i].system_timestamp;
					media_high_log.dpa = p_high_media_logs[i].dpa;
					media_high_log.pda = p_high_media_logs[i].pda;
					media_high_log.transaction_type = p_high_media_logs[i].transaction_type;
					media_high_log.error_flags = p_high_media_logs[i].error_flags;
					media_high_log.error_type = p_high_media_logs[i].error_type;
					media_high_log.range = p_high_media_logs[i].range;
					db_save_fw_media_high_log_entry_state(p_store, history_id, &media_high_log);
				}
			}
			free(large_buffer);
			large_buffer = NULL;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_low_priority_thermal_logs(PersistentStore *p_store, int history_id,
	NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// get the total number of low priority thermal log entries
	unsigned int error_count = fw_get_fw_error_log_count(device_handle.handle,
			DEV_FW_ERR_LOG_LOW, DEV_FW_ERR_LOG_THERMAL);
	if (error_count <= 0)
	{
		rc = error_count;
	}
	else
	{
		// get low priority thermal error log entries
		NVM_UINT8 *large_buffer = calloc(1, error_count * sizeof (struct pt_fw_thermal_log_entry));
		if (large_buffer != NULL)
		{
			int temprc = fw_get_fw_error_logs(device_handle.handle,
							error_count, large_buffer,
							DEV_FW_ERR_LOG_LOW, DEV_FW_ERR_LOG_THERMAL);
			if (temprc != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F(
				"Failed to get low priority firmware thermal error logs for dimm %d",
				device_handle.handle);
				KEEP_ERROR(rc, temprc);
			}
			else
			{
				struct pt_fw_thermal_log_entry *p_thermal_logs =
					(struct pt_fw_thermal_log_entry *)large_buffer;
				struct db_fw_thermal_low_log_entry thermal_low_log;
				for (int i = 0; i < error_count; i++)
				{
					memset(&thermal_low_log, 0, sizeof (thermal_low_log));
					thermal_low_log.device_handle = device_handle.handle;
					thermal_low_log.host_reported_temp_data =
							p_thermal_logs[i].host_reported_temp_data;
					thermal_low_log.system_timestamp = p_thermal_logs[i].system_timestamp;
					db_save_fw_thermal_low_log_entry_state(p_store, history_id, &thermal_low_log);
				}
			}
			free(large_buffer);
			large_buffer = NULL;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_high_priority_thermal_logs(PersistentStore *p_store, int history_id,
	NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	// get the total number of high priority thermal log entries
	unsigned int error_count = fw_get_fw_error_log_count(device_handle.handle,
		DEV_FW_ERR_LOG_HIGH, DEV_FW_ERR_LOG_THERMAL);
	if (error_count <= 0)
	{
		rc = error_count;
	}
	else
	{
		// get high priority thermal error log entries
		NVM_UINT8 *large_buffer = calloc(1, error_count * sizeof (struct pt_fw_thermal_log_entry));
		if (large_buffer != NULL)
		{
			int temprc = fw_get_fw_error_logs(device_handle.handle,
							error_count, large_buffer,
							DEV_FW_ERR_LOG_HIGH, DEV_FW_ERR_LOG_THERMAL);
			if (temprc != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F(
				"Failed to get high priority firmware thermal error logs for dimm %d",
				device_handle.handle);
				KEEP_ERROR(rc, temprc);
			}
			else
			{
				struct pt_fw_thermal_log_entry *p_thermal_logs =
					(struct pt_fw_thermal_log_entry *)large_buffer;
				struct db_fw_thermal_high_log_entry thermal_high_log;
				for (int i = 0; i < error_count; i++)
				{
					memset(&thermal_high_log, 0, sizeof (thermal_high_log));
					thermal_high_log.device_handle = device_handle.handle;
					thermal_high_log.host_reported_temp_data =
							p_thermal_logs[i].host_reported_temp_data;
					thermal_high_log.system_timestamp = p_thermal_logs[i].system_timestamp;
					db_save_fw_thermal_high_log_entry_state(p_store, history_id, &thermal_high_log);
				}
			}
			free(large_buffer);
			large_buffer = NULL;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_high_priority_thermal_log_info(PersistentStore *p_store, int history_id,
	NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct pt_payload_fw_log_info_data log_info_data;
	memset(&log_info_data, 0, sizeof (log_info_data));
	// get the total number of high priority thermal log entries
	unsigned int temprc = fw_get_fw_error_log_info_data(device_handle.handle,
		DEV_FW_ERR_LOG_HIGH, DEV_FW_ERR_LOG_THERMAL, &log_info_data);
	if (temprc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F(
		"Failed to get high priority firmware thermal error log info for dimm %d",
		device_handle.handle);
		KEEP_ERROR(rc, temprc);
	}
	else
	{
		struct db_fw_thermal_high_log_info db_log_info;
		memset(&db_log_info, 0, sizeof (db_log_info));
		db_log_info.device_handle = device_handle.handle;
		db_log_info.max_log_entries = log_info_data.max_log_entries;
		db_log_info.new_log_entries = log_info_data.new_log_entries;
		db_log_info.oldest_log_entry_timestamp = log_info_data.oldest_log_entry_timestamp;
		db_log_info.newest_log_entry_timestamp = log_info_data.newest_log_entry_timestamp;
		db_save_fw_thermal_high_log_info_state(p_store, history_id, &db_log_info);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_low_priority_thermal_log_info(PersistentStore *p_store, int history_id,
	NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct pt_payload_fw_log_info_data log_info_data;
	memset(&log_info_data, 0, sizeof (log_info_data));
	// get the total number of low priority thermal log entries
	unsigned int temprc = fw_get_fw_error_log_info_data(device_handle.handle,
		DEV_FW_ERR_LOG_LOW, DEV_FW_ERR_LOG_THERMAL, &log_info_data);
	if (temprc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F(
		"Failed to get high priority firmware thermal error log info for dimm %d",
		device_handle.handle);
		KEEP_ERROR(rc, temprc);
	}
	else
	{
		struct db_fw_thermal_low_log_info db_log_info;
		memset(&db_log_info, 0, sizeof (db_log_info));
		db_log_info.device_handle = device_handle.handle;
		db_log_info.max_log_entries = log_info_data.max_log_entries;
		db_log_info.new_log_entries = log_info_data.new_log_entries;
		db_log_info.oldest_log_entry_timestamp = log_info_data.oldest_log_entry_timestamp;
		db_log_info.newest_log_entry_timestamp = log_info_data.newest_log_entry_timestamp;
		db_save_fw_thermal_low_log_info_state(p_store, history_id, &db_log_info);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_high_priority_media_log_info(PersistentStore *p_store, int history_id,
	NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct pt_payload_fw_log_info_data log_info_data;
	memset(&log_info_data, 0, sizeof (log_info_data));
	// get the total number of high priority media log entries
	unsigned int temprc = fw_get_fw_error_log_info_data(device_handle.handle,
		DEV_FW_ERR_LOG_HIGH, DEV_FW_ERR_LOG_MEDIA, &log_info_data);
	if (temprc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F(
		"Failed to get high priority firmware thermal error log info for dimm %d",
		device_handle.handle);
		KEEP_ERROR(rc, temprc);
	}
	else
	{
		struct db_fw_media_high_log_info db_log_info;
		memset(&db_log_info, 0, sizeof (db_log_info));
		db_log_info.device_handle = device_handle.handle;
		db_log_info.max_log_entries = log_info_data.max_log_entries;
		db_log_info.new_log_entries = log_info_data.new_log_entries;
		db_log_info.oldest_log_entry_timestamp = log_info_data.oldest_log_entry_timestamp;
		db_log_info.newest_log_entry_timestamp = log_info_data.newest_log_entry_timestamp;
		db_save_fw_media_high_log_info_state(p_store, history_id, &db_log_info);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_low_priority_media_log_info(PersistentStore *p_store, int history_id,
	NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct pt_payload_fw_log_info_data log_info_data;
	memset(&log_info_data, 0, sizeof (log_info_data));
	// get the total number of low priority media log entries
	unsigned int temprc = fw_get_fw_error_log_info_data(device_handle.handle,
		DEV_FW_ERR_LOG_LOW, DEV_FW_ERR_LOG_MEDIA, &log_info_data);
	if (temprc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F(
		"Failed to get high priority firmware thermal error log info for dimm %d",
		device_handle.handle);
		KEEP_ERROR(rc, temprc);
	}
	else
	{
		struct db_fw_media_low_log_info db_log_info;
		memset(&db_log_info, 0, sizeof (db_log_info));
		db_log_info.device_handle = device_handle.handle;
		db_log_info.max_log_entries = log_info_data.max_log_entries;
		db_log_info.new_log_entries = log_info_data.new_log_entries;
		db_log_info.oldest_log_entry_timestamp = log_info_data.oldest_log_entry_timestamp;
		db_log_info.newest_log_entry_timestamp = log_info_data.newest_log_entry_timestamp;
		db_save_fw_media_low_log_info_state(p_store, history_id, &db_log_info);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_fw_error_logs(PersistentStore *p_store, int history_id,
	NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	KEEP_ERROR(rc,
		get_low_priority_media_log_info(p_store, history_id, device_handle));
	KEEP_ERROR(rc,
		get_high_priority_media_log_info(p_store, history_id, device_handle));
	KEEP_ERROR(rc,
		get_low_priority_thermal_log_info(p_store, history_id, device_handle));
	KEEP_ERROR(rc,
		get_high_priority_thermal_log_info(p_store, history_id, device_handle));

	KEEP_ERROR(rc,
		get_low_priority_media_logs(p_store, history_id, device_handle));
	KEEP_ERROR(rc,
		get_high_priority_media_logs(p_store, history_id, device_handle));
	KEEP_ERROR(rc,
		get_low_priority_thermal_logs(p_store, history_id, device_handle));
	KEEP_ERROR(rc,
		get_high_priority_thermal_logs(p_store, history_id, device_handle));

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_fw_debug_logs(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle.handle;
	cmd.opcode = PT_GET_LOG;
	cmd.sub_opcode = SUBOP_FW_DBG_LOG;

	struct pt_payload_input_get_fw_dbg_log input;
	memset(&input, 0, sizeof (input));
	struct pt_payload_output_get_fw_dbg_log output;
	memset(&output, 0, sizeof (output));
	input.log_action = RETRIEVE_LOG_SIZE;
	cmd.input_payload_size = sizeof (input);
	cmd.input_payload = &input;
	cmd.output_payload = &output;
	cmd.output_payload_size = sizeof (output);
	cmd.large_output_payload = NULL;
	cmd.large_output_payload_size = 0;
	rc = ioctl_passthrough_cmd(&cmd);
	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to get log size for dimm %d", device_handle.handle);
	}
	else
	{
		// if the log size doesn't land on a 1MB page boundary, get the whole page
		NVM_UINT8 log_count = round(output.log_size);
		if (log_count)
		{
			memset(&cmd, 0, sizeof (struct fw_cmd));
			cmd.device_handle = device_handle.handle;
			cmd.opcode = PT_GET_LOG;
			cmd.sub_opcode = SUBOP_FW_DBG_LOG;

			memset(&input, 0, sizeof (input));
			input.log_action = GET_LOG_PAGE;
			cmd.input_payload_size = sizeof (input);
			cmd.input_payload = &input;
			char fw_log_data[log_count][DEV_FW_LOG_PAGE_SIZE];
			cmd.large_output_payload = fw_log_data;
			cmd.large_output_payload_size = log_count * DEV_FW_LOG_PAGE_SIZE;
			cmd.output_payload = NULL;
			cmd.output_payload_size = 0;
			rc = ioctl_passthrough_cmd(&cmd);
			if (rc != NVM_SUCCESS)
			{
				COMMON_LOG_ERROR_F("Failed to get log for dimm %d", device_handle.handle);
			}
			else
			{
				struct db_dimm_fw_debug_log dimm_fw_debug_log;
				for (int log_index = 0; log_index < log_count; log_index++)
				{
					memset(&dimm_fw_debug_log, 0, sizeof (dimm_fw_debug_log));
					dimm_fw_debug_log.device_handle = device_handle.handle;
					memmove(dimm_fw_debug_log.fw_log, fw_log_data[log_index],
							DIMM_FW_DEBUG_LOG_FW_LOG_LEN);
					db_save_dimm_fw_debug_log_state(p_store,
							history_id, &dimm_fw_debug_log);
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_driver_capabilities(PersistentStore *p_store, int history_id)
{

	int rc = NVM_SUCCESS;
	struct nvm_driver_capabilities nvm_caps;
	memset(&nvm_caps, 0, sizeof (nvm_caps));
	if (NVM_SUCCESS != (rc = get_driver_capabilities(&nvm_caps)))
	{
		COMMON_LOG_ERROR("Failed to get driver capabilities information");
	}
	else
	{
		struct db_driver_capabilities db_capabilities;
		memset(&db_capabilities, 0, sizeof (db_capabilities));

		db_capabilities.min_namespace_size = nvm_caps.min_namespace_size;
		db_capabilities.num_block_sizes = nvm_caps.num_block_sizes;
		for (unsigned int i = 0; i < nvm_caps.num_block_sizes; i++)
		{
			db_capabilities.block_sizes[i] = nvm_caps.block_sizes[i];
		}
		db_capabilities.namespace_memory_page_allocation_capable =
				nvm_caps.namespace_memory_page_allocation_capable;

		struct db_driver_features db_features;
		memset(&db_features, 0, sizeof (db_features));

		db_features.get_platform_capabilities = nvm_caps.features.get_platform_capabilities;
		db_features.get_topology = nvm_caps.features.get_topology;
		db_features.get_interleave = nvm_caps.features.get_interleave;
		db_features.get_dimm_detail = nvm_caps.features.get_dimm_detail;
		db_features.get_namespaces = nvm_caps.features.get_namespaces;
		db_features.get_namespace_detail = nvm_caps.features.get_namespace_detail;
		db_features.get_address_scrub_data = nvm_caps.features.get_address_scrub_data;
		db_features.get_platform_config_data = nvm_caps.features.get_platform_config_data;
		db_features.get_boot_status = nvm_caps.features.get_boot_status;
		db_features.get_power_data = nvm_caps.features.get_power_data;
		db_features.get_log_page = nvm_caps.features.get_log_page;
		db_features.get_features = nvm_caps.features.get_features;
		db_features.set_features = nvm_caps.features.set_features;
		db_features.create_namespace = nvm_caps.features.create_namespace;
		db_features.rename_namespace = nvm_caps.features.rename_namespace;
		db_features.grow_namespace = nvm_caps.features.grow_namespace;
		db_features.shrink_namespace = nvm_caps.features.shrink_namespace;
		db_features.delete_namespace = nvm_caps.features.delete_namespace;
		db_features.enable_namespace = nvm_caps.features.enable_namespace;
		db_features.disable_namespace = nvm_caps.features.disable_namespace;
		db_features.get_security_state = nvm_caps.features.get_security_state;
		db_features.set_security_state = nvm_caps.features.set_security_state;
		db_features.enable_logging = nvm_caps.features.enable_logging;
		db_features.run_diagnostic = nvm_caps.features.run_diagnostic;
		db_features.set_platform_config = nvm_caps.features.set_platform_config;
		db_features.passthrough = nvm_caps.features.passthrough;
		db_features.start_address_scrub = nvm_caps.features.start_address_scrub;
		db_features.app_direct_mode = nvm_caps.features.app_direct_mode;
		db_features.storage_mode = nvm_caps.features.storage_mode;

		db_save_driver_features_state(p_store, history_id, &db_features);

		rc =  NVM_SUCCESS;
	}

	return rc;
}

// add die sparing
int support_store_die_sparing(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct db_dimm_die_sparing db_die_sparing;
	memset(&db_die_sparing, 0, sizeof (db_die_sparing));

	struct pt_get_die_spare_policy spare_policy;
	memset(&spare_policy, 0, sizeof (spare_policy));

	rc = get_fw_die_spare_policy(device_handle, &spare_policy);
	if (NVM_SUCCESS != rc)
	{
		COMMON_LOG_ERROR_F("Unable to get the device die sparing policy \
				for handle: [%d]", device_handle.handle);
	}
	else
	{
		db_die_sparing.aggressiveness = spare_policy.aggressiveness;
		db_die_sparing.device_handle = device_handle.handle;
		db_die_sparing.enable = spare_policy.enable;
		db_die_sparing.supported_by_rank[0] = (spare_policy.supported & 0x01) ? 1 : 0;
		db_die_sparing.supported_by_rank[1] = (spare_policy.supported & 0x02) ? 1 : 0;
		db_die_sparing.supported_by_rank[2] = (spare_policy.supported & 0x04) ? 1 : 0;
		db_die_sparing.supported_by_rank[3] = (spare_policy.supported & 0x08) ? 1 : 0;

		db_save_dimm_die_sparing_state(p_store, history_id, &db_die_sparing);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

// add optional config data
int support_store_optional_config_data(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct pt_payload_get_config_data_policy config_data;
	rc = fw_get_config_data_policy(
			device_handle.handle, &config_data);
	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Unable to get the optional configuration data policy \
				for handle: [%d]", device_handle.handle);
	}
	else
	{
		struct db_dimm_optional_config_data db_optional_config_data;
		memset(&db_optional_config_data, 0, sizeof (db_optional_config_data));

		db_optional_config_data.device_handle = device_handle.handle;
		db_optional_config_data.first_fast_refresh_enable = config_data.first_fast_refresh;
		db_optional_config_data.viral_policy_enable = config_data.viral_policy_enable;
		db_optional_config_data.viral_status = config_data.viral_status;

		db_save_dimm_optional_config_data_state(p_store, history_id, &db_optional_config_data);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


int support_store_partition_change_table(PersistentStore *p_store,
		int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle,
		const enum config_table_type table_type,
		struct partition_size_change_extension_table *p_partition)
{
	int rc = NVM_SUCCESS;

	struct db_dimm_partition_change db_partition;
db_get_next_dimm_partition_change_id(p_store, &db_partition.id);
	db_partition.device_handle = device_handle.handle;
	db_partition.config_table_type = table_type;
	db_partition.extension_table_type = PARTITION_CHANGE_TABLE;
	db_partition.length = p_partition->header.length;
	db_partition.partition_size = p_partition->partition_size;
	db_partition.status = p_partition->status;

	rc = db_save_dimm_partition_change_state(p_store, history_id, &db_partition);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int support_store_interleave_set(PersistentStore *p_store,
		int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle,
		const enum config_table_type table_type,
		struct interleave_info_extension_table *p_interleave_set)
{
	int rc = NVM_SUCCESS;

	struct db_dimm_interleave_set db_set;
	db_get_next_dimm_interleave_set_id(p_store, &db_set.id);
	db_set.device_handle = device_handle.handle;
	db_set.config_table_type = table_type;
	db_set.extension_table_type = INTERLEAVE_TABLE;
	db_set.length = p_interleave_set->header.length;
	db_set.index_id = p_interleave_set->index;
	db_set.dimm_count = p_interleave_set->dimm_count;
	db_set.memory_type = p_interleave_set->memory_type;
	db_set.interleave_format = p_interleave_set->interleave_format;
	db_set.mirror_enable = p_interleave_set->mirror_enable;
	db_set.status = p_interleave_set->status;

	rc = db_save_dimm_interleave_set_state(p_store, history_id, &db_set);

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
		db_get_next_interleave_set_dimm_info_id(p_store, &db_dimm.id);
		db_dimm.config_table_type = table_type;
		db_dimm.index_id = p_interleave_set->index;
		db_dimm.device_handle = device_handle.handle;
		// convert manufacturer to uint16
		db_dimm.manufacturer = MANUFACTURER_TO_UINT(p_dimm_info->manufacturer);
		// convert serial number to uint32
		db_dimm.serial_num = SERIAL_NUMBER_TO_UINT(p_dimm_info->serial_number);
		s_strcpy(db_dimm.model_num, p_dimm_info->model_number, NVM_MODEL_LEN);
		db_dimm.offset = p_dimm_info->offset;
		db_dimm.size = p_dimm_info->size;

		rc = db_save_interleave_set_dimm_info_state(p_store, history_id, &db_dimm);
		offset += sizeof (struct dimm_info_extension_table);
	}
	return rc;
}

int support_store_extension_tables_in_db(PersistentStore *p_store,
		int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle,
		const enum config_table_type table_type,
		struct extension_table_header *p_top,
		const NVM_UINT32 size)
{
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
			support_store_partition_change_table(p_store,
					history_id,
					device_handle,
					table_type,
					(struct partition_size_change_extension_table *)p_header);
		}
		// is it a interleave table
		else if (p_header->type == INTERLEAVE_TABLE)
		{
			// store it in the db
			rc = support_store_interleave_set(p_store,
					history_id,
					device_handle,
					table_type,
					(struct interleave_info_extension_table *)p_header);
		}
		// else unrecognized table, go to next
		offset += p_header->length;
	}

	return rc;
}

int support_store_platform_config_data(PersistentStore *p_store, int history_id,
		NVM_NFIT_DEVICE_HANDLE device_handle)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct platform_config_data *p_config = NULL;
	rc = get_dimm_platform_config(device_handle, &p_config);

	// make sure we have good data
	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("get_dimm_platform_config failed with return code = %d", rc);
	}
	else
	{
		// platform config data for this dimm
		// header
		struct db_dimm_platform_config db_config;
		memset(&db_config, 0, sizeof (db_config));

		db_config.device_handle = device_handle.handle;
		memmove(db_config.signature, p_config->header.signature, SIGNATURE_LEN);
		db_config.length = p_config->header.length;
		db_config.revision = p_config->header.revision;
		db_config.checksum = p_config->header.checksum;
		memmove(db_config.oem_id, p_config->header.oem_id, OEM_ID_LEN);
		memmove(db_config.oem_table_id, p_config->header.oem_table_id, OEM_TABLE_ID_LEN);
		db_config.oem_revision = p_config->header.oem_revision;
		db_config.creator_id = p_config->header.creator_id;
		db_config.creator_revision = p_config->header.creator_revision;
		db_config.current_config_size = p_config->current_config_size;
		db_config.current_config_offset = p_config->current_config_offset;
		db_config.config_input_size = p_config->config_input_size;
		db_config.config_input_offset = p_config->config_input_offset;
		db_config.config_output_size = p_config->config_output_size;
		db_config.config_output_offset = p_config->config_output_offset;
		KEEP_ERROR(rc, db_save_dimm_platform_config_state(p_store, history_id, &db_config));

		// write config input
		if (p_config->config_input_size)
		{
			struct config_input_table *p_config_input = cast_config_input(p_config);
			if (p_config_input)
			{
				struct db_dimm_config_input db_config_input;
				memset(&db_config_input, 0, sizeof (db_config_input));

				db_config_input.device_handle = device_handle.handle;
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

				KEEP_ERROR(rc, db_save_dimm_config_input_state(p_store,
						history_id, &db_config_input));

				// write the extension tables
				if (p_config_input->header.length > sizeof (struct config_input_table))
				{
					KEEP_ERROR(rc, support_store_extension_tables_in_db(p_store,
						history_id,
						device_handle,
						TABLE_TYPE_CONFIG_INPUT,
						(struct extension_table_header *)(NVM_UINT8 *)&p_config_input->p_ext_tables,
						p_config_input->header.length - sizeof (struct config_input_table)));
				}
			}
		}

		// write output
		if (p_config->config_output_size)
		{
			struct config_output_table *p_config_output = cast_config_output(p_config);
			if (p_config_output)
			{
				struct db_dimm_config_output db_config_output;
				db_config_output.device_handle = device_handle.handle;
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

				KEEP_ERROR(rc, db_save_dimm_config_output_state(p_store,
						history_id, &db_config_output));

				// write the extension tables
				if (p_config_output->header.length > sizeof (struct config_output_table))
				{
					KEEP_ERROR(rc, support_store_extension_tables_in_db(p_store,
						history_id,
						device_handle,
						TABLE_TYPE_CONFIG_OUTPUT,
					(struct extension_table_header *)(NVM_UINT8 *)&p_config_output->p_ext_tables,
						p_config_output->header.length - sizeof (struct config_output_table)));

				}

			}
		}

		// write current config
		if (p_config->current_config_size)
		{
			struct current_config_table *p_current_config = cast_current_config(p_config);

			if (p_current_config)
			{
				struct db_dimm_current_config db_current_config;
				memset(&db_current_config, 0, sizeof (db_current_config));

				db_current_config.device_handle = device_handle.handle;
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

				KEEP_ERROR(rc, db_save_dimm_current_config_state(p_store,
						history_id, &db_current_config));

				// write the extension tables
				if (p_current_config->header.length > sizeof (struct current_config_table))
				{
					KEEP_ERROR(rc, support_store_extension_tables_in_db(p_store,
							history_id,
						device_handle,
						TABLE_TYPE_CURRENT_CONFIG,
					(struct extension_table_header *)(NVM_UINT8 *)&p_current_config->p_ext_tables,
						p_current_config->header.length - sizeof (struct current_config_table)));
				}
			}
		}
	}
	free(p_config);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
