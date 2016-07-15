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
 * This file contains the implementation of Windows device adapter interface for general operations.
 */

#include "device_adapter.h"
#include <persistence/logging.h>
#include "nvm_types.h"
#include "platform_capabilities.h"
#include "smbios_utilities.h"
#include "utility.h"
#include "system.h"
#include <os/os_adapter.h>
#include <system/system.h>
#include <string/s_str.h>
#include <string/revision.h>
#include <windows.h>
#include <winioctl.h>
#include <ntddscsi.h>
#include <initguid.h>
#include "win_adapter.h"
#include <windows/PrivateIoctlDefinitions.h>
#include <windows/DiagnosticExport.h>
#include "device_utilities.h"

#define	WIN_DRIVER_VERSION_MAJOR_MIN	1
#define	WIN_DRIVER_VERSION_MAJOR_MAX	1
#define	SCSI_PORT_MAX 32
/*
 * Bit selection as defined for the MSR_DRAM_POWER_LIMIT register,
 * 618h from the IA64/32 Software Developers Manual
 */
#define	POWER_LIMIT_ENABLE_BIT	0x80

short g_scsi_port = -1;

// Helper function declarations
enum label_area_health_result convert_label_health_result(LABEL_AREA_HEALTH_EVENT event);
enum ns_health_result convert_ns_health_status(NAMESPACE_HEALTH_EVENT event);
DIAGNOSTIC_TEST convert_diagnostic_test(enum driver_diagnostic diagnostic);

/*
 * Support is determined by driver version
 */
NVM_BOOL is_supported_driver_available()
{
	COMMON_LOG_ENTRY();
	NVM_BOOL is_supported = 0;

	NVM_VERSION driver_rev;
	int rc = get_vendor_driver_revision(driver_rev, NVM_VERSION_LEN);
	if (rc == NVM_SUCCESS)
	{
		// parse the version string into parts
		NVM_UINT16 major, minor, hotfix, build = 0;
		parse_main_revision(&major, &minor, &hotfix, &build, driver_rev,
				NVM_VERSION_LEN);

		if (major >= WIN_DRIVER_VERSION_MAJOR_MIN &&
				major <= WIN_DRIVER_VERSION_MAJOR_MAX)
		{
			is_supported = 1;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(is_supported);
	return is_supported;
}

/*
 * Retrieve the NVDIMM driver version.
 * For the purpose of testing if a given SCSI port is valid
 */
static int __get_driver_revision(short scsi_port)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	if (scsi_port < 0)
	{
		COMMON_LOG_ERROR("Invalid test port");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		HANDLE handle = INVALID_HANDLE_VALUE;

		// Open the Drivers IOCTL File Handle
		if ((rc = open_ioctl_target(&handle, scsi_port)) == NVM_SUCCESS)
		{
			CR_GET_DRIVER_REVISION_IOCTL payload_in;
			CR_GET_DRIVER_REVISION_IOCTL payload_out;

			memset(&payload_in, 0, sizeof (CR_GET_DRIVER_REVISION_IOCTL));

			// Verify IOCTL is sent successfully and that the driver had no errors
			if ((rc = send_ioctl_command(handle,
				IOCTL_CR_GET_VENDOR_DRIVER_REVISION,
				&payload_in,
				sizeof (CR_GET_DRIVER_REVISION_IOCTL),
				&payload_out,
				sizeof (CR_GET_DRIVER_REVISION_IOCTL))) == NVM_SUCCESS)
			{
				rc = ind_err_to_nvm_lib_err(payload_out.ReturnCode);
			}

			CloseHandle(handle);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the vendor specific NVDIMM driver version.
 */
int get_vendor_driver_revision(NVM_VERSION version_str, const NVM_SIZE str_len)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	if (version_str == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, version buffer in NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (str_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, version buffer length is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		CR_GET_DRIVER_REVISION_IOCTL ioctl_data;

		memset(&ioctl_data, 0, sizeof (ioctl_data));

		if ((rc = execute_ioctl(sizeof (ioctl_data), &ioctl_data,
			IOCTL_CR_GET_VENDOR_DRIVER_REVISION)) == NVM_SUCCESS &&
			(rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode)) == NVM_SUCCESS)
		{
			s_strcpy(version_str, (char *)ioctl_data.OutputPayload.VendorDriverVersion,
				str_len);
		}
		else if (rc == NVM_ERR_DRIVERFAILED)
		{
			COMMON_LOG_ERROR("Failed to communicate with driver to get version");
			rc = NVM_ERR_BADDRIVER;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function - assumes string length >= 4
 */
DWORD string_to_dword(const char *str)
{
	union
	{
		DWORD dword;
		char string[4];
	} fw_table_signature;

	memmove(fw_table_signature.string, str, sizeof (fw_table_signature.string));

	return fw_table_signature.dword;
}

/*
 * Get the DWORD-formatted Windows signature for fetching the ACPI tables
 */
DWORD get_acpi_provider_signature()
{
	// Endian-flipped "ACPI"
	static const char *ACPI_PROVIDER_SIGNATURE = "IPCA";
	return string_to_dword(ACPI_PROVIDER_SIGNATURE);
}

/*
 * Gets the data size of an ACPI table with a given signature.
 * Assumes inputs are non-NULL.
 */
int get_acpi_table_size(const char *table_signature, NVM_UINT64 *p_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	DWORD acpi_table_sig = string_to_dword(table_signature);
	UINT buf_size = GetSystemFirmwareTable(get_acpi_provider_signature(), acpi_table_sig, NULL, 0);
	if (buf_size > 0)
	{
		*p_size = buf_size;
	}
	else
	{
		COMMON_LOG_ERROR_F("Windows reported no ACPI with sig '%.4s' table (size = 0)",
				table_signature);
		rc = NVM_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_acpi_table(const char *table_signature, NVM_UINT8 *p_acpi_table,
		const NVM_UINT64 table_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	DWORD acpi_table_sig = string_to_dword(table_signature);
	UINT size_fetched = GetSystemFirmwareTable(get_acpi_provider_signature(), acpi_table_sig,
			p_acpi_table, table_size);
	if (size_fetched == 0)
	{
		COMMON_LOG_ERROR_F("Windows reported no ACPI '%.4s' table",
				table_signature);
		rc = NVM_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the actual size of the platform capabilities table
 */
int get_platform_capabilities_size(NVM_UINT64 *p_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (p_size == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, size is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		rc = get_acpi_table_size(PCAT_TABLE_SIGNATURE, p_size);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the capabilities of the host platform
 */
int get_platform_capabilities(struct bios_capabilities *p_capabilities,
		const NVM_UINT32 cap_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	NVM_UINT64 size = 0; // Actual PCAT table size

	if (p_capabilities == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, capabilities is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = get_platform_capabilities_size(&size)) != NVM_SUCCESS)
	{
		// hold onto the return code
	}
	else if (size > cap_len)
	{
		COMMON_LOG_ERROR_F("cap_len=%u not large enough for capabilities table of size=%llu",
				cap_len, size);
		rc = NVM_ERR_BADSIZE;
	}
	else
	{
		memset(p_capabilities, 0, cap_len);

		rc = get_acpi_table(PCAT_TABLE_SIGNATURE, (NVM_UINT8 *)p_capabilities, cap_len);

		COMMON_LOG_DEBUG_F("Capabilities: (expected length=%u)", cap_len);
		COMMON_LOG_DEBUG_F("Signature: %.4s", p_capabilities->header.signature);
		COMMON_LOG_DEBUG_F("Length: %u", p_capabilities->header.length);
		COMMON_LOG_DEBUG_F("Revision: %hhu", p_capabilities->header.revision);
		COMMON_LOG_DEBUG_F("Checksum: %hhu", p_capabilities->header.checksum);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the number of DIMMs in the system's memory topology
 */
int get_topology_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	CR_GET_TOPOLOGY_IOCTL ioctl_data;
	memset(&ioctl_data, 0, sizeof (ioctl_data));

	// Windows expects an output payload that is exactly the size of count
	// If Count 0 is entered and the implict payload 1 is not removed bufferoverrun will
	// be returned as error.
	size_t buf_size = sizeof (CR_GET_TOPOLOGY_IOCTL)
		- sizeof (CR_GET_TOPOLOGY_OUTPUT_PAYLOAD);

	// If NvmdimmCount is not equal to the number of NVDIMMS found internally by the driver
	// during device enumeration, this var is set to the exact value expected by the driver
	ioctl_data.InputPayload.TopologiesCount = 0;

	if ((rc = execute_ioctl(buf_size, &ioctl_data, IOCTL_CR_GET_TOPOLOGY)) == NVM_SUCCESS)
	{
		// For windows driver if the buffer provided is less than the
		// amount of space the driver would expect underrun is returned
		// and the count variable is set to the expected number of elements
		if (ioctl_data.ReturnCode == CR_RETURN_CODE_BUFFER_OVERRUN ||
				ioctl_data.ReturnCode == CR_RETURN_CODE_BUFFER_UNDERRUN ||
				ioctl_data.ReturnCode == CR_RETURN_CODE_SUCCESS)
		{
			rc = (int)ioctl_data.InputPayload.TopologiesCount;
		}
		else
		{
			rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode);
		}
	}

	return rc;
}

void copy_interface_fmt_codes(struct nvm_topology *p_topo,
		NVDIMM_TOPOLOGY *p_drv_topo)
{
	COMMON_LOG_ENTRY();

	p_topo->fmt_interface_codes[0] = p_drv_topo->FmtInterfaceCode;
	// Additional IFCs from driver - start after the first one
	for (int i = 1, drv_i = 0;
			(i < NVM_MAX_IFCS_PER_DIMM) && (drv_i < MAX_ADDITIONAL_FIC_COUNT);
			i++, drv_i++)
	{
		p_topo->fmt_interface_codes[i] =
				p_drv_topo->AdditionalFmtInterfaceCodes[drv_i];
	}

	COMMON_LOG_EXIT();
}

/*
 * Get the system's memory topology
 */
int get_topology(const NVM_UINT8 count, struct nvm_topology *p_dimm_topo)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (p_dimm_topo == NULL)
	{
		COMMON_LOG_ERROR("p_dimm_topo is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = get_topology_count()) > 0)
	{
		memset(p_dimm_topo, 0, sizeof (struct nvm_topology) * count);

		int actual_count = rc;

		size_t buf_size = sizeof (CR_GET_TOPOLOGY_IOCTL) +
				(actual_count - 1) * sizeof (NVDIMM_TOPOLOGY);
		CR_GET_TOPOLOGY_IOCTL *p_ioctl_data = calloc(1, buf_size);

		if (p_ioctl_data)
		{
			p_ioctl_data->InputPayload.TopologiesCount = actual_count;

			if ((rc = execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_GET_TOPOLOGY))
				== NVM_SUCCESS &&
				(rc = ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == NVM_SUCCESS)
			{
				NVM_UINT8 *p_smbios_table = NULL;
				size_t smbios_table_size = 0;
				rc = get_smbios_table_alloc(&p_smbios_table, &smbios_table_size);
				if (rc == NVM_SUCCESS)
				{
					int return_count = 0;
					if (count < actual_count)
					{
						return_count = count;
						COMMON_LOG_ERROR("array too small to hold entire topology");
						KEEP_ERROR(rc, NVM_ERR_ARRAYTOOSMALL);
					}
					else
					{
						return_count = actual_count;
						KEEP_ERROR(rc, actual_count);
					}

					for (int i = 0; i < return_count; i++)
					{
						p_dimm_topo[i].device_handle.handle =
							p_ioctl_data->OutputPayload.TopologiesList[i].
								NfitDeviceHandle.DeviceHandle;
						p_dimm_topo[i].id = p_ioctl_data->OutputPayload.TopologiesList[i].Id;
						p_dimm_topo[i].vendor_id =
							p_ioctl_data->OutputPayload.TopologiesList[i].VendorId;
						p_dimm_topo[i].device_id =
							p_ioctl_data->OutputPayload.TopologiesList[i].DeviceId;
						p_dimm_topo[i].revision_id =
							p_ioctl_data->OutputPayload.TopologiesList[i].RevisionId;
						p_dimm_topo[i].subsystem_vendor_id =
								p_ioctl_data->OutputPayload.TopologiesList[i].SubsystemVendorId;
						p_dimm_topo[i].subsystem_device_id =
								p_ioctl_data->OutputPayload.TopologiesList[i].SubsystemDeviceId;
						p_dimm_topo[i].subsystem_revision_id =
								p_ioctl_data->OutputPayload.TopologiesList[i].SubsystemRevisionId;
						p_dimm_topo[i].manufacturing_info_valid =
								p_ioctl_data->OutputPayload.TopologiesList[i].
									ManufacturingInfoValid;
						p_dimm_topo[i].manufacturing_location =
								p_ioctl_data->OutputPayload.TopologiesList[i].ManufacturingLocation;
						p_dimm_topo[i].manufacturing_date =
								p_ioctl_data->OutputPayload.TopologiesList[i].ManufacturingDate;

						copy_interface_fmt_codes(&(p_dimm_topo[i]),
								&(p_ioctl_data->OutputPayload.TopologiesList[i]));

						int mem_type = get_device_memory_type_from_smbios_table(
								p_smbios_table, smbios_table_size,
								p_dimm_topo[i].id);
						if (mem_type < 0)
						{
							KEEP_ERROR(rc, mem_type);
						}
						else
						{
							p_dimm_topo[i].type = mem_type;
						}
					}
				}

				if (p_smbios_table)
				{
					free(p_smbios_table);
				}
			}

			free(p_ioctl_data);
		}
		else
		{
			COMMON_LOG_ERROR("failed to dynamically allocate output payload");
			rc = NVM_ERR_NOMEMORY;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the details of a specific dimm
 */
int get_dimm_details(NVM_NFIT_DEVICE_HANDLE device_handle, struct nvm_details *p_dimm_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_dimm_details == NULL)
	{
		COMMON_LOG_ERROR("nvm_details pointer was NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		rc = get_dimm_physical_id_from_handle(device_handle);
		if (rc >= 0)
		{
			NVM_UINT16 physical_id = rc;
			rc = get_dimm_details_for_physical_id(physical_id, p_dimm_details);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_smbios_inventory_count()
{
	COMMON_LOG_ENTRY();
	int rc = 0;

	NVM_UINT8 *p_smbios_table = NULL;
	size_t smbios_table_size = 0;
	rc = get_smbios_table_alloc(&p_smbios_table, &smbios_table_size);
	if (rc == NVM_SUCCESS)
	{
		rc = smbios_get_populated_memory_device_count(p_smbios_table, smbios_table_size);
	}

	if (p_smbios_table)
	{
		free(p_smbios_table);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}


int get_smbios_inventory(const NVM_UINT8 count, struct nvm_details *p_smbios_inventory)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_smbios_inventory == NULL)
	{
		COMMON_LOG_ERROR("nvm_details pointer was NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		memset(p_smbios_inventory, 0, sizeof (struct nvm_details) * count);

		NVM_UINT8 *p_smbios_table = NULL;
		size_t smbios_table_size = 0;
		rc = get_smbios_table_alloc(&p_smbios_table, &smbios_table_size);
		if (rc == NVM_SUCCESS)
		{
			rc = smbios_table_to_nvm_details_array(p_smbios_table,
					smbios_table_size, p_smbios_inventory, count);
		}

		if (p_smbios_table)
		{
			free(p_smbios_table);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Return driver capabilities
 */
int get_driver_capabilities(struct nvm_driver_capabilities *p_capabilities)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (p_capabilities == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, capabilities is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		CR_GET_DRIVER_CAPABILITIES_IOCTL ioctl_data;

		memset(&ioctl_data, 0, sizeof (ioctl_data));

		if ((rc = execute_ioctl(sizeof (ioctl_data), &ioctl_data, IOCTL_CR_GET_DRIVER_CAPABILITIES))
			== NVM_SUCCESS && (rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode)) == NVM_SUCCESS)
		{
			memset(p_capabilities, 0, sizeof (struct nvm_driver_capabilities));

			p_capabilities->min_namespace_size =
					ioctl_data.OutputPayload.Capabilities.MinNamespaceSize;

			NVM_UINT32 num_block_sizes =
					ioctl_data.OutputPayload.Capabilities.NumBlockSizes;
			p_capabilities->num_block_sizes = num_block_sizes;
			for (NVM_UINT32 i = 0; (i < num_block_sizes) && (i < MAX_NUMBER_OF_BLOCK_SIZES);
					i++)
			{
				p_capabilities->block_sizes[i] =
						ioctl_data.OutputPayload.Capabilities.BlockSizes[i];
			}
			p_capabilities->namespace_memory_page_allocation_capable = 0;

			p_capabilities->features.get_platform_capabilities = 1;
			p_capabilities->features.get_topology =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.GetTopology;
			p_capabilities->features.get_interleave =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.GetInterleave;
			p_capabilities->features.get_dimm_detail =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.GetDimmDetail;
			p_capabilities->features.get_namespaces =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.GetNamespaces;
			p_capabilities->features.get_namespace_detail =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.GetNamespaceDetail;
			p_capabilities->features.get_boot_status =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.SendPassthru;
			p_capabilities->features.get_power_data =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.GetPowerData;
			p_capabilities->features.get_security_state =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.SendPassthru;
			p_capabilities->features.get_log_page =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.SendPassthru;
			p_capabilities->features.get_features =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.SendPassthru;
			p_capabilities->features.set_features =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.SendPassthru;
			p_capabilities->features.create_namespace =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.CreateNamespace;
			p_capabilities->features.rename_namespace =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.RenameNamespace;
			p_capabilities->features.delete_namespace =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.DeleteNamespace;
			p_capabilities->features.set_security_state =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.SendPassthru;
			p_capabilities->features.enable_logging =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.SendPassthru;
			p_capabilities->features.run_diagnostic =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.RunDiagnostic;
			p_capabilities->features.passthrough =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.SendPassthru;
			p_capabilities->features.app_direct_mode =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.PmemNamespace;
			p_capabilities->features.storage_mode =
					ioctl_data.OutputPayload.Capabilities.SupportedFeatures.BlockNamespace;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int ind_err_to_nvm_lib_err(CR_RETURN_CODES ind_err)
{
	COMMON_LOG_ENTRY();

	int ret = NVM_SUCCESS;
	if (ind_err)
	{
		COMMON_LOG_ERROR_F("device driver error = %d", ind_err);
		switch (ind_err)
		{
			case CR_RETURN_CODE_SUCCESS :
				ret = NVM_SUCCESS;
				break;
			case CR_RETURN_CODE_NOTSUPPORTED :
				ret = NVM_ERR_NOTSUPPORTED;
				break;
			case CR_RETURN_CODE_NOTALLOWED :
				ret = NVM_ERR_DRIVERNOTALLOWED;
				break;
			case CR_RETURN_CODE_INVALIDPARAMETER :
				ret = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_BUFFER_OVERRUN :
				ret = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_BUFFER_UNDERRUN :
				ret = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_NOMEMORY :
				ret = NVM_ERR_NOMEMORY;
				break;
			case CR_RETURN_CODE_NAMESPACE_CANT_BE_MODIFIED :
				ret = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_NAMESPACE_CANT_BE_REMOVED :
				ret = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_LOCKED_DIMM :
				ret = NVM_ERR_BADSECURITYSTATE;
				break;
			case CR_RETURN_CODE_UNKNOWN :
				ret = NVM_ERR_DRIVERFAILED;
				break;
			default :
				ret = NVM_ERR_DRIVERFAILED;
		}
		COMMON_LOG_ERROR_F("nvm lib error = %d", ret);
	}

	COMMON_LOG_EXIT_RETURN_I(ret);
	return (ret);
}

int init_scsi_port()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	int i = 0;

	while (g_scsi_port < 0 && i < SCSI_PORT_MAX)
	{
		int temp_rc = NVM_ERR_UNKNOWN;

		if ((temp_rc = __get_driver_revision(i)) == NVM_SUCCESS)
		{
			g_scsi_port = i;
		}

		i++;
	}

	if (g_scsi_port < 0)
	{
		COMMON_LOG_ERROR("Unable to locate NVDIMM IOCTL Target SCSI Port");
		rc = NVM_ERR_DRIVERFAILED;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int open_ioctl_target(PHANDLE p_handle, short scsi_port)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;

	char ioctl_target[256];

	sprintf_s(ioctl_target, 256, "\\\\.\\Scsi%d:", scsi_port);

	*p_handle = CreateFile(ioctl_target,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (*p_handle == INVALID_HANDLE_VALUE)
	{
		COMMON_LOG_ERROR("Error: CreateFile Failed, Verify if driver installed.");
		rc = NVM_ERR_DRIVERFAILED;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int send_ioctl_command(HANDLE handle,
						unsigned long io_controlcode,
						void *p_in_buffer,
						size_t in_size,
						void *p_out_buffer,
						size_t out_size)
{
	COMMON_LOG_ENTRY();

	int rc = NVM_SUCCESS;
	unsigned char io_result;
	DWORD bytes_returned = 0;

	io_result = (unsigned char)DeviceIoControl(handle,
		io_controlcode,
		p_in_buffer,
		in_size,
		p_out_buffer,
		out_size,
		&bytes_returned,
		NULL);

	if (!io_result)
	{
		COMMON_LOG_ERROR_F("IOCTL send failed with Windows Error: %d", GetLastError());
		rc = NVM_ERR_DRIVERFAILED;
	}
	else if (out_size != bytes_returned)
	{
		COMMON_LOG_ERROR("IOCTL succeeded, but did not return enough data.");
		rc = NVM_ERR_DRIVERFAILED;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Fetch the dimm power limited count
 */
int get_dimm_power_limited_count()
{
	COMMON_LOG_ENTRY();
	int rc;

	// Windows expects an output payload that is exactly the size of count
	// If Count 0 is entered and the implict payload 1 is not removed bufferoverrun will
	// be returned as error.
	size_t bufSize = sizeof (GET_RAPL_IOCTL) - sizeof (GET_RAPL_OUTPUT_PAYLOAD);

	GET_RAPL_IOCTL ioctl_data;
	memset(&ioctl_data, 0, bufSize);

	// set count to 0 so we get the true count from the driver
	ioctl_data.InputPayload.RaplCount = 0;

	if ((rc = execute_ioctl(bufSize, &ioctl_data, IOCTL_CR_GET_RAPL)) == NVM_SUCCESS)
	{
		if ((ioctl_data.ReturnCode == CR_RETURN_CODE_BUFFER_OVERRUN) ||
				(ioctl_data.ReturnCode == CR_RETURN_CODE_BUFFER_UNDERRUN) ||
				ioctl_data.ReturnCode == CR_RETURN_CODE_SUCCESS)
		{
			// on overrun or underrun return, actual count was set by driver
			rc = ioctl_data.InputPayload.RaplCount;
		}
		else
		{
			// anything else is a failure
			COMMON_LOG_ERROR_F("unexpected driver error code (%u) getting dimm power limit count.",
				ioctl_data.ReturnCode);
			rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Determine if power is limited
 * Return error code or whether or not power is limited (1 = yes, 0 = no)
 */
int get_dimm_power_limited(NVM_UINT16 socket_id)
{
	COMMON_LOG_ENTRY();
	int rc;

	if ((rc = get_dimm_power_limited_count()) > socket_id)
	{
		int actual_count = rc;

		// Windows expects an output payload that is exactly the size of count
		size_t bufSize = sizeof (GET_RAPL_IOCTL) +
				(sizeof (GET_RAPL_OUTPUT_PAYLOAD) * (actual_count - 1));
		BYTE buffer[bufSize];
		GET_RAPL_IOCTL *p_ioctl_data = (GET_RAPL_IOCTL *)buffer;
		memset(buffer, 0, bufSize);

		// we have something we can work with
		p_ioctl_data->InputPayload.RaplCount = actual_count;

		if ((rc = execute_ioctl(bufSize, p_ioctl_data, IOCTL_CR_GET_RAPL)) == NVM_SUCCESS)
		{
			if (p_ioctl_data->ReturnCode != CR_RETURN_CODE_SUCCESS)
			{
				rc = ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode);
			}
			else
			{
				rc = (p_ioctl_data->OutputPayload.RaplData[socket_id].PowerLimitMsr &
						POWER_LIMIT_ENABLE_BIT) ? 1 : 0;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Sending the EXECUTE_DIAGNOSTIC_TEST ioctl without enough output payload will result
 * in the driver returning an error code, but will give the number of results to expect.
 */
int get_test_result_count(enum driver_diagnostic diagnostic)
{
	COMMON_LOG_ENTRY();
	int rc;
	size_t bufSize = sizeof (CR_RUN_DIAGNOSTIC_IOCTL);

	CR_RUN_DIAGNOSTIC_IOCTL ioctl_data;
	memset(&ioctl_data, 0, bufSize);

	ioctl_data.InputPayload.Test = convert_diagnostic_test(diagnostic);

	if ((rc = execute_ioctl(bufSize, &ioctl_data, IOCTL_CR_EXECUTE_DIAGNOSTIC_TEST)) == NVM_SUCCESS)
	{
		// expect buffer overrun because output payload isn't big enough. That's okay,
		// we just need the result data count.
		if (ioctl_data.ReturnCode != CR_RETURN_CODE_SUCCESS &&
				ioctl_data.ReturnCode != CR_RETURN_CODE_BUFFER_OVERRUN)
		{
			rc = ind_err_to_nvm_lib_err((CR_RETURN_CODES)ioctl_data.ReturnCode);
		}
		else
		{
			rc = (int)ioctl_data.OutputPayload.DataCount;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Execute a driver diagnostic
 */
int run_test(enum driver_diagnostic diagnostic, const NVM_UINT32 count,
		struct health_event results[])
{
	COMMON_LOG_ENTRY();
	int rc;
	// - 1 because CR_RUN_DIAGNOSTIC_IOCTL already has 1
	size_t bufSize = (sizeof (CR_RUN_DIAGNOSTIC_IOCTL) +
			(count - 1) * sizeof (HEALTH_EVENT));

	PCR_RUN_DIAGNOSTIC_IOCTL p_ioctl_data = malloc(bufSize);
	if (p_ioctl_data == NULL)
	{
		COMMON_LOG_ERROR("Unable to allocate memory for p_ioctl_data.");
		rc = NVM_ERR_NOMEMORY;
	}
	else
	{
		memset(p_ioctl_data, 0, bufSize);

		p_ioctl_data->InputPayload.Test = convert_diagnostic_test(diagnostic);

		if ((rc = execute_ioctl(bufSize, p_ioctl_data, IOCTL_CR_EXECUTE_DIAGNOSTIC_TEST))
			== NVM_SUCCESS)
		{
			if (p_ioctl_data->ReturnCode != 0)
			{
				rc = ind_err_to_nvm_lib_err((CR_RETURN_CODES)p_ioctl_data->ReturnCode);
			}
			else
			{
				for (int i = 0; i < p_ioctl_data->OutputPayload.DataCount; i++)
				{
					if (p_ioctl_data->OutputPayload.Data[i].HealthEventType ==
							NAMESPACE_HEALTH_EVENT_TYPE)
					{
						NAMESPACE_HEALTH_EVENT event =
								p_ioctl_data->OutputPayload.Data[i].Health.NamespaceHealthEvent;
						results[i].event_type = HEALTH_EVENT_TYPE_NAMESPACE;
						results[i].health.namespace_event.health_flag =
								convert_ns_health_status(event);
						win_guid_to_uid(event.NamespaceId,
										results[i].health.namespace_event.namespace_uid);
					}
					else if (p_ioctl_data->OutputPayload.Data[i].HealthEventType ==
							LABEL_AREA_HEALTH_EVENT_TYPE)
					{
						LABEL_AREA_HEALTH_EVENT event =
								p_ioctl_data->OutputPayload.Data[0].Health.LabelAreaHealthEvent;
						results[i].event_type = HEALTH_EVENT_TYPE_LABEL_AREA;
						results[i].health.label_area_event.device_handle =
								event.DeviceHandle.DeviceHandle;
						results[i].health.label_area_event.health_flag =
								convert_label_health_result(event);
					}
				}
			}
		}
		free(p_ioctl_data);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function for driver diagnostics
 */
enum ns_health_result convert_ns_health_status(NAMESPACE_HEALTH_EVENT event)
{
	enum ns_health_result result = (enum ns_health_result)0;
	switch (event.NamespaceHealthStatus)
	{

		case NAMESPACE_HEALTH_FLAGS_OK:
			result = NAMESPACE_HEALTH_RESULT_OK;
			break;
		case NAMESPACE_HEALTH_FLAGS_MISSING:
			result = NAMESPACE_HEALTH_RESULT_MISSING;
			break;
		case NAMESPACE_HEALTH_FLAGS_MISSING_LABEL:
			result = NAMESPACE_HEALTH_RESULT_MISSING_LABEL;
			break;
		case NAMESPACE_HEALTH_FLAGS_CORRUPT_INTERLEAVE_SET:
			result = NAMESPACE_HEALTH_RESULT_CORRUPT_INTERLEAVE_SET;
			break;
		case NAMESPACE_HEALTH_FLAGS_INCONSISTENT_LABELS:
			result = NAMESPACE_HEALTH_RESULT_INCONSISTENT_LABELS;
			break;
		case NAMESPACE_HEALTH_FLAGS_INVALID_BLOCK_SIZE:
			result = NAMESPACE_HEALTH_RESULT_INVALID_BLOCK_SIZE;
			break;
		case NAMESPACE_HEALTH_FLAGS_CORRUPT_BTT_METADATA:
			result = NAMESPACE_HEALTH_RESULT_CORRUPT_BTT_METADATA;
			break;
		default:
			COMMON_LOG_ERROR_F("%d is an unknown namespace health status",
			(int)event.NamespaceHealthStatus);
			break;
	}
	return result;
}

/*
 * Helper function for driver diagnostics
 */
DIAGNOSTIC_TEST convert_diagnostic_test(enum driver_diagnostic diagnostic)
{
	DIAGNOSTIC_TEST diagnostic_test = (DIAGNOSTIC_TEST)0;

	switch (diagnostic)
	{
		case DRIVER_DIAGNOSTIC_PM_METADATA_CHECK:
			diagnostic_test = EXECUTE_PM_METADATA_CHECKS;
			break;
		default:
			COMMON_LOG_ERROR_F("%d is an unknown diagnostic",
			(int)diagnostic);
			break;
	}
	return diagnostic_test;
}

enum label_area_health_result convert_label_health_result(LABEL_AREA_HEALTH_EVENT event)
{
	enum label_area_health_result result = (enum label_area_health_result)0;
	switch (event.LabelAreaHealthStatus)
	{
		case LABEL_AREA_HEALTH_FLAGS_OK:
			result = LABEL_AREA_HEALTH_RESULT_OK;
			break;
		case LABEL_AREA_HEALTH_FLAGS_MISSING_PCD:
			result = LABEL_AREA_HEALTH_RESULT_MISSING_PCD;
			break;
		case LABEL_AREA_HEALTH_FLAGS_MISSING_VALID_INDEX_BLOCK:
			result = LABEL_AREA_HEALTH_RESULT_MISSING_VALID_INDEX_BLOCK;
			break;
		case LABEL_AREA_HEALTH_FLAGS_INSUFFICIENT_PERSISTENT_MEMORY:
			result = LABEL_AREA_HEALTH_RESULT_INSUFFICIENT_PERSISTENT_MEMORY;
			break;
		case LABEL_AREA_HEALTH_FLAGS_LABELS_OVERLAP:
			result = LABEL_AREA_HEALTH_RESULT_LABELS_OVERLAP;
			break;
		default:
			COMMON_LOG_ERROR_F("%d is an unknown label area health status",
					event.LabelAreaHealthStatus);
			break;
	}
	return result;
}


int get_job_count()
{
	int rc = NVM_ERR_NOTSUPPORTED;
	return rc;
}

/*
 * Send an IOCTL payload to the driver
 */
int execute_ioctl(size_t bufSize, void *p_ioctl_data, unsigned long io_controlcode)
{
	COMMON_LOG_ENTRY();
	int rc;
	HANDLE handle;
	if ((rc = init_scsi_port()) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Unable to find valid SCSI Port");
	}
	else
	{
		if ((rc = open_ioctl_target(&handle, g_scsi_port)) == NVM_SUCCESS)
		{
			rc = send_ioctl_command(handle, io_controlcode, p_ioctl_data, bufSize, p_ioctl_data,
				bufSize);
			CloseHandle(handle);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void win_guid_to_uid(const GUID guid, COMMON_UID uid)
{
	COMMON_GUID tmp;
	memmove(tmp, &guid, COMMON_GUID_LEN);
	guid_to_uid(tmp, uid);
}

void win_uid_to_guid(const COMMON_UID uid, GUID *p_guid)
{
	COMMON_GUID tmp;
	str_to_guid(uid, tmp);
	memmove(p_guid, tmp, COMMON_GUID_LEN);
}
