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
		p_host->sku_violation = capabilities.sku_capabilities.sku_violation;
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
		memset(p_host, 0, sizeof (struct host));
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

// Helper function for get_mapped_memory_info and get_mapped_memory_info_db
int get_socket_sku(struct bios_capabilities *p_pcat, struct socket *p_socket)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	// iterate over all the extension tables
	NVM_UINT32 offset = PCAT_TABLE_SIZE; // Size occupied by ACPI Table Header
	while (offset < p_pcat->header.length)
	{
		struct pcat_extension_table_header *p_header =
				(struct pcat_extension_table_header *)((NVM_UINT8 *)p_pcat + offset);

		// check the length for validity
		if (p_header->length == 0 || (p_header->length + offset) > p_pcat->header.length)
		{
			COMMON_LOG_ERROR_F("Extension table length %d invalid",	p_header->length);
			rc = NVM_ERR_BADPCAT;
			break;
		}

		// socket SKU info table
		if (p_header->type == PCAT_TABLE_SOCKET_INFO)
		{
			struct socket_information_ext_table *p_socket_info =
					(struct socket_information_ext_table *)p_header;

			if (p_socket_info->node_id == p_socket->id)
			{
				if (!p_socket_info->mapped_memory_limit)
				{
					COMMON_LOG_ERROR("Mapped memory limit is not defined");
				}
				else if (p_socket_info->total_mapped_memory >= p_socket_info->mapped_memory_limit)
				{
					COMMON_LOG_ERROR("Occupied mapped memory is greater than limit");
				}

				p_socket->mapped_memory_limit = p_socket_info->mapped_memory_limit;
				p_socket->total_mapped_memory = p_socket_info->total_mapped_memory;
				break;
			}
		}

		// Loop through all PCAT tables
		offset += p_header->length;
	}

	return rc;
}

// NOTE: This function is called in <OS>_system.c after populating the list of available sockets.
int get_mapped_memory_info(struct socket *p_socket)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (NULL == p_socket)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_socket is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		struct bios_capabilities *p_pcat = calloc(1, sizeof (struct bios_capabilities));
		// retrieve from ACPI table

		if (NVM_SUCCESS != (rc = get_pcat(p_pcat)))
		{
			COMMON_LOG_ERROR("Retrieving PCAT from ACPI table failed.");
		}
		else
		{
			rc = get_socket_sku(p_pcat, p_socket);
		}

		free(p_pcat);
	}

	return rc;
}
