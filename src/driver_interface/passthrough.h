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

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>

struct pt_fw_cmd
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

#define PT_IOCTL_SUCCESS(code) (code == PT_SUCCESS)

enum pt_ioctl_result
{
	PT_SUCCESS = 0,
	PT_ERR_UNKNOWN = 1,
	PT_ERR_BADDEVICE = 2,
	PT_ERR_BADSIZE = 2,
	PT_ERR_BADDEVICEHANDLE = 3,
	PT_ERR_BADSECURITY = 4,
	PT_ERR_DRIVERFAILED = 5,
	PT_ERR_DEVICEBUSY = 6,
	PT_ERR_INVALIDPERMISSIONS = 7,
	PT_ERR_NOMEMORY = 8,
};

#define	PT_IS_SUCCESS(result) (*((int*)&result) == 0)

#define PT_RESULT_ENCODE(result, code) memmove(&(code), &(result), sizeof(code))
#define PT_RESULT_DECODE(code, result) memmove(&(result), &(code), sizeof(code))

#define PT_DEV_SMALL_PAYLOAD_SIZE	128 /* 128B - Size for a passthrough command small payload */
#define PT_BUILD_DSM_OPCODE(opcode, subop_code) (unsigned int)(subop_code << 8 | opcode)

#define	PT_BIOS_EMULATED_COMMAND 0xFD
#define	PT_SUBOP_GET_PAYLOAD_SIZE 0x00
#define	PT_SUBOP_WRITE_LARGE_PAYLOAD_INPUT 0x01
#define	PT_SUBOP_READ_LARGE_PAYLOAD_OUTPUT 0x02
#define	PT_SUBOP_GET_BOOT_STATUS 0x03

struct pt_pt_bios_get_size
{
	unsigned int large_input_payload_size;
	unsigned int large_output_payload_size;
	unsigned int rw_size;
}__attribute__((packed));


typedef struct
{
	unsigned char func: 4;
	unsigned char driver: 4;
	unsigned char pt;
	unsigned char fw_status;
	unsigned char fw_ext_status;
} pt_result;

void pt_get_error_message(unsigned int code, char message[1024], size_t message_len);

unsigned int pt_ioctl_cmd(struct pt_fw_cmd *p_fw_cmd);

#ifdef __cplusplus
}
#endif

#endif //BPS_PROTOTYPE_PASSTHROUGH_H
