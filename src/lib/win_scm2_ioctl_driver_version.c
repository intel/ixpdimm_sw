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

#include "win_scm2_ioctl_driver_version.h"
#include "win_scm2_adapter.h"
#include "win_scm2_ioctl.h"
#include <string.h>
#include <string/s_str.h>

#define	IOCTL_CR_GET_VENDOR_DRIVER_REVISION CTL_CODE(NVDIMM_IOCTL, 0x980, \
	METHOD_BUFFERED, FILE_ANY_ACCESS)

struct GET_VENDOR_DRIVER_REVISION_OUTPUT_PAYLOAD
{
	unsigned char VendorDriverVersion[DRIVER_VERSION_LEN];
};

struct GET_REVISION_IOCTL
{
	unsigned char ReturnCode; // CR_RETURN_CODES enum
	struct GET_VENDOR_DRIVER_REVISION_OUTPUT_PAYLOAD OutputPayload;
} CR_GET_DRIVER_REVISION_IOCTL;

/*
static int ioctl_driver_version(unsigned short nfit_handle,
		struct GET_VENDOR_DRIVER_REVISION_OUTPUT_PAYLOAD *payload);
*/

int win_scm2_ioctl_driver_version(unsigned short nfit_handle,
		char driver_revision[DRIVER_VERSION_LEN])
{
	int scm_rc = NVM_SUCCESS;
	memset(driver_revision, 0, DRIVER_VERSION_LEN);
	/*
	struct GET_VENDOR_DRIVER_REVISION_OUTPUT_PAYLOAD payload;
	int scm_rc = ioctl_driver_version(nfit_handle, &payload);

	if (WIN_SCM2_IOCTL_SUCCESS(scm_rc))
	{
		memmove(driver_revision, payload.VendorDriverVersion, DRIVER_VERSION_LEN);
	}
	*/

	// Temporary until we get Microsoft Windows inbox driver for NVDIMM
	s_strcpy(driver_revision, "0.0.0.0", DRIVER_VERSION_LEN);

	return scm_rc;
}

/*
static int ioctl_driver_version(unsigned short nfit_handle,
		struct GET_VENDOR_DRIVER_REVISION_OUTPUT_PAYLOAD *payload)
{
	int rc = WIN_SCM2_SUCCESS;
	memset(payload, 0, sizeof (*payload));
	struct GET_REVISION_IOCTL ioctl;

	enum WIN_SCM2_IOCTL_RETURN_CODES ioctl_rc = win_scm2_ioctl_execute(nfit_handle,
			(WIN_SCM2_IOCTL_REQUEST *) &ioctl, IOCTL_CR_GET_VENDOR_DRIVER_REVISION);

	if (!WIN_SCM2_IOCTL_SUCCESS(ioctl_rc))
	{
		rc = (int)ioctl_rc;
		SCM_LOG_ERROR_F("ioctl for win_scm2_ioctl_driver_version failed with: %d\n", rc);
	}
	else if (ioctl.ReturnCode > 0)
	{
		rc = (int)ioctl.ReturnCode;
		SCM_LOG_ERROR_F("win_scm2_ioctl_driver_version failed with ReturnCode: %d\n", rc);
	}
	else
	{
		memmove(payload, &(ioctl.OutputPayload), sizeof (*payload));
	}

	return rc;
}
*/
