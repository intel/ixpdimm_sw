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
 */

#include "device_adapter.h"
#include "utility.h"
#include <windows.h>
#include "win_leg_adapter_shared.h"

#define	SCSI_PORT_MAX 32

short g_scsi_port = -1;

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

	*p_handle = CreateFile(ioctl_target, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (*p_handle == INVALID_HANDLE_VALUE)
	{
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
 * Send an IOCTL payload to the driver
 */
int execute_ioctl(size_t bufSize, void *p_ioctl_data, unsigned long io_controlcode)
{
	COMMON_LOG_ENTRY();
	int rc;
	HANDLE handle;
	if ((rc = init_scsi_port()) != NVM_SUCCESS)
	{
		// COMMON_LOG_INFO("Unable to find valid SCSI Port");
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
