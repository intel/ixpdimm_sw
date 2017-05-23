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
#include "win_scm2_version_info.h"
#include "win_scm2_ioctl.h"
#include "win_scm2_adapter.h"

#define	IOCTL_CR_GET_INTERFACE_VERSION CTL_CODE(NVDIMM_IOCTL, 0x981, \
	METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_INTERFACE_LEGACY_DRIVER_MAJOR_VERSION 1
#define	IOCTL_INTERFACE_RS2_SCM_DRIVER_MAJOR_VERSION 2
#define	IOCTL_INTERFACE_MINOR_VERSION 0

typedef struct _GET_INTERFACE_VERSION_IOCTL {
	ULONG ReturnCode; // CR_RETURN_CODES enum
	GET_INTERFACE_VERSION_OUTPUT_PAYLOAD OutputPayload;
} CR_GET_INTERFACE_VERSION_IOCTL, *PCR_GET_INTERFACE_VERSION_IOCTL;


int win_scm2_version_info(unsigned short nfit_handle,
		GET_INTERFACE_VERSION_OUTPUT_PAYLOAD *p_version_info)
{
	int rc = WIN_SCM2_SUCCESS;
	memset(p_version_info, 0, sizeof (*p_version_info));
	CR_GET_INTERFACE_VERSION_IOCTL ioctl;

	enum WIN_SCM2_IOCTL_RETURN_CODES ioctl_rc = win_scm2_ioctl_execute(nfit_handle, sizeof (ioctl),
			&ioctl, IOCTL_CR_GET_INTERFACE_VERSION);

	if (!WIN_SCM2_IOCTL_SUCCESS(ioctl_rc))
	{
		rc = (int)ioctl_rc;
		SCM_LOG_ERROR_F("ioctl for win_scm2_version_info failed with: %d\n", rc);
	}
	else if (ioctl.ReturnCode > 0)
	{
		rc = (int)ioctl.ReturnCode;
		SCM_LOG_ERROR_F("win_scm2_version_info failed with ReturnCode: %d\n", rc);
	}
	else
	{
		memmove(p_version_info, &(ioctl.OutputPayload), sizeof (*p_version_info));
	}

	return rc;
}