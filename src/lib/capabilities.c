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
 * This file contains the implementation of native API functions to fetch the
 * capabilities of the platform and software.
 */

#include "capabilities.h"
#include "device_adapter.h"
#include "platform_capabilities.h"
#include "utility.h"
#include <persistence/lib_persistence.h>
#include "device_utilities.h"
#include "nvm_context.h"
#include "system.h"

/*
 * Capabilities are determined based on the combination of
 * driver, platform and NVM-DIMM SKU support. This functions
 * turns on features based on what the driver supports.
 */
int driver_features_to_nvm_features(
		const struct driver_feature_flags *p_driver_features,
		struct nvm_features *p_nvm_features)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	p_nvm_features->get_platform_capabilities = p_driver_features->get_platform_capabilities;
	p_nvm_features->get_devices = p_driver_features->get_topology;

	// Features that require Get Topology
	if (p_driver_features->get_topology)
	{
		if (get_topology_count() > 0)
		{
			// get SMBIOS - Get Topology, Get Dimm Detail
			p_nvm_features->get_device_smbios = p_driver_features->get_dimm_detail;

			// get device health - Get Topology, Passthrough
			p_nvm_features->get_device_health = p_driver_features->passthrough;

			// get device settings - Get Topology, Passthrough
			p_nvm_features->get_device_settings = p_driver_features->passthrough;

			// modify device settings - Get Topology, Passthrough
			p_nvm_features->modify_device_settings = p_driver_features->passthrough;

			// get device security - Get Topology, Passthrough
			p_nvm_features->get_device_security = p_driver_features->passthrough;

			// modify device security - Get Topology, Passthrough
			p_nvm_features->modify_device_security = p_driver_features->passthrough;

			// get device performance - Get Topology, Passthrough
			p_nvm_features->get_device_performance = p_driver_features->passthrough;

			// get device firmware - Get Topology, Passthrough
			p_nvm_features->get_device_firmware = p_driver_features->passthrough;

			// update device firmware - Get Topology, Passthrough
			p_nvm_features->update_device_firmware = p_driver_features->passthrough;

			// get sensors - Get Topology, Passthrough
			p_nvm_features->get_sensors = p_driver_features->passthrough;

			// modify sensors - Get Topology, Set Features
			p_nvm_features->modify_sensors = p_driver_features->passthrough;

			// get device capacity - Get Topology, Passthrough
			p_nvm_features->get_device_capacity = p_driver_features->passthrough;

			// modify device capacity - Get Topology, Get Platform Capabilities, Passthrough
			p_nvm_features->modify_device_capacity = (p_driver_features->get_platform_capabilities &
					p_driver_features->passthrough);

			// get pools
			p_nvm_features->get_pools = (p_driver_features->passthrough &
					p_driver_features->get_interleave);

			// get address scrub data - Get Topology, Get Address Scrub
			p_nvm_features->get_address_scrub_data = p_driver_features->get_address_scrub_data;

			// start address scrub - Get Topology, Passthrough
			p_nvm_features->start_address_scrub = p_driver_features->passthrough;

			// quick diagnostic - Get Topology, Passthrough
			p_nvm_features->quick_diagnostic = p_driver_features->passthrough;

			// security diagnostic - Get Topology, Passthrough
			p_nvm_features->security_diagnostic = p_driver_features->passthrough;

			// platform config diagnostic - Get Topology, Get Platform Capabilities
			p_nvm_features->platform_config_diagnostic =
					p_driver_features->get_platform_capabilities;

			// fw consistency diagnostic - Get Topology, Passthrough
			p_nvm_features->fw_consistency_diagnostic = p_driver_features->passthrough;

			p_nvm_features->error_injection = p_driver_features->passthrough;
		}
		else
		{
			COMMON_LOG_DEBUG("There are no devices in the system");
		}
	}

	// Namespace features correlate directly to driver features
	p_nvm_features->get_namespaces = p_driver_features->get_namespaces;
	p_nvm_features->get_namespace_details = p_driver_features->get_namespace_detail;
	p_nvm_features->create_namespace = p_driver_features->create_namespace;
	p_nvm_features->rename_namespace = p_driver_features->rename_namespace;
	p_nvm_features->grow_namespace = p_driver_features->grow_namespace;
	p_nvm_features->shrink_namespace = p_driver_features->shrink_namespace;
	p_nvm_features->enable_namespace = p_driver_features->enable_namespace;
	p_nvm_features->disable_namespace = p_driver_features->disable_namespace;
	p_nvm_features->delete_namespace = p_driver_features->delete_namespace;

	// PM metadata diagnostic - Run Diagnostic
	p_nvm_features->pm_metadata_diagnostic = p_driver_features->run_diagnostic;

	// Driver memory mode capabilities
	p_nvm_features->app_direct_mode = p_driver_features->app_direct_mode;
	p_nvm_features->storage_mode = p_driver_features->storage_mode;

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve driver capabilities and update the nvm_capabilities structure.
 */
int apply_driver_capabilities(struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct nvm_driver_capabilities driver_caps;
	memset(&driver_caps, 0, sizeof (driver_caps));
	int tmp_rc = get_driver_capabilities(&driver_caps);
	if (tmp_rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("get_driver_capabilities returned %d", tmp_rc);
		rc = tmp_rc;
	}
	else
	{
		p_capabilities->sw_capabilities.min_namespace_size =
						(driver_caps.min_namespace_size < BYTES_PER_GB) ? BYTES_PER_GB :
								driver_caps.min_namespace_size;
		NVM_UINT32 num_block_sizes = driver_caps.num_block_sizes;
		if (num_block_sizes > NVM_MAX_BLOCK_SIZES)
		{
			COMMON_LOG_ERROR_F(
					"The driver returned invalid number of block sizes: %u", num_block_sizes);
			num_block_sizes = NVM_MAX_BLOCK_SIZES;
		}
		p_capabilities->sw_capabilities.block_size_count = num_block_sizes;
		for (NVM_UINT32 i = 0; i < num_block_sizes; i++)
		{
			p_capabilities->sw_capabilities.block_sizes[i] = driver_caps.block_sizes[i];
		}
		p_capabilities->sw_capabilities.namespace_memory_page_allocation_capable =
				driver_caps.namespace_memory_page_allocation_capable;

		// apply driver supported features to host software supported features
		driver_features_to_nvm_features(&driver_caps.features, &p_capabilities->nvm_features);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Capabilities are determined based on the combination of
 * driver, platform and NVM-DIMM SKU support. This functions
 * turns on/off select features based on the BIOS support.
 */
void platform_capabilities_to_nvm_features(const struct platform_capabilities *p_pcat,
		struct nvm_features *p_features)
{
	// capacity provisioning feature needs BIOS support
	if (!p_pcat->bios_config_support)
	{
		p_features->modify_device_capacity = 0;
	}

	// driver doesn't have concept of Memory Mode so set based on bios only
	p_features->memory_mode = p_pcat->memory_mode.supported;

	// turn off app direct if bios doesn't support
	if (!p_pcat->app_direct_mode.supported)
	{
		p_features->app_direct_mode = 0;
	}
	// turn off storage mode if bios doesn't support
	if (!p_pcat->storage_mode_supported)
	{
		p_features->storage_mode = 0;
	}
}

enum app_direct_mode get_current_app_direct_mode(NVM_UINT8 current_mode)
{
	enum app_direct_mode app_direct_mode;

	app_direct_mode = ((current_mode >> 2) & 0b11); // bits 3:2
	if ((app_direct_mode != APP_DIRECT_MODE_DISABLED) &&
		(app_direct_mode != APP_DIRECT_MODE_ENABLED))
	{
		app_direct_mode = APP_DIRECT_MODE_UNKNOWN;
	}

	return app_direct_mode;
}

/*
 * Retrieve BIOS platform capabilities and update the nvm_capabilities structure.
 */
int apply_bios_capabilities(struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (!p_capabilities)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_capabilities is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
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
				// check the table
				rc = check_pcat(p_pcat);
				if (rc == NVM_SUCCESS)
				{
					// iterate over all the extension tables
					NVM_UINT32 offset = PCAT_TABLE_SIZE;
					while (offset < p_pcat->header.length)
					{
						struct pcat_extension_table_header *p_header =
								(struct pcat_extension_table_header *)
								((NVM_UINT8 *)p_pcat + offset);

						// check the length for validity
						if (p_header->length == 0 ||
								(p_header->length + offset) > p_pcat->header.length)
						{
							COMMON_LOG_ERROR_F("Extension table length %d invalid",
									p_header->length);
							rc = NVM_ERR_BADPCAT;
							break;
						}

						// platform capability info table
						if (p_header->type == PCAT_TABLE_PLATFORM_INFO)
						{
							struct platform_capabilities_ext_table *p_plat_info =
									(struct platform_capabilities_ext_table *)p_header;

							p_capabilities->platform_capabilities.bios_config_support =
									MGMT_SW_CONFIG_SUPPORTED(p_plat_info->mgmt_sw_config_support);
							p_capabilities->platform_capabilities.bios_runtime_support =
									RUNTIME_CONFIG_SUPPORTED(p_plat_info->mgmt_sw_config_support);
							p_capabilities->platform_capabilities.current_volatile_mode =
									CURRENT_VOLATILE_MODE(p_plat_info->current_mem_mode);
							p_capabilities->platform_capabilities.current_app_direct_mode =
									get_current_app_direct_mode(p_plat_info->current_mem_mode);
							p_capabilities->platform_capabilities.memory_mirror_supported =
									PMEM_MIRROR_SUPPORTED(p_plat_info->pmem_ras_capabilities);
							p_capabilities->platform_capabilities.memory_spare_supported =
									PMEM_SPARE_SUPPORTED(p_plat_info->pmem_ras_capabilities);
							p_capabilities->platform_capabilities.memory_migration_supported =
									PMEM_MIGRATION_SUPPORTED(p_plat_info->pmem_ras_capabilities);

							if (p_plat_info->mem_mode_capabilities & MEM_MODE_1LM)
							{
								p_capabilities->platform_capabilities.one_lm_mode.supported = 1;
							}
							if (p_plat_info->mem_mode_capabilities & MEM_MODE_MEMORY)
							{
								p_capabilities->platform_capabilities.memory_mode.supported = 1;
							}
							if (p_plat_info->mem_mode_capabilities & MEM_MODE_APP_DIRECT)
							{
								p_capabilities->platform_capabilities.app_direct_mode.supported = 1;
							}
							if (p_plat_info->mem_mode_capabilities & MEM_MODE_STORAGE)
							{
								p_capabilities->platform_capabilities.storage_mode_supported = 1;
							}
						}
						// memory interleave capability table
						else if (p_header->type == PCAT_TABLE_MEMORY_INTERLEAVE_INFO)
						{
							struct memory_interleave_capabilities_ext_table *p_mic =
									(struct memory_interleave_capabilities_ext_table *)p_header;
							struct memory_capabilities *p_cap = NULL;
							switch (p_mic->memory_mode)
							{
								case INTERLEAVE_MEM_MODE_1LM:
									p_cap = &(p_capabilities->platform_capabilities.one_lm_mode);
									break;
								case INTERLEAVE_MEM_MODE_2LM:
									p_cap = &(p_capabilities->platform_capabilities.memory_mode);
									break;
								case INTERLEAVE_MEM_MODE_APP_DIRECT:
									p_cap = &(p_capabilities->platform_capabilities.
											app_direct_mode);
									break;
							}
							if (p_cap == NULL)
							{
								COMMON_LOG_ERROR_F("memory_mode %d not supported",
										p_mic->memory_mode);
								break;
							}
							else
							{
								if (!(p_cap->supported))
								{
									COMMON_LOG_WARN_F(
										"memory capability for mode %d is not supported, "
										"but capability structure still provided",
										p_mic->memory_mode);
								}
								p_cap->interleave_formats_count = 0;
								p_cap->interleave_alignment_size =
										p_mic->interleave_alignment_size;
								for (int j = 0; j < p_mic->supported_interleave_count; j++)
								{
									// BIOS does not differentiate channel ways as
									// separate formats, this code does that.
									int cr_channel_ways = ((p_mic->interleave_format_list[j] >>
											PCAT_FORMAT_WAYS_SHIFT) & PCAT_FORMAT_WAYS_MASK);
									// see enum bitmap_interleave_ways, 8 is max bit position
									for (int way_bit = 0; way_bit <= 8; way_bit++)
									{
										if ((cr_channel_ways >> way_bit) & 1) // is way supported?
										{
											interleave_format_to_struct(
													p_mic->interleave_format_list[j],
													&(p_cap->interleave_formats
														[p_cap->interleave_formats_count]));
											// reset the way to the correct value
											p_cap->interleave_formats
												[p_cap->interleave_formats_count].ways =
												interleave_way_to_enum(
														cr_channel_ways & (1 << way_bit));
											p_cap->interleave_formats_count++;
										}
									}
								}
							}
						}
						// else, just ignore it other table types
						offset += p_header->length;
					}
				}

				// translate BIOS supported capabilities into nvm features
				platform_capabilities_to_nvm_features(
						&p_capabilities->platform_capabilities,
						&p_capabilities->nvm_features);
			}
			// clean up
			free(p_pcat);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Capabilities are determined based on the combination of
 * driver, platform and NVM-DIMM SKU support. This functions
 * turns off select features based on the NVM-DIMM SKU support.
 */
void dimm_sku_capabilities_to_nvm_features(
		const struct dimm_sku_capabilities *p_sku_cap,
		struct nvm_features *p_nvm_features)
{
	// mixed SKU = no support for capacity provisioning
	if (p_sku_cap->mixed_sku)
	{
		p_nvm_features->modify_device_capacity = 0;
		p_nvm_features->modify_device_security = 0;
		p_nvm_features->modify_device_settings = 0;
		p_nvm_features->get_pools = 0;
		p_nvm_features->get_namespaces = 0;
		p_nvm_features->get_namespace_details = 0;
		p_nvm_features->create_namespace = 0;
		p_nvm_features->rename_namespace = 0;
		p_nvm_features->grow_namespace = 0;
		p_nvm_features->shrink_namespace = 0;
		p_nvm_features->enable_namespace = 0;
		p_nvm_features->disable_namespace = 0;
		p_nvm_features->delete_namespace = 0;
	}

	// No NVM-DIMMs support memory mode, turn off
	if (!p_sku_cap->memory_sku)
	{
		p_nvm_features->memory_mode = 0;
	}

	// No NVM-DIMMs support app direct mode, turn off
	if (!p_sku_cap->app_direct_sku)
	{
		p_nvm_features->app_direct_mode = 0;
	}

	// No NVM-DIMMs support storage mode, turn off
	if (!p_sku_cap->storage_sku)
	{
		p_nvm_features->storage_mode = 0;
	}
}

/*
 * Retrieve the NVM-DIMM SKU capabilities and update the nvm_capabilities structure.
 */
int apply_dimm_sku_capabilities(struct nvm_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	rc = nvm_get_device_count();
	if (rc > 0)
	{
		int dev_count = rc;
		struct device_discovery devices[dev_count];
		memset(&devices, 0, dev_count * sizeof (struct device_discovery));
		rc = nvm_get_devices(devices, dev_count);
		if (rc == dev_count)
		{
			rc = NVM_SUCCESS;
			NVM_BOOL sku_set = 0;
			NVM_UINT32 dimm_sku = 0;
			for (int i = 0; i < dev_count; i++)
			{
				// set the SKU to check on the first NVM-DIMM
				// then check all others against it
				if (!sku_set)
				{
					dimm_sku = devices[i].dimm_sku;
					sku_set = 1;
				}
				else if (devices[i].dimm_sku != dimm_sku)
				{
					p_capabilities->sku_capabilities.mixed_sku = 1;
				}

				// only check manageable dimms for supported modes
				if (devices[i].manageability == MANAGEMENT_VALIDCONFIG)
				{
					// at least one manageable dimm supports memory mode
					if (devices[i].dimm_sku & SKU_MEMORY_MODE_ENABLED)
					{
						p_capabilities->sku_capabilities.memory_sku = 1;
					}
					// at least one manageable dimm supports app direct mode
					if (devices[i].dimm_sku & SKU_APP_DIRECT_MODE_ENABLED)
					{
						p_capabilities->sku_capabilities.app_direct_sku = 1;
					}
					// at least one manageable dimm supports storage mode
					if (devices[i].dimm_sku & SKU_STORAGE_MODE_ENABLED)
					{
						p_capabilities->sku_capabilities.storage_sku = 1;
					}
				}
			}
		}
	}

	// apply dimm SKU capabilities to host SW supported features
	dimm_sku_capabilities_to_nvm_features(
			&p_capabilities->sku_capabilities,
			&p_capabilities->nvm_features);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve capabilities of the NVM-DIMM host software based on
 * the supported driver, platform and NVM-DIMM SKU capabilities.
 */
int nvm_get_nvm_capabilities(struct nvm_capabilities *p_capabilities)
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
	else if (p_capabilities == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_capabilties is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (get_nvm_context_capabilities(p_capabilities) != NVM_SUCCESS)
	{
		// all capabilities are disabled by default
		memset(p_capabilities, 0, sizeof (struct nvm_capabilities));

		// Start by retrieving and set the capabilities based
		// on what the driver supports.
		int tmprc = apply_driver_capabilities(p_capabilities);
		if (tmprc != NVM_SUCCESS)
		{
			// ignore failures due to missing drivers,
			// but keep other types of errors.
			if (tmprc != NVM_ERR_DRIVERFAILED)
			{
				rc = tmprc;
			}
			COMMON_LOG_ERROR("Failed to retrieve the driver capabilities");
		}
		else
		{
			// Then retrieve and apply the platform BIOS supported capabilities
			// (if the driver supports the IOCTL)
			if (p_capabilities->nvm_features.get_platform_capabilities)
			{
				rc = apply_bios_capabilities(p_capabilities);
			}

			// Finally retrieve and apply the NVM-DIMM SKU capabilities
			// (if the driver supports the IOCTL)
			if (p_capabilities->nvm_features.get_devices)
			{
				KEEP_ERROR(rc, apply_dimm_sku_capabilities(p_capabilities));
			}
		}

		// Second most common configuration is memory mode only.
		// In this case, don't expose any PM features to the user.
		if (!p_capabilities->nvm_features.app_direct_mode &&
				!p_capabilities->nvm_features.storage_mode)
		{
			p_capabilities->nvm_features.get_namespaces = 0;
			p_capabilities->nvm_features.get_namespace_details = 0;
			p_capabilities->nvm_features.create_namespace = 0;
			p_capabilities->nvm_features.rename_namespace = 0;
			p_capabilities->nvm_features.grow_namespace = 0;
			p_capabilities->nvm_features.shrink_namespace = 0;
			p_capabilities->nvm_features.enable_namespace = 0;
			p_capabilities->nvm_features.disable_namespace = 0;
			p_capabilities->nvm_features.delete_namespace = 0;
		}

		if (rc == NVM_SUCCESS)
		{
			set_nvm_context_capabilities(p_capabilities);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int check_device_app_direct_namespaces_for_sku_violation(
		const struct nvm_capabilities *p_capabilities,
		const NVM_NFIT_DEVICE_HANDLE device_handle, NVM_BOOL *p_sku_violation)
{
	int rc = NVM_SUCCESS;
	if (!p_capabilities->nvm_features.app_direct_mode)
	{
		int has_namespaces = dimm_has_namespaces_of_type(device_handle, NAMESPACE_TYPE_APP_DIRECT);
		if (has_namespaces < 0)
		{
			KEEP_ERROR(rc, has_namespaces);
		}
		else if (has_namespaces)
		{
			COMMON_LOG_ERROR_F("An unsupported App Direct namespace exists on " NVM_DIMM_NAME " %u",
					device_handle.handle);
			*p_sku_violation = 1;
		}
	}
	return rc;
}

int check_device_storage_namespaces_for_sku_violation(
		const struct nvm_capabilities *p_capabilities,
		const NVM_NFIT_DEVICE_HANDLE device_handle, NVM_BOOL *p_sku_violation)
{
	int rc = NVM_SUCCESS;
	if (!p_capabilities->nvm_features.storage_mode)
	{
		int has_namespaces = dimm_has_namespaces_of_type(device_handle, NAMESPACE_TYPE_STORAGE);
		if (has_namespaces < 0)
		{
			KEEP_ERROR(rc, has_namespaces);
		}
		else if (has_namespaces)
		{
			COMMON_LOG_ERROR_F("An unsupported storage namespace exists on " NVM_DIMM_NAME " %u",
					device_handle.handle);
			*p_sku_violation = 1;
		}
	}
	return rc;
}


/*
 * Helper function to determine if an NVM-DIMM is in violation of it's supported license.
 */
int device_in_sku_violation(const NVM_NFIT_DEVICE_HANDLE device_handle,
		const struct nvm_capabilities *p_capabilities,
		NVM_BOOL *p_sku_violation)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	*p_sku_violation = 0;
	if (!p_capabilities->nvm_features.get_device_capacity)
	{
		rc = NVM_ERR_NOTSUPPORTED;
	}
	else
	{
		struct device_capacities capacities;
		memset(&capacities, 0, sizeof (capacities));
		int rc = get_dimm_capacities(device_handle, p_capabilities, &capacities);
		if (rc == NVM_SUCCESS && capacities.inaccessible_capacity > 0)
		{
			*p_sku_violation = 1;
		}

		// check for storage namespaces when storage mode is not supported
		if (!(*p_sku_violation))
		{
			int tmprc = check_device_storage_namespaces_for_sku_violation(p_capabilities,
					device_handle, p_sku_violation);
			if (tmprc != NVM_ERR_NOTSUPPORTED) // ignore not supported errors
			{
				KEEP_ERROR(rc, tmprc);
			}
		}

		// check for app direct namespaces when app direct mode is not supported
		if (!(*p_sku_violation))
		{
			int tmprc = check_device_app_direct_namespaces_for_sku_violation(p_capabilities,
					device_handle, p_sku_violation);
			if (tmprc != NVM_ERR_NOTSUPPORTED) // ignore not supported
			{
				KEEP_ERROR(rc, tmprc);
			}
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to determine if any NVM-DIMMs in the system
 * are in violation of the supported license.
 */
int system_in_sku_violation(const struct nvm_capabilities *p_capabilities,
		NVM_BOOL *p_sku_violation)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	*p_sku_violation = 0;
	if (p_capabilities->nvm_features.get_devices)
	{
		rc = nvm_get_device_count();
		if (rc > 0)
		{
			int device_count = rc;
			struct device_discovery devices[device_count];
			rc = nvm_get_devices(devices, device_count);
			if (rc > 0)
			{
				device_count = rc;
				rc = NVM_SUCCESS;
				for (int i = 0; i < device_count; i++)
				{
					if (devices[i].manageability == MANAGEMENT_VALIDCONFIG)
					{
						KEEP_ERROR(rc, device_in_sku_violation(devices[i].device_handle,
								p_capabilities, p_sku_violation));
					}
					// stop if sku violation detected
					if (*p_sku_violation)
					{
						COMMON_LOG_ERROR(
							"One more " NVM_DIMM_NAME "s are configured in "
							"violation of the license.");
						break;
					}
				}
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_devices_is_supported()
{
	int rc = NVM_ERR_NOTSUPPORTED;

	// if capabilities are already in context, use them
	// rather than making another round trip to the driver
	struct nvm_capabilities capabilities;
	if (get_nvm_context_capabilities(&capabilities) == NVM_SUCCESS &&
			capabilities.nvm_features.get_devices)
	{
		rc = NVM_SUCCESS;
	}
	else
	{
		// directly calling get_driver_capabilities instead of
		// nvm_get_nvm_capabilities to avoid recursive looping
		// between nvm_get_devices and nvm_get_nvm_capabilitiies
		struct nvm_driver_capabilities driver_caps;
		memset(&driver_caps, 0, sizeof (driver_caps));
		if ((get_driver_capabilities(&driver_caps) == NVM_SUCCESS) &&
				driver_caps.features.get_topology)
		{
			rc = NVM_SUCCESS;
		}
	}
	return rc;
}
