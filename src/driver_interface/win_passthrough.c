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
 * This file implements the Windows driver adapter interface for issuing IOCTL
 * passthrough commands.
 */
#include "passthrough.h"

#include <windows.h>
#include <windows/PrivateIoctlDefinitions.h>
#include <windows/DiagnosticExport.h>
#include <stdio.h>

// SCSI port used as IOCTL target
extern short g_scsi_port;
#define DSM_MAILBOX_ERROR_SHIFT (16)
#define DSM_BACKGROUND_OP_STATE_SHIFT (24)
#define DSM_VENDOR_ERROR_SHIFT (0)

int pt_ind_err_to_nvm_lib_err(CR_RETURN_CODES ind_err);

int pt_open_ioctl_target(PHANDLE p_handle, short scsi_port);
int pt_send_ioctl_command(HANDLE handle,
	unsigned long io_controlcode,
	void *p_in_buffer,
	size_t in_size,
	void *p_out_buffer,
	size_t out_size);
int pt_execute_ioctl(size_t bufSize, void *p_ioctl_data, unsigned long io_controlcode);

unsigned int pt_win_dsm_status_to_int(DSM_STATUS win_dsm_status)
{
	return win_dsm_status.BackgroundOperationState << DSM_BACKGROUND_OP_STATE_SHIFT
		   | win_dsm_status.MailboxStatusCode << DSM_MAILBOX_ERROR_SHIFT
		   | win_dsm_status.DsmStatus << DSM_VENDOR_ERROR_SHIFT;
}

/*
 * Execute an emulated BIOS ioctl to retrieve information about the bios large mailboxes
 */
int pt_bios_get_large_payload_size(unsigned int device_handle,
	GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD *p_large_payload_size)
{
		int rc = PT_ERR_UNKNOWN;

	size_t arg3_size = sizeof(DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD) -
					   ARG3_OPCODE_PARAMETER_DATA_BUFFER_PLACEHOLDER +
					   sizeof(DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
					   - DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD_PLACEHOLDERS
					   + sizeof(GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD);

	size_t buf_size = sizeof(ULONG) + sizeof(NFIT_DEVICE_HANDLE) + arg3_size;

	PPASS_THROUGH_IOCTL p_ioctl_data = calloc(1, buf_size);
	if (p_ioctl_data)
	{
		p_ioctl_data->NfitDeviceHandle.DeviceHandle = device_handle;

		p_ioctl_data->InputPayload.Arg3OpCode = PT_BUILD_DSM_OPCODE(PT_BIOS_EMULATED_COMMAND,
			PT_SUBOP_GET_PAYLOAD_SIZE);
		p_ioctl_data->InputPayload.Arg3OpCodeParameterDataLength = 0;

		PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD p_DsmOutputPayload =
			(PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
				(&p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer);

		if ((rc = pt_execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_PASS_THROUGH))
			== PT_SUCCESS &&
			(rc = pt_ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == PT_SUCCESS)
		{
			if ((rc = pt_win_dsm_status_to_int(p_DsmOutputPayload->Arg3Status))
				== PT_SUCCESS)
			{
				memmove(p_large_payload_size, p_DsmOutputPayload->Arg3OutputBuffer,
					sizeof(GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD));
			}
		}

		free(p_ioctl_data);
	}
	else
	{
				rc = PT_ERR_NOMEMORY;
	}

		return rc;
}

/*
 * Populate the emulated bios large input mailbox
 */
int pt_bios_write_large_payload(struct pt_fw_cmd *p_fw_cmd)
{
	int rc = PT_SUCCESS;

	GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD large_payload_size;
	if ((rc = pt_bios_get_large_payload_size(p_fw_cmd->device_handle,
		&large_payload_size)) == PT_SUCCESS)
	{
		if (large_payload_size.LargeInputPayloadSize < p_fw_cmd->large_input_payload_size)
		{
			rc = PT_ERR_DRIVERFAILED;
		}
		else
		{
			unsigned int write_size = large_payload_size.MaxReadWriteBytes;
			unsigned int current_offset = 0;

			while (rc == PT_SUCCESS && current_offset < p_fw_cmd->large_input_payload_size)
			{
				if ((current_offset + large_payload_size.MaxReadWriteBytes)
					> p_fw_cmd->large_input_payload_size)
				{
					write_size = p_fw_cmd->large_output_payload_size - current_offset;
				}

				size_t arg3_size = sizeof(DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD)
								   - ARG3_OPCODE_PARAMETER_DATA_BUFFER_PLACEHOLDER
								   + sizeof(WRITE_LARGE_PAYLOAD_INPUT_PAYLOAD) -
								   WRITE_LARGE_PAYLOAD_PLACEHOLDERS
								   + write_size + sizeof(DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
								   - DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD_PLACEHOLDERS;

				size_t buf_size = sizeof(ULONG) + sizeof(NFIT_DEVICE_HANDLE) + arg3_size;

				PPASS_THROUGH_IOCTL p_ioctl_data = calloc(1, buf_size);
				if (p_ioctl_data)
				{
					p_ioctl_data->NfitDeviceHandle.DeviceHandle
						= p_fw_cmd->device_handle;

					p_ioctl_data->InputPayload.Arg3OpCode = PT_BUILD_DSM_OPCODE(PT_BIOS_EMULATED_COMMAND,
						PT_SUBOP_WRITE_LARGE_PAYLOAD_INPUT);
					p_ioctl_data->InputPayload.Arg3OpCodeParameterDataLength =
						sizeof(WRITE_LARGE_PAYLOAD_INPUT_PAYLOAD)
						- WRITE_LARGE_PAYLOAD_PLACEHOLDERS + write_size;

					PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD p_outputPayload =
						(PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
							(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer +
							 sizeof(WRITE_LARGE_PAYLOAD_INPUT_PAYLOAD)
							 - WRITE_LARGE_PAYLOAD_PLACEHOLDERS
							 + write_size);

					PWRITE_LARGE_PAYLOAD_INPUT_PAYLOAD p_write_payload =
						(PWRITE_LARGE_PAYLOAD_INPUT_PAYLOAD)
							(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer);
					p_write_payload->NumberOfBytesToTransfer = write_size;

					p_write_payload->LargeInputPayloadOffset = current_offset;
					memmove(p_write_payload->BytesToWrite,
						p_fw_cmd->large_input_payload + current_offset,
						write_size);

					if ((rc = pt_execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_PASS_THROUGH))
						== PT_SUCCESS &&
						(rc = pt_ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == PT_SUCCESS)
					{
						if ((rc = (pt_win_dsm_status_to_int(p_outputPayload->Arg3Status))) == PT_SUCCESS)
						{
							current_offset += write_size;
						}
					}
				}
				else
				{
					rc = PT_ERR_NOMEMORY;
				}
			}

			if (rc == PT_SUCCESS &&
				current_offset != p_fw_cmd->large_input_payload_size)
			{
				rc = PT_ERR_UNKNOWN;
			}
		}
	}

		return rc;
}

/*
 * Read the emulated bios large input mailbox
 */
int pt_bios_read_large_payload(struct pt_fw_cmd *p_fw_cmd)
{
		int rc = PT_SUCCESS;

	GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD large_payload_size;
	if ((rc = pt_bios_get_large_payload_size(p_fw_cmd->device_handle,
		&large_payload_size)) == PT_SUCCESS)
	{
		if (large_payload_size.LargeOutputPayloadSize < p_fw_cmd->large_output_payload_size)
		{
			rc = PT_ERR_DRIVERFAILED;
		}
		else
		{
			unsigned int read_size = large_payload_size.MaxReadWriteBytes;
			unsigned int current_offset = 0;

			memset(p_fw_cmd->large_output_payload, 0, p_fw_cmd->large_output_payload_size);

			while (rc == PT_SUCCESS &&
				   current_offset < p_fw_cmd->large_output_payload_size)
			{
				if ((current_offset + large_payload_size.MaxReadWriteBytes)
					> p_fw_cmd->large_output_payload_size)
				{
					read_size = p_fw_cmd->large_output_payload_size - current_offset;
				}

				size_t arg3_size = sizeof(DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD)
								   - ARG3_OPCODE_PARAMETER_DATA_BUFFER_PLACEHOLDER
								   + sizeof(READ_LARGE_PAYLOAD_INPUT_PAYLOAD)
								   + sizeof(DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
								   - DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD_PLACEHOLDERS
								   + read_size;

				size_t buf_size = sizeof(ULONG) + sizeof(NFIT_DEVICE_HANDLE) + arg3_size;

				PPASS_THROUGH_IOCTL p_ioctl_data = calloc(1, buf_size);
				if (p_ioctl_data)
				{
					p_ioctl_data->NfitDeviceHandle.DeviceHandle = p_fw_cmd->device_handle;

					p_ioctl_data->InputPayload.Arg3OpCode = PT_BUILD_DSM_OPCODE(PT_BIOS_EMULATED_COMMAND,
						PT_SUBOP_READ_LARGE_PAYLOAD_OUTPUT);
					p_ioctl_data->InputPayload.Arg3OpCodeParameterDataLength =
						sizeof(READ_LARGE_PAYLOAD_INPUT_PAYLOAD);

					PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD p_outputPayload =
						(PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
							(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer
							 + sizeof(READ_LARGE_PAYLOAD_INPUT_PAYLOAD));

					PREAD_LARGE_PAYLOAD_INPUT_PAYLOAD p_read_payload =
						(PREAD_LARGE_PAYLOAD_INPUT_PAYLOAD)
							(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer);

					p_read_payload->NumberOfBytesToTransfer = read_size;

					p_read_payload->LargeOutputPayloadOffset = current_offset;

					if ((rc = pt_execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_PASS_THROUGH))
						== PT_SUCCESS &&
						(rc = pt_ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == PT_SUCCESS)
					{
						if ((rc = (pt_win_dsm_status_to_int(p_outputPayload->Arg3Status))) == PT_SUCCESS)
						{
							memmove(p_fw_cmd->large_output_payload + current_offset,
								p_outputPayload->Arg3OutputBuffer, read_size);
							current_offset += read_size;
						}
					}
					free(p_ioctl_data);
				}
				else
				{
					rc = PT_ERR_NOMEMORY;
				}
			}

			if (rc == PT_SUCCESS &&
				current_offset != p_fw_cmd->large_output_payload_size)
			{
				rc = PT_ERR_UNKNOWN;
			}
		}
	}

		return rc;
}

/*
 * Execute a passthrough IOCTL
 */
int adapter_pt_ioctl_cmd(struct pt_fw_cmd *p_cmd)
{
		int rc = PT_ERR_UNKNOWN;


	size_t input_buf_size = sizeof(DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD)
							- ARG3_OPCODE_PARAMETER_DATA_BUFFER_PLACEHOLDER +
							p_cmd->input_payload_size;

	size_t output_buf_size = sizeof(DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
							 - DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD_PLACEHOLDERS
							 + PT_DEV_SMALL_PAYLOAD_SIZE;

	size_t arg3_size = input_buf_size + output_buf_size;
	size_t buf_size = sizeof(ULONG) + sizeof(NFIT_DEVICE_HANDLE) + arg3_size;

	PPASS_THROUGH_IOCTL p_ioctl_data = calloc(1, buf_size);
	if (p_ioctl_data)
	{
		p_ioctl_data->NfitDeviceHandle.DeviceHandle = p_cmd->device_handle;
		p_ioctl_data->InputPayload.Arg3OpCodeParameterDataLength = p_cmd->input_payload_size;
		p_ioctl_data->InputPayload.Arg3OpCode = PT_BUILD_DSM_OPCODE(p_cmd->opcode, p_cmd->sub_opcode);

		if (p_cmd->input_payload_size > 0)
		{
			memmove(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer,
				p_cmd->input_payload,
				p_cmd->input_payload_size);
		}

		if (p_cmd->large_input_payload_size > 0)
		{
			rc = pt_bios_write_large_payload(p_cmd);
		}

		PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD p_output_payload = NULL;

		p_output_payload =
			(PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
				(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer
				 + p_cmd->input_payload_size);

		rc = pt_execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_PASS_THROUGH);

		if (rc == PT_SUCCESS &&
			(rc = pt_ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == PT_SUCCESS)
		{
			if ((rc = (pt_win_dsm_status_to_int(p_output_payload->Arg3Status))) == PT_SUCCESS)
			{
				if (p_cmd->output_payload_size > 0)
				{
					size_t bytes_to_copy = p_cmd->output_payload_size;
					if (p_output_payload->Arg3OutputBufferLength <
						p_cmd->output_payload_size)
					{
						// User expected more data than the command returned
						// We can safely copy it, but there could be a developer error
						bytes_to_copy = p_output_payload->Arg3OutputBufferLength;
					}
					memmove(p_cmd->output_payload, p_output_payload->Arg3OutputBuffer,
						bytes_to_copy);
				}

				if (p_cmd->large_output_payload_size > 0)
				{
					rc = pt_bios_read_large_payload(p_cmd);
				}
			}
		}

		free(p_ioctl_data);
	}
	else
	{
				rc = PT_ERR_NOMEMORY;
	}

		return rc;
}



#define	WIN_DRIVER_VERSION_MAJOR_MIN	1
#define	WIN_DRIVER_VERSION_MAJOR_MAX	1
#define	SCSI_PORT_MAX 32
/*
 * Bit selection as defined for the MSR_DRAM_POWER_LIMIT register,
 * 618h from the IA64/32 Software Developers Manual
 */
#define	POWER_LIMIT_ENABLE_BIT	0x80

short pt_g_scsi_port = -1;

// Helper function declarations
//enum label_area_health_result convert_label_health_result(LABEL_AREA_HEALTH_EVENT event);
//enum ns_health_result convert_ns_health_status(NAMESPACE_HEALTH_EVENT event);
//DIAGNOSTIC_TEST convert_diagnostic_test(enum driver_diagnostic diagnostic);

/*
 * Support is determined by driver version
 */

int pt_ind_err_to_nvm_lib_err(CR_RETURN_CODES ind_err)
{
	return (ind_err);
}

static int __get_driver_revision(short scsi_port)
{
	int rc = 0;

	HANDLE handle = INVALID_HANDLE_VALUE;

	// Open the Drivers IOCTL File Handle
	if ((rc = pt_open_ioctl_target(&handle, scsi_port)) == 0)
	{
		CR_GET_DRIVER_REVISION_IOCTL payload_in;
		CR_GET_DRIVER_REVISION_IOCTL payload_out;

		memset(&payload_in, 0, sizeof (CR_GET_DRIVER_REVISION_IOCTL));

		// Verify IOCTL is sent successfully and that the driver had no errors
		if ((rc = pt_send_ioctl_command(handle,
			IOCTL_CR_GET_VENDOR_DRIVER_REVISION,
			&payload_in,
			sizeof (CR_GET_DRIVER_REVISION_IOCTL),
			&payload_out,
			sizeof (CR_GET_DRIVER_REVISION_IOCTL))) == 0)
		{
			rc = pt_ind_err_to_nvm_lib_err(payload_out.ReturnCode);
		}

		CloseHandle(handle);
	}


	return rc;
}

int pt_init_scsi_port()
{
	int rc = 0;
	int i = 0;

	while (pt_g_scsi_port < 0 && i < SCSI_PORT_MAX)
	{
		int temp_rc = -1;

		if ((temp_rc = __get_driver_revision(i)) == 0)
		{
			pt_g_scsi_port = i;
		}

		i++;
	}

	if (pt_g_scsi_port < 0)
	{
		rc = -1;
	}

	return rc;
}



int pt_open_ioctl_target(PHANDLE p_handle, short scsi_port)
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

int pt_send_ioctl_command(HANDLE handle,
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
int pt_execute_ioctl(size_t bufSize, void *p_ioctl_data, unsigned long io_controlcode)
{
	int rc;
	HANDLE handle;
	if ((rc = pt_init_scsi_port()) != 0)
	{
	}
	else
	{
		if ((rc = pt_open_ioctl_target(&handle, pt_g_scsi_port)) == 0)
		{
			rc = pt_send_ioctl_command(handle, io_controlcode, p_ioctl_data, bufSize, p_ioctl_data,
				bufSize);
			CloseHandle(handle);
		}
	}

	return rc;
}
