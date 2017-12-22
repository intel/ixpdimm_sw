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

#include "win_scm2_passthrough.h"
#include "win_scm2_ioctl_passthrough.h"
#include "win_scm2_adapter.h"

static int do_passthrough(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		unsigned char opcode, unsigned char sub_op_code,
		void *input_payload, unsigned int input_payload_size,
		void *output_payload, unsigned int output_payload_size);
static int do_passthrough_fix_output(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		unsigned char opcode, unsigned char sub_op_code,
		void *input_payload, unsigned int input_payload_size,
		void *output_payload, unsigned int output_payload_size);
static int get_large_payload_sizes(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		struct pt_bios_get_size *p_size);
static int read_large_ouptut_payload(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		void *large_output_payload, unsigned int large_output_payload_size);
static int write_large_input_payload(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		void *large_input_payload,
		unsigned int large_input_payload_size);

#define	NO_ERRORS(scm_err, p_dsm_status) ((scm_err == 0) && ((*p_dsm_status) == 0))
#define	IS_BIOS_EMULATED_COMMAND(opcode) (opcode) == 0xfd

int win_scm2_passthrough(struct fw_cmd *p_cmd, unsigned int *p_dsm_status)
{
	SCM_LOG_ENTRY();

	int scm_err = 0;
	*p_dsm_status = 0;
	scm_err = write_large_input_payload(scm_err, p_dsm_status,
			p_cmd->device_handle, p_cmd->large_input_payload, p_cmd->large_input_payload_size);

	scm_err = do_passthrough_fix_output(scm_err, p_dsm_status, p_cmd->device_handle,
			p_cmd->opcode, p_cmd->sub_opcode,
			p_cmd->input_payload, p_cmd->input_payload_size,
			p_cmd->output_payload, p_cmd->output_payload_size);

	scm_err = read_large_ouptut_payload(scm_err, p_dsm_status, p_cmd->device_handle,
			p_cmd->large_output_payload, p_cmd->large_output_payload_size);

	SCM_LOG_EXIT_RETURN_I(scm_err);
	return scm_err;
}

/*
 * For non-large_payload commands, the Windows driver always expects an output payload of
 * size 128, even if the FIS doesn't indicate there is one, or even if it's smaller than
 * 128 bytes. It should never be greater than 128 bytes.
 */
#define PAYLOAD_SIZE 128

static int do_passthrough_fix_output(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		unsigned char opcode, unsigned char sub_op_code,
		void *input_payload, unsigned int input_payload_size,
		void *output_payload, unsigned int output_payload_size)
{
	unsigned char tmp_output_payload[PAYLOAD_SIZE];
	memset(tmp_output_payload, 0, PAYLOAD_SIZE);

	scm_err = do_passthrough(scm_err, p_dsm_status,
			handle,
			opcode, sub_op_code,
			input_payload, input_payload_size,
			tmp_output_payload, PAYLOAD_SIZE);

	if (NO_ERRORS(scm_err, p_dsm_status))
	{
		unsigned int transfer_size = output_payload_size;
		if (transfer_size > PAYLOAD_SIZE)
		{
			SCM_LOG_ERROR_F("This should not have happened. "
					"output_payload_size(%d) > PAYLOAD_SIZE(%d)",
					output_payload_size, PAYLOAD_SIZE);
			transfer_size = PAYLOAD_SIZE;
		}

		memmove(output_payload, tmp_output_payload, transfer_size);
	}
	return scm_err;
}

static int do_passthrough(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		unsigned char opcode, unsigned char sub_op_code,
		void *input_payload, unsigned int input_payload_size,
		void *output_payload, unsigned int output_payload_size)
{
	SCM_LOG_ENTRY();

	if (NO_ERRORS(scm_err, p_dsm_status))
	{
			scm_err = win_scm2_ioctl_passthrough_cmd(handle,
					opcode, sub_op_code,
					input_payload, input_payload_size,
					output_payload, output_payload_size,
					p_dsm_status);


	}

	SCM_LOG_EXIT_RETURN_I(scm_err);
	return scm_err;
}

static int get_large_payload_sizes(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		struct pt_bios_get_size *p_size)
{
	SCM_LOG_ENTRY();

	if (NO_ERRORS(scm_err, p_dsm_status))
	{
		unsigned input_buffer[128];
		unsigned output_buffer[128];

		scm_err = do_passthrough(scm_err, p_dsm_status, handle,
				BIOS_EMULATED_COMMAND, SUBOP_GET_PAYLOAD_SIZE,
				input_buffer, 128,
				output_buffer, 128);
		memmove(p_size, output_buffer, sizeof (*p_size));
	}

	SCM_LOG_EXIT_RETURN_I(scm_err);
	return scm_err;
}

#define LARGE_PAYLOAD_SIZE 4096

static int write_large_input_payload(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		void *large_input_payload,
		unsigned int large_input_payload_size)
{
	SCM_LOG_ENTRY();

	if (NO_ERRORS(scm_err, p_dsm_status) && large_input_payload_size > 0)
	{
		struct pt_bios_get_size size;
		scm_err = get_large_payload_sizes(scm_err, p_dsm_status, handle, &size);
		if (!NO_ERRORS(scm_err, p_dsm_status))
		{
			SCM_LOG_ERROR_F("Unable to get the large payload size. SCM RC: %d, dsm_status: %d",
					scm_err, (*p_dsm_status));
		}
		else if (large_input_payload_size > size.large_input_payload_size)
		{
			SCM_LOG_ERROR("large_input_payload_size > size.large_input_payload_size");
			scm_err = WIN_SCM2_ERR_UNKNOWN;
		}
		else
		{
			// write to the large input payload
			unsigned int current_offset = 0;
			unsigned int total_transfer_size = large_input_payload_size;
			int i = 0;

			struct bios_input_payload
			{
				unsigned int bytes_to_transfer;
				unsigned int large_input_payload_offset;
                unsigned char buffer[LARGE_PAYLOAD_SIZE];
			};

			struct bios_input_payload *p_input_payload =
					malloc(sizeof (struct bios_input_payload));

			p_input_payload->bytes_to_transfer = size.rw_size;

			while (current_offset < total_transfer_size && NO_ERRORS(scm_err, p_dsm_status))
			{
				p_input_payload->large_input_payload_offset = current_offset;

				unsigned int transfer_size = size.rw_size;
				if (transfer_size + current_offset > total_transfer_size)
				{
					transfer_size = total_transfer_size - current_offset;
				}

				memmove(p_input_payload->buffer, (unsigned char *)large_input_payload + current_offset,
						sizeof (p_input_payload->buffer));

				scm_err = do_passthrough(scm_err, p_dsm_status, handle,
						BIOS_EMULATED_COMMAND, SUBOP_WRITE_LARGE_PAYLOAD_INPUT,
						p_input_payload, sizeof (*p_input_payload),
						NULL, 0);

				current_offset += transfer_size;
				i++;
			}

			if (!NO_ERRORS(scm_err, p_dsm_status))
			{
				SCM_LOG_ERROR_F("There was an error in the while loop on "
						"iteration %d. SCM Error: %d, DSM Status: %d", i, scm_err, *p_dsm_status);
			}
		}
	}

	SCM_LOG_EXIT_RETURN_I(scm_err);
	return scm_err;
}

static int read_large_ouptut_payload(int scm_err, unsigned int *p_dsm_status, unsigned int handle,
		void *large_output_payload, unsigned int large_output_payload_size)
{
	SCM_LOG_ENTRY();

	if (NO_ERRORS(scm_err, p_dsm_status) && large_output_payload_size > 0)
	{
		// get the sizes used for writing to the large payload
		struct pt_bios_get_size size;
		scm_err = get_large_payload_sizes(scm_err, p_dsm_status, handle, &size);
		if (!NO_ERRORS(scm_err, p_dsm_status))
		{
			SCM_LOG_ERROR_F("Unable to get the large payload size. RC: %d, dsm_status: %d",
					scm_err, *p_dsm_status);
		}
		else if (large_output_payload_size > size.large_input_payload_size)
		{
			scm_err = WIN_SCM2_ERR_UNKNOWN;
		}
		else
		{
			struct
			{
				unsigned int bytes_to_transfer;
				unsigned int large_output_payload_offset;
			} input_payload;
			input_payload.bytes_to_transfer = size.rw_size;
			unsigned int offset = 0;
			input_payload.large_output_payload_offset = offset;

			unsigned int total_transfer_size = large_output_payload_size;
			while (input_payload.large_output_payload_offset < total_transfer_size &&
				NO_ERRORS(scm_err, p_dsm_status))
			{
				if (input_payload.bytes_to_transfer + input_payload.large_output_payload_offset
					> total_transfer_size)
				{
					input_payload.bytes_to_transfer =
							total_transfer_size - input_payload.large_output_payload_offset;
				}

				unsigned char *buffer = malloc(input_payload.bytes_to_transfer);

				scm_err = do_passthrough(scm_err, p_dsm_status, handle,
						BIOS_EMULATED_COMMAND, SUBOP_READ_LARGE_PAYLOAD_OUTPUT,
						&input_payload, sizeof (input_payload),
						buffer,
						input_payload.bytes_to_transfer);

				memmove((unsigned char*)large_output_payload + offset, buffer, input_payload.bytes_to_transfer);
                
                free(buffer);

				input_payload.large_output_payload_offset += input_payload.bytes_to_transfer;
				offset += input_payload.bytes_to_transfer;
			}
		}
	}

	SCM_LOG_EXIT_RETURN_I(scm_err);
	return scm_err;
}
