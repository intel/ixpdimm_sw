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
 * This file contains the implementation of system management functions of the Native API.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string/s_str.h>

#include "nvm_management.h"
#include <persistence/logging.h>
#include "device_adapter.h"
#include <persistence/lib_persistence.h>
#include "system.h"
#include "device_utilities.h"
#include "capabilities.h"
#include "system.h"

int fill_host_sku_status(struct host *p_host)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// set the mixed sku field
	struct nvm_capabilities capabilities;
	rc = nvm_get_nvm_capabilities(&capabilities);
	if (rc == NVM_SUCCESS)
	{
		p_host->mixed_sku = capabilities.sku_capabilities.mixed_sku;
		rc = system_in_sku_violation(&capabilities, &p_host->sku_violation);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve the host server name
 */
int nvm_get_host_name(char *host_name, const NVM_SIZE host_name_len)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (host_name == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, host_name is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (host_name_len == 0)
	{
		COMMON_LOG_ERROR("Invalid parameter, host_name_len is 0");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		struct host host;
		memset(&host, 0, sizeof (host));
		// call the system adapter
		if ((rc = get_host(&host)) == NVM_SUCCESS)
		{
			s_strncpy(host_name, host_name_len, host.name, NVM_COMPUTERNAME_LEN);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieve basic information about the host server the native API library is running on.
 */
int nvm_get_host(struct host *p_host)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_host == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_host is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		// call the system adapter
		rc = get_host(p_host);
		KEEP_ERROR(rc, fill_host_sku_status(p_host));
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Harvest the driver version and compatibility into sw_inventory
 */
void get_driver_version_info(struct sw_inventory *p_inventory)
{
	COMMON_LOG_ENTRY();

	int rc = get_vendor_driver_revision(p_inventory->vendor_driver_revision,
			NVM_VERSION_LEN);
	if (rc == NVM_SUCCESS)
	{
		// Got the driver version - can we talk to it?
		p_inventory->vendor_driver_compatible = is_supported_driver_available();
		if (!p_inventory->vendor_driver_compatible)
		{
			COMMON_LOG_ERROR_F("Driver version %s is not supported",
					p_inventory->vendor_driver_revision);
		}
	}
	else
	{
		COMMON_LOG_ERROR("Couldn't get driver version");
		s_strcpy(p_inventory->vendor_driver_revision, "", NVM_VERSION_LEN);
		p_inventory->vendor_driver_compatible = 0;
	}

	COMMON_LOG_EXIT();
}

/*
 * Retrieves a list of installed software versions related to NVM DIMM management.
 */
int nvm_get_sw_inventory(struct sw_inventory *p_inventory)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_inventory == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_inventory is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		memset(p_inventory, 0, sizeof (struct sw_inventory));

		// if we fail to get the driver info, it's just not compatible
		get_driver_version_info(p_inventory);

		// management software version - not likely to fail
		rc = nvm_get_version(p_inventory->mgmt_sw_revision, NVM_VERSION_LEN);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieves the number of NUMA nodes in the system, which is is equivalent
 * to the number of physical processors (or sockets).
 */
int nvm_get_socket_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		rc = get_socket_count();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieves NUMA information about each physical processor in the system
 */
int nvm_get_sockets(struct socket *p_node, const NVM_UINT16 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_node == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_node is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		rc = get_sockets(p_node, count);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Retrieves NUMA information about a given NUMA node number
 */
int nvm_get_socket(const NVM_UINT16 node_id, struct socket *p_node)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_node == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_node is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		rc = get_socket(node_id, p_node);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
