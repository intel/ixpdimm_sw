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
#include "win_scm2_capabilities.h"
#include "win_scm2_ioctl.h"
#include "win_scm2_adapter.h"

#define	IOCTL_CR_GET_DRIVER_CAPABILITIES CTL_CODE(NVDIMM_IOCTL, 0x902, \
	METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _GET_DRIVER_CAPABILITIES_OUTPUT_PAYLOAD {
	DRIVER_CAPABILITIES Capabilities;
} GET_DRIVER_CAPABILITIES_OUTPUT_PAYLOAD;

typedef struct GET_DRIVER_CAPABILITIES_IOCTL {
	unsigned long ReturnCode; // CR_RETURN_CODES enum
	GET_DRIVER_CAPABILITIES_OUTPUT_PAYLOAD OutputPayload;
} CR_GET_DRIVER_CAPABILITIES_IOCTL;

int win_scm2_ioctl_get_driver_capabilities(unsigned short nfit_handle,
		DRIVER_CAPABILITIES *p_capabilities)
{
	CR_GET_DRIVER_CAPABILITIES_IOCTL cmd;
	int rc = WIN_SCM2_SUCCESS;
	memset(&cmd, 0, sizeof (cmd));
	void *ioctl_data = &cmd;
	enum WIN_SCM2_IOCTL_RETURN_CODES ioctl_rc = win_scm2_ioctl_execute(nfit_handle,
			sizeof (cmd), ioctl_data, IOCTL_CR_GET_DRIVER_CAPABILITIES);
	if (!WIN_SCM2_IOCTL_SUCCESS(ioctl_rc))
	{
		rc = ioctl_rc;
	}
	else if (cmd.ReturnCode > 0)
	{
		SCM_LOG_ERROR_F("win_scm2_ioctl_get_driver_capabilities failed with ReturnCode: %d\n",
				(int)cmd.ReturnCode);
		rc = (int)cmd.ReturnCode;
	}
	else
	{
		memmove(p_capabilities, &(cmd.OutputPayload.Capabilities), sizeof (*p_capabilities));
	}

	return rc;
}