/*
 * Copyright (c) 2015 2017, Intel Corporation
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
 * This file contains the implementation of device management functions of the Native API
 */

#include <stdlib.h>
#include <uid/uid.h>
#include <os/os_adapter.h>
#include <string/s_str.h>

#include "nvm_management.h"
#include <persistence/logging.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include <firmware_interface/fw_commands.h>
#include "platform_config_data.h"
#include "device_utilities.h"
#include "config_goal.h"
#include "capabilities.h"
#include "nvm_context.h"
#include "system.h"
#include "nvm_types.h"

#define	NFIT_DIMM_STATE_IS_DISABLED(flag) ((flag >> 6) & 1)

/*
 * **************************************************************************
 * API Functions
 * **************************************************************************
 */

int nvm_get_memory_topology_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;
	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		rc = get_smbios_inventory_count();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void nvm_details_to_memory_topology(struct nvm_details *p_details,
		struct memory_topology *p_mem_device)
{
	COMMON_LOG_ENTRY();

	p_mem_device->physical_id = p_details->id;
	p_mem_device->memory_type = get_memory_type_from_smbios_memory_type(p_details->type,
			p_details->type_detail_bits);
	p_mem_device->form_factor =
			get_device_form_factor_from_smbios_form_factor(p_details->form_factor);
	p_mem_device->raw_capacity = p_details->size;
	p_mem_device->data_width = p_details->data_width;
	p_mem_device->total_width = p_details->total_width;
	p_mem_device->speed = p_details->speed;
	memmove(p_mem_device->part_number, p_details->part_number, NVM_PART_NUM_LEN);
	memmove(p_mem_device->device_locator, p_details->device_locator, NVM_DEVICE_LOCATOR_LEN);
	memmove(p_mem_device->bank_label, p_details->bank_label, NVM_BANK_LABEL_LEN);

	COMMON_LOG_EXIT();
}

int nvm_get_memory_topology(struct memory_topology *p_memory_devices, const NVM_UINT8 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_memory_devices == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_devices is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = nvm_get_memory_topology_count()) > 0)
	{
		memset(p_memory_devices, 0, sizeof (struct memory_topology) * count);

		struct nvm_details details[count];
		rc = get_smbios_inventory(count, details);
		if ((rc > 0) || (rc == NVM_ERR_ARRAYTOOSMALL))
		{
			NVM_UINT8 smbios_inv_count = (rc >= 0) ? rc : count;
			for (NVM_UINT8 i = 0; i < smbios_inv_count; i++)
			{
				nvm_details_to_memory_topology(&(details[i]), &(p_memory_devices[i]));
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Returns the number of NVM DIMMs installed in the system whether they are
 * fully compatible with the current Management library version or not.
 * This method should be called before nvm_get_devices.
 */
int nvm_get_device_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = get_nvm_context_device_count()) < 0)
	{
		rc = get_topology_count();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Convert locked value into lock state enumeration
 */
enum lock_state security_state_to_enum(unsigned char security_status)
{
	COMMON_LOG_ENTRY();
	enum lock_state lock_state;
	if (security_status & SEC_NOT_SUPPORTED)
	{
		lock_state = LOCK_STATE_NOT_SUPPORTED;
	}
	else if (security_status & SEC_FROZEN)
	{
		lock_state = LOCK_STATE_FROZEN;
	}
	else if (!(security_status & SEC_ENABLED))
	{
		lock_state = LOCK_STATE_DISABLED;
	}
	else if (security_status & SEC_COUNT_EXP)
	{
		lock_state = LOCK_STATE_PASSPHRASE_LIMIT;
	}
	else if (security_status & SEC_LOCKED)
	{
		lock_state = LOCK_STATE_LOCKED;
	}
	else
	{
		lock_state = LOCK_STATE_UNLOCKED;
	}
	COMMON_LOG_EXIT();
	return lock_state;
}

int add_security_state_to_device(struct device_discovery *p_device)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_get_security_state security_state;
	rc = fw_get_security_state(p_device->device_handle.handle,
			&security_state);
	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F(
			"Unable to get security state information for handle: [%d]",
			p_device->device_handle);
		p_device->lock_state = LOCK_STATE_UNKNOWN;
	}
	else // successfully got security state
	{
		p_device->lock_state =
				security_state_to_enum(security_state.security_status);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void add_identify_dimm_properties_to_device(struct device_discovery *p_device,
		struct pt_payload_identify_dimm *p_id_dimm)
{
	COMMON_LOG_ENTRY();

	// set manageability of the dimm based on the FW rev
	set_device_manageability_from_firmware(p_id_dimm, &p_device->manageability);

	p_device->dimm_sku = p_id_dimm->dimm_sku;

	memmove(p_device->manufacturer, p_id_dimm->mf,
			DEV_MFR_LEN);

	memmove(p_device->part_number, p_id_dimm->pn,
			DEV_PARTNUM_LEN);

	// convert fw version to string
	FW_VER_ARR_TO_STR(p_id_dimm->fwr, p_device->fw_revision,
			NVM_VERSION_LEN);

	// convert fw api version to string
	build_fw_revision(p_device->fw_api_version, NVM_VERSION_LEN,
			get_fw_api_major_version(p_id_dimm->api_ver),
			get_fw_api_minor_version(p_id_dimm->api_ver));

	p_device->capacity = MULTIPLES_TO_BYTES((NVM_UINT64)p_id_dimm->rc);

	add_ifcs_from_identify_dimm_to_device(p_device, p_id_dimm);

	COMMON_LOG_EXIT();
}

int add_firmware_properties_to_device(struct device_discovery *p_device)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// So far manageability is based on topology data
	if (IS_DEVICE_MANAGEABLE(p_device))
	{
		// send a pass through command to get the dimm identify info
		struct pt_payload_identify_dimm id_dimm;
		if ((rc = fw_get_identify_dimm(p_device->device_handle.handle,
				&id_dimm)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F(
					"Unable to get identify dimm information for handle: [%d]",
					p_device->device_handle.handle);
		}
		else
		{
			add_identify_dimm_properties_to_device(p_device, &id_dimm);

			// only get security state if still manageable
			if (IS_DEVICE_MANAGEABLE(p_device))
			{
				add_security_state_to_device(p_device);
			}
		}
		s_memset(&id_dimm, sizeof (id_dimm));
	}
	else
	{
		COMMON_LOG_WARN_F("Device with handle %u has a controller or programming interface "
				"that is not supported."
				"SubsystemVendorID=%hu, SubsystemDeviceID=%hu, "
				"first Interface Format Code=%hu",
				p_device->device_handle.handle,
				p_device->subsystem_vendor_id, p_device->subsystem_device_id,
				p_device->interface_format_codes[0]);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int add_firmware_properties_to_populated_devices(struct device_discovery *p_devices,
		const NVM_UINT8 dev_count)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;

	for (NVM_UINT8 i = 0; i < dev_count; i++)
	{
		rc = add_firmware_properties_to_device(&(p_devices[i]));
		if (rc != NVM_SUCCESS)
		{
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void add_smbios_properties_to_populated_devices(struct device_discovery *p_devices,
		const NVM_UINT8 dev_count)
{
	COMMON_LOG_ENTRY();

	for (int i = 0; i < dev_count; i++)
	{
		struct nvm_details smbios_details;
		memset(&smbios_details, 0, sizeof (smbios_details));
		int rc = get_dimm_details(p_devices[i].device_handle, &smbios_details);
		if (rc == NVM_SUCCESS)
		{
			p_devices[i].capacity = smbios_details.size;
			p_devices[i].memory_type = get_memory_type_from_smbios_memory_type(
					smbios_details.type,
					smbios_details.type_detail_bits);
		}
		else
		{
			COMMON_LOG_ERROR_F("SMBIOS details unavailable for device %u",
					p_devices[i].device_handle.handle);
		}
	}

	COMMON_LOG_EXIT();
}

enum manageability_state get_manageability_from_topology(struct nvm_topology *p_topology)
{
	COMMON_LOG_ENTRY();
	enum manageability_state result = MANAGEMENT_INVALIDCONFIG;
	if (!NFIT_DIMM_STATE_IS_DISABLED(p_topology->state_flags) &&
		is_device_interface_format_supported(p_topology) &&
		is_device_subsystem_controller_supported(p_topology))
	{
		result = MANAGEMENT_VALIDCONFIG;
	}

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

void nvm_topology_to_device(struct nvm_topology *p_topology, struct device_discovery *p_device)
{
	COMMON_LOG_ENTRY();

	p_device->device_handle = p_topology->device_handle;
	p_device->physical_id = p_topology->id;
	p_device->vendor_id = p_topology->vendor_id;
	p_device->device_id = p_topology->device_id;
	p_device->revision_id = p_topology->revision_id;
	p_device->subsystem_vendor_id = p_topology->subsystem_vendor_id;
	p_device->subsystem_device_id = p_topology->subsystem_device_id;
	p_device->subsystem_revision_id = p_topology->subsystem_revision_id;
	p_device->manufacturing_info_valid =
			p_topology->manufacturing_info_valid;
	if (p_device->manufacturing_info_valid == 1)
	{
		p_device->manufacturing_location = p_topology->manufacturing_location;
		p_device->manufacturing_date = p_topology->manufacturing_date;
	}
	else
	{
		p_device->manufacturing_location = 0;
		p_device->manufacturing_date = 0;
	}

	memmove(p_device->serial_number, p_topology->serial_number, NVM_SERIAL_LEN);

	// Could be multiple IFCs - copy them all
	for (int i = 0; i < NVM_MAX_IFCS_PER_DIMM; i++)
	{
		p_device->interface_format_codes[i] = p_topology->fmt_interface_codes[i];
	}

	p_device->manageability = get_manageability_from_topology(p_topology);

	// Populate values derived from handle
	p_device->socket_id =
			(NVM_UINT16)p_device->device_handle.parts.socket_id;
	p_device->memory_controller_id =
			(NVM_UINT16)p_device->device_handle.parts.memory_controller_id;
	p_device->node_controller_id =
			(NVM_UINT16)p_device->device_handle.parts.node_controller_id;
	p_device->channel_id =
			(NVM_UINT16)p_device->device_handle.parts.mem_channel_id;
	p_device->channel_pos =
			(NVM_UINT16)p_device->device_handle.parts.mem_channel_dimm_num;

	COMMON_LOG_EXIT();
}

int populate_devices_from_topologies(struct device_discovery *p_devices,
		const NVM_UINT8 dev_count,
		struct nvm_topology *p_topologies,
		const NVM_UINT8 topo_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	int copy_count = 0;
	for (int i = 0; i < topo_count; i++)
	{
		if (i >= dev_count)
		{
			COMMON_LOG_ERROR("p_devices buffer is too small for dimm list");
			rc = NVM_ERR_ARRAYTOOSMALL;
			break;
		}

		copy_count++;
		nvm_topology_to_device(&(p_topologies[i]), &(p_devices[i]));
	}

	if (rc == NVM_SUCCESS)
	{
		rc = copy_count;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void calculate_uids_for_populated_devices(struct device_discovery *p_devices,
		const NVM_UINT8 count)
{
	COMMON_LOG_ENTRY();

	for (NVM_UINT8 i = 0; i < count; i++)
	{
		calculate_device_uid(&(p_devices[i]));
	}

	COMMON_LOG_EXIT();
}

void calculate_capabilities_for_populated_devices(struct device_discovery *p_devices,
		const NVM_UINT8 count)
{
	COMMON_LOG_ENTRY();

	for (NVM_UINT8 i = 0; i < count; i++)
	{
		calculate_device_capabilities(&(p_devices[i]));
		map_sku_security_capabilities(p_devices[i].dimm_sku,
				&(p_devices[i].security_capabilities));
	}

	COMMON_LOG_EXIT();
}

int populate_devices(struct device_discovery *p_devices,
		const NVM_UINT8 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	int topo_count = get_topology_count();
	if (topo_count <= 0)
	{
		rc = topo_count;
	}
	else
	{
		struct nvm_topology topologies[topo_count];
		if ((rc = get_topology(topo_count, topologies)) > 0)
		{
			rc = populate_devices_from_topologies(p_devices, count,
					topologies, topo_count);

			int populated_count = (rc == NVM_ERR_ARRAYTOOSMALL) ?
					count : rc;
			if (populated_count > 0)
			{
				add_smbios_properties_to_populated_devices(p_devices, populated_count);

				int fw_rc = add_firmware_properties_to_populated_devices(p_devices,
						populated_count);
				if (fw_rc != NVM_SUCCESS)
				{
					rc = fw_rc;
				}
				calculate_capabilities_for_populated_devices(p_devices,
						populated_count);
				calculate_uids_for_populated_devices(p_devices, populated_count);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve discovery information about each CR device in the system whether it's
 * fully compatible with the current Management library version or not.
 * To allocate the array of device_discovery structures, call nvm_get_device_count
 * before calling this method.
 */
int nvm_get_devices(struct device_discovery *p_devices, const NVM_UINT8 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if (p_devices == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_devices is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	// read from the cache
	else if ((rc = get_nvm_context_devices(p_devices, count)) < 0 &&
			rc != NVM_ERR_ARRAYTOOSMALL)
	{
		memset(p_devices, 0, count * sizeof (struct device_discovery));
		rc = populate_devices(p_devices, count);
		if (rc > 0)
		{
			// Successfully populated devices, now update the context
			set_nvm_context_devices(p_devices, rc);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve discovery information about the device specified
 */
int nvm_get_device_discovery(const NVM_UID device_uid,
		struct device_discovery *p_discovery)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("device_uid cannot be NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_discovery == NULL)
	{
		COMMON_LOG_ERROR("p_discovery cannot be NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		memset(p_discovery, 0, sizeof (struct device_discovery));
		rc = lookup_dev_uid(device_uid, p_discovery);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to get die spare policy
 */
int get_fw_die_spare_policy(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct pt_get_die_spare_policy *payload)
{
	int rc = NVM_SUCCESS;
	struct fw_cmd fw_cmd;
	memset(&fw_cmd, 0, sizeof (fw_cmd));
	fw_cmd.device_handle = dimm_handle.handle;
	fw_cmd.opcode = PT_GET_FEATURES;
	fw_cmd.sub_opcode = SUBOP_POLICY_DIE_SPARING;
	fw_cmd.output_payload_size = sizeof (*payload);
	fw_cmd.output_payload = payload;
	rc = ioctl_passthrough_cmd(&fw_cmd);
	return rc;
}

/*
 * Helper function to set last config status in device_status struct
 */
int fill_device_config_status(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_capabilities *p_capabilities,
		struct device_status *p_status)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// get dimm platform config data to set config status
	p_status->config_status = CONFIG_STATUS_ERR_CORRUPT;
	p_status->is_new = 0;

	if (!p_capabilities->nvm_features.get_device_capacity)
	{
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else
	{
		p_status->config_status = CONFIG_STATUS_NOT_CONFIGURED;
		struct platform_config_data *p_cfg_data = NULL;
		rc = get_dimm_platform_config(device_handle, &p_cfg_data);
		if (rc == NVM_SUCCESS)
		{
			// get current config
			struct current_config_table *p_current_config = cast_current_config(p_cfg_data);
			if (!p_current_config)
			{
				COMMON_LOG_ERROR("Failed to find current config table in platform config data.");
				p_status->config_status = CONFIG_STATUS_UNKNOWN;
			}
			else
			{
				p_status->config_status = get_config_status_from_current_config(p_current_config);
				if (p_status->config_status == CONFIG_STATUS_NOT_CONFIGURED)
				{
					p_status->is_new = 1;
				}
			}
			free(p_cfg_data);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 *  Helper function to set health,last_shutdown_status, last_shutdown_time and
 *  ait_dram_enabled in device_status struct
 */
int fill_device_status_from_smart_health(unsigned int device_handle,
		struct device_status *p_status)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_smart_health dimm_smart;
	rc = fw_get_smart_health(device_handle, &dimm_smart);
	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to retrieve the DIMM smart data with error %d", rc);
	}
	else
	{
			if (dimm_smart.validation_flags.parts.health_status_field)
		{
			p_status->health = smart_health_status_to_device_health(dimm_smart.health_status);
		}
		else
		{
			p_status->health = DEVICE_HEALTH_UNKNOWN;
		}

		if (dimm_smart.validation_flags.parts.sizeof_vendor_data_field)
		{
			p_status->last_shutdown_status = dimm_smart.vendor_data.lss_details;
			memcpy(p_status->last_shutdown_status_extended, &dimm_smart.vendor_data.lss_extended_details, sizeof(dimm_smart.vendor_data.lss_extended_details));
			p_status->last_shutdown_time = dimm_smart.vendor_data.last_shutdown_time;
		}

		if (dimm_smart.validation_flags.parts.ait_dram_status_field)
		{
			p_status->ait_dram_enabled = dimm_smart.ait_dram_status;
		}

		p_status->injected_media_errors = dimm_smart.vendor_data.injected_media_errors;
		p_status->injected_non_media_errors = dimm_smart.vendor_data.injected_non_media_errors;

	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * Helper function to read fw error info for a single type and level
 */
int update_status_fw_error_log_info_by_type_and_level(const NVM_NFIT_DEVICE_HANDLE device_handle,
		unsigned char log_type, unsigned char log_level, struct device_status *p_status)
{
	int rc = NVM_SUCCESS;
	struct pt_payload_fw_log_info_data p_log_info_data;
	memset(&p_log_info_data, 0, sizeof (p_log_info_data));
	KEEP_ERROR(rc, fw_get_fw_error_log_info_data(
		device_handle.handle, log_level, log_type, &p_log_info_data));
	p_status->new_error_count +=
			(p_log_info_data.current_sequence_number - p_log_info_data.oldest_sequence_number);
	if (p_log_info_data.newest_log_entry_timestamp > p_status->newest_error_log_timestamp)
	{
		p_status->newest_error_log_timestamp = p_log_info_data.newest_log_entry_timestamp;
	}
	return rc;
}

/*
 * Helper function to set fw error info in device_status struct
 */
int fill_fw_error_log_status(const NVM_NFIT_DEVICE_HANDLE device_handle,
		struct device_status *p_status)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	p_status->new_error_count = 0;
	p_status->newest_error_log_timestamp = 0;

	KEEP_ERROR(rc, update_status_fw_error_log_info_by_type_and_level(device_handle,
		DEV_FW_ERR_LOG_MEDIA, DEV_FW_ERR_LOG_LOW, p_status));
	KEEP_ERROR(rc, update_status_fw_error_log_info_by_type_and_level(device_handle,
		DEV_FW_ERR_LOG_MEDIA, DEV_FW_ERR_LOG_HIGH, p_status));
	KEEP_ERROR(rc, update_status_fw_error_log_info_by_type_and_level(device_handle,
		DEV_FW_ERR_LOG_THERMAL, DEV_FW_ERR_LOG_LOW, p_status));
	KEEP_ERROR(rc, update_status_fw_error_log_info_by_type_and_level(device_handle,
		DEV_FW_ERR_LOG_THERMAL, DEV_FW_ERR_LOG_HIGH, p_status));

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper to get the device sku violation status
 */
int get_sku_violation_state_for_device(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		NVM_BOOL *p_sku_violation)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;

	rc = nvm_get_device_count();

	if (rc > 0)
	{
		int dev_count = rc;
		struct device_discovery devices[dev_count];
		rc = nvm_get_devices(devices, dev_count);
		if (rc == dev_count)
		{
			rc = NVM_SUCCESS;

			for (int dimmIdx = 0; dimmIdx < dev_count; dimmIdx++)
			{
				if (devices[dimmIdx].device_handle.handle == dimm_handle.handle)
				{
					KEEP_ERROR(rc, device_in_sku_violation(&devices[dimmIdx],
							p_sku_violation));
				}
			}
		}
		else
		{
			COMMON_LOG_ERROR_F(
				"Failed to get the correct number of devices with error %d", rc);
		}
	}
	else
	{
		COMMON_LOG_ERROR_F(
			"Failed to get the correct number of devices with error %d", rc);
	}
	return rc;
}

NVM_BOOL is_fw_version_greater_than_1_3(NVM_NFIT_DEVICE_HANDLE dimm_handle)
{
	COMMON_LOG_ENTRY();

	NVM_BOOL result = 0;
	struct device_discovery discovery;

	if ((lookup_dev_handle(dimm_handle, &discovery) == NVM_SUCCESS) &&
			atof(discovery.fw_api_version) > 1.3)
	{
		result = 1;
	}
	return result;
}

/*
 * Retrieve the status of the device specified
 */
void get_device_status_by_handle(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct device_status *p_status, struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	memset(p_status, 0, sizeof (*p_status));

	// TODO: implement these
	p_status->is_missing = 0;

	// send a pass through command to get the smart data
	p_status->health = DEVICE_HEALTH_UNKNOWN;
	p_status->last_shutdown_status = SHUTDOWN_STATUS_UNKNOWN;

	// get die sparing information
	p_status->die_spares_available = 0;
	struct pt_get_die_spare_policy spare_payload;
	memset(&spare_payload, 0, sizeof (spare_payload));
	if ((rc = get_fw_die_spare_policy(dimm_handle, &spare_payload)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to retrieve the DIMM die spare status with error %d",
				rc);
	}
	else
	{
		// ensures backward compatibility with FIS 1.3
		if (is_fw_version_greater_than_1_3(dimm_handle))
		{
			p_status->die_spares_available = spare_payload.supported;
		}
		else
		{
			// supported rank of 1 means die is still available, so not used.
			p_status->die_spares_available = (spare_payload.supported & 0x01)
													+ ((spare_payload.supported & 0x02) >> 1)
													+ ((spare_payload.supported & 0x04) >> 2)
													+ ((spare_payload.supported & 0x08) >> 3);
		}
	}

	// get ARS and sanitize status
	if ((rc = get_long_status(dimm_handle, &p_status->ars_status, &p_status->sanitize_status))
			!= NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to get the DIMM ARS and sanitize status, rc=%d", rc);
	}

	if ((rc = fill_device_status_from_smart_health(dimm_handle.handle, p_status)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to get the DIMM SMART health status, rc=%d", rc);
	}

	// fill the last config status and is new flag
	if ((rc = fill_device_config_status(dimm_handle, p_capabilities, p_status)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to get the DIMM config status, rc=%d", rc);
	}

	// System-level mixed SKU
	p_status->mixed_sku = p_capabilities->sku_capabilities.mixed_sku;

	if ((rc = get_sku_violation_state_for_device(dimm_handle, &p_status->sku_violation))
			!= NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to get the DIMM SKU violation state, rc=%d", rc);
	}

	struct pt_payload_get_config_data_policy config_data;
	if ((rc = fw_get_config_data_policy(dimm_handle.handle, &config_data)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to retrieve DIMM optional configuration data policy with "
				"error %d", rc);
	}
	else
	{
		p_status->viral_state = config_data.viral_status;
	}

	if ((rc = fill_fw_error_log_status(dimm_handle, p_status)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to retrieve DIMM FW error log status, error %d",
				rc);
	}

	if ((rc = fw_get_bsr(dimm_handle, &(p_status->boot_status))) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to retrieve DIMM BSR with error %d",
				rc);
	}
}

/*
 * Retrieve the status of the device specified
 */
int nvm_get_device_status(const NVM_UID device_uid,
		struct device_status *p_status)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;
	struct nvm_capabilities capabilities;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = nvm_get_nvm_capabilities(&capabilities)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve the system capabilities.");
	}
	else if (!capabilities.nvm_features.get_device_health) // also confirms pass through
	{
		COMMON_LOG_ERROR("Retrieving device status is not supported.");
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else if (p_status == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_status is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		get_device_status_by_handle(discovery.device_handle, p_status, &capabilities);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_details(const NVM_UID device_uid,
		struct device_discovery *p_discovery,
		struct device_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct nvm_details dimm_details;
	if (NVM_SUCCESS
			== (rc = get_dimm_details(p_discovery->device_handle, &dimm_details)))
	{
		p_details->form_factor =
				get_device_form_factor_from_smbios_form_factor(dimm_details.form_factor);
		p_details->data_width = dimm_details.data_width;
		p_details->total_width = dimm_details.total_width;
		p_details->speed = dimm_details.speed;
		s_strcpy((p_details->device_locator), dimm_details.device_locator, NVM_DEVICE_LOCATOR_LEN);
		s_strcpy((p_details->bank_label), dimm_details.bank_label, NVM_BANK_LABEL_LEN);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to get firmware power management policy
 */
int get_fw_power_mgmt_policy(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct pt_payload_power_mgmt_policy *payload)
{
	int rc = NVM_SUCCESS;
	struct fw_cmd fw_cmd;
	memset(&fw_cmd, 0, sizeof (fw_cmd));
	fw_cmd.device_handle = dimm_handle.handle;
	fw_cmd.opcode = PT_GET_FEATURES;
	fw_cmd.sub_opcode = SUBOP_POLICY_POW_MGMT;
	fw_cmd.output_payload_size = sizeof (*payload);
	fw_cmd.output_payload = payload;
	rc = ioctl_passthrough_cmd(&fw_cmd);
	return rc;
}

/*
 * Retrieve #device_settings information about the device specified
 */
int nvm_get_device_settings(const NVM_UID device_uid,
		struct device_settings *p_settings)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(get_device_settings)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving device settings is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_settings == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_settings is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		memset(p_settings, 0, sizeof (*p_settings));
		struct pt_payload_get_config_data_policy config_data;
		rc = fw_get_config_data_policy(
				discovery.device_handle.handle, &config_data);
		if (rc != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Unable to get the optional configuration data policy \
					for handle: [%d]", discovery.device_handle.handle);
		}
		else
		{
			p_settings->first_fast_refresh = config_data.first_fast_refresh;
			p_settings->viral_policy = config_data.viral_policy_enable;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Set one or more configurable properties on the specified device.
 * A given property change may require similar changes to related devices to
 * represent a consistent correct configuration.
 */
int nvm_modify_device_settings(const NVM_UID device_uid,
		const struct device_settings *p_settings)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
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
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_settings == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_settings is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		struct pt_payload_set_config_data_policy config_data;
		memset(&config_data, 0, sizeof (struct pt_payload_set_config_data_policy));
		config_data.first_fast_refresh = p_settings->first_fast_refresh;
		config_data.viral_policy_enable = p_settings->viral_policy;
		rc = fw_set_config_data_policy(discovery.device_handle.handle, &config_data);
		if (rc != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Unable to update the optional configuration data policy \
					for handle: [%d]", discovery.device_handle.handle);
		}

		// updated the dimm, clear the device cache
		if (rc == NVM_SUCCESS)
		{
			invalidate_devices();
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int populate_power_mgmt_policy_details(struct device_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_power_mgmt_policy power_payload;
	memset(&power_payload, 0, sizeof (power_payload));
	if ((rc = get_fw_power_mgmt_policy(p_details->discovery.device_handle,
			&power_payload)) == NVM_SUCCESS)
	{
		p_details->power_management_enabled = power_payload.enabled;
		p_details->power_limit = power_payload.tdp;
		p_details->peak_power_budget = power_payload.peak_power_budget;
		p_details->avg_power_budget
				= power_payload.average_power_budget;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int populate_die_sparing_policy_details(struct device_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_get_die_spare_policy spare_payload;
	memset(&spare_payload, 0, sizeof (spare_payload));
	if ((rc = get_fw_die_spare_policy(p_details->discovery.device_handle,
			&spare_payload)) == NVM_SUCCESS)
	{
		p_details->die_sparing_enabled = spare_payload.enable;
		p_details->die_sparing_level = spare_payload.aggressiveness;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int populate_manageable_device_details(const NVM_UID device_uid,
		struct device_details *p_details, struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	KEEP_ERROR(rc, nvm_get_device_status(device_uid, &(p_details->status)));

	KEEP_ERROR(rc, nvm_get_device_fw_image_info(device_uid, &(p_details->fw_info)));

	KEEP_ERROR(rc, nvm_get_device_performance(device_uid,
			&(p_details->performance)));

	KEEP_ERROR(rc, nvm_get_sensors(device_uid, p_details->sensors,
			NVM_MAX_DEVICE_SENSORS));

	KEEP_ERROR(rc, get_details(device_uid, &p_details->discovery, p_details));

	if (p_capabilities->nvm_features.get_device_capacity)
	{
		KEEP_ERROR(rc, get_dimm_capacities(&p_details->discovery, p_capabilities,
				&p_details->capacities));
	}

	KEEP_ERROR(rc, nvm_get_device_settings(device_uid, &(p_details->settings)));

	KEEP_ERROR(rc, populate_power_mgmt_policy_details(p_details));
	KEEP_ERROR(rc, populate_die_sparing_policy_details(p_details));

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int populate_device_details(const NVM_UID device_uid,
		struct device_details *p_details, struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	memset(p_details, 0, sizeof (*p_details));
	if ((rc = lookup_dev_uid(device_uid, &p_details->discovery)) == NVM_SUCCESS)
	{
		// get details from SMBIOS - for any existing DIMM
		KEEP_ERROR(rc, get_details(device_uid, &p_details->discovery, p_details));

		if (p_details->discovery.manageability == MANAGEMENT_VALIDCONFIG)
		{
			KEEP_ERROR(rc, populate_manageable_device_details(device_uid,
					p_details, p_capabilities));
		}

		// TODO: workaround for Simics - returns as much data as possible to wbem
		// this doesn't seem to hurt the unit tests so skip errors for now
		rc = NVM_SUCCESS;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve detailed information about the device specified
 */
int nvm_get_device_details(const NVM_UID device_uid,
		struct device_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct nvm_capabilities capabilities;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = nvm_get_nvm_capabilities(&capabilities)) == NVM_SUCCESS)
	{
		if (!capabilities.nvm_features.get_devices)
		{
			rc = NVM_ERR_NOTSUPPORTED;
		}
		else if (device_uid == NULL)
		{
			COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
			rc = NVM_ERR_INVALIDPARAMETER;
		}
		else if (p_details == NULL)
		{
			COMMON_LOG_ERROR("Invalid parameter, p_details is NULL");
			rc = NVM_ERR_INVALIDPARAMETER;
		}
		else if (get_nvm_context_device_details(device_uid, p_details) != NVM_SUCCESS)
		{
			rc = populate_device_details(device_uid, p_details, &capabilities);
			if (rc == NVM_SUCCESS)
			{
				set_nvm_context_device_details(device_uid, p_details);
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve a snapshot of the performance metrics for the device specified.
 */
int nvm_get_device_performance(const NVM_UID device_uid,
		struct device_performance *p_performance)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(get_device_performance)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving device performance is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_performance == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_performance is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_uid, &discovery, 1)) == NVM_SUCCESS)
	{
		NVM_UINT32 handle = discovery.device_handle.handle;

		memset(p_performance, 0, sizeof (*p_performance));
		p_performance->time = time(NULL);

		struct pt_payload_memory_info_page1 meminfo_page_1;
		if (NVM_SUCCESS == (rc =
			fw_get_memory_info_page(handle, 1, &meminfo_page_1, sizeof (meminfo_page_1))))
		{
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(meminfo_page_1.total_bytes_read,
				p_performance->bytes_read)
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(meminfo_page_1.total_bytes_written,
				p_performance->bytes_written)
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(meminfo_page_1.total_read_reqs,
				p_performance->host_reads)
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(meminfo_page_1.total_write_reqs,
				p_performance->host_writes)
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(meminfo_page_1.total_block_read_reqs,
				p_performance->block_reads);
			NVM_8_BYTE_ARRAY_TO_64_BIT_VALUE(meminfo_page_1.total_block_write_reqs,
				p_performance->block_writes);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the aggregate capacities across all NVM DIMMs in the system.
 */
int nvm_get_nvm_capacities(struct device_capacities *p_capacities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else
	{
		// retrieve the capabilities struct once
		struct nvm_capabilities capabilities;
		memset(&capabilities, 0, sizeof (capabilities));
		if ((rc = nvm_get_nvm_capabilities(&capabilities)) == NVM_SUCCESS)
		{
			if (!capabilities.nvm_features.get_device_capacity)
			{
				rc = NVM_ERR_NOTSUPPORTED;
			}
			else if (p_capacities == NULL)
			{
				COMMON_LOG_ERROR("Invalid parameter, p_capacities is NULL");
				rc = NVM_ERR_INVALIDPARAMETER;
			}
			else
			{
				// clear the structure
				memset(p_capacities, 0, sizeof (*p_capacities));
				rc = nvm_get_device_count();
				if (rc > 0)
				{
					int dev_count = rc;
					struct device_discovery devices[dev_count];
					rc = nvm_get_devices(devices, dev_count);
					if (rc == dev_count)
					{
						rc = NVM_SUCCESS;
						// iterate through all devices and add up the capacities
						for (int i = 0; i < dev_count; i++)
						{
							if (devices[i].manageability == MANAGEMENT_VALIDCONFIG)
							{
								struct device_capacities dimm_capacities;
								int temp_rc = get_dimm_capacities(&devices[i],
										&capabilities, &dimm_capacities);
								if (temp_rc == NVM_SUCCESS)
								{
									p_capacities->capacity +=
											dimm_capacities.capacity;
									p_capacities->memory_capacity +=
											dimm_capacities.memory_capacity;
									p_capacities->app_direct_capacity +=
											dimm_capacities.app_direct_capacity;
									p_capacities->unconfigured_capacity +=
											dimm_capacities.unconfigured_capacity;
									p_capacities->storage_capacity +=
											dimm_capacities.storage_capacity;
									p_capacities->inaccessible_capacity +=
											dimm_capacities.inaccessible_capacity;
									p_capacities->reserved_capacity +=
											dimm_capacities.reserved_capacity;
								}
								// Keep error and continue looping, error is
								// logged in get_dimm_capacities
								else
								{
									KEEP_ERROR(rc, temp_rc);
								}
							}
						}
					}
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

#if __ADD_MANUFACTURING__
/*
 * Send a firmware command directly to the specified device without
 * checking for valid input.
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_UNKNOWN @n
 * 		#NVM_ERR_BADDEVICE @n
 */
int nvm_send_device_passthrough_cmd(const NVM_UID device_uid,
		struct device_pt_cmd *p_cmd)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(get_device_health)) != NVM_SUCCESS)
	{ // also confirms pass through
		COMMON_LOG_ERROR("Retrieving " NVM_DIMM_NAME " health is not supported.");
	}
	else if (device_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_cmd == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_cmd is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	// get the device_handle
	else if ((rc = exists_and_manageable(device_uid, &discovery, 0)) == NVM_SUCCESS)
	{
		// send the pass through command
		struct fw_cmd fw_cmd;
		fw_cmd.device_handle = discovery.device_handle.handle;
		fw_cmd.opcode = p_cmd->opcode;
		fw_cmd.sub_opcode = p_cmd->sub_opcode;
		fw_cmd.input_payload_size = p_cmd->input_payload_size;
		fw_cmd.input_payload = p_cmd->input_payload;
		fw_cmd.output_payload_size = p_cmd->output_payload_size;
		fw_cmd.output_payload = p_cmd->output_payload;
		fw_cmd.large_input_payload_size = p_cmd->large_input_payload_size;
		fw_cmd.large_input_payload = p_cmd->large_input_payload;
		fw_cmd.large_output_payload_size = p_cmd->large_output_payload_size;
		fw_cmd.large_output_payload = p_cmd->large_output_payload;
		// capture the return code
		p_cmd->result = ioctl_passthrough_cmd(&fw_cmd);
		rc = NVM_SUCCESS;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
#endif
