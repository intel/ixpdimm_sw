/*
 * Copyright (c) 2016, Intel Corporation
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
#ifndef BPS_PROTOTYPE_PASSTHROUGH_H
#define BPS_PROTOTYPE_PASSTHROUGH_H

#define	COMMON_LOG_ENTRY()
#define	COMMON_LOG_ERROR(error)
#define	COMMON_LOG_EXIT_RETURN_I(rc)
#define	COMMON_LOG_DEBUG_F(fmt, ...)
#define	COMMON_LOG_ERROR_F(fmt, ...)
#define	COMMON_LOG_DEBUG(str)
#define	COMMON_LOG_EXIT()

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>

struct fw_cmd
{
	unsigned int device_handle; // as reported by NFIT
	unsigned char opcode;
	unsigned char sub_opcode;
	unsigned int input_payload_size;
	void *input_payload;
	unsigned int output_payload_size;
	void *output_payload;
	unsigned int large_input_payload_size;
	void *large_input_payload;
	unsigned int large_output_payload_size;
	void *large_output_payload;
};

enum IOCTL_PASSTHROUGH_RESULT
{
	PT_SUCCESS = 0,
	PT_ERR_UNKNOWN = 1,
	PT_ERR_BADDEVICEHANDLE = 2,
	PT_ERR_NOMEMORY = 3,
	PT_ERR_DRIVERFAILED = 4,
};

#define	PT_RC_IS_SUCCESS(rc) ((rc) == 0)
#define SET_FUNC_RC(rc, code) rc |= ((code) << 0)
#define SET_DRIVER_RC(rc, code) rc |= ((code) << 4)
#define SET_PT_RC(rc, code) rc |= ((code) << 8)
#define SET_FW_STATUS(rc, code) rc |= ((code) << 16)
#define SET_FW_EXT_STATUS(rc, code) rc |= ((code) << 24)

#define DEV_SMALL_PAYLOAD_SIZE	128 /* 128B - Size for a passthrough command small payload */
#define BUILD_DSM_OPCODE(opcode, subop_code) (unsigned int)(subop_code << 8 | opcode)

#define	BIOS_EMULATED_COMMAND 0xFD
#define	SUBOP_GET_PAYLOAD_SIZE 0x00
#define	SUBOP_WRITE_LARGE_PAYLOAD_INPUT 0x01
#define	SUBOP_READ_LARGE_PAYLOAD_OUTPUT 0x02
#define	SUBOP_GET_BOOT_STATUS 0x03

struct pt_bios_get_size
{
	unsigned int large_input_payload_size;
	unsigned int large_output_payload_size;
	unsigned int rw_size;
}__attribute__((packed));

union pt_error
{
	unsigned int code;
	struct
	{
		unsigned char func: 4;
		unsigned char driver: 4;
		unsigned char ioctl:8;
		unsigned char fw_status;
		unsigned char fw_ext_status;
	} parts;
};

void pt_get_error_message(unsigned int code, char message[1024], size_t message_len);

int ioctl_passthrough_cmd(struct fw_cmd *p_fw_cmd);

#ifdef __cplusplus
}
#endif

#endif //BPS_PROTOTYPE_PASSTHROUGH_H
