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
 * This file contains the implementation of Windows device adapter interface for general operations.
 * Driver specifics are in the legacy or scm2 specific adapter files.
 */

#include <common/persistence/logging.h>
#include "nvm_types.h"
#include "nfit_utilities.h"
#include "smbios_utilities.h"
#include "system.h"
#include "nfit_utilities.h"
#include "win_leg_adapter.h"
#include "win_scm2_adapter.h"

enum DRIVER_TYPE
{
	DRIVER_TYPE_UNKNOWN = 0,
	DRIVER_TYPE_LEGACY = 1,
	DRIVER_TYPE_SCM2 = 2
};

enum DRIVER_TYPE get_driver_type()
{
	enum DRIVER_TYPE result = DRIVER_TYPE_UNKNOWN;

	if (win_scm_adp_is_supported_driver_available())
	{
		result = DRIVER_TYPE_SCM2;
	}
	else if (win_leg_adp_is_supported_driver_available())
	{
		result = DRIVER_TYPE_LEGACY;
	}

	return result;
}

NVM_BOOL is_supported_driver_available()
{
	NVM_BOOL result = (NVM_BOOL) (win_leg_adp_is_supported_driver_available() ||
			win_scm_adp_is_supported_driver_available());
	return result;
}

int get_vendor_driver_revision(NVM_VERSION version_str, const NVM_SIZE str_len)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_get_vendor_driver_revision(version_str, str_len);
			break;
		case DRIVER_TYPE_SCM2:
			rc = win_scm_adp_get_vendor_driver_revision(version_str, str_len);
			break;
	}

	return rc;
}

int get_platform_capabilities(struct bios_capabilities *p_capabilities)
{
	return get_pcat(p_capabilities);
}

int get_topology_count()
{
	return get_topology_count_from_nfit();
}

int get_topology(const NVM_UINT8 count, struct nvm_topology *p_dimm_topo)
{
	return get_topology_from_nfit(count, p_dimm_topo);
}

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
			NVM_UINT16 physical_id = (NVM_UINT16) rc;
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

int get_interleave_set_count()
{
	COMMON_LOG_ENTRY();
	int rc = get_interleave_set_count_from_nfit();
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_interleave_sets(const NVM_UINT32 count, struct nvm_interleave_set *p_interleaves)
{
	COMMON_LOG_ENTRY();
	int rc = get_interleave_sets_from_nfit(count, p_interleaves);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_namespace_count()
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_get_namespace_count();
			break;
		case DRIVER_TYPE_SCM2:
			rc = win_scm2_get_namespace_count();
			break;
	}

	return rc;
}

int get_namespaces(const NVM_UINT32 count,
		struct nvm_namespace_discovery *p_namespaces)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_get_namespaces(count, p_namespaces);
			break;
		case DRIVER_TYPE_SCM2:
			rc = win_scm2_get_namespaces(count, p_namespaces);
			break;
	}

	return rc;
}

int get_namespace_details(
		const NVM_UID namespace_uid,
		struct nvm_namespace_details *p_details)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_get_namespace_details(namespace_uid, p_details);
			break;
		case DRIVER_TYPE_SCM2:
			rc = win_scm2_get_namespace_details(namespace_uid, p_details);
			break;
	}

	return rc;
}

int create_namespace(
		NVM_UID *p_namespace_uid,
		const struct nvm_namespace_create_settings *p_settings)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_create_namespace(p_namespace_uid, p_settings);
			break;
		case DRIVER_TYPE_SCM2:
			COMMON_LOG_WARN_F("%s() not supported", __FUNCTION__);
			rc = NVM_ERR_NOTSUPPORTED;
			break;
	}

	return rc;
}

int delete_namespace(const NVM_UID namespace_uid)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_delete_namespace(namespace_uid);
			break;
		case DRIVER_TYPE_SCM2:
			COMMON_LOG_WARN_F("%s() not supported", __FUNCTION__);
			rc = NVM_ERR_NOTSUPPORTED;
			break;
	}

	return rc;
}

int modify_namespace_name(
		const NVM_UID namespace_uid,
		const NVM_NAMESPACE_NAME name)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_modify_namespace_name(namespace_uid, name);
			break;
		case DRIVER_TYPE_SCM2:
			COMMON_LOG_WARN_F("%s() not supported", __FUNCTION__);
			rc = NVM_ERR_NOTSUPPORTED;
			break;
	}

	return rc;
}

int modify_namespace_block_count(
		const NVM_UID namespace_uid,
		const NVM_UINT64 block_count)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_modify_namespace_block_count(namespace_uid, block_count);
			break;
		case DRIVER_TYPE_SCM2:
			COMMON_LOG_WARN_F("%s() not supported", __FUNCTION__);
			rc = NVM_ERR_NOTSUPPORTED;
			break;
	}

	return rc;
}

int modify_namespace_enabled(const NVM_UID namespace_uid,
		const enum namespace_enable_state enabled)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_modify_namespace_enabled(namespace_uid, enabled);
			break;
		case DRIVER_TYPE_SCM2:
			COMMON_LOG_WARN_F("%s() not supported", __FUNCTION__);
			rc = NVM_ERR_NOTSUPPORTED;
			break;
	}

	return rc;
}

int get_driver_capabilities(struct nvm_driver_capabilities *p_capabilities)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_get_driver_capabilities(p_capabilities);
			break;
		case DRIVER_TYPE_SCM2:
			rc = win_scm_adp_get_driver_capabilities(p_capabilities);
			break;
	}

	return rc;
}

int ioctl_passthrough_cmd(struct fw_cmd *p_cmd)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_ioctl_passthrough_cmd(p_cmd);
			break;
		case DRIVER_TYPE_SCM2:
			rc = win_scm_adp_ioctl_passthrough_cmd(p_cmd);
			break;
	}

	return rc;
}

int get_dimm_power_limited(NVM_UINT16 socket_id)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_get_dimm_power_limited(socket_id);
			break;
		case DRIVER_TYPE_SCM2:
			rc = win_scm2_adp_get_dimm_power_limited(socket_id);
			break;
	}

	return rc;
}

int get_job_count()
{
	int rc = NVM_ERR_NOTSUPPORTED;
	return rc;
}

int get_test_result_count(enum driver_diagnostic diagnostic)
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_get_test_result_count(diagnostic);
			break;
		case DRIVER_TYPE_SCM2:
			COMMON_LOG_WARN_F("%s() not supported", __FUNCTION__);
			rc = NVM_ERR_NOTSUPPORTED;
			break;
	}

	return rc;
}

int run_test(enum driver_diagnostic diagnostic, const NVM_UINT32 count,
		struct health_event results[])
{
	int rc = NVM_ERR_UNKNOWN;
	switch (get_driver_type())
	{
		case DRIVER_TYPE_UNKNOWN:
			rc = NVM_ERR_BADDRIVER;
			break;
		case DRIVER_TYPE_LEGACY:
			rc = win_leg_adp_run_test(diagnostic, count, results);
			break;
		case DRIVER_TYPE_SCM2:
			COMMON_LOG_WARN_F("%s() not supported", __FUNCTION__);
			rc = NVM_ERR_NOTSUPPORTED;
			break;
	}

	return rc;
}

int reenumerate_namespaces(NVM_NFIT_DEVICE_HANDLE device_handle)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_NOTSUPPORTED;

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Not implemented - needs implementation of namespace functions in windows driver
 */
int init_label_dimm(const NVM_UINT32 device_handle, NVM_UINT16 major_version, NVM_UINT16  minor_version)
{
        COMMON_LOG_ENTRY();
        int rc = NVM_ERR_NOTSUPPORTED;

        COMMON_LOG_EXIT_RETURN_I(rc);
        return rc;
}
