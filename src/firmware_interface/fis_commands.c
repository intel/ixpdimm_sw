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

#include "fis_commands.h"

#include <common/string/s_str.h>
#include <driver_interface/passthrough.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned int fis_identify_dimm(const unsigned int device_handle, struct pt_output_identify_dimm *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x01;
	cmd.sub_opcode = 0x00;
	cmd.output_payload_size = sizeof (struct pt_output_identify_dimm);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_identify_dimm_characteristics(const unsigned int device_handle, struct pt_output_identify_dimm_characteristics *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x01;
	cmd.sub_opcode = 0x01;
	cmd.output_payload_size = sizeof (struct pt_output_identify_dimm_characteristics);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_get_security_state(const unsigned int device_handle, struct pt_output_get_security_state *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x02;
	cmd.sub_opcode = 0x00;
	cmd.output_payload_size = sizeof (struct pt_output_get_security_state);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_set_passphrase(const unsigned int device_handle, struct pt_input_set_passphrase *p_input_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x03;
	cmd.sub_opcode = 0xF1;
	cmd.input_payload = p_input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_set_passphrase);
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_disable_passphrase(const unsigned int device_handle, struct pt_input_disable_passphrase *p_input_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x03;
	cmd.sub_opcode = 0xF2;
	cmd.input_payload = p_input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_disable_passphrase);
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_unlock_unit(const unsigned int device_handle, struct pt_input_unlock_unit *p_input_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x03;
	cmd.sub_opcode = 0xF3;
	cmd.input_payload = p_input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_unlock_unit);
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_secure_erase(const unsigned int device_handle, struct pt_input_secure_erase *p_input_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x03;
	cmd.sub_opcode = 0xF5;
	cmd.input_payload = p_input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_secure_erase);
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_freeze_lock(const unsigned int device_handle)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x03;
	cmd.sub_opcode = 0xF6;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_get_alarm_threshold(const unsigned int device_handle, struct pt_output_get_alarm_threshold *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x04;
	cmd.sub_opcode = 0x01;
	cmd.output_payload_size = sizeof (struct pt_output_get_alarm_threshold);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_power_management_policy(const unsigned int device_handle, struct pt_output_power_management_policy *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x04;
	cmd.sub_opcode = 0x02;
	cmd.output_payload_size = sizeof (struct pt_output_power_management_policy);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_die_sparing_policy(const unsigned int device_handle, struct pt_output_die_sparing_policy *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x04;
	cmd.sub_opcode = 0x03;
	cmd.output_payload_size = sizeof (struct pt_output_die_sparing_policy);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_address_range_scrub(const unsigned int device_handle, struct pt_output_address_range_scrub *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x04;
	cmd.sub_opcode = 0x04;
	cmd.output_payload_size = sizeof (struct pt_output_address_range_scrub);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_optional_configuration_data_policy(const unsigned int device_handle, struct pt_output_optional_configuration_data_policy *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x04;
	cmd.sub_opcode = 0x06;
	cmd.output_payload_size = sizeof (struct pt_output_optional_configuration_data_policy);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_pmon_registers(const unsigned int device_handle, struct pt_input_pmon_registers *p_input_payload, struct pt_output_pmon_registers *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x04;
	cmd.sub_opcode = 0x07;
	cmd.output_payload_size = sizeof (struct pt_output_pmon_registers);
	cmd.output_payload = p_output_payload;
	cmd.input_payload = p_input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_pmon_registers);
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_set_alarm_threshold(const unsigned int device_handle, struct pt_input_set_alarm_threshold *p_input_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x05;
	cmd.sub_opcode = 0x01;
	cmd.input_payload = p_input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_set_alarm_threshold);
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_system_time(const unsigned int device_handle, struct pt_output_system_time *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x00;
	cmd.output_payload_size = sizeof (struct pt_output_system_time);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_platform_config_data(const unsigned int device_handle, struct pt_input_platform_config_data *p_input_payload, struct pt_output_platform_config_data *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x01;
	cmd.large_output_payload_size = sizeof (struct pt_output_platform_config_data);
	cmd.large_output_payload = p_output_payload;
	cmd.input_payload = p_input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_platform_config_data);
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_dimm_partition_info(const unsigned int device_handle, struct pt_output_dimm_partition_info *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x02;
	cmd.output_payload_size = sizeof (struct pt_output_dimm_partition_info);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_fw_debug_log_level(const unsigned int device_handle, struct pt_input_fw_debug_log_level *p_input_payload, struct pt_output_fw_debug_log_level *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x03;
	cmd.output_payload_size = sizeof (struct pt_output_fw_debug_log_level);
	cmd.output_payload = p_output_payload;
	cmd.input_payload = p_input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_fw_debug_log_level);
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_fw_load_flag(const unsigned int device_handle, struct pt_output_fw_load_flag *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x04;
	cmd.output_payload_size = sizeof (struct pt_output_fw_load_flag);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_config_lockdown(const unsigned int device_handle, struct pt_output_config_lockdown *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x05;
	cmd.output_payload_size = sizeof (struct pt_output_config_lockdown);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_ddrt_io_init_info(const unsigned int device_handle, struct pt_output_ddrt_io_init_info *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x06;
	cmd.output_payload_size = sizeof (struct pt_output_ddrt_io_init_info);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_get_supported_sku_features(const unsigned int device_handle, struct pt_output_get_supported_sku_features *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x07;
	cmd.output_payload_size = sizeof (struct pt_output_get_supported_sku_features);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_enable_dimm(const unsigned int device_handle, struct pt_output_enable_dimm *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x08;
	cmd.output_payload_size = sizeof (struct pt_output_enable_dimm);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_smart_health_info(const unsigned int device_handle, struct pt_output_smart_health_info *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x08;
	cmd.sub_opcode = 0x00;
	cmd.output_payload_size = sizeof (struct pt_output_smart_health_info);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_firmware_image_info(const unsigned int device_handle, struct pt_output_firmware_image_info *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x08;
	cmd.sub_opcode = 0x01;
	cmd.output_payload_size = sizeof (struct pt_output_firmware_image_info);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_firmware_debug_log(const unsigned int device_handle, struct pt_input_firmware_debug_log *p_input_payload, struct pt_output_firmware_debug_log *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x08;
	cmd.sub_opcode = 0x02;
	cmd.output_payload_size = sizeof (struct pt_output_firmware_debug_log);
	cmd.output_payload = p_output_payload;
	cmd.input_payload = p_input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_firmware_debug_log);
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_long_operation_status(const unsigned int device_handle, struct pt_output_long_operation_status *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0x08;
	cmd.sub_opcode = 0x04;
	cmd.output_payload_size = sizeof (struct pt_output_long_operation_status);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

unsigned int fis_bsr(const unsigned int device_handle, struct pt_output_bsr *p_output_payload)
{
	struct pt_fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = 0xFD;
	cmd.sub_opcode = 0x03;
	cmd.output_payload_size = sizeof (struct pt_output_bsr);
	cmd.output_payload = p_output_payload;
	return pt_ioctl_cmd(&cmd);
}

void fis_get_error_message(unsigned int code, char *message, size_t message_size)
{
	switch (code)
	{
	case FIS_ERR_SUCCESS:
		s_strcpy(message, "Success", message_size);
		break;
	case FIS_ERR_INVALID_COMMAND_PARAMETER:
		s_strcpy(message, "Invalid Command Parameter", message_size);
		break;
	case FIS_ERR_DATA_TRANSFER_ERROR:
		s_strcpy(message, "Data Transfer Error", message_size);
		break;
	case FIS_ERR_INTERNAL_DEVICE_ERROR:
		s_strcpy(message, "Internal Device Error", message_size);
		break;
	case FIS_ERR_UNSUPPORTED_COMMAND:
		s_strcpy(message, "Unsupported Command", message_size);
		break;
	case FIS_ERR_DEVICE_BUSY:
		s_strcpy(message, "Device Busy", message_size);
		break;
	case FIS_ERR_INCORRECT_SECURITY_NONCE:
		s_strcpy(message, "Incorrect Security Nonce", message_size);
		break;
	case FIS_ERR_FW_AUTHENTICATION_FAILED:
		s_strcpy(message, "FW Authentication Failed", message_size);
		break;
	case FIS_ERR_INVALID_SECURITY_STATE:
		s_strcpy(message, "Invalid Security State", message_size);
		break;
	case FIS_ERR_SYSTEM_TIME_NOT_SET:
		s_strcpy(message, "System Time Not Set", message_size);
		break;
	case FIS_ERR_DATA_NOT_SET:
		s_strcpy(message, "Data Not Set", message_size);
		break;
	case FIS_ERR_ABORTED:
		s_strcpy(message, "Aborted", message_size);
		break;
	case FIS_ERR_NO_NEW_FW_TO_EXECUTE:
		s_strcpy(message, "No New FW to Execute", message_size);
		break;
	case FIS_ERR_REVISION_FAILURE:
		s_strcpy(message, "Revision Failure", message_size);
		break;
	case FIS_ERR_INJECTION_NOT_ENABLED:
		s_strcpy(message, "Injection Not Enabled", message_size);
		break;
	case FIS_ERR_CONFIG_LOCKED:
		s_strcpy(message, "Config Locked", message_size);
		break;
	case FIS_ERR_INVALID_ALIGNMENT:
		s_strcpy(message, "Invalid Alignment", message_size);
		break;
	case FIS_ERR_INCOMPATIBLE_DIMM_TYPE:
		s_strcpy(message, "Incompatible DIMM Type", message_size);
		break;
	case FIS_ERR_TIMEOUT_OCCURRED:
		s_strcpy(message, "Timeout Occurred", message_size);
		break;
	case FIS_ERR_RESERVED:
		s_strcpy(message, "Reserved", message_size);
		break;
	case FIS_ERR_MEDIA_DISABLED:
		s_strcpy(message, "Media Disabled", message_size);
		break;
	case FIS_ERR_FW_UPDATE_ALREADY_OCCURED:
		s_strcpy(message, "FW Update Already Occured", message_size);
		break;
	case FIS_ERR_NO_RESOURCES_AVAILABLE:
		s_strcpy(message, "No Resources Available", message_size);
		break;
	default:
		s_strcpy(message, "Unknown error code", message_size);
	}
}