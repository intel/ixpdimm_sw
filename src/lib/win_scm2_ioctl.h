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

#ifndef CR_MGMT_SCM2_IOCTL_C_H
#define	CR_MGMT_SCM2_IOCTL_C_H

#include <stdlib.h>
#include <windows.h>
#include <winioctl.h>

#define FILE_DEVICE_PERSISTENT_MEMORY   0x00000059 // defined in devioctl.h

#define	NVDIMM_IOCTL FILE_DEVICE_PERSISTENT_MEMORY //!< NVDIMM IOCTLs codes base value

//
// Functions 0 to 0x2FF are reserved for the bus device.
// Functions 0x300 to 0x5FF are reserved for the logical disk device.
// Functions from 0x600 are reserved for the physical NVDIMM device.
//
#define IOCTL_SCM_PHYSICAL_DEVICE_FUNCTION_BASE     0x600

#define SCM_PHYSICAL_DEVICE_FUNCTION(x) (IOCTL_SCM_PHYSICAL_DEVICE_FUNCTION_BASE + x)

typedef struct _WIN_SCM2_IOCTL_REQUEST {
    ULONG ReturnCode;
    size_t InputDataSize;
    size_t OutputDataSize;
    void * pInputData;
    void * pOutputData;
} WIN_SCM2_IOCTL_REQUEST;

// defined in Crystal Ridge RS2+ SCM Based Windows Driver SAS
typedef enum _CR_RETURN_CODES
{
	CR_RETURN_CODE_SUCCESS = 0,
	CR_RETURN_CODE_NOTSUPPORTED = 1,
	CR_RETURN_CODE_NOTALLOWED = 2,
	CR_RETURN_CODE_INVALIDPARAMETER = 3,
	CR_RETURN_CODE_BUFFER_OVERRUN = 4,
	CR_RETURN_CODE_BUFFER_UNDERRUN = 5,
	CR_RETURN_CODE_NOMEMORY = 6,
	CR_RETURN_CODE_NAMESPACE_CANT_BE_MODIFIED = 7,
	CR_RETURN_CODE_NAMESPACE_CANT_BE_REMOVED = 8,
	CR_RETURN_CODE_LOCKED_DIMM = 9,
	CR_RETURN_CODE_MAXIMUM_NAMESPACES_REACHED = 10,
	CR_RETURN_CODE_UNKNOWN = 11
} CR_RETURN_CODES;

#define	WIN_SCM2_IOCTL_SUCCESS(rc) (rc) == WIN_SCM2_IOCTL_RC_SUCCESS
enum WIN_SCM2_IOCTL_RETURN_CODES
{
	WIN_SCM2_IOCTL_RC_SUCCESS = 0,
	WIN_SCM2_IOCTL_RC_ERR = -1
};

enum WIN_SCM2_IOCTL_RETURN_CODES win_scm2_ioctl_execute(unsigned short nfit_handle,
		WIN_SCM2_IOCTL_REQUEST *p_ioctl_data,
		int io_controlcode);

#endif // CR_MGMT_SCM2_IOCTL_C_H
