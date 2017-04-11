#include <stddef.h>

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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURmessageE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE messageSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "passthrough.h"
#include <common/string/s_str.h>

extern int adapter_pt_ioctl_cmd(struct pt_fw_cmd *p_fw_cmd);

unsigned int pt_ioctl_cmd(struct pt_fw_cmd *p_fw_cmd)
{
	return (unsigned int)adapter_pt_ioctl_cmd(p_fw_cmd);
}

void pt_get_error_message(unsigned int code, char message[1024], size_t message_len)
{
	memset(message, 0, message_len);
	size_t len = 0;
	pt_result result;
	PT_RESULT_DECODE(code, result);
	if (result.func != PT_SUCCESS)
	{
		switch((enum pt_ioctl_result)result.func)
		{
			case PT_SUCCESS:
				len += s_snprintf(message, message_len, "Unknown");
				break;
			case PT_ERR_UNKNOWN:
				len += s_snprintf(message, message_len, "Unknown");
				break;
			case PT_ERR_BADDEVICEHANDLE:
				len += s_snprintf(message, message_len, "Bad Device Handle");
				break;
			case PT_ERR_NOMEMORY:
				len += s_snprintf(message, message_len, "No Memory");
				break;
			case PT_ERR_DRIVERFAILED:
				len += s_snprintf(message, message_len, "Driver failed for an unknown reason");
				break;
			case PT_ERR_BADDEVICE:
				len += s_snprintf(message, message_len, "Bad Device");
				break;
			case PT_ERR_BADSECURITY:
				len += s_snprintf(message, message_len, "Bad Security");
				break;
			case PT_ERR_DEVICEBUSY:
				len += s_snprintf(message, message_len, "Device Busy");
				break;
			case PT_ERR_INVALIDPERMISSIONS:
				len += s_snprintf(message, message_len, "Invalid Permissions");
				break;
		}
		s_strcat(message, message_len, "\n");
	}

	if (result.driver != 0)
	{
		len += s_snprintf(message + len, message_len - len,
			"\tDriver failed with error: 0x%x\n",
			result.driver);
	}
	if (result.pt != 0)
	{
		len += s_snprintf(message + len, message_len - len,
			"\tPassthrough IOCTL failed with error: 0x%x\n",
			result.pt);
	}
	if (result.fw_status != 0)
	{
		len += s_snprintf(message + len, message_len - len,
			"\tFW Status: 0x%x\n", result.fw_status);
	}
	if (result.fw_ext_status != 0)
	{
		s_snprintf(message + len, message_len - len,
			"\tFW Extended Status: 0x%x\n", result.fw_ext_status);
	}
}
