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

#include <stdio.h>
#include <windows.h>
#include "win_scm2_ioctl.h"
#include "win_scm2_adapter.h"

static enum WIN_SCM2_IOCTL_RETURN_CODES scm2_open_ioctl_target(PHANDLE p_handle,
		unsigned short dimm_handle)
{
	enum WIN_SCM2_IOCTL_RETURN_CODES rc = WIN_SCM2_IOCTL_RC_SUCCESS;

	char ioctl_target[256];
	//	\\.\PhysicalNvdimm{NFIT handle in hex}
	sprintf_s(ioctl_target, 256, "\\\\.\\PhysicalNvdimm%x", dimm_handle);

	*p_handle = CreateFile(ioctl_target,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL);

	if (*p_handle == INVALID_HANDLE_VALUE)
	{
		SCM_LOG_ERROR_F("CreateFile Failed, Verify if driver installed. Error: %d",
				(int)GetLastError());
		rc = WIN_SCM2_IOCTL_RC_ERR;
	}
	return rc;
}

static enum WIN_SCM2_IOCTL_RETURN_CODES scm2_send_ioctl_command(HANDLE handle,
		unsigned long io_controlcode, void *p_in_buffer,
		size_t in_size, void *p_out_buffer, size_t out_size)
{
	SCM_LOG_ENTRY();

	enum WIN_SCM2_IOCTL_RETURN_CODES rc = WIN_SCM2_IOCTL_RC_SUCCESS;
	WINBOOL io_result;
	DWORD bytes_returned = 0;

	// DeviceIoControl documentation:
	// (msdn.microsoft.com/en-us/library/windows/desktop/aa363216(v=vs.85).aspx)
	// If the operation completes successfully, the return value is nonzero.
	// If the operation fails or is pending, the return value is zero.
	// To get extended error information, call GetLastError.
	io_result = (unsigned char)DeviceIoControl(handle, io_controlcode, p_in_buffer, in_size,
			p_out_buffer, out_size, &bytes_returned, NULL);

	if (!io_result)
	{
		SCM_LOG_ERROR_F("IOCTL send failed with Windows Error: %d", GetLastError());
		rc = WIN_SCM2_IOCTL_RC_ERR;
	}
	else if (out_size != bytes_returned)
	{
		SCM_LOG_ERROR("IOCTL succeeded, but did not return enough data.");
		rc = WIN_SCM2_IOCTL_RC_ERR;
	}

	SCM_LOG_EXIT_RETURN_I(rc);
	return rc;
}

enum WIN_SCM2_IOCTL_RETURN_CODES win_scm2_ioctl_execute(unsigned short nfit_handle,
		size_t bufSize, void *p_ioctl_data, int io_controlcode)
{
	SCM_LOG_ENTRY();
	HANDLE handle;

	enum WIN_SCM2_IOCTL_RETURN_CODES rc = scm2_open_ioctl_target(&handle, nfit_handle);
	if (WIN_SCM2_IOCTL_SUCCESS(rc))
	{
		rc = scm2_send_ioctl_command(handle, (unsigned long)io_controlcode,
				p_ioctl_data, bufSize, p_ioctl_data, bufSize);
		CloseHandle(handle);
	}

	SCM_LOG_EXIT_RETURN_I(rc);

	return rc;
}
