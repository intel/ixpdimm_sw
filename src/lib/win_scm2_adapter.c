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
#include <string/s_str.h>
#include <acpi/nfit.h>

#include "device_fw.h"
#include "win_scm2_adapter.h"
#include "win_scm2_version_info.h"
#include "win_scm2_capabilities.h"
#include "win_scm2_ioctl.h"
#include "win_scm2_passthrough.h"
#include "win_scm2_ioctl_driver_version.h"


static enum return_code win_scm_to_nvm_err(int scm_rc);
static unsigned int win_scm2_get_first_handle();

/*
 * From the Crystal Ridge RS2+ SCM Based Windows Driver SAS v0.34:
 * SCM Based Driver should return the following GET_INTERFACE_VERSION_OUTPUT_PAYLOAD as follows
 * for initial RS2 support:
 * GET_INTERFACE_VERSION_OUTPUT_PAYLOAD.MajorVersion = IOCTL_INTERFACE_RS2_SCM_DRIVER_MAJOR_VERSION
 * GET_INTERFACE_VERSION_OUTPUT_PAYLOAD.MinorVersion = 0
 */
#define	IOCTL_INTERFACE_RS2_SCM_DRIVER_MAJOR_VERSION 2
#define	IOCTL_INTERFACE_MINOR_VERSION 0
NVM_BOOL win_scm_adp_is_supported_driver_available()
{
	COMMON_LOG_ENTRY();
	NVM_BOOL is_supported = 0;

	unsigned int handle = win_scm2_get_first_handle();
	GET_INTERFACE_VERSION_OUTPUT_PAYLOAD payload;

	if (WIN_SCM2_IS_SUCCESS(win_scm2_version_info(handle, &payload)))
	{
		if ((payload.MajorVersion == IOCTL_INTERFACE_RS2_SCM_DRIVER_MAJOR_VERSION) &&
				(payload.MinorVersion == IOCTL_INTERFACE_MINOR_VERSION))
		{
			is_supported = 1;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(is_supported);
	return is_supported;
}

int win_scm_adp_get_vendor_driver_revision(NVM_VERSION version_str, const NVM_SIZE str_len)
{
	enum return_code rc = NVM_SUCCESS;

	unsigned int handle = win_scm2_get_first_handle();
	char tmp[DRIVER_VERSION_LEN];
	int scm_rc = win_scm2_ioctl_driver_version(handle, tmp);
	if (WIN_SCM2_IOCTL_SUCCESS(scm_rc))
	{
		s_strcpy(version_str, tmp, str_len);
	}
	else
	{
		rc = win_scm_to_nvm_err(scm_rc);
	}

	return rc;
}

int win_scm_adp_get_driver_capabilities(struct nvm_driver_capabilities *p_caps)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	DRIVER_CAPABILITIES drv_caps;
	unsigned int handle = win_scm2_get_first_handle();
	int scm_rc = win_scm2_ioctl_get_driver_capabilities((unsigned short)handle, &drv_caps);
	if (!WIN_SCM2_IS_SUCCESS(scm_rc))
	{
		rc = win_scm_to_nvm_err(scm_rc);
	}
	else
	{
		memset(p_caps, 0, sizeof (struct nvm_driver_capabilities));

		p_caps->min_namespace_size = BYTES_PER_GIB;
		p_caps->num_block_sizes = 1;
		p_caps->block_sizes[0] = 1;
		p_caps->namespace_memory_page_allocation_capable = 0;
		p_caps->features.get_platform_capabilities = 1;
		p_caps->features.get_topology = drv_caps.SupportedFeatures.GetTopology;
		p_caps->features.get_interleave = 1;
		p_caps->features.get_dimm_detail = drv_caps.SupportedFeatures.GetDimmDetail;
		p_caps->features.get_namespaces = 1;
		p_caps->features.get_namespace_detail = 1;
		p_caps->features.get_boot_status = drv_caps.SupportedFeatures.SendPassthru;
		p_caps->features.get_power_data = drv_caps.SupportedFeatures.GetPowerData;
		p_caps->features.get_security_state = drv_caps.SupportedFeatures.SendPassthru;
		p_caps->features.get_log_page = drv_caps.SupportedFeatures.SendPassthru;
		p_caps->features.get_features = drv_caps.SupportedFeatures.SendPassthru;
		p_caps->features.set_features = drv_caps.SupportedFeatures.SendPassthru;
		p_caps->features.create_namespace = 0;
		p_caps->features.rename_namespace = 0;
		p_caps->features.delete_namespace = 0;
		p_caps->features.set_security_state = drv_caps.SupportedFeatures.SendPassthru;
		p_caps->features.enable_logging = drv_caps.SupportedFeatures.SendPassthru;
		p_caps->features.run_diagnostic = drv_caps.SupportedFeatures.RunDiagnostic;
		p_caps->features.passthrough = drv_caps.SupportedFeatures.SendPassthru;
		p_caps->features.app_direct_mode = 1;
		p_caps->features.storage_mode = 0;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int win_scm_adp_ioctl_passthrough_cmd(struct fw_cmd *p_cmd)
{
	unsigned int dsm_status = 0;
	int scm_rc = win_scm2_passthrough(p_cmd, &dsm_status);
	int rc = NVM_SUCCESS;

	if (!WIN_SCM2_IOCTL_SUCCESS(scm_rc))
	{
		rc = win_scm_to_nvm_err(scm_rc);
		COMMON_LOG_ERROR_F("IOCTL passthrough failed: Windows SCM driver "
			"returned error %d for command with opcode -"
				" 0x%x Subopcode - 0x%x", scm_rc, p_cmd->opcode, p_cmd->sub_opcode);
	}
	else if (dsm_status != 0)
	{
		rc = dsm_err_to_nvm_lib_err(dsm_status);
		COMMON_LOG_ERROR_F("IOCTL passthrough failed: DSM returned error %d "
				"for command with opcode - "
						"0x%x Subopcode - 0x%x", dsm_status, p_cmd->opcode, p_cmd->sub_opcode);
	}

	return rc;
}

static enum return_code win_scm_to_nvm_err(int scm_rc)
{
	enum return_code rc = NVM_SUCCESS;

	if (scm_rc < 0)
	{
		switch ((enum WIN_SCM2_RETURN_CODES)scm_rc)
		{
			case WIN_SCM2_SUCCESS:
				rc = NVM_SUCCESS;
				break;
			case WIN_SCM2_ERR_UNKNOWN:
				rc = NVM_ERR_DRIVERFAILED;
				break;
			case WIN_SCM2_ERR_DRIVERFAILED:
				rc = NVM_ERR_DRIVERFAILED;
				break;
			case WIN_SCM2_ERR_NOMEMORY:
				rc = NVM_ERR_NOMEMORY;
				break;
		}
	}
	else
	{
		switch ((CR_RETURN_CODES)scm_rc)
		{

			case CR_RETURN_CODE_SUCCESS:
				rc = NVM_SUCCESS;
				break;
			case CR_RETURN_CODE_NOTSUPPORTED:
				rc = NVM_ERR_NOTSUPPORTED;
				break;
			case CR_RETURN_CODE_NOTALLOWED:
				rc = NVM_ERR_DRIVERNOTALLOWED;
				break;
			case CR_RETURN_CODE_INVALIDPARAMETER:
				rc = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_BUFFER_OVERRUN:
				rc = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_BUFFER_UNDERRUN:
				rc = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_NOMEMORY:
				rc = NVM_ERR_NOMEMORY;
				break;
			case CR_RETURN_CODE_NAMESPACE_CANT_BE_MODIFIED:
				rc = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_NAMESPACE_CANT_BE_REMOVED:
				rc = NVM_ERR_DRIVERFAILED;
				break;
			case CR_RETURN_CODE_LOCKED_DIMM:
				rc = NVM_ERR_BADSECURITYSTATE;
				break;
			case CR_RETURN_CODE_UNKNOWN:
				rc = NVM_ERR_DRIVERFAILED;
				break;
			default:
				rc = NVM_ERR_DRIVERFAILED;
		}
	}

	return (rc);
}

static unsigned int win_scm2_get_first_handle()
{
	unsigned int handle = 0;
	int dimm_count = nfit_get_dimms(0, NULL);
	if (dimm_count > 0)
	{
		// TODO-RYON(6/26/2017): Need to look through all NFIT DIMMs and return handle of first
		// Intel DIMM (TA44191)
		struct nfit_dimm dimms[dimm_count];
		if (nfit_get_dimms(dimm_count, dimms) > 0)
		{
			handle = dimms[0].handle;
		}
	}
	return handle;
}
