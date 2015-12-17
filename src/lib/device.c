/*
 * Copyright (c) 2015, Intel Corporation
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
#include <guid/guid.h>
#include <os/os_adapter.h>
#include <string/s_str.h>
#include <string/revision.h>
#include <file_ops/file_ops_adapter.h>

#include "nvm_management.h"
#include <fw_header.h>
#include <persistence/logging.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include "platform_config_data.h"
#include "device_utilities.h"
#include "monitor.h"
#include "config_goal.h"
#include "capabilities.h"
#include "nvm_context.h"
#include "system.h"

const unsigned int NUM_RANKS = 4;

// define prototypes for helper functions
int read_file_bytes(const NVM_PATH path, const NVM_SIZE path_len,
		unsigned char **pp_buf, unsigned int *p_buf_len);

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
	else if ((rc = get_devices_is_supported()) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving devices is not supported.");
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
	else if (!(security_status & SEC_ENABLED))
	{
		lock_state = LOCK_STATE_DISABLED;
	}
	else if (security_status & SEC_COUNT_EXP)
	{
		lock_state = LOCK_STATE_PASSPHRASE_LIMIT;
	}
	else if (security_status & SEC_FROZEN)
	{
		lock_state = LOCK_STATE_FROZEN;
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
	int topo_count;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if ((rc = get_devices_is_supported()) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving devices is not supported.");
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
		topo_count = get_topology_count();
		if (topo_count <= 0)
		{
			rc = topo_count;
		}
		else
		{
			struct nvm_topology dimm_list[topo_count];
			if ((rc = get_topology(topo_count, dimm_list)) > NVM_SUCCESS)
			{
				int copy_count = 0;
				for (int i = 0; i < topo_count; i++)
				{
					if (i > count)
					{
						COMMON_LOG_ERROR("p_devices buffer is too small for dimm list");
						rc = NVM_ERR_ARRAYTOOSMALL;
						break;
					}

					// send a pass through command to get the dimm identify info
					struct pt_payload_identify_dimm id_dimm;
					struct fw_cmd cmd;
					memset(&cmd, 0, sizeof (struct fw_cmd));
					cmd.device_handle = dimm_list[i].device_handle.handle;
					cmd.opcode = PT_IDENTIFY_DIMM;
					cmd.sub_opcode = 0;
					cmd.output_payload_size = sizeof (id_dimm);
					cmd.output_payload = &id_dimm;
					if ((rc = ioctl_passthrough_cmd(&cmd)) != NVM_SUCCESS)
					{
						COMMON_LOG_ERROR_F(
								"Unable to get identify dimm information for handle: [%d]",
								dimm_list[i].device_handle.handle);
						break;
					}
					else
					{
						copy_count++;

						calculate_device_guid(p_devices[i].guid,
								id_dimm.mf, DEV_MFR_LEN,
								(char *)id_dimm.mn, DEV_MODELNUM_LEN,
								id_dimm.sn, DEV_SN_LEN);
						p_devices[i].device_handle = dimm_list[i].device_handle;
						p_devices[i].physical_id = dimm_list[i].id;
						p_devices[i].vendor_id = dimm_list[i].vendor_id;
						p_devices[i].device_id = dimm_list[i].device_id;
						p_devices[i].revision_id = dimm_list[i].revision_id;
						p_devices[i].socket_id =
								p_devices[i].device_handle.parts.socket_id;
						p_devices[i].memory_controller_id =
								p_devices[i].device_handle.parts.memory_controller_id;
						// we only get NVMDIMMS from the driver
						p_devices[i].memory_type = MEMORY_TYPE_NVMDIMM;

						// set manageability of the dimm based on the FW and driver revisions
						NVM_VERSION driver_revision;
						s_strcpy(driver_revision, "0.0.0.0", NVM_VERSION_LEN); // default version
						get_vendor_driver_revision(driver_revision, NVM_VERSION_LEN);
						set_device_manageability(driver_revision,
								&id_dimm, &p_devices[i].manageability);

						map_sku_security_capabilities(id_dimm.dimm_sku,
							&(p_devices[i].security_capabilities));
						convert_sku_to_device_capabilities(id_dimm.dimm_sku,
								&(p_devices[i].device_capabilities));
						p_devices[i].dimm_sku = id_dimm.dimm_sku;

						p_devices[i].interface_format_code = id_dimm.ifc;
						memmove(p_devices[i].manufacturer, id_dimm.mf,
								DEV_MFR_LEN);
						memmove(p_devices[i].serial_number, id_dimm.sn,
								DEV_SN_LEN);
						memmove(p_devices[i].model_number, id_dimm.mn,
								DEV_MODELNUM_LEN);

						// convert fw version to string
						build_revision(p_devices[i].fw_revision, NVM_VERSION_LEN,
							((((id_dimm.fwr[4] >> 4) & 0xF) * 10) + (id_dimm.fwr[4] & 0xF)),
							((((id_dimm.fwr[3] >> 4) & 0xF) * 10) + (id_dimm.fwr[3] & 0xF)),
							((((id_dimm.fwr[2] >> 4) & 0xF) * 10) + (id_dimm.fwr[2] & 0xF)),
							(((id_dimm.fwr[1] >> 4) & 0xF) * 1000) + (id_dimm.fwr[1] & 0xF) * 100 +
							(((id_dimm.fwr[0] >> 4) & 0xF) * 10) + (id_dimm.fwr[0] & 0xF));

						// convert fw api version to string
						build_fw_revision(p_devices[i].fw_api_version, NVM_VERSION_LEN,
								((id_dimm.api_ver >> 4) & 0xF), (id_dimm.api_ver & 0xF));

						p_devices[i].capacity = MULTIPLES_TO_BYTES((NVM_UINT64)id_dimm.rc);

						// only get security state if manageable
						if (p_devices[i].manageability == MANAGEMENT_VALIDCONFIG)
						{
							struct pt_payload_get_security_state security_state;
							if (fw_get_security_state(dimm_list[i].device_handle.handle,
								&security_state) != NVM_SUCCESS)
							{
								COMMON_LOG_ERROR_F(
									"Unable to get security state information for handle: [%d]",
										dimm_list[i].device_handle);
							}
							else // successfully got security state
							{
								p_devices[i].lock_state =
										security_state_to_enum(security_state.security_status);
							}
						}
					}
				}

				if (rc == NVM_SUCCESS)
				{
					rc = copy_count;
				}

				// update the context
				set_nvm_context_devices(p_devices, copy_count);

			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve discovery information about the device specified
 */
int nvm_get_device_discovery(const NVM_GUID device_guid,
		struct device_discovery *p_discovery)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if ((rc = get_devices_is_supported()) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving devices is not supported.");
	}
	else if (device_guid == NULL)
	{
		COMMON_LOG_ERROR("device_guid cannot be NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_discovery == NULL)
	{
		COMMON_LOG_ERROR("p_discovery cannot be NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		rc = lookup_dev_guid(device_guid, p_discovery);
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
				COMMON_LOG_ERROR("Failed to find current config table in platform config data");
				rc = NVM_ERR_BADDEVICECONFIG;
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
 * Retrieve the status of the device specified
 */
int get_device_status_by_handle(NVM_NFIT_DEVICE_HANDLE dimm_handle,
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
	p_status->die_spares_used = 0;
	struct pt_get_die_spare_policy spare_payload;
	memset(&spare_payload, 0, sizeof (spare_payload));
	int temprc = get_fw_die_spare_policy(dimm_handle, &spare_payload);
	if (temprc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to retrieve the DIMM die spare status with error %d",
				temprc);
		KEEP_ERROR(rc, temprc);
	}
	else
	{
		// supported rank of 1 means die is still available, so not used.  Total of 4
		// possible die spares, so start with NUM_RANKS, and subtract each remaining unused to
		// get the number of die spares used.
		p_status->die_spares_used = NUM_RANKS - (spare_payload.supported & 0x01)
						- ((spare_payload.supported & 0x02) >> 1)
						- ((spare_payload.supported & 0x04) >> 2)
						- ((spare_payload.supported & 0x08) >> 3);
	}

	struct pt_payload_smart_health dimm_smart;
	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = dimm_handle.handle;
	cmd.opcode = PT_GET_LOG;
	cmd.sub_opcode = SUBOP_SMART_HEALTH;
	cmd.output_payload_size = sizeof (dimm_smart);
	cmd.output_payload = &dimm_smart;
	temprc = ioctl_passthrough_cmd(&cmd);
	if (temprc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to retrieve the DIMM smart data with error %d", temprc);
		KEEP_ERROR(rc, temprc);
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

		// convert last shutdown status to enumeration value
		if (dimm_smart.validation_flags.parts.sizeof_vendor_data_field)
		{
			p_status->last_shutdown_status = dimm_smart.vendor_data.lss_details;
			p_status->last_shutdown_time = dimm_smart.vendor_data.last_shutdown_time;
		}
	}

	// fill the last config status and is new flag
	KEEP_ERROR(rc, fill_device_config_status(dimm_handle,
			p_capabilities, p_status));

	// determine if the system has mixed dimm SKUs
	p_status->mixed_sku = p_capabilities->sku_capabilities.mixed_sku;

	// determine if the dimm is in violation of it's supported sku
	KEEP_ERROR(rc, device_in_sku_violation(dimm_handle,
			p_capabilities, &p_status->sku_violation));

	return rc;
}

/*
 * Retrieve the status of the device specified
 */
int nvm_get_device_status(const NVM_GUID device_guid,
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
	else if (device_guid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_guid is NULL");
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
	else if ((rc = exists_and_manageable(device_guid, &discovery, 1)) == NVM_SUCCESS)
	{
		rc = get_device_status_by_handle(discovery.device_handle, p_status, &capabilities);

	} // dimm manageable

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to populate current values for the sensors using the smart log
 */
int get_details(const NVM_GUID device_guid,
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
		s_strcpy((p_details->part_number), dimm_details.part_number, NVM_PART_NUM_LEN);
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
int nvm_get_device_settings(const NVM_GUID device_guid,
		struct device_settings *p_settings)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(get_device_settings)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving device settings is not supported.");
	}
	else if (device_guid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_guid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_settings == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_settings is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_guid, &discovery, 1)) == NVM_SUCCESS)
	{
		memset(p_settings, 0, sizeof (*p_settings));
		struct pt_payload_config_data_policy config_data;
		struct fw_cmd cmd;
		memset(&cmd, 0, sizeof (cmd));
		cmd.device_handle = discovery.device_handle.handle;
		cmd.opcode = PT_GET_FEATURES;
		cmd.sub_opcode = SUBOP_OPT_CONFIG_DATA_POLICY;
		cmd.output_payload_size = sizeof (config_data);
		cmd.output_payload = &config_data;
		rc = ioctl_passthrough_cmd(&cmd);
		if (rc != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Unable to get the optional configuration data policy \
					for handle: [%d]", discovery.device_handle.handle);
		}
		else
		{
			p_settings->first_fast_refresh = config_data.first_fast_refresh;
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
int nvm_modify_device_settings(const NVM_GUID device_guid,
		const struct device_settings *p_settings)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_settings)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying device settings is not supported.");
	}
	else if (device_guid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_guid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_settings == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_settings is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_guid, &discovery, 1)) == NVM_SUCCESS)
	{
		struct pt_payload_config_data_policy config_data;
		struct fw_cmd cmd;
		memset(&cmd, 0, sizeof (cmd));
		cmd.device_handle = discovery.device_handle.handle;
		cmd.opcode = PT_SET_FEATURES;
		cmd.sub_opcode = SUBOP_OPT_CONFIG_DATA_POLICY;
		cmd.input_payload_size = sizeof (config_data);
		cmd.input_payload = &config_data;
		config_data.first_fast_refresh = p_settings->first_fast_refresh;
		rc = ioctl_passthrough_cmd(&cmd);
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

/*
 * Retrieve detailed information about the device specified
 */
int nvm_get_device_details(const NVM_GUID device_guid,
		struct device_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		struct nvm_capabilities capabilities;
		rc = nvm_get_nvm_capabilities(&capabilities);
		if (rc == NVM_SUCCESS)
		{
			if (!capabilities.nvm_features.get_devices)
			{
				rc = NVM_ERR_NOTSUPPORTED;
			}
			else if (device_guid == NULL)
			{
				COMMON_LOG_ERROR("Invalid parameter, device_guid is NULL");
				rc = NVM_ERR_INVALIDPARAMETER;
			}
			else if (p_details == NULL)
			{
				COMMON_LOG_ERROR("Invalid parameter, p_details is NULL");
				rc = NVM_ERR_INVALIDPARAMETER;
			}
			else if (get_nvm_context_device_details(device_guid, p_details) != NVM_SUCCESS)
			{
				memset(p_details, 0, sizeof (*p_details));
				if ((rc = exists_and_manageable(device_guid,
						&p_details->discovery, 1)) == NVM_SUCCESS)
				{
					// get status
					int temprc = nvm_get_device_status(device_guid, &(p_details->status));
					KEEP_ERROR(rc, temprc);

					// get performance
					temprc = nvm_get_device_performance(device_guid, &(p_details->performance));
					KEEP_ERROR(rc, temprc);

					// get sensors
					temprc = nvm_get_sensors(device_guid,
							p_details->sensors, NVM_MAX_DEVICE_SENSORS);
					KEEP_ERROR(rc, temprc);

					// get details
					temprc = get_details(device_guid, &p_details->discovery, p_details);
					KEEP_ERROR(rc, temprc);

					// get capacities
					if (!capabilities.nvm_features.get_device_capacity)
					{
						KEEP_ERROR(rc, NVM_ERR_NOTSUPPORTED);
					}
					else
					{
						temprc = get_dimm_capacities(p_details->discovery.device_handle,
								&capabilities,
								&p_details->capacities);
						KEEP_ERROR(rc, temprc);
					}

					struct pt_payload_power_mgmt_policy power_payload;
					memset(&power_payload, 0, sizeof (power_payload));
					if (NVM_SUCCESS == (temprc = get_fw_power_mgmt_policy(
							p_details->discovery.device_handle, &power_payload)))
					{
						p_details->power_management_enabled = power_payload.enabled;
						p_details->power_limit = power_payload.tdp;
						p_details->peak_power_budget = power_payload.peak_power_budget;
						p_details->avg_power_budget
								= power_payload.average_power_budget;
					}
					KEEP_ERROR(rc, temprc);

					struct pt_get_die_spare_policy spare_payload;
					memset(&spare_payload, 0, sizeof (spare_payload));
					if (NVM_SUCCESS == (temprc = get_fw_die_spare_policy(
							p_details->discovery.device_handle, &spare_payload)))
					{
						p_details->die_sparing_enabled = spare_payload.enable;
						p_details->die_sparing_level = spare_payload.aggressiveness;
					}
					KEEP_ERROR(rc, temprc);

					// get device_settings
					temprc = nvm_get_device_settings(device_guid, &(p_details->settings));
					KEEP_ERROR(rc, temprc);

					// TODO: workaround for Simics - returns as much data as possible to wbem
					// this doesn't seem to hurt the unit tests so skip errors for now
					rc = NVM_SUCCESS;
					set_nvm_context_device_details(device_guid, p_details);
				}
				else
				{
					KEEP_ERROR(rc, NVM_ERR_NOTSUPPORTED);
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve a snapshot of the performance metrics for the device specified.
 */
int nvm_get_device_performance(const NVM_GUID device_guid,
		struct device_performance *p_performance)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(get_device_performance)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Retrieving device performance is not supported.");
	}
	else if (device_guid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_guid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_performance == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_performance is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = exists_and_manageable(device_guid, &discovery, 1)) == NVM_SUCCESS)
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
 * Push a new FW image to the device specified.
 */
int nvm_update_device_fw(const NVM_GUID device_guid, const NVM_PATH path,
		const NVM_SIZE path_len, const NVM_BOOL activate, const NVM_BOOL force)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if ((rc = IS_NVM_FEATURE_SUPPORTED(modify_device_settings)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Modifying device settings is not supported.");
	}
	else if (device_guid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_guid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path == NULL)
	{
		COMMON_LOG_ERROR("File path is NULL");
		rc = NVM_ERR_BADFILE;
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
	else if ((rc = exists_and_manageable(device_guid, &discovery, 1)) == NVM_SUCCESS)
	{
		NVM_VERSION fw_version;
		if ((rc = nvm_examine_device_fw(device_guid, path, path_len, fw_version, NVM_VERSION_LEN))
				== NVM_SUCCESS ||
				(rc == NVM_ERR_REQUIRESFORCE && force == 1))
		{
			// copy the file to the input buffer
			unsigned int fw_size = 0;
			unsigned char *p_fw = NULL;
			if ((rc = read_file_bytes(path, path_len, &p_fw, &fw_size)) == NVM_SUCCESS)
			{
#if __LARGE_PAYLOAD__
				// send a pass through command to update the fw
				struct fw_cmd cmd;
				memset(&cmd, 0, sizeof (cmd));
				cmd.device_handle = discovery.device_handle.handle;
				cmd.opcode = PT_UPDATE_FW;
				cmd.sub_opcode = SUBOP_UPDATE_FW;
				cmd.large_input_payload_size = fw_size;
				cmd.large_input_payload = p_fw;
				rc = ioctl_passthrough_cmd(&cmd);
#else
				struct fw_cmd cmd;
				memset(&cmd, 0, sizeof (cmd));
				cmd.device_handle = discovery.device_handle.handle;
				cmd.opcode = PT_UPDATE_FW;
				cmd.sub_opcode = SUBOP_UPDATE_FW;

				struct pt_update_fw_small_payload input_payload;
				memset(&input_payload, 0, sizeof (input_payload));
				input_payload.payload_selector = TRANSFER_VIA_SMALL_PAYLOAD;
				NVM_UINT16 packet_number = 0;
				unsigned int offset = 0;
				printf("Transferring the firmware to DIMM %u...\n",
						discovery.device_handle.handle);
				float num_packets = fw_size / TRANSFER_SIZE;
				int old_percent_complete = 0; // print status
				while (offset < fw_size)
				{
					// status
					int size = TRANSFER_SIZE;
					if (offset == 0)
					{
						input_payload.transfer_header =
							TRANSFER_HEADER(TRANSFER_TYPE_INITIATE, packet_number);
					}
					else if ((offset + TRANSFER_SIZE) < fw_size)
					{
						input_payload.transfer_header =
							TRANSFER_HEADER(TRANSFER_TYPE_CONTINUE, packet_number);
					}
					else if ((offset + TRANSFER_SIZE) == fw_size)
					{
						input_payload.transfer_header =
							TRANSFER_HEADER(TRANSFER_TYPE_END, packet_number);
					}
					else if ((offset + TRANSFER_SIZE) > fw_size)
					{
						size = fw_size - offset;
						input_payload.transfer_header =
							TRANSFER_HEADER(TRANSFER_TYPE_END, packet_number);
					}
					memmove(input_payload.data, (void *)p_fw + offset, size);
					cmd.input_payload = &input_payload;
					cmd.input_payload_size = sizeof (input_payload);
					rc = ioctl_passthrough_cmd(&cmd);
					if (rc != NVM_SUCCESS)
					{
						COMMON_LOG_ERROR_F("Failed to transfer FW packet_number %u",
								packet_number);
						break;
					}
					offset += size;
					packet_number++;

					// update status
					int percent_complete = (int)(((float)packet_number / num_packets) * 100);
					if (percent_complete > old_percent_complete)
					{
						printf(".");
						old_percent_complete = percent_complete;
					}
				}
				printf("\n");

#endif
				if (rc == NVM_SUCCESS)
				{
					// Log an event indicating we successfully updated
					NVM_EVENT_ARG guid_arg;
					guid_to_event_arg(device_guid, guid_arg);
					NVM_EVENT_ARG version_arg;
					s_strcpy(version_arg, fw_version, NVM_EVENT_ARG_LEN);
					log_mgmt_event(EVENT_SEVERITY_INFO,
							EVENT_CODE_MGMT_FIRMWARE_UPDATE,
							device_guid,
							0, // no action required
							guid_arg, version_arg, NULL);

					if (activate == 1)
					{
						// activate the firmware
						struct fw_cmd activate_cmd;
						memset(&activate_cmd, 0, sizeof (activate_cmd));
						activate_cmd.device_handle = discovery.device_handle.handle;
						activate_cmd.opcode = PT_UPDATE_FW;
						activate_cmd.sub_opcode = SUBOP_EXECUTE_FW;
						rc = ioctl_passthrough_cmd(&activate_cmd);
					}
				}
			}

			if (p_fw != NULL)
			{
				free(p_fw);
			}

			// successfully updated the FW, clear the device cache
			if (rc == NVM_SUCCESS)
			{
				invalidate_devices();
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Determine if the FW image is valid to load onto a device
 */
int nvm_examine_device_fw(const NVM_GUID device_guid, const NVM_PATH path, const NVM_SIZE path_len,
		NVM_VERSION image_version, const NVM_SIZE image_version_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct device_discovery discovery;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (device_guid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_guid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path == NULL)
	{
		COMMON_LOG_ERROR("File path is NULL");
		rc = NVM_ERR_BADFILE;
	}
	else if (path_len >= NVM_PATH_LEN)
	{
		COMMON_LOG_ERROR_F(
				"Invalid parameter, path length is too big: Path length (%d) must be less than %d",
				path_len, NVM_PATH_LEN);
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (path_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, path length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (image_version == NULL)
	{
		COMMON_LOG_ERROR("Image version is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (image_version_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, image_version_len length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (!file_exists(path, path_len))
	{
		COMMON_LOG_ERROR_F("File %s does not exist", path);
		rc = NVM_ERR_BADFILE;
	}
	else if ((rc = exists_and_manageable(device_guid, &discovery, 1)) == NVM_SUCCESS)
	{
		unsigned int buf_len = 0;
		unsigned char *p_buf = NULL;

		memset(image_version, 0, image_version_len);
		if ((rc = read_file_bytes(path, path_len, &p_buf, &buf_len)) == NVM_SUCCESS)
		{
			if (buf_len < sizeof (fwImageHeader))
			{
				COMMON_LOG_ERROR("The FW image file is not valid. Image is too small.");
				rc = NVM_ERR_BADFIRMWARE;
			}
			else
			{
				int lt_moduletype_css;
				get_config_value_int(SQL_KEY_LT_MODULETYPE_CSS, &lt_moduletype_css);
				fwImageHeader *p_header = (fwImageHeader *)p_buf;
				// check some of the header values
				if (p_header->moduleType != lt_moduletype_css ||
						p_header->moduleVendor != FW_HEADER_MODULEVENDOR)
				{
					COMMON_LOG_ERROR("The FW image file is not valid. ");
					rc = NVM_ERR_BADFIRMWARE;
					// no need to continue checking already know the FW is bad
				}
				else
				{
					unsigned short int current_major;
					unsigned short int current_minor;
					unsigned short int current_hotfix;
					unsigned short int current_build;
					int image_major = p_header->imageVersion.majorVer.version;
					int image_minor = p_header->imageVersion.minorVer.version;
					int image_hotfix = p_header->imageVersion.hotfixVer.version;
					int image_build = p_header->imageVersion.buildVer.build;
					build_revision(image_version, image_version_len, image_major,
							image_minor, image_hotfix, image_build);

					parse_main_revision(&current_major, &current_minor, &current_hotfix,
							&current_build, discovery.fw_revision, NVM_VERSION_LEN);

					if (image_major < current_major)
					{
						COMMON_LOG_ERROR("The FW image file is not valid. "
								"Cannot downgrade major versions.");
						rc = NVM_ERR_BADFIRMWARE;
					}
					else if (image_major == current_major && image_minor < current_minor)
					{
						COMMON_LOG_ERROR("The FW image file is not valid. "
								"Cannot downgrade minor versions.");
						rc = NVM_ERR_REQUIRESFORCE;
					}
				}
			}
		}
		if (p_buf != NULL)
		{
			free(p_buf);
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
								int temp_rc = get_dimm_capacities(devices[i].device_handle,
										&capabilities, &dimm_capacities);
								if (temp_rc == NVM_SUCCESS)
								{
									p_capacities->capacity +=
											dimm_capacities.capacity;
									p_capacities->volatile_capacity +=
											dimm_capacities.volatile_capacity;
									p_capacities->persistent_capacity +=
											dimm_capacities.persistent_capacity;
									p_capacities->unconfigured_capacity +=
											dimm_capacities.unconfigured_capacity;
									p_capacities->block_capacity +=
											dimm_capacities.block_capacity;
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
int nvm_send_device_passthrough_cmd(const NVM_GUID device_guid,
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
		COMMON_LOG_ERROR("Retrieving "NVM_DIMM_NAME" health is not supported.");
	}
	else if (device_guid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device_guid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_cmd == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_cmd is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	// get the device_handle
	else if ((rc = exists_and_manageable(device_guid, &discovery, 0)) == NVM_SUCCESS)
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

/*
 * **************************************************************************
 * Helper Functions
 * **************************************************************************
 */

/*
 * Helper function to read all bytes from a file and return an NVM error code
 */
int read_file_bytes(const NVM_PATH path, const NVM_SIZE path_len,
		unsigned char **pp_buf, unsigned int *p_buf_len)
{

	int rc;

	switch (copy_file_to_buffer(path, path_len, (void **)pp_buf, p_buf_len))
	{
		case COMMON_SUCCESS:
			rc = NVM_SUCCESS;
			break;
		case COMMON_ERR_NOMEMORY:
			COMMON_LOG_ERROR("Not enough memory to copy the FW file to a buffer");
			rc = NVM_ERR_NOMEMORY;
			break;
		case COMMON_ERR_BADFILE:
			COMMON_LOG_ERROR("The FW image file is not valid.");
			rc = NVM_ERR_BADFILE;
			break;
		default:
			COMMON_LOG_ERROR("Failed to copy the FW image file to a buffer.");
			rc = NVM_ERR_BADFILE;
			break;
	}
	return rc;
}
