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

#include "device_adapter.h"
#include "win_adapter.h"
#include <os/os_adapter.h>

/*
 * Execute an emulated BIOS ioctl to retrieve information about the bios large mailboxes
 */
int bios_get_large_payload_size(unsigned int device_handle,
		GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD *p_large_payload_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	size_t arg3_size = sizeof (DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD) -
		ARG3_OPCODE_PARAMETER_DATA_BUFFER_PLACEHOLDER +
		sizeof (DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
		- DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD_PLACEHOLDERS
		+ sizeof (GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD);

	size_t buf_size = sizeof (ULONG) + sizeof (NFIT_DEVICE_HANDLE) + arg3_size;

	PPASS_THROUGH_IOCTL p_ioctl_data = calloc(1, buf_size);
	if (p_ioctl_data)
	{
		p_ioctl_data->NfitDeviceHandle.DeviceHandle = device_handle;

		p_ioctl_data->InputPayload.Arg3OpCode = BUILD_DSM_OPCODE(BIOS_EMULATED_COMMAND,
			SUBOP_GET_PAYLOAD_SIZE);
		p_ioctl_data->InputPayload.Arg3OpCodeParameterDataLength = 0;

		PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD p_DsmOutputPayload =
				(PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
				(&p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer);

		if ((rc = execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_PASS_THROUGH))
			== NVM_SUCCESS &&
			(rc = ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == NVM_SUCCESS)
		{
			if ((rc = dsm_err_to_nvm_lib_err(win_dsm_status_to_int(p_DsmOutputPayload->Arg3Status)))
					== NVM_SUCCESS)
			{
				memmove(p_large_payload_size, p_DsmOutputPayload->Arg3OutputBuffer,
					sizeof (GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD));
			}
		}

		free(p_ioctl_data);
	}
	else
	{
		COMMON_LOG_ERROR("couldn't allocate input payload");
		rc = NVM_ERR_NOMEMORY;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Populate the emulated bios large input mailbox
 */
int bios_write_large_payload(struct fw_cmd *p_fw_cmd)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD large_payload_size;
	if ((rc = bios_get_large_payload_size(p_fw_cmd->device_handle,
			&large_payload_size)) == NVM_SUCCESS)
	{
		if (large_payload_size.LargeInputPayloadSize < p_fw_cmd->large_input_payload_size)
		{
			rc = NVM_ERR_BADSIZE;
		}
		else
		{
			unsigned int write_size = large_payload_size.MaxReadWriteBytes;
			unsigned int current_offset = 0;

			while (rc == NVM_SUCCESS && current_offset < p_fw_cmd->large_input_payload_size)
			{
				if ((current_offset + large_payload_size.MaxReadWriteBytes)
					> p_fw_cmd->large_input_payload_size)
				{
					write_size = p_fw_cmd->large_output_payload_size - current_offset;
				}

				size_t arg3_size = sizeof (DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD)
					- ARG3_OPCODE_PARAMETER_DATA_BUFFER_PLACEHOLDER
					+ sizeof (WRITE_LARGE_PAYLOAD_INPUT_PAYLOAD) - WRITE_LARGE_PAYLOAD_PLACEHOLDERS
					+ write_size + sizeof (DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
					- DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD_PLACEHOLDERS;

				size_t buf_size = sizeof (ULONG) + sizeof (NFIT_DEVICE_HANDLE) + arg3_size;

				PPASS_THROUGH_IOCTL p_ioctl_data = calloc(1, buf_size);
				if (p_ioctl_data)
				{
					p_ioctl_data->NfitDeviceHandle.DeviceHandle
						= p_fw_cmd->device_handle;

					p_ioctl_data->InputPayload.Arg3OpCode = BUILD_DSM_OPCODE(BIOS_EMULATED_COMMAND,
						SUBOP_WRITE_LARGE_PAYLOAD_INPUT);
					p_ioctl_data->InputPayload.Arg3OpCodeParameterDataLength =
						sizeof (WRITE_LARGE_PAYLOAD_INPUT_PAYLOAD)
						- WRITE_LARGE_PAYLOAD_PLACEHOLDERS + write_size;

					PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD p_outputPayload =
						(PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
						(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer +
						sizeof (WRITE_LARGE_PAYLOAD_INPUT_PAYLOAD)
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

					if ((rc = execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_PASS_THROUGH))
						== NVM_SUCCESS &&
						(rc = ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == NVM_SUCCESS)
					{
						if ((rc = dsm_err_to_nvm_lib_err(
							win_dsm_status_to_int(p_outputPayload->Arg3Status))) == NVM_SUCCESS)
						{
							current_offset += write_size;
						}
					}
				}
				else
				{
					rc = NVM_ERR_NOMEMORY;
				}
			}

			if (rc == NVM_SUCCESS &&
				current_offset != p_fw_cmd->large_input_payload_size)
			{
				rc = NVM_ERR_UNKNOWN;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Read the emulated bios large input mailbox
 */
int bios_read_large_payload(struct fw_cmd *p_fw_cmd)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	GET_LARGE_PAYLOAD_SIZE_OUTPUT_PAYLOAD large_payload_size;
	if ((rc = bios_get_large_payload_size(p_fw_cmd->device_handle,
			&large_payload_size)) == NVM_SUCCESS)
	{
		if (large_payload_size.LargeOutputPayloadSize < p_fw_cmd->large_output_payload_size)
		{
			rc = NVM_ERR_BADSIZE;
		}
		else
		{
			unsigned int read_size = large_payload_size.MaxReadWriteBytes;
			unsigned int current_offset = 0;

			memset(p_fw_cmd->large_output_payload, 0, p_fw_cmd->large_output_payload_size);

			while (rc == NVM_SUCCESS &&
				current_offset < p_fw_cmd->large_output_payload_size)
			{
				if ((current_offset + large_payload_size.MaxReadWriteBytes)
					> p_fw_cmd->large_output_payload_size)
				{
					read_size = p_fw_cmd->large_output_payload_size - current_offset;
				}

				size_t arg3_size = sizeof (DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD)
					- ARG3_OPCODE_PARAMETER_DATA_BUFFER_PLACEHOLDER
					+ sizeof (READ_LARGE_PAYLOAD_INPUT_PAYLOAD)
					+ sizeof (DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
					- DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD_PLACEHOLDERS
					+ read_size;

				size_t buf_size = sizeof (ULONG) + sizeof (NFIT_DEVICE_HANDLE) + arg3_size;

				PPASS_THROUGH_IOCTL p_ioctl_data = calloc(1, buf_size);
				if (p_ioctl_data)
				{
					p_ioctl_data->NfitDeviceHandle.DeviceHandle = p_fw_cmd->device_handle;

					p_ioctl_data->InputPayload.Arg3OpCode = BUILD_DSM_OPCODE(BIOS_EMULATED_COMMAND,
						SUBOP_READ_LARGE_PAYLOAD_OUTPUT);
					p_ioctl_data->InputPayload.Arg3OpCodeParameterDataLength =
						sizeof (READ_LARGE_PAYLOAD_INPUT_PAYLOAD);

					PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD p_outputPayload =
						(PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
						(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer
						+ sizeof (READ_LARGE_PAYLOAD_INPUT_PAYLOAD));

					PREAD_LARGE_PAYLOAD_INPUT_PAYLOAD p_read_payload =
						(PREAD_LARGE_PAYLOAD_INPUT_PAYLOAD)
						(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer);

					p_read_payload->NumberOfBytesToTransfer = read_size;

					p_read_payload->LargeOutputPayloadOffset = current_offset;

					if ((rc = execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_PASS_THROUGH))
						== NVM_SUCCESS &&
						(rc = ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == NVM_SUCCESS)
					{
						if ((rc = dsm_err_to_nvm_lib_err(
							win_dsm_status_to_int(p_outputPayload->Arg3Status))) == NVM_SUCCESS)
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
					rc = NVM_ERR_NOMEMORY;
				}
			}

			if (rc == NVM_SUCCESS &&
				current_offset != p_fw_cmd->large_output_payload_size)
			{
				rc = NVM_ERR_UNKNOWN;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Execute a passthrough IOCTL
 */
int ioctl_passthrough_cmd(struct fw_cmd *p_cmd)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (p_cmd == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, cmd is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((p_cmd->input_payload_size > 0 && p_cmd->input_payload == NULL) ||
			(p_cmd->input_payload != NULL && p_cmd->input_payload_size == 0) ||
			(p_cmd->output_payload_size > 0 && p_cmd->output_payload == NULL) ||
			(p_cmd->output_payload != NULL && p_cmd->output_payload_size == 0) ||
			(p_cmd->large_input_payload_size > 0 && p_cmd->large_input_payload == NULL) ||
			(p_cmd->large_input_payload != NULL && p_cmd->large_input_payload_size == 0) ||
			(p_cmd->large_output_payload_size > 0 && p_cmd->large_output_payload == NULL) ||
			(p_cmd->large_output_payload != NULL && p_cmd->large_output_payload_size == 0))
	{
		COMMON_LOG_ERROR("bad input/output payload(s)");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	// avoid any commands that require large payloads
#if __LARGE_PAYLOAD__ == 0
	else if ((p_cmd->opcode == 0x08 && p_cmd->sub_opcode == 0x02) || // get fw debug log
			(p_cmd->opcode == 0x08 && p_cmd->sub_opcode == 0x05) || // get error log
			(p_cmd->opcode == 0x0A)) // inject error
	{
		COMMON_LOG_ERROR_F("Intel DIMM Gen 1 FW command OpCode: 0x%x SubOpCode: "
				"0x%x is not supported",
				p_cmd->opcode, p_cmd->sub_opcode);
		rc = NVM_ERR_NOTSUPPORTED;
	}
#endif
	else
	{
		size_t input_buf_size = sizeof (DSM_VENDOR_SPECIFIC_COMMAND_INPUT_PAYLOAD)
				- ARG3_OPCODE_PARAMETER_DATA_BUFFER_PLACEHOLDER	+ p_cmd->input_payload_size;

		size_t output_buf_size = sizeof (DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
			- DSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD_PLACEHOLDERS
			+ DEV_SMALL_PAYLOAD_SIZE;

		size_t arg3_size = input_buf_size + output_buf_size;
		size_t buf_size = sizeof (ULONG) + sizeof (NFIT_DEVICE_HANDLE) + arg3_size;

		PPASS_THROUGH_IOCTL p_ioctl_data = calloc(1, buf_size);
		if (p_ioctl_data)
		{
			p_ioctl_data->NfitDeviceHandle.DeviceHandle =
					p_cmd->device_handle;

			p_ioctl_data->InputPayload.Arg3OpCodeParameterDataLength = p_cmd->input_payload_size;
			p_ioctl_data->InputPayload.Arg3OpCode = BUILD_DSM_OPCODE(p_cmd->opcode,
					p_cmd->sub_opcode);

			if (p_cmd->input_payload_size > 0)
			{
				memmove(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer,
						p_cmd->input_payload,
						p_cmd->input_payload_size);
			}

			if (p_cmd->large_input_payload_size > 0)
			{
				rc = bios_write_large_payload(p_cmd);
			}

			PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD p_output_payload = NULL;

			p_output_payload =
				(PDSM_VENDOR_SPECIFIC_COMMAND_OUTPUT_PAYLOAD)
				(p_ioctl_data->InputPayload.Arg3OpCodeParameterDataBuffer
					+ p_cmd->input_payload_size);

			if ((rc = execute_ioctl(buf_size, p_ioctl_data, IOCTL_CR_PASS_THROUGH))
				== NVM_SUCCESS &&
				(rc = ind_err_to_nvm_lib_err(p_ioctl_data->ReturnCode)) == NVM_SUCCESS)
			{
				if ((rc = dsm_err_to_nvm_lib_err(
					win_dsm_status_to_int(p_output_payload->Arg3Status))) == NVM_SUCCESS)
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
							COMMON_LOG_WARN_F(
									"output buffer size %llu larger than size returned %u",
									p_cmd->output_payload_size,
									p_output_payload->Arg3OutputBufferLength);
						}
						memmove(p_cmd->output_payload, p_output_payload->Arg3OutputBuffer,
								bytes_to_copy);
					}

					if (p_cmd->large_output_payload_size > 0)
					{
						rc = bios_read_large_payload(p_cmd);
					}
				}
			}

			free(p_ioctl_data);
		}
		else
		{
			COMMON_LOG_ERROR("couldn't allocate input payload");
			rc = NVM_ERR_NOMEMORY;
		}
	}

	s_memset(&p_cmd, sizeof (p_cmd));
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
