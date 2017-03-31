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

void pt_get_error_message(unsigned int code, char message[1024], size_t message_len)
{
	memset(message, 0, message_len);
	size_t len = 0;
	union pt_error error;
	error.code = code;
	if (error.parts.func != PT_SUCCESS)
	{
		switch((enum IOCTL_PASSTHROUGH_RESULT)error.parts.func)
		{
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
			case PT_SUCCESS:break;
		}
		s_strcat(message, message_len, "\n");
	}

	if (error.parts.driver != 0)
	{
		len += s_snprintf(message + len, message_len - len, "\tDriver failed with error: 0x%x\n",
			error.parts.driver);
	}
	if (error.parts.ioctl != 0)
	{
		len += s_snprintf(message + len, message_len - len, "\tPassthrough IOCTL failed with error: 0x%x\n",
			error.parts.ioctl);
	}
	if (error.parts.fw_status != 0)
	{
		len += s_snprintf(message + len, message_len - len, "\tFW Status: 0x%x\n", error.parts.fw_status);
	}
	if (error.parts.fw_ext_status != 0)
	{
		s_snprintf(message + len, message_len - len, "\tFW Extended Status: 0x%x\n", error.parts.fw_ext_status);
	}
}
