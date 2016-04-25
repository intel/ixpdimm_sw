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
 * This file implements the Windows driver adapter interface for managing namespaces.
 */

#include "device_adapter.h"
#include <string/s_str.h>
#include <initguid.h>
#include "win_adapter.h"
#include "utility.h"

/*
 * Get the number of existing namespaces
 */
int get_namespace_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	GET_NAMESPACES_IOCTL ioctl_data;

	// Windows expects an output payload that is exactly the size of count
	// If Count 0 is entered and the implict payload 1 is not removed bufferoverrun will
	// be returned as error.
	NVM_SIZE buf_size = sizeof (GET_NAMESPACES_IOCTL)
		- sizeof (GET_NAMESPACES_OUTPUT_PAYLOAD);
	memset(&ioctl_data, 0, buf_size);

	ioctl_data.InputPayload.NamespaceCount = 0;

	if ((rc = execute_ioctl(buf_size, &ioctl_data, IOCTL_CR_GET_NAMESPACES)) == NVM_SUCCESS)
	{
		// For windows driver if the buffer provided is less than the
		// amount of space the driver would expect underrun is returned
		// and the count variable is set to the expected number of elements
		if (ioctl_data.ReturnCode == CR_RETURN_CODE_BUFFER_UNDERRUN ||
				ioctl_data.ReturnCode == CR_RETURN_CODE_BUFFER_OVERRUN ||
				ioctl_data.ReturnCode == CR_RETURN_CODE_SUCCESS)
		{
			rc = (int)ioctl_data.InputPayload.NamespaceCount;
		}
		else
		{
			rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the discovery information for a given number of namespaces
 */
int get_namespaces(const NVM_UINT32 count,
		struct nvm_namespace_discovery *p_namespaces)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_namespaces == NULL)
	{
		COMMON_LOG_ERROR("p_namespaces is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else if ((rc = get_namespace_count()) > 0) // there are items to fetch
	{
		memset(p_namespaces, 0, sizeof (struct nvm_namespace_discovery) * count);
		int actual_count = rc;

		// Subtracting 1 is required since the GET_NAMESPACES_IOCTL structure implicitly
		// contains 1 NVM_NAMESPACE_DISCOVERY structure.
		NVM_SIZE buf_size = sizeof (GET_NAMESPACES_IOCTL) +
				(actual_count - 1) * sizeof (NVM_NAMESPACE_DISCOVERY);
		GET_NAMESPACES_IOCTL *p_ioctl_data = calloc(1, buf_size);
		if (p_ioctl_data)
		{
			p_ioctl_data->InputPayload.NamespaceCount = actual_count;

			if ((rc = execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_GET_NAMESPACES))
				== NVM_SUCCESS &&
				(rc = ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == NVM_SUCCESS)
			{
				int return_count = 0;
				if (count < actual_count)
				{
					return_count = count;
					COMMON_LOG_ERROR("array too small to hold all namespaces");
					rc = NVM_ERR_ARRAYTOOSMALL;
				}
				else
				{
					return_count = actual_count;
					rc = actual_count;
				}

				for (int i = 0; i < return_count; i++)
				{
					win_guid_to_uid(p_ioctl_data->OutputPayload.NamespaceList[i].NamespaceUuid,
						p_namespaces[i].namespace_uid);
					s_strcpy(p_namespaces[i].friendly_name,
						p_ioctl_data->OutputPayload.NamespaceList[i].FriendlyName,
						NVM_NAMESPACE_NAME_LEN);
				}
			}

			free(p_ioctl_data);
		}
		else
		{
			COMMON_LOG_ERROR("failed to dynamically allocate payload");
			rc = NVM_ERR_NOMEMORY;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Get the details for a specific namespace
 */
int get_namespace_details(
		const NVM_UID namespace_uid,
		struct nvm_namespace_details *p_details)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("namespace uid cannot be NULL.");
		rc = NVM_ERR_UNKNOWN;
	}
	else if (p_details == NULL)
	{
		COMMON_LOG_ERROR("nvm_namespace_details is NULL");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		memset(p_details, 0, sizeof (struct nvm_namespace_details));

		GET_NAMESPACE_DETAILS_IOCTL ioctl_data;
		memset(&ioctl_data, 0, sizeof (ioctl_data));

		win_uid_to_guid(namespace_uid, &(ioctl_data.InputPayload.NamespaceUuid));

		if ((rc = execute_ioctl(sizeof (ioctl_data), &ioctl_data, IOCTL_CR_GET_NAMESPACE_DETAILS))
			== NVM_SUCCESS && (rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode)) == NVM_SUCCESS)
		{
			// Windows Namespaces are always enabled
			p_details->enabled = NAMESPACE_ENABLE_STATE_ENABLED;

			s_strcpy(p_details->discovery.friendly_name,
				ioctl_data.OutputPayload.FriendlyName,
				NVM_NAMESPACE_NAME_LEN);

			uid_copy(namespace_uid, p_details->discovery.namespace_uid);

			if (ioctl_data.OutputPayload.NamespaceAttributes & NAMESPACE_ATTRIB_BTT)
			{
				p_details->btt = TRUE;
			}

			if (ioctl_data.OutputPayload.NamespaceAttributes & NAMESPACE_ATTRIB_LOCAL)
			{
				p_details->type = NAMESPACE_TYPE_STORAGE;
				p_details->block_size = ioctl_data.OutputPayload.Ns.BlockNamespace.LogicalBlockSize;
				p_details->namespace_creation_id.device_handle.handle =
					ioctl_data.OutputPayload.Ns.BlockNamespace.DeviceHandle.DeviceHandle;
				p_details->block_count = ioctl_data.OutputPayload.Ns.BlockNamespace
						.LogicalBlockCount;
			}
			else
			{
				p_details->type = NAMESPACE_TYPE_APP_DIRECT;
				p_details->block_size = 1;
				p_details->namespace_creation_id.interleave_setid =
					ioctl_data.OutputPayload.Ns.PmemNamespace.InterleaveSetId;
				p_details->block_count = ioctl_data.OutputPayload.Ns.PmemNamespace.ByteCount;
			}

			switch (ioctl_data.OutputPayload.Health)
			{
			case NAMESPACE_HEALTH_FLAGS_OK:
				p_details->health = NAMESPACE_HEALTH_NORMAL;
				break;
			case NAMESPACE_HEALTH_FLAGS_INCONSISTENT_LABELS:
			case NAMESPACE_HEALTH_FLAGS_MISSING:
			case NAMESPACE_HEALTH_FLAGS_INVALID_BLOCK_SIZE:
			case NAMESPACE_HEALTH_FLAGS_MISSING_LABEL:
			case NAMESPACE_HEALTH_FLAGS_CORRUPT_INTERLEAVE_SET:
			case NAMESPACE_HEALTH_FLAGS_CORRUPT_BTT_METADATA:
				p_details->health = NAMESPACE_HEALTH_CRITICAL;
				break;
			default:
				p_details->health = NAMESPACE_HEALTH_UNKNOWN;
				break;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Create a new namespace
 */
int create_namespace(
		NVM_UID *p_namespace_uid,
		const struct nvm_namespace_create_settings *p_settings)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if (p_settings == NULL)
	{
		COMMON_LOG_ERROR("namespace create settings structure cannot be NULL.");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("namespace uid pointer cannot be NULL.");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		CREATE_NAMESPACE_IOCTL ioctl_data;
		memset(&ioctl_data, 0, sizeof (ioctl_data));

		memmove(ioctl_data.InputPayload.FriendlyName, p_settings->friendly_name, NSLABEL_NAME_LEN);

		ioctl_data.InputPayload.RawNamespaceSize =
			adjust_namespace_size(p_settings->block_size, p_settings->block_count);

		if (p_settings->type == NAMESPACE_TYPE_STORAGE)
		{
			ioctl_data.InputPayload.Ns.BlockNamespace.DeviceHandle.DeviceHandle =
				p_settings->namespace_creation_id.device_handle.handle;
			ioctl_data.InputPayload.NamespaceAttributes |= NAMESPACE_ATTRIB_LOCAL;
			ioctl_data.InputPayload.Ns.BlockNamespace.LogicalBlockSize = p_settings->block_size;
		}
		if (p_settings->type == NAMESPACE_TYPE_APP_DIRECT)
		{
			ioctl_data.InputPayload.Ns.PmemNamespace.InterleaveSetId =
				p_settings->namespace_creation_id.interleave_setid;
		}

		if (p_settings->btt)
		{
			ioctl_data.InputPayload.NamespaceAttributes |= NAMESPACE_ATTRIB_BTT;
		}

		if ((rc = execute_ioctl(sizeof (ioctl_data), &ioctl_data, IOCTL_CR_CREATE_NAMESPACE))
			== NVM_SUCCESS && (rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode)) == NVM_SUCCESS)
		{
			win_guid_to_uid(ioctl_data.OutputPayload.NamespaceUuid, *p_namespace_uid);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Delete an existing namespace
 */
int delete_namespace(const NVM_UID namespace_uid)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (namespace_uid == NULL)
	{
			COMMON_LOG_ERROR("namespace uid cannot be NULL.");
			rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		DELETE_NAMESPACE_IOCTL ioctl_data;
		memset(&ioctl_data, 0, sizeof (ioctl_data));

		win_uid_to_guid(namespace_uid, &(ioctl_data.InputPayload.NamespaceUuid));

		if ((rc = execute_ioctl(sizeof (ioctl_data), &ioctl_data, IOCTL_CR_DELETE_NAMESPACE))
			== NVM_SUCCESS)
		{
			rc = ind_err_to_nvm_lib_err(ioctl_data.ReturnCode);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Modify an existing namespace name
 */
int modify_namespace_name(
		const NVM_UID namespace_uid,
		const NVM_NAMESPACE_NAME name)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	if (name == NULL)
	{
		COMMON_LOG_ERROR("namespace modify settings structure cannot be NULL.");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (namespace_uid == NULL)
	{
		COMMON_LOG_ERROR("namespace UID pointer cannot be NULL.");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = init_scsi_port()) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Unable to find valid SCSI Port");
	}
	else
	{
		HANDLE handle = INVALID_HANDLE_VALUE;
		if ((rc = open_ioctl_target(&handle, g_scsi_port)) == NVM_SUCCESS)
		{
			RENAME_NAMESPACE_IOCTL payload;
			memset(&payload, 0, sizeof (payload));
			win_uid_to_guid(namespace_uid, &(payload.InputPayload.NamespaceUuid));
			memmove(payload.InputPayload.FriendlyName, name, NSLABEL_NAME_LEN);

			// Verify IOCTL is sent successfully and that the driver had no errors
			if ((rc = send_ioctl_command(handle, IOCTL_CR_RENAME_NAMESPACE, &payload,
					sizeof (RENAME_NAMESPACE_IOCTL), &payload,
					sizeof (RENAME_NAMESPACE_IOCTL))) == NVM_SUCCESS)
			{
				rc = ind_err_to_nvm_lib_err(payload.ReturnCode);
			}

			CloseHandle(handle);
		}
	}

	return rc;
}

/*
 * Modify an existing namespace size
 */
int modify_namespace_block_count(
		const NVM_UID namespace_uid,
		const NVM_UINT64 block_count)
{
	int rc = NVM_ERR_NOTSUPPORTED;
	return rc;
}

/*
 * Modify an existing namespace enable
 */
int modify_namespace_enabled(
		const NVM_UID namespace_uid,
		const enum namespace_enable_state enabled)
{
	int rc = NVM_ERR_NOTSUPPORTED;
	return rc;
}
