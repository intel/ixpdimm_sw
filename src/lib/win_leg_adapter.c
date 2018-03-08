/*
 * Copyright (c) 2017, Intel Corporation
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

#include <common/persistence/logging.h>
#include <string/revision.h>

#include "win_leg_adapter.h"
#include "win_leg_adapter_shared.h"

#include <windows.h>
#include <DiagnosticExport.h>
#include <Diagnostic.h>
#include <GetVendorDriverRevision.h>

#define	WIN_DRIVER_VERSION_MAJOR_MIN	1
#define	WIN_DRIVER_VERSION_MAJOR_MAX	1

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

DIAGNOSTIC_TEST convert_diagnostic_test(enum driver_diagnostic diagnostic)
{
	DIAGNOSTIC_TEST diagnostic_test = (DIAGNOSTIC_TEST)0;

	switch (diagnostic)
	{
		case DRIVER_DIAGNOSTIC_PM_METADATA_CHECK:
			diagnostic_test = EXECUTE_PM_METADATA_CHECKS;
			break;
		default:
			COMMON_LOG_ERROR_F("%d is an unknown diagnostic", (int)diagnostic);
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

NVM_BOOL win_leg_adp_is_supported_driver_available()
{
	COMMON_LOG_ENTRY();
	NVM_BOOL is_supported = 0;

	NVM_VERSION driver_rev;
	int rc = win_leg_adp_get_vendor_driver_revision(driver_rev, NVM_VERSION_LEN);
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

int win_leg_adp_get_vendor_driver_revision(NVM_VERSION version_str, const NVM_SIZE str_len)
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
			rc = NVM_ERR_BADDRIVER;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int win_leg_adp_get_driver_capabilities(struct nvm_driver_capabilities *p_capabilities)
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
			for (NVM_UINT32 i = 0; (i < num_block_sizes) && (i < MAX_NUMBER_OF_BLOCK_SIZES); i++)
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

int win_leg_adp_get_test_result_count(enum driver_diagnostic diagnostic)
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

int win_leg_adp_run_test(enum driver_diagnostic diagnostic, const NVM_UINT32 count,
	struct health_event *results)
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
