/*
 * Copyright (c) 2015, Intel Corporation
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
 * Functions to interface with the FW (pass-through IOCTLS)
 */

#include "device_fw.h"
#include "device_adapter.h"

// send a pass-through command to get the current alarm thresholds
int fw_get_alarm_thresholds(NVM_UINT32 const device_handle,
		struct pt_payload_alarm_thresholds *p_thresholds)
{
	COMMON_LOG_ENTRY();
	memset(p_thresholds, 0, sizeof (*p_thresholds));
	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_GET_FEATURES;
	cmd.sub_opcode = SUBOP_ALARM_THRESHOLDS;
	cmd.output_payload_size = sizeof ((*p_thresholds));
	cmd.output_payload = p_thresholds;

	int rc = ioctl_passthrough_cmd(&cmd);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

// send a pass-through command to set the current alarm thresholds
int fw_set_alarm_thresholds(NVM_UINT32 const device_handle,
		struct pt_payload_alarm_thresholds *p_thresholds)
{
	COMMON_LOG_ENTRY();

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_SET_FEATURES;
	cmd.sub_opcode = SUBOP_ALARM_THRESHOLDS;
	cmd.input_payload_size = sizeof (*p_thresholds);
	cmd.input_payload = p_thresholds;

	int rc = ioctl_passthrough_cmd(&cmd);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

// send a pass-through command to get the smart health logs
int fw_get_smart_health(const NVM_UINT32 device_handle, struct pt_payload_smart_health
		*p_dimm_smart)
{
	COMMON_LOG_ENTRY();
	memset(p_dimm_smart, 0, sizeof (*p_dimm_smart));

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_GET_LOG;
	cmd.sub_opcode = SUBOP_SMART_HEALTH;
	cmd.output_payload_size = sizeof ((*p_dimm_smart));
	cmd.output_payload = p_dimm_smart;

	int rc = ioctl_passthrough_cmd(&cmd);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

// send a pass-through command to get a specified memory info page log
int fw_get_memory_info_page(const NVM_UINT32 device_handle, const unsigned char page_num,
	void *p_memory_info_page, const unsigned int page_size)
{
	COMMON_LOG_ENTRY();
	memset(p_memory_info_page, 0, page_size);

	struct pt_payload_input_memory_info mem_input;
	memset(&mem_input, 0, sizeof (mem_input));
	mem_input.memory_page = page_num;

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_GET_LOG;
	cmd.sub_opcode = SUBOP_MEM_INFO;
	cmd.input_payload_size = sizeof (mem_input);
	cmd.input_payload = &mem_input;
	cmd.output_payload_size = page_size;
	cmd.output_payload = p_memory_info_page;

	int rc = ioctl_passthrough_cmd(&cmd);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

// send a pass-through command to get the fw error log count
int fw_get_fw_error_log_count(const NVM_UINT32 device_handle,
	const unsigned char log_level,
	const unsigned char log_type)
{
	COMMON_LOG_ENTRY();
	struct pt_input_payload_fw_error_log input_payload;
	memset(&input_payload, 0, sizeof (input_payload));
	input_payload.params = log_level | log_type
		| DEV_FW_ERR_LOG_RETRIEVE_ENTRIES | DEV_FW_ERR_LOG_SMALL_PAYLOAD;
	// '0' is used to request the count of the logs without getting the logs
	input_payload.request_count = 0;

	struct pt_output_payload_fw_error_log output_payload;
	memset(&output_payload, 0, sizeof (output_payload));

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));

	cmd.device_handle = device_handle;
	cmd.opcode = PT_GET_LOG;
	cmd.sub_opcode = SUBOP_ERROR_LOG;
	cmd.input_payload_size = sizeof (input_payload);
	cmd.input_payload = &input_payload;
	cmd.output_payload_size = sizeof (output_payload);
	cmd.output_payload = &output_payload;

	int rc = ioctl_passthrough_cmd(&cmd);
	if (rc == NVM_SUCCESS)
	{
		rc = output_payload.number_total_entries;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int fw_get_fw_error_logs(const NVM_UINT32 device_handle,
		const unsigned int error_count,
		NVM_UINT8 *p_large_buffer,
		const unsigned char log_level,
		const unsigned char log_type)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct pt_input_payload_fw_error_log input;
	memset(&input, 0, sizeof (input));
	input.offset = 0;
	struct pt_output_payload_fw_error_log fw_error_log;
	memset(&fw_error_log, 0, sizeof (fw_error_log));

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_GET_LOG;
	cmd.sub_opcode = SUBOP_ERROR_LOG;
	cmd.input_payload_size = sizeof (input);
	cmd.input_payload = &input;

	input.params = log_level | log_type
			| DEV_FW_ERR_LOG_RETRIEVE_ENTRIES | DEV_FW_ERR_LOG_LARGE_PAYLOAD;
	input.request_count = error_count;
	if (log_type == DEV_FW_ERR_LOG_MEDIA)
	{
		cmd.large_output_payload_size = error_count * sizeof (struct pt_fw_media_log_entry);
	}
	else
	{
		cmd.large_output_payload_size = error_count * sizeof (struct pt_fw_thermal_log_entry);
	}
	cmd.large_output_payload = p_large_buffer;
	rc = ioctl_passthrough_cmd(&cmd);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

// get the security state using a pass through ioctl
int fw_get_security_state(const NVM_UINT32 device_handle,
	struct pt_payload_get_security_state *p_security_state)
{
	COMMON_LOG_ENTRY();
	memset(p_security_state, 0, sizeof (*p_security_state));

	struct fw_cmd sec_cmd;
	memset(&sec_cmd, 0, sizeof (struct fw_cmd));
	sec_cmd.device_handle = device_handle;
	sec_cmd.opcode = PT_GET_SEC_INFO;
	sec_cmd.sub_opcode = SUBOP_GET_SEC_STATE;
	sec_cmd.output_payload_size = sizeof (*p_security_state);
	sec_cmd.output_payload = p_security_state;
	int rc = ioctl_passthrough_cmd(&sec_cmd);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Note: For the celsius conversion functions - FW represents celsius as a 16 bit value
 * 		bit 16 = sign value
 * 		bits 14 - 4 = exponent
 * 		bits 3 - 0 = fraction (.0625 resolution)
 */
float fw_convert_fw_celsius_to_float(unsigned short fw_celsius)
{
	COMMON_LOG_ENTRY();
	unsigned char is_negative = (unsigned char)(fw_celsius >> 15);
	unsigned short digit = (unsigned short)((fw_celsius >> 4) & 0x7FF);
	unsigned char fraction = (unsigned char)((fw_celsius >> 0) & 0xF);

	float celsius = digit;

	int divisor = 2;
	for (int i = 3; i >= 0; i--)
	{
		unsigned char fraction_bit = (unsigned char)((fraction >> i) & 1);
		float decimal = (float)fraction_bit / divisor;
		celsius += decimal;
		divisor *= 2;
	}

	if (is_negative)
	{
		celsius *= -1;
	}

	COMMON_LOG_EXIT();
	return celsius;
}

unsigned short fw_convert_float_to_fw_celsius(float celsius)
{
	COMMON_LOG_ENTRY();
	unsigned short digit = (short)celsius;
	unsigned short fraction = 0;
	unsigned short is_negative = 0;

	float left = celsius - digit;

	if (celsius < 0)
	{
		is_negative = 1;
		left *= -1;
	}

	for (int i = 3; i >= 0; i--)
	{
		left *= 2;
		if (left >= 1.0f)
		{
			fraction |= 1 << i;
			left -= (int)left;
		}
	}

	unsigned short fw_celsius =
			is_negative << 15 |
			(digit << 4) |
			fraction;

	COMMON_LOG_EXIT();
	return fw_celsius;
}
