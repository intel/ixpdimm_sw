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
#include <windows.h>
#include "win_adapter.h"

#include <stdio.h>

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
//enum label_area_health_result convert_label_health_result(LABEL_AREA_HEALTH_EVENT event);
//enum ns_health_result convert_ns_health_status(NAMESPACE_HEALTH_EVENT event);
//DIAGNOSTIC_TEST convert_diagnostic_test(enum driver_diagnostic diagnostic);

/*
 * Support is determined by driver version
 */

int ind_err_to_nvm_lib_err(CR_RETURN_CODES ind_err)
{
	return (ind_err);
}

static int __get_driver_revision(short scsi_port)
{
	int rc = 0;

	HANDLE handle = INVALID_HANDLE_VALUE;

	// Open the Drivers IOCTL File Handle
	if ((rc = open_ioctl_target(&handle, scsi_port)) == 0)
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
			sizeof (CR_GET_DRIVER_REVISION_IOCTL))) == 0)
		{
			rc = ind_err_to_nvm_lib_err(payload_out.ReturnCode);
		}

		CloseHandle(handle);
	}


		return rc;
}

int init_scsi_port()
{
		int rc = 0;
	int i = 0;

	while (g_scsi_port < 0 && i < SCSI_PORT_MAX)
	{
		int temp_rc = -1;

		if ((temp_rc = __get_driver_revision(i)) == 0)
		{
			g_scsi_port = i;
		}

		i++;
	}

	if (g_scsi_port < 0)
	{
				rc = -1;
	}

		return rc;
}



int open_ioctl_target(PHANDLE p_handle, short scsi_port)
{
	
	int rc = 0;

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
				rc = -1;
	}

		return rc;
}

int send_ioctl_command(HANDLE handle,
						unsigned long io_controlcode,
						void *p_in_buffer,
						size_t in_size,
						void *p_out_buffer,
						size_t out_size)
{
	
	int rc = 0;
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
				rc = -1;
	}
	else if (out_size != bytes_returned)
	{
				rc = -1;
	}

		return rc;
}

/*
 * Send an IOCTL payload to the driver
 */
int execute_ioctl(size_t bufSize, void *p_ioctl_data, unsigned long io_controlcode)
{
		int rc;
	HANDLE handle;
	if ((rc = init_scsi_port()) != 0)
	{
			}
	else
	{
		if ((rc = open_ioctl_target(&handle, g_scsi_port)) == 0)
		{
			rc = send_ioctl_command(handle, io_controlcode, p_ioctl_data, bufSize, p_ioctl_data,
				bufSize);
			CloseHandle(handle);
		}
	}

		return rc;
}