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

#include "win_scm2_ioctl_passthrough.h"
#include "win_scm2_ioctl.h"
#include "win_scm2_adapter.h"

#define	IOCTL_CR_PASS_THROUGH CTL_CODE(NVDIMM_IOCTL, 0x960, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	SCM_BUILD_DSM_OPCODE(opcode, subop_code) (unsigned int)(subop_code << 8 | opcode)

typedef enum _DSM_STATUS_ENUM
{
	STATUS_DSM_SUCCESS = 0,
	STATUS_DSM_NOT_SUPPORTED,
	STATUS_DSM_NON_EXISTING_DEVICE,
	STATUS_DSM_RESERVED,
	STATUS_DSM_VENDOR_SPECIFIC_ERROR
} DSM_STATUS_ENUM;

typedef struct _DSM_STATUS {
	UINT32 DsmStatus:16;
	/* DSM_STATUS_ENUM */
#define	MAILBOX_STATUS_CODE_INVALID_LARGE_PAYLOAD_OFFSET 0x80
#define	MAILBOX_STATUS_CODE_INVALID_TRANSFER_LENGTH 0x81
	UINT32 MailboxStatusCode:
			8;
	UINT32 BackgroundOperationState:1;
	UINT32 Reserved:7;
} DSM_STATUS;


typedef struct _DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD {
	ULONG Arg3OpCode;
	ULONG Arg3OpCodeParameterDataLength;
	UCHAR Arg3OpCodeParameterDataBuffer[1];
} DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD;

typedef struct _DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD {
	DSM_STATUS Arg3Status;
	ULONG Arg3OutputDataLength;
	UCHAR Arg3OutputDataBuffer[1];
} DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD;


typedef struct _DEVICE_HANDLE {
	union {
		struct {
			UINT32 DimmNumber:4;
			UINT32 MemoryChannel:4;
			UINT32 MemoryControllerId:4;
			UINT32 SocketId:4;
			UINT32 NodeControllerId:12;
			UINT32 Reserved:4;
		};
		UINT32 DeviceHandle;
	};
} DEVICE_HANDLE;

typedef struct _CR_DSM_PASS_THROUGH_IOCTL
{
	ULONG ReturnCode; // CR_RETURN_CODES enum
	DEVICE_HANDLE NfitDeviceHandle;
	DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD InputPayload;
	DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD OutputPayload;
} CR_DSM_PASS_THROUGH_IOCTL;

int win_scm2_ioctl_passthrough_cmd(unsigned short nfit_handle,
		unsigned short op_code, unsigned short sub_op_code,
		void *input_payload, unsigned long input_payload_size,
		void *output_payload, unsigned long output_payload_size,
		unsigned int *p_dsm_status)
{
	SCM_LOG_ENTRY();
	SCM_LOG_INFO_F("handle: %d, opcode 0x%x, sub op: 0x%x, "
			"input payload size: %lu, output payload size: %lu",
			nfit_handle, op_code, sub_op_code, input_payload_size, output_payload_size);

	int rc = 0;
	*p_dsm_status = 0;
	size_t buf_size = sizeof (CR_DSM_PASS_THROUGH_IOCTL) +
			input_payload_size + output_payload_size;

	// Because the CR_DSM_PASS_THROUGH_IOCTL struct has a byte for the input and output payloads
	// already, we need to subtract those bytes from the total buffer size
	buf_size -= 8; // minus 2 bytes

	SCM_LOG_INFO_F("buf_size (%xh, %xh): %d", op_code, sub_op_code, (int)buf_size);

	CR_DSM_PASS_THROUGH_IOCTL *p_ioctl_data = calloc(1, buf_size);
	if (p_ioctl_data)
	{
		p_ioctl_data->NfitDeviceHandle.DeviceHandle = nfit_handle;

		p_ioctl_data->InputPayload.Arg3OpCodeParameterDataLength = input_payload_size;
		p_ioctl_data->InputPayload.Arg3OpCode = SCM_BUILD_DSM_OPCODE(op_code, sub_op_code);

		if (input_payload_size > 0)
		{
			memmove(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer,
					input_payload,
					input_payload_size);
		}

		DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD *p_output_payload =
				(DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD *)
						(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer +
								input_payload_size);

		enum WIN_SCM2_IOCTL_RETURN_CODES ioctl_rc = win_scm2_ioctl_execute(nfit_handle, buf_size,
						p_ioctl_data, IOCTL_CR_PASS_THROUGH);
		if (!WIN_SCM2_IS_SUCCESS(ioctl_rc))
		{
			rc = (int)ioctl_rc;
			SCM_LOG_ERROR_F("Issue with ioctl for %s. RC= %d", __FUNCTION__, rc);
		}
		else if (p_ioctl_data->ReturnCode > 0)
		{
			rc = (int)p_ioctl_data->ReturnCode;
			SCM_LOG_ERROR_F("Error with passthrough command (%xh, %xh). IOCTL Return Code: 0x%x",
					op_code, sub_op_code, (int)p_ioctl_data->ReturnCode);
		}
		else if (p_output_payload->Arg3Status.DsmStatus != 0)
		{
			unsigned int status = p_output_payload->Arg3Status.DsmStatus |
					(p_output_payload->Arg3Status.MailboxStatusCode << DSM_MAILBOX_ERROR_SHIFT);
			*p_dsm_status = status;

			SCM_LOG_ERROR_F("Error with FW Command (%xh, %xh). DSM Status: 0x%x",
					op_code, sub_op_code, status);
		}
		else
		{
			if (output_payload_size > 0)
			{
				size_t bytes_to_copy = output_payload_size;
				if (p_output_payload->Arg3OutputDataLength < output_payload_size)
				{
					// User expected more data than the command returned
					// We can safely copy it, but there could be a developer error
					bytes_to_copy = p_output_payload->Arg3OutputDataLength;
				}
				memmove(output_payload, p_output_payload->Arg3OutputDataBuffer, bytes_to_copy);
			}
		}

		free(p_ioctl_data);
	}
	else
	{
		rc = WIN_SCM2_ERR_NOMEMORY;
	}

	SCM_LOG_EXIT_RETURN_I(rc);

	return rc;
}