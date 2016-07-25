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
 * This file contains the implementation of support/helper functions for device
 * management in the native API.
 */

#include "device_utilities.h"
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include <os/os_adapter.h>
#include <uid/uid.h>
#include <platform_config_data.h>
#include <string/s_str.h>
#include <string/revision.h>
#include "pool_utilities.h"
#include <utility.h>

/*
 * Check if a device exists and is manageable
 *
 * returns NVM_SUCCESS if it exists and is manageable,
 *         NVM_BADDEVICE if device is not in the lookup table
 *         NVM_ERR_NOTMANAGEABLE if device is not manageable
 */
int exists_and_manageable(const NVM_UID device_uid, struct device_discovery *p_dev,
		NVM_BOOL check_manageability)
{
	int rc = lookup_dev_uid(device_uid, p_dev);
	if (rc == NVM_SUCCESS)
	{
		if (check_manageability &&
			p_dev->manageability != MANAGEMENT_VALIDCONFIG)
		{
			COMMON_LOG_DEBUG("Device is not manageable");
			rc = NVM_ERR_NOTMANAGEABLE;
		}
	}
	return rc;
}

void calculate_uid_without_manufacturing_info(NVM_UID uid,
		NVM_UINT16 vendor_id, NVM_SERIAL_NUMBER serial_number)
{
	s_snprintf(uid, NVM_MAX_UID_LEN, "%04x-%02x%02x%02x%02x", vendor_id,
		serial_number[0],
		serial_number[1],
		serial_number[2],
		serial_number[3]);
}

void calculate_uid_with_valid_manufacturing_info(NVM_UID uid,
		NVM_UINT16 vendor_id, NVM_SERIAL_NUMBER serial_number,
		NVM_UINT8 manufacturing_loc, NVM_UINT16 manufacturing_date)
{
	s_snprintf(uid, NVM_MAX_UID_LEN, "%04x-%02x-%04x-%02x%02x%02x%02x",
		vendor_id,
		manufacturing_loc,
		manufacturing_date,
		serial_number[0],
		serial_number[1],
		serial_number[2],
		serial_number[3]);
}

/*
 * Used to update the uid of a device_discovery struct.
 *
 * Valid device UID comes from ACPI 6.1 spec. Valid UIDs are one of:
 * "XXXX-XXXXXXXX"
 * <vendor id>-<serial number>
 * or
 * "XXXX-XX-XXXX-XXXXXXXX"
 * <vendor id>-<manufacturer location>-<manufacturer date><serial number>
 *
 */
int calculate_device_uid(struct device_discovery *p_device)
{
	int rc;

	if (p_device == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_device is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		rc = NVM_SUCCESS;
		if (p_device->manufacturing_info_valid)
		{
			calculate_uid_with_valid_manufacturing_info(
					p_device->uid,
					p_device->vendor_id,
					p_device->serial_number,
					p_device->manufacturing_location,
					p_device->manufacturing_date);
		}
		else
		{
			calculate_uid_without_manufacturing_info(
					p_device->uid,
					p_device->vendor_id,
					p_device->serial_number);
		}
	}

	return rc;
}

/*
 * Helper to retrieve the device list
 * NOTE: caller must free returned array
 */
int get_devices(struct device_discovery **pp_devices)
{
	int rc = nvm_get_device_count();
	if (rc > 0)
	{
		*pp_devices = calloc(rc, sizeof (struct device_discovery));
		if (*pp_devices)
		{
			rc = nvm_get_devices(*pp_devices, rc);
		}
		else
		{
			COMMON_LOG_ERROR("Failed to allocate memory for device list");
			rc = NVM_ERR_NOMEMORY;
		}
	}
	return rc;
}

/*
 * Lookup a device from the uid
 */

int lookup_dev_uid(const NVM_UID dev_uid, struct device_discovery *p_dev)
{
	int rc = NVM_ERR_BADDEVICE;

	if (dev_uid == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, device uid is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		struct device_discovery *p_devices = NULL;
		int dev_count = get_devices(&p_devices);
		if (dev_count < 0)
		{
			rc = dev_count;
		}
		else if (dev_count > 0)
		{
			for (int i = 0; i < dev_count; i++)
			{
				if (uid_cmp(dev_uid, p_devices[i].uid))
				{
					rc = NVM_SUCCESS;
					if (p_dev)
					{
						memmove(p_dev, &p_devices[i], sizeof (struct device_discovery));
					}
					break;
				}
			}
		}
		free(p_devices);
	}
	return rc;
}

/*
 * Look up a device from the handle
 */

int lookup_dev_handle(const NVM_NFIT_DEVICE_HANDLE device_handle, struct device_discovery *p_dev)
{
	int rc = NVM_ERR_BADDEVICE;
	struct device_discovery *p_devices = NULL;
	int dev_count = get_devices(&p_devices);
	if (dev_count < 0)
	{
		rc = dev_count;
	}
	else if (dev_count > 0)
	{
		for (int i = 0; i < dev_count; i++)
		{
			if (device_handle.handle == p_devices[i].device_handle.handle)
			{
				rc = NVM_SUCCESS;
				if (p_dev)
				{
					memmove(p_dev, &p_devices[i], sizeof (struct device_discovery));
				}
				break;
			}
		}
	}
	free(p_devices);
	return rc;
}

/*
 * Look up a device from the manufacturer and serial number
 */

int lookup_dev_manufacturer_serial_model(const unsigned char *manufacturer,
		const unsigned char *serial_number, const char *model_number,
		struct device_discovery *p_dev)
{
	int rc = NVM_ERR_BADDEVICE;
	struct device_discovery *p_devices;
	int dev_count = get_devices(&p_devices);
	if (dev_count < 0)
	{
		rc = dev_count;
	}
	else if (dev_count > 0)
	{
		for (int i = 0; i < dev_count; i++)
		{
			// do the manufacturer and serial and model numbers match?
			if ((cmp_bytes(p_devices[i].manufacturer,
					manufacturer, NVM_MANUFACTURER_LEN) == 1) &&
				(cmp_bytes(p_devices[i].serial_number,
					serial_number, NVM_SERIAL_LEN) == 1) &&
				(cmp_bytes((unsigned char *)p_devices[i].model_number,
					(unsigned char *)model_number, NVM_MODEL_LEN-1) == 1))
			{
				rc = NVM_SUCCESS;
				if (p_dev)
				{
					memmove(p_dev, &p_devices[i], sizeof (struct device_discovery));
				}
				break;
			}
		}
	}
	free(p_devices);
	return rc;
}

/*
 * Helper function to set device manageability in the device discovery struct
 * based on the FW version
 */
void set_device_manageability_from_firmware(
	struct pt_payload_identify_dimm *p_id_dimm,
	enum manageability_state *p_manageability)
{
	// limit tracing in this path to reduce overall log counts
	*p_manageability = MANAGEMENT_VALIDCONFIG;

	// check FW API version
	if (!check_firmware_revision(p_id_dimm->api_ver))
	{
		*p_manageability = MANAGEMENT_INVALIDCONFIG;
	}
}

/*
 * Helper function to translate ars status
 */
enum device_ars_status translate_to_ars_status(struct pt_payload_long_op_stat *long_op_payload)
{
	COMMON_LOG_ENTRY();
	enum device_ars_status status = 0;

	struct pt_return_address_range_scrub *ars_command_return_data =
		(struct pt_return_address_range_scrub *)(long_op_payload->command_specific_data);

	if (((long_op_payload->command & 0x00FF) == PT_GET_LOG) &&
			(((long_op_payload->command & 0xFF00) >> 8) == SUBOP_POLICY_ADDRESS_RANGE_SCRUB))
	{
		if ((ars_command_return_data->ars_state & (ARS_STATUS_ENDED_EARLY<<0)) == 0)
		{
			if (long_op_payload->percent_complete == 100)
			{
				status = DEVICE_ARS_STATUS_COMPLETE;
			}
			else
			{
				status = DEVICE_ARS_STATUS_INPROGRESS;
			}
		}
		else
		{
			status = DEVICE_ARS_STATUS_INPROGRESS;
		}
	}
	else
	{
		status = DEVICE_ARS_STATUS_NOTSTARTED;
	}

	COMMON_LOG_EXIT_RETURN_I(status);
	return status;
}

int get_ars_status(const NVM_NFIT_DEVICE_HANDLE dimm_handle, enum device_ars_status *p_ars_status)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_long_op_stat long_op_payload;
	memset(&long_op_payload, 0, sizeof (long_op_payload));
	rc = fw_get_status_for_long_op(dimm_handle, &long_op_payload);
	if (rc != NVM_SUCCESS)
	{
		*p_ars_status = DEVICE_ARS_STATUS_UNKNOWN;
		COMMON_LOG_ERROR_F("Failed to retrieve the ARS long operation status with error %d",
			rc);
	}
	else
	{
		*p_ars_status = translate_to_ars_status(&long_op_payload);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL is_device_interface_format_supported(struct device_discovery *p_device)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL supported = 0;

	for (int i = 0; i < NVM_MAX_IFCS_PER_DIMM; i++)
	{
		if (p_device->interface_format_codes[i] == FORMAT_BYTE_STANDARD ||
				p_device->interface_format_codes[i] == FORMAT_BLOCK_STANDARD)
		{
			supported = 1;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(supported);
	return supported;
}

NVM_BOOL is_device_subsystem_controller_supported(struct device_discovery *p_device)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL supported = 0;

	// Only supporting Intel DIMMs with AEP controllers
	if (p_device->subsystem_vendor_id == NVM_INTEL_VENDOR_ID)
	{
		for (int i = 0; i < NUM_SUPPORTED_DEVICE_IDS; i++)
		{
			if (p_device->subsystem_device_id == SUPPORTED_DEVICE_IDS[i])
			{
				supported = 1;
				break;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(supported);
	return supported;
}

NVM_BOOL can_communicate_with_device_firmware(struct device_discovery *p_device)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL supported = 0;

	if (is_device_interface_format_supported(p_device) &&
			is_device_subsystem_controller_supported(p_device))
	{
		supported = 1;
	}

	COMMON_LOG_EXIT_RETURN_I(supported);
	return supported;
}

/*
 * Helper function to compare the firmware API version to the supported version
 * to determine if the device is manageable. Because the firmware is backwards
 * compatible need to make sure API version is >= supported API.
 */
int check_firmware_revision(unsigned char fw_api_version)
{
	int rc = 0;

	// parse the firmware API revision into major and minor
	unsigned int major = (fw_api_version > 4) & 0xF;
	unsigned int minor = fw_api_version & 0xF;

	rc = is_fw_api_version_supported(major, minor);
	if (rc == 0)
	{
		COMMON_LOG_ERROR_F(
			"FW API revision %u.%u is not supported by the host software",
			major, minor);
	}

	return rc;
}

/*
 * Helper function to get the health of a dimm. Used in get pools to roll up health info
 * to a pool
 */
int get_dimm_health(NVM_NFIT_DEVICE_HANDLE device_handle, enum device_health *p_health)
{
	COMMON_LOG_ENTRY();

	// send a pass through command to get the smart data
	struct pt_payload_smart_health dimm_smart;
	int rc = fw_get_smart_health(device_handle.handle, &dimm_smart);
	if ((NVM_SUCCESS == rc) &&
			dimm_smart.validation_flags.parts.health_status_field)
	{
		*p_health = smart_health_status_to_device_health(dimm_smart.health_status);
	}
	else
	{
		*p_health = DEVICE_HEALTH_UNKNOWN;
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

enum device_health smart_health_status_to_device_health(enum smart_health_status smart_health)
{
	enum device_health dev_health;

	switch (smart_health)
	{
		case SMART_NORMAL:
			dev_health = DEVICE_HEALTH_NORMAL;
			break;
		case SMART_NON_CRITICAL:
			dev_health = DEVICE_HEALTH_NONCRITICAL;
			break;
		case SMART_CRITICAL:
			dev_health = DEVICE_HEALTH_CRITICAL;
			break;
		case SMART_FATAL:
			dev_health = DEVICE_HEALTH_FATAL;
			break;
		default:
			dev_health = DEVICE_HEALTH_UNKNOWN;
			break;
	}

	return dev_health;
}

/*
 * Convert firmware type into firmware type enumeration
 */
enum device_fw_type firmware_type_to_enum(unsigned char fw_type)
{
	COMMON_LOG_ENTRY();

	enum device_fw_type fw_type_enum;
	if (fw_type == FW_TYPE_PRODUCTION)
	{
		fw_type_enum = DEVICE_FW_TYPE_PRODUCTION;
	}
	else if (fw_type == FW_TYPE_DFX)
	{
		fw_type_enum = DEVICE_FW_TYPE_DFX;
	}
	else if (fw_type == FW_TYPE_DEBUG)
	{
		fw_type_enum = DEVICE_FW_TYPE_DEBUG;
	}
	else
	{
		fw_type_enum = DEVICE_FW_TYPE_UNKNOWN;
	}

	COMMON_LOG_EXIT();
	return fw_type_enum;
}

/*
 * Helper function to fetch partition info from the DIMM
 */
int get_partition_info(const NVM_NFIT_DEVICE_HANDLE device_handle,
	struct pt_payload_get_dimm_partition_info *p_pi)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	memset(p_pi, 0, sizeof (struct pt_payload_get_dimm_partition_info));
	// send get admin features dimm partition info pt command
	struct fw_cmd partition_cmd;
	memset(&partition_cmd, 0, sizeof (partition_cmd));
	partition_cmd.device_handle = device_handle.handle;
	partition_cmd.opcode = PT_GET_ADMIN_FEATURES;
	partition_cmd.sub_opcode = SUBOP_DIMM_PARTITION_INFO;
	partition_cmd.output_payload_size = sizeof (struct pt_payload_get_dimm_partition_info);
	partition_cmd.output_payload = p_pi;
	rc = ioctl_passthrough_cmd(&partition_cmd);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_dimm_manageability(const NVM_NFIT_DEVICE_HANDLE device_handle,
		enum manageability_state *p_manageability)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct pt_payload_identify_dimm id_dimm;
	if ((rc = fw_get_identify_dimm(device_handle.handle, &id_dimm)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Unable to get identify dimm information for handle: [%d]",
				device_handle.handle);
	}
	else
	{
		set_device_manageability_from_firmware(&id_dimm, p_manageability);
	}
	s_memset(&id_dimm, sizeof (id_dimm));

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Quick comparison to determine if a device is manageable
 */
int is_device_manageable(const NVM_NFIT_DEVICE_HANDLE handle)
{
	COMMON_LOG_ENTRY();
	enum manageability_state manageability = MANAGEMENT_UNKNOWN;
	int rc = get_dimm_manageability(handle, &manageability);
	if (rc == NVM_SUCCESS)
	{
		if (manageability == MANAGEMENT_VALIDCONFIG)
		{
			rc = 1;
		}
		else
		{
			rc = 0;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

NVM_BOOL device_is_encryption_enabled(enum lock_state lock_state)
{
	NVM_BOOL is_encryption_enabled = 0;
	if ((lock_state != LOCK_STATE_DISABLED) &&
		(lock_state != LOCK_STATE_UNKNOWN))
	{
		is_encryption_enabled = 1;
	}

	return is_encryption_enabled;
}

NVM_BOOL device_is_erase_capable(struct device_security_capabilities security_capabilities)
{
	NVM_BOOL is_erase_capable = 0;
	if (security_capabilities.erase_crypto_capable)
	{
		is_erase_capable = 1;
	}

	return is_erase_capable;
}

NVM_BOOL device_has_interface_format_code(struct device_discovery *p_dev, enum nvm_format ifc)
{
	COMMON_LOG_ENTRY();

	NVM_BOOL found = 0;
	for (int i = 0; i < NVM_MAX_IFCS_PER_DIMM; i++)
	{
		if (p_dev->interface_format_codes[i] == ifc)
		{
			found = 1;
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(found);
	return found;
}

void update_device_mode_capabilities_from_interface_format_codes(struct device_discovery *p_dev)
{
	COMMON_LOG_ENTRY();

	if (p_dev->device_capabilities.app_direct_mode_capable &&
			!device_has_interface_format_code(p_dev, FORMAT_BYTE_STANDARD))
	{
		p_dev->device_capabilities.app_direct_mode_capable = 0;
	}

	if (p_dev->device_capabilities.storage_mode_capable &&
			!device_has_interface_format_code(p_dev, FORMAT_BLOCK_STANDARD))
	{
		p_dev->device_capabilities.storage_mode_capable = 0;
	}

	COMMON_LOG_EXIT();
}

void calculate_device_capabilities(struct device_discovery *p_dev)
{
	COMMON_LOG_ENTRY();

	convert_sku_to_device_capabilities(p_dev->dimm_sku, &(p_dev->device_capabilities));
	update_device_mode_capabilities_from_interface_format_codes(p_dev);

	COMMON_LOG_EXIT();
}

void convert_sku_to_device_capabilities(
		const int sku_bits, struct device_capabilities *p_capabilities)
{
	memset(p_capabilities, 0, sizeof (struct device_capabilities));

	if (sku_bits & SKU_MEMORY_MODE_ENABLED)
	{
		p_capabilities->memory_mode_capable = 1;
	}

	if (sku_bits & SKU_APP_DIRECT_MODE_ENABLED)
	{
		p_capabilities->app_direct_mode_capable = 1;
	}

	if (sku_bits & SKU_STORAGE_MODE_ENABLED)
	{
		p_capabilities->storage_mode_capable = 1;
	}

	if (sku_bits & SKU_DIE_SPARING_ENABLED)
	{
		p_capabilities->die_sparing_capable = 1;
	}
}

/*
 * Determine the supported security capabilities based on the DIMM SKU
 * info from Identify DIMM Return Data (Table 14) of FIS 0.83
 */
void map_sku_security_capabilities(unsigned int dimm_sku,
	struct device_security_capabilities *p_security_capabilities)
{
	memset(p_security_capabilities, 0, sizeof (struct device_security_capabilities));

	if (ENCRYPTION_ENABLED(dimm_sku))
	{
		p_security_capabilities->passphrase_capable = 1;
		p_security_capabilities->unlock_device_capable = 1;
		p_security_capabilities->erase_crypto_capable = 1;
	}
}

enum memory_type get_memory_type_from_smbios_memory_type(const NVM_UINT8 smbios_type,
		const NVM_UINT16 smbios_type_detail_bits)
{
	enum memory_type mem_type = MEMORY_TYPE_UNKNOWN;

	// Not 100% clear how DMTF is defining NVM-DIMMs
	// Likely DDR4 + non-volatile bit in type detail
	switch (smbios_type)
	{
		case SMBIOS_MEMORY_TYPE_DDR4:
			if (smbios_type_detail_bits & SMBIOS_MEMORY_TYPE_DETAIL_NONVOLATILE)
			{
				mem_type = MEMORY_TYPE_NVMDIMM;
			}
			else
			{
				mem_type = MEMORY_TYPE_DDR4;
			}
			break;

		case SMBIOS_MEMORY_TYPE_NVMDIMM:
			mem_type = MEMORY_TYPE_NVMDIMM;
			break;

		default:
			break;
	}

	return mem_type;
}

enum device_form_factor get_device_form_factor_from_smbios_form_factor(
		const NVM_UINT16 smbios_form_factor)
{
	enum device_form_factor form_factor = DEVICE_FORM_FACTOR_UNKNOWN;

	switch (smbios_form_factor)
	{
		case SMBIOS_FORM_FACTOR_DIMM:
			form_factor = DEVICE_FORM_FACTOR_DIMM;
			break;
		case SMBIOS_FORM_FACTOR_SODIMM:
			form_factor = DEVICE_FORM_FACTOR_SODIMM;
			break;
		default:
			break;
	}

	return form_factor;
}

int get_dimm_storage_capacity(NVM_UINT32 handle, NVM_UINT64 *p_storage_capacity)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if ((rc = get_topology_count()) > 0)
	{
		int num_dimms = rc;
		struct nvm_storage_capacities capacities[num_dimms];
		memset(&capacities, 0, (sizeof (struct nvm_storage_capacities) * num_dimms));
		if ((rc = get_dimm_storage_capacities(num_dimms, capacities)) > 0)
		{
			num_dimms = rc;
			rc = NVM_ERR_BADDEVICE;
			*p_storage_capacity = 0;
			for (int i = 0; i < num_dimms; i++)
			{
				if (handle == capacities[i].device_handle.handle)
				{
					*p_storage_capacity = capacities[i].total_storage_capacity;
					rc = NVM_SUCCESS;
					break;
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int update_capacities_based_on_sku(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_capabilities *p_capabilities,
		struct device_capacities *p_capacities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// get the dimm sku
	struct device_discovery discovery;
	if ((rc = lookup_dev_handle(device_handle, &discovery)) == NVM_SUCCESS)
	{
		NVM_UINT64 raw_capacity = p_capacities->capacity;
		NVM_UINT64 memory_capacity = p_capacities->memory_capacity;
		NVM_UINT64 pm_capacity = p_capacities->app_direct_capacity +
				p_capacities->unconfigured_capacity;

		// no support, all capacity in inaccessible
		if ((!p_capabilities->nvm_features.memory_mode &&
			!p_capabilities->nvm_features.app_direct_mode &&
			!p_capabilities->nvm_features.storage_mode) ||
			(!discovery.device_capabilities.memory_mode_capable &&
			!discovery.device_capabilities.app_direct_mode_capable &&
			!discovery.device_capabilities.storage_mode_capable))
		{
			p_capacities->inaccessible_capacity += raw_capacity;
			p_capacities->memory_capacity = 0;
			p_capacities->app_direct_capacity = 0;
			p_capacities->storage_capacity = 0;
			p_capacities->unconfigured_capacity = 0;
		}
		// check memory mode support
		else if (memory_capacity > 0 &&
				(!p_capabilities->nvm_features.memory_mode ||
				!discovery.device_capabilities.memory_mode_capable))
		{
			p_capacities->inaccessible_capacity += memory_capacity;
			p_capacities->memory_capacity = 0;
		}
		else if (pm_capacity > 0)
		{
			// no PM support at all
			if ((!p_capabilities->nvm_features.app_direct_mode &&
				!p_capabilities->nvm_features.storage_mode) ||
				(!discovery.device_capabilities.app_direct_mode_capable &&
				!discovery.device_capabilities.storage_mode_capable))
			{
				p_capacities->inaccessible_capacity += pm_capacity;
				p_capacities->app_direct_capacity = 0;
				p_capacities->storage_capacity = 0;
				p_capacities->unconfigured_capacity = 0;
			}
			// Storage but no App Direct
			else if (p_capacities->app_direct_capacity > 0 &&
					(!p_capabilities->nvm_features.app_direct_mode ||
							!discovery.device_capabilities.app_direct_mode_capable))
			{
				p_capacities->inaccessible_capacity += p_capacities->app_direct_capacity;
				p_capacities->app_direct_capacity = 0;
			}
			// App direct but no Storage
			else if (p_capacities->storage_capacity > 0 &&
					(!p_capabilities->nvm_features.storage_mode ||
					!discovery.device_capabilities.storage_mode_capable))
			{
				// Storage capacity is considered "un-configured", not inaccessible
				p_capacities->storage_capacity = 0;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to populate the capacities of a single dimm
 */
int get_dimm_capacities(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_capabilities *p_capabilities,
		struct device_capacities *p_capacities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	memset(p_capacities, 0, sizeof (struct device_capacities));
	// get total FW reported capacities from the dimm partition info struct
	struct pt_payload_get_dimm_partition_info pi;
	memset(&pi, 0, sizeof (pi));
	if ((rc = get_partition_info(device_handle, &pi)) == NVM_SUCCESS)
	{
		p_capacities->capacity = MULTIPLES_TO_BYTES(pi.raw_capacity);
		p_capacities->memory_capacity = MULTIPLES_TO_BYTES(pi.volatile_capacity);
		if (p_capacities->memory_capacity)
		{
			p_capacities->reserved_capacity = RESERVED_CAPACITY_BYTES(p_capacities->capacity);
			p_capacities->memory_capacity -= p_capacities->reserved_capacity;
		}
		p_capacities->app_direct_capacity = MULTIPLES_TO_BYTES(pi.pmem_capacity);

		// get BIOS mapped capacities from the platform config data
		struct platform_config_data *p_cfg_data = NULL;
		// on failure or missing current config table, default mapped values are 0
		if ((rc = get_dimm_platform_config(device_handle, &p_cfg_data)) == NVM_SUCCESS)
		{
			struct current_config_table *p_current_config =
					cast_current_config(p_cfg_data);
			if (p_current_config)
			{
				p_capacities->app_direct_capacity =
					p_current_config->mapped_app_direct_capacity;
				get_dimm_storage_capacity(device_handle.handle,
						&p_capacities->storage_capacity);
				p_capacities->unconfigured_capacity =
						p_capacities->capacity
						- p_capacities->memory_capacity
						- p_current_config->mapped_app_direct_capacity
						- p_capacities->reserved_capacity;
			}
		}
		free(p_cfg_data);

		// update capacities based on DIMM SKU
		KEEP_ERROR(rc, update_capacities_based_on_sku(device_handle,
				p_capabilities, p_capacities));
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


/*
 * Helper function to determine if there are existing namespaces
 * from the capacity of the specified NVM-DIMM.
 * Use NAMESPACE_TYPE_UNKNOWN to check for any namespaces on the dimm
 */
int dimm_has_namespaces_of_type(const NVM_NFIT_DEVICE_HANDLE dimm_handle,
		const enum namespace_type ns_type)
{
	COMMON_LOG_ENTRY();
	int matched = 0;

	int rc = get_namespace_count();
	if (rc > 0)
	{
		int ns_count = rc;
		struct nvm_namespace_discovery namespaces[ns_count];
		if ((rc = get_namespaces(ns_count, namespaces)) > 0)
		{
			ns_count = rc;
			rc = NVM_SUCCESS;
			for (int i = 0; i < ns_count && rc == NVM_SUCCESS; i++)
			{
				struct nvm_namespace_details ns_details;
				memset(&ns_details, 0, sizeof (ns_details));
				if ((rc = get_namespace_details(namespaces[i].namespace_uid,
						&ns_details)) == NVM_SUCCESS)
				{
					// check type we're looking for
					if (ns_type == NAMESPACE_TYPE_UNKNOWN || ns_details.type == ns_type)
					{
						// storage, check if dimm handle matches
						if (ns_details.type == NAMESPACE_TYPE_STORAGE)
						{
							if (ns_details.namespace_creation_id.device_handle.handle ==
									dimm_handle.handle)
							{
								matched++;
							}
						}
						// check if dimm is in interleave set
						else if (ns_details.type == NAMESPACE_TYPE_APP_DIRECT)
						{
							struct nvm_interleave_set set;
							memset(&set, 0, sizeof (set));
							if ((rc = get_interleaveset_by_driver_id(
									ns_details.namespace_creation_id.interleave_setid, &set))
									== NVM_SUCCESS)
							{
								for (int j = 0; j < set.dimm_count; j++)
								{
									if (set.dimms[j].handle == dimm_handle.handle)
									{
										matched++;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (rc == NVM_SUCCESS)
	{
		rc = matched;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
