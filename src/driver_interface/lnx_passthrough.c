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
#include "passthrough.h"
#include <string.h>
#include <stdlib.h>
#include <ndctl/libndctl.h>
#include <asm/errno.h>
#include <stdio.h>

#define    BIOS_INPUT(NAME, IN_LEN)     \
struct NAME {                           \
    unsigned int size;                  \
    unsigned int offset;                \
    unsigned char  buffer[IN_LEN];      \
}

/*
 * Execute an emulated BIOS ioctl to retrieve information about the bios large mailboxes
 */
int bios_get_payload_size(struct ndctl_dimm *p_dimm, struct pt_bios_get_size *p_bios_mb_size)
{
	int rc;

	memset(p_bios_mb_size, 0, sizeof(*p_bios_mb_size));
	struct ndctl_cmd *p_vendor_cmd = NULL;
	if ((p_vendor_cmd = ndctl_dimm_cmd_new_vendor_specific(p_dimm,
		BUILD_DSM_OPCODE(BIOS_EMULATED_COMMAND, SUBOP_GET_PAYLOAD_SIZE), 128,
		sizeof(struct pt_bios_get_size))) == NULL)
	{
		rc = PT_ERR_DRIVERFAILED;
	}
	else
	{
		if (((rc = ndctl_cmd_submit(p_vendor_cmd)) == PT_SUCCESS) &&
			((rc = ndctl_cmd_get_firmware_status(p_vendor_cmd))) == PT_SUCCESS)
		{
			ssize_t return_size = ndctl_cmd_vendor_get_output(p_vendor_cmd,
				p_bios_mb_size, sizeof(struct pt_bios_get_size));
			if (return_size != sizeof(struct pt_bios_get_size))
			{
				rc = PT_ERR_DRIVERFAILED;
			}
		}
		ndctl_cmd_unref(p_vendor_cmd);
	}
		return rc;
}

/*
 * Populate the emulated bios large input mailbox
 */
int bios_write_large_payload(struct ndctl_dimm *p_dimm, struct fw_cmd *p_fw_cmd)
{
	int rc = PT_SUCCESS;
	struct pt_bios_get_size mb_size;

	if ((rc = bios_get_payload_size(p_dimm, &mb_size)) == PT_SUCCESS)
	{
		if (mb_size.large_input_payload_size < p_fw_cmd->large_input_payload_size)
		{
			rc = PT_ERR_UNKNOWN;
		}
		else
		{
			unsigned int transfer_size = mb_size.rw_size;
			unsigned int current_offset = 0;

			while (current_offset < p_fw_cmd->large_input_payload_size && rc == PT_SUCCESS)
			{
				if ((current_offset + mb_size.rw_size) > p_fw_cmd->large_input_payload_size)
				{
					transfer_size = p_fw_cmd->large_input_payload_size - current_offset;
				}

				BIOS_INPUT(bios_input_payload, transfer_size);
				struct bios_input_payload *p_dsm_input = calloc(1,
					sizeof(struct bios_input_payload));
				if (p_dsm_input)
				{
					struct ndctl_cmd *p_vendor_cmd = NULL;
					unsigned int opcode = BUILD_DSM_OPCODE(BIOS_EMULATED_COMMAND,
						SUBOP_WRITE_LARGE_PAYLOAD_INPUT);
					p_vendor_cmd = ndctl_dimm_cmd_new_vendor_specific(p_dimm, opcode,
						sizeof(*p_dsm_input), 0);
					if (p_vendor_cmd == NULL)
					{
						rc = PT_ERR_DRIVERFAILED;
					}
					else
					{
						p_dsm_input->size = transfer_size;
						p_dsm_input->offset = current_offset;

						memmove(p_dsm_input->buffer,
							p_fw_cmd->large_input_payload + current_offset,
							sizeof(p_dsm_input->buffer));

						ssize_t bytes_written = ndctl_cmd_vendor_set_input(
							p_vendor_cmd, p_dsm_input, sizeof(*p_dsm_input));

						if (bytes_written != sizeof(*p_dsm_input))
						{
							rc = PT_ERR_DRIVERFAILED;
						}
						else if ((rc = ndctl_cmd_submit(p_vendor_cmd)) == PT_SUCCESS &&
								 (rc = ndctl_cmd_get_firmware_status(p_vendor_cmd)) == PT_SUCCESS)
						{
							current_offset += transfer_size;
						}

						ndctl_cmd_unref(p_vendor_cmd);
					}
					free(p_dsm_input);
				}
				else
				{
					rc = PT_ERR_NOMEMORY;
				}
			} // end while

			if (current_offset != p_fw_cmd->large_input_payload_size)
			{
				rc = PT_ERR_UNKNOWN;
			}
		}
	}

	return rc;
}

/*
 * Read the emulated bios large output mailbox
 */
int bios_read_large_payload(struct ndctl_dimm *p_dimm, struct fw_cmd *p_fw_cmd)
{
	int rc;
	struct pt_bios_get_size mb_size;

	if ((rc = bios_get_payload_size(p_dimm, &mb_size)) == PT_SUCCESS)
	{
		if (mb_size.large_output_payload_size < p_fw_cmd->large_output_payload_size)
		{
			rc = PT_ERR_UNKNOWN;
		}
		else
		{
			unsigned int transfer_size = mb_size.rw_size;
			unsigned int current_offset = 0;
			rc = PT_SUCCESS;
			while (current_offset < p_fw_cmd->large_output_payload_size &&
				   rc == PT_SUCCESS)
			{
				if ((current_offset + mb_size.rw_size) > p_fw_cmd->large_output_payload_size)
				{
					transfer_size = p_fw_cmd->large_output_payload_size - current_offset;
				}

				BIOS_INPUT(bios_input_payload, 0);
				struct bios_input_payload *p_dsm_input =
					calloc(1, sizeof(struct bios_input_payload));
				if (p_dsm_input)
				{
					struct ndctl_cmd *p_vendor_cmd = NULL;
					if ((p_vendor_cmd = ndctl_dimm_cmd_new_vendor_specific(p_dimm,
						BUILD_DSM_OPCODE(BIOS_EMULATED_COMMAND,
							SUBOP_READ_LARGE_PAYLOAD_OUTPUT),
						sizeof(*p_dsm_input), transfer_size)) == NULL)
					{
						rc = PT_ERR_DRIVERFAILED;
					}
					else
					{
						p_dsm_input->size = transfer_size;
						p_dsm_input->offset = current_offset;

						ssize_t bytes_written = ndctl_cmd_vendor_set_input(
							p_vendor_cmd, p_dsm_input, sizeof(*p_dsm_input));
						if (bytes_written != sizeof(*p_dsm_input))
						{
							rc = PT_ERR_DRIVERFAILED;
						}
						else
						{
							if (((rc = ndctl_cmd_submit(p_vendor_cmd)) == PT_SUCCESS) &&
								((rc = ndctl_cmd_get_firmware_status(p_vendor_cmd))) == PT_SUCCESS)
							{
								ssize_t return_size = ndctl_cmd_vendor_get_output(p_vendor_cmd,
									p_fw_cmd->large_output_payload + current_offset, transfer_size);
								if (return_size != transfer_size)
								{
									rc = PT_ERR_DRIVERFAILED;
								}
								else
								{
									current_offset += transfer_size;
								}
							}
						}

						ndctl_cmd_unref(p_vendor_cmd);
					}
					free(p_dsm_input);
				}
				else
				{
					rc = PT_ERR_NOMEMORY;
				}
			}  // end while

			if (current_offset != p_fw_cmd->large_output_payload_size)
			{
				rc = PT_ERR_UNKNOWN;
			}
		}
	}

	return rc;
}

int get_dimm_by_handle(struct ndctl_ctx *ctx, unsigned int handle, struct ndctl_dimm **dimm)
{
	int found = 0;
	struct ndctl_dimm *target_dimm;
	*dimm = NULL;

	struct ndctl_bus *bus;
	ndctl_bus_foreach(ctx, bus)
	{
		target_dimm = ndctl_dimm_get_by_handle(bus, handle);

		if (target_dimm)
		{
			*dimm = target_dimm;
			found = 1;
			break;
		}
	}

	if (!found)
	{
			}

	return found;
}

/*
 * Execute a passthrough IOCTL
 */
int ioctl_passthrough_cmd(struct fw_cmd *p_fw_cmd)
{
	int rc;
	struct ndctl_ctx *ctx;
	if ((rc = ndctl_new(&ctx)) >= 0)
	{
		struct ndctl_dimm *p_dimm = NULL;
		if (get_dimm_by_handle(ctx, p_fw_cmd->device_handle, &p_dimm))
		{
			unsigned int opcode = BUILD_DSM_OPCODE(p_fw_cmd->opcode, p_fw_cmd->sub_opcode);
			struct ndctl_cmd *p_vendor_cmd = NULL;
			if ((p_vendor_cmd = ndctl_dimm_cmd_new_vendor_specific(p_dimm, opcode,
				DEV_SMALL_PAYLOAD_SIZE, DEV_SMALL_PAYLOAD_SIZE)) == NULL)
			{
				rc = PT_ERR_DRIVERFAILED;
			}
			else
			{
				if (p_fw_cmd->input_payload_size > 0)
				{
					size_t bytes_written = (size_t) ndctl_cmd_vendor_set_input(p_vendor_cmd,
						p_fw_cmd->input_payload, p_fw_cmd->input_payload_size);

					if (bytes_written != p_fw_cmd->input_payload_size)
					{
						rc = PT_ERR_DRIVERFAILED;
					}
				}

				if (rc == PT_SUCCESS && p_fw_cmd->large_input_payload_size > 0)
				{
					rc = bios_write_large_payload(p_dimm, p_fw_cmd);
				}

				if (rc == PT_SUCCESS)
				{
					if ((rc = ndctl_cmd_submit(p_vendor_cmd)) < 0)
					{
											}
					else
					{
						if ((rc = ndctl_cmd_get_firmware_status(p_vendor_cmd)) != PT_SUCCESS)
						{
													}
						else
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
					}

				}
				ndctl_cmd_unref(p_vendor_cmd);
			}
		}
		else
		{
			rc = PT_ERR_BADDEVICEHANDLE;
		}
		ndctl_unref(ctx);
	}

	return rc;
}
