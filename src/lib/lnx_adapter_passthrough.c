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
 * This file implements the Linux driver adapter interface for issuing IOCTL
 * passthrough commands.
 */

#include "device_adapter.h"
#include "lnx_adapter.h"
#include <os/os_adapter.h>

#define	BIOS_INPUT(NAME, IN_LEN)		\
struct NAME {								\
	NVM_UINT32 size;						\
	NVM_UINT32 offset;						\
	NVM_UINT8  buffer[IN_LEN];				\
}

/*
 * Execute an emulated BIOS ioctl to retrieve information about the bios large mailboxes
 */
int bios_get_payload_size(struct ndctl_dimm *p_dimm, struct pt_bios_get_size *p_bios_mb_size)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_dimm == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, Dimm is null");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_bios_mb_size == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, size struct is null");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		memset(p_bios_mb_size, 0, sizeof (*p_bios_mb_size));
		struct ndctl_cmd *p_vendor_cmd = NULL;
		if ((p_vendor_cmd = ndctl_dimm_cmd_new_vendor_specific(p_dimm,
			BUILD_DSM_OPCODE(BIOS_EMULATED_COMMAND, SUBOP_GET_PAYLOAD_SIZE), 128,
			sizeof (struct pt_bios_get_size))) == NULL)
		{
			rc = NVM_ERR_DRIVERFAILED;
			COMMON_LOG_ERROR("Failed to get vendor command from driver");
		}
		else
		{
			if (((rc = linux_err_to_nvm_lib_err(ndctl_cmd_submit(
				p_vendor_cmd))) == NVM_SUCCESS) &&
				((rc = dsm_err_to_nvm_lib_err(ndctl_cmd_get_firmware_status(
				p_vendor_cmd))) == NVM_SUCCESS))
			{
				NVM_SIZE return_size = ndctl_cmd_vendor_get_output(p_vendor_cmd,
					p_bios_mb_size, sizeof (struct pt_bios_get_size));
				if (return_size != sizeof (struct pt_bios_get_size))
				{
					rc = NVM_ERR_DRIVERFAILED;
					COMMON_LOG_ERROR("Small Payload returned less data than requested");
				}
			}
			ndctl_cmd_unref(p_vendor_cmd);
		}
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Populate the emulated bios large input mailbox
 */
int bios_write_large_payload(struct ndctl_dimm *p_dimm, struct fw_cmd *p_fw_cmd)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct pt_bios_get_size mb_size;

	if (!p_dimm)
	{
		COMMON_LOG_ERROR("Invalid parameter, Dimm is null");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = bios_get_payload_size(p_dimm, &mb_size)) == NVM_SUCCESS)
	{
		if (mb_size.large_input_payload_size < p_fw_cmd->large_input_payload_size)
		{
			rc = NVM_ERR_BADSIZE;
		}
		else
		{
			unsigned int transfer_size = mb_size.rw_size;
			unsigned int current_offset = 0;

			while (current_offset < p_fw_cmd->large_input_payload_size &&
					rc == NVM_SUCCESS)
			{
				if ((current_offset + mb_size.rw_size) > p_fw_cmd->large_input_payload_size)
				{
					transfer_size = p_fw_cmd->large_input_payload_size - current_offset;
				}

				BIOS_INPUT(bios_input_payload, transfer_size);
				struct bios_input_payload *p_dsm_input = calloc(1,
					sizeof (struct bios_input_payload));
				if (p_dsm_input)
				{
					struct ndctl_cmd *p_vendor_cmd = NULL;
					if ((p_vendor_cmd = ndctl_dimm_cmd_new_vendor_specific(
							p_dimm, BUILD_DSM_OPCODE(BIOS_EMULATED_COMMAND,
							SUBOP_WRITE_LARGE_PAYLOAD_INPUT),
							sizeof (*p_dsm_input), 0)) == NULL)
					{
						COMMON_LOG_ERROR("Failed to get vendor command from driver");
						rc = NVM_ERR_DRIVERFAILED;
					}
					else
					{
						p_dsm_input->size = transfer_size;
						p_dsm_input->offset = current_offset;

						memmove(p_dsm_input->buffer,
							p_fw_cmd->large_input_payload + current_offset,
							sizeof (p_dsm_input->buffer));

						NVM_SIZE bytes_written = ndctl_cmd_vendor_set_input(
							p_vendor_cmd, p_dsm_input, sizeof (*p_dsm_input));

						if (bytes_written != sizeof (*p_dsm_input))
						{
							COMMON_LOG_ERROR("Failed to write input payload");
							rc = NVM_ERR_DRIVERFAILED;
						}
						else if (((rc = linux_err_to_nvm_lib_err(ndctl_cmd_submit(
								p_vendor_cmd))) == NVM_SUCCESS) &&
								((rc = dsm_err_to_nvm_lib_err(
								ndctl_cmd_get_firmware_status(p_vendor_cmd))) == NVM_SUCCESS))
						{
							current_offset += transfer_size;
						}

						ndctl_cmd_unref(p_vendor_cmd);
					}
					free(p_dsm_input);
				}
				else
				{
					COMMON_LOG_ERROR("Failed to allocate memory for BIOS input payload");
					rc = NVM_ERR_NOMEMORY;
				}
			} // end while

			if (current_offset != p_fw_cmd->large_input_payload_size)
			{
				COMMON_LOG_ERROR("Failed to write large payload");
				rc = NVM_ERR_UNKNOWN;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Read the emulated bios large output mailbox
 */
int bios_read_large_payload(struct ndctl_dimm *p_dimm, struct fw_cmd *p_fw_cmd)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct pt_bios_get_size mb_size;

	if (!p_dimm)
	{
		COMMON_LOG_ERROR("Invalid parameter, Dimm is null");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = bios_get_payload_size(p_dimm, &mb_size)) == NVM_SUCCESS)
	{
		if (mb_size.large_output_payload_size < p_fw_cmd->large_output_payload_size)
		{
			rc = NVM_ERR_BADSIZE;
		}
		else
		{
			unsigned int transfer_size = mb_size.rw_size;
			unsigned int current_offset = 0;
			rc = NVM_SUCCESS;
			while (current_offset < p_fw_cmd->large_output_payload_size &&
					rc == NVM_SUCCESS)
			{
				if ((current_offset + mb_size.rw_size) > p_fw_cmd->large_output_payload_size)
				{
					transfer_size = p_fw_cmd->large_output_payload_size - current_offset;
				}

				BIOS_INPUT(bios_input_payload, 0);
				struct bios_input_payload *p_dsm_input =
					calloc(1, sizeof (struct bios_input_payload));
				if (p_dsm_input)
				{
					struct ndctl_cmd *p_vendor_cmd = NULL;
					if ((p_vendor_cmd = ndctl_dimm_cmd_new_vendor_specific(p_dimm,
						BUILD_DSM_OPCODE(BIOS_EMULATED_COMMAND, SUBOP_READ_LARGE_PAYLOAD_OUTPUT),
						sizeof (*p_dsm_input), transfer_size)) == NULL)
					{
						rc = NVM_ERR_DRIVERFAILED;
						COMMON_LOG_ERROR("Failed to get vendor command from driver");
					}
					else
					{
						p_dsm_input->size = transfer_size;
						p_dsm_input->offset = current_offset;

						NVM_SIZE bytes_written = ndctl_cmd_vendor_set_input(
							p_vendor_cmd, p_dsm_input, sizeof (*p_dsm_input));
						if (bytes_written != sizeof (*p_dsm_input))
						{
							COMMON_LOG_ERROR("Failed to write input payload");
							rc = NVM_ERR_DRIVERFAILED;
						}

						if (((rc = linux_err_to_nvm_lib_err(ndctl_cmd_submit(p_vendor_cmd)))
							== NVM_SUCCESS) && ((rc = dsm_err_to_nvm_lib_err(
								ndctl_cmd_get_firmware_status(p_vendor_cmd))) == NVM_SUCCESS))
						{
							NVM_SIZE return_size = ndctl_cmd_vendor_get_output(p_vendor_cmd,
								p_fw_cmd->large_output_payload + current_offset, transfer_size);
							if (return_size != transfer_size)
							{
								rc = NVM_ERR_DRIVERFAILED;
								COMMON_LOG_ERROR("Large Payload returned less data than requested");
							}
							else
							{
								current_offset += transfer_size;
							}
						}
						ndctl_cmd_unref(p_vendor_cmd);
					}
					free(p_dsm_input);
				}
				else
				{
					COMMON_LOG_ERROR("Failed to allocate memory for BIOS payload");
					rc = NVM_ERR_NOMEMORY;
				}
			}  // end while

			if (current_offset != p_fw_cmd->large_output_payload_size)
			{
				COMMON_LOG_ERROR("Failed to read large payload");
				rc = NVM_ERR_UNKNOWN;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_dimm_by_handle(struct ndctl_ctx *ctx, unsigned int handle, struct ndctl_dimm **dimm)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_DRIVERFAILED;
	struct ndctl_dimm *target_dimm;
	*dimm = NULL;

	struct ndctl_bus *bus;
	ndctl_bus_foreach(ctx, bus)
	{
		target_dimm = ndctl_dimm_get_by_handle(bus, handle);

		if (target_dimm)
		{
			*dimm = target_dimm;
			rc = NVM_SUCCESS;
			break;
		}
	}

	if (*dimm == NULL)
	{
		COMMON_LOG_ERROR("Failed to get DIMM from driver");
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Execute a passthrough IOCTL
 */
int ioctl_passthrough_cmd(struct fw_cmd *p_fw_cmd)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	struct ndctl_ctx *ctx;
	// check input parameters
	if (p_fw_cmd == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, cmd struct is null");
		rc = NVM_ERR_UNKNOWN;
	}
	else if ((p_fw_cmd->input_payload_size > 0 && p_fw_cmd->input_payload == NULL) ||
			(p_fw_cmd->input_payload != NULL && p_fw_cmd->input_payload_size == 0) ||
			(p_fw_cmd->output_payload_size > 0 && p_fw_cmd->output_payload == NULL) ||
			(p_fw_cmd->output_payload != NULL && p_fw_cmd->output_payload_size == 0) ||
			(p_fw_cmd->large_input_payload_size > 0 && p_fw_cmd->large_input_payload == NULL) ||
			(p_fw_cmd->large_input_payload != NULL && p_fw_cmd->large_input_payload_size == 0) ||
			(p_fw_cmd->large_output_payload_size > 0 && p_fw_cmd->large_output_payload == NULL) ||
			(p_fw_cmd->large_output_payload != NULL && p_fw_cmd->large_output_payload_size == 0))
	{
		COMMON_LOG_ERROR("Invalid input or output payloads specified");
		rc = NVM_ERR_UNKNOWN;
	}
#if __LARGE_PAYLOAD__ == 0
	else if ((p_fw_cmd->opcode == 0x08 && p_fw_cmd->sub_opcode == 0x02) || // get fw debug log
				(p_fw_cmd->opcode == 0x0A)) // inject error
	{
		COMMON_LOG_ERROR_F("Intel DIMM Gen 1 FW command OpCode: 0x%x SubOpCode: "
				"0x%x is not supported",
				p_fw_cmd->opcode, p_fw_cmd->sub_opcode);
		rc = NVM_ERR_NOTSUPPORTED;
	}
#endif
	else if ((rc = ndctl_new(&ctx)) < 0)
	{
		COMMON_LOG_ERROR("Failed to retrieve ctx");
		rc = linux_err_to_nvm_lib_err(rc);
	}
	else
	{
		struct ndctl_dimm *p_dimm = NULL;
		if ((rc = get_dimm_by_handle(ctx, p_fw_cmd->device_handle, &p_dimm)) == NVM_SUCCESS)
		{
			unsigned int opcode = BUILD_DSM_OPCODE(p_fw_cmd->opcode, p_fw_cmd->sub_opcode);
			struct ndctl_cmd *p_vendor_cmd = NULL;
			if ((p_vendor_cmd = ndctl_dimm_cmd_new_vendor_specific(
					p_dimm, opcode, DEV_SMALL_PAYLOAD_SIZE,
					DEV_SMALL_PAYLOAD_SIZE)) == NULL)
			{
				rc = NVM_ERR_DRIVERFAILED;
				COMMON_LOG_ERROR("Failed to get vendor command from driver");
			}
			else
			{
				if (p_fw_cmd->input_payload_size > 0)
				{
					NVM_SIZE bytes_written = ndctl_cmd_vendor_set_input(p_vendor_cmd,
						p_fw_cmd->input_payload, p_fw_cmd->input_payload_size);

					if (bytes_written != p_fw_cmd->input_payload_size)
					{
						COMMON_LOG_ERROR("Failed to write input payload");
						rc = NVM_ERR_DRIVERFAILED;
					}
				}

				if (rc == NVM_SUCCESS && p_fw_cmd->large_input_payload_size > 0)
				{
					rc = bios_write_large_payload(p_dimm, p_fw_cmd);
				}

				if (rc == NVM_SUCCESS && ((rc = linux_err_to_nvm_lib_err(
						ndctl_cmd_submit(p_vendor_cmd))) == NVM_SUCCESS) &&
						((rc = dsm_err_to_nvm_lib_err(
						ndctl_cmd_get_firmware_status(p_vendor_cmd))) == NVM_SUCCESS))
				{
					if (p_fw_cmd->output_payload_size > 0)
					{
						ndctl_cmd_vendor_get_output(p_vendor_cmd,
							p_fw_cmd->output_payload,
							p_fw_cmd->output_payload_size);
					}

					if (p_fw_cmd->large_output_payload_size > 0)
					{
						rc = bios_read_large_payload(p_dimm, p_fw_cmd);
					}
				}
				ndctl_cmd_unref(p_vendor_cmd);
			}
		}
		ndctl_unref(ctx);
	}

	s_memset(&p_fw_cmd, sizeof (p_fw_cmd));
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
