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
 * Functions to interface with the FW (pass-through IOCTLS)
 */

#include "device_fw.h"
#include "device_adapter.h"
#include "utility.h"

// Intel DIMM Device IDs running FW we support
const int NUM_SUPPORTED_DEVICE_IDS = 2;
const NVM_UINT16 SUPPORTED_DEVICE_IDS[] = {
		0x979,
		0x97A
};

/*
 * Minimum supported version of FW API: 1.0
 */
#define	DEV_FW_API_VERSION_MAJOR_MIN	1
#define	DEV_FW_API_VERSION_MINOR_MIN	0

int local_ioctl_passthrough_cmd(struct fw_cmd *p_cmd);

NVM_BOOL is_fw_api_version_supported(const unsigned int major_version,
		const unsigned int minor_version)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL is_supported = 0;

	if (version_cmp(major_version, minor_version,
			DEV_FW_API_VERSION_MAJOR_MIN, DEV_FW_API_VERSION_MINOR_MIN) >= 0)
	{
		is_supported = 1;
	}

	COMMON_LOG_EXIT_RETURN_I(is_supported);
	return is_supported;
}

int fw_get_identify_dimm(const NVM_UINT32 device_handle,
		struct pt_payload_identify_dimm *p_id_dimm)
{
	COMMON_LOG_ENTRY();

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_IDENTIFY_DIMM;
	cmd.sub_opcode = 0;
	cmd.output_payload_size = sizeof (struct pt_payload_identify_dimm);
	cmd.output_payload = p_id_dimm;

	int rc = ioctl_passthrough_cmd(&cmd);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int fw_get_id_dimm_device_characteristics(unsigned int device_handle,
	struct pt_payload_device_characteristics *p_payload)
{
	COMMON_LOG_ENTRY();
	memset(p_payload, 0, sizeof (*p_payload));
	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_IDENTIFY_DIMM;
	cmd.sub_opcode = SUBOP_IDENTIFY_DIMM_CHARACTERISTICS;
	cmd.output_payload = p_payload;
	cmd.output_payload_size = sizeof (struct pt_payload_device_characteristics);

	int rc = local_ioctl_passthrough_cmd(&cmd);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

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
	cmd.output_payload_size = sizeof (*p_thresholds);
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

int fw_get_fw_error_log_info_data(const NVM_UINT32 device_handle,
		const unsigned char log_level,
		const unsigned char log_type,
		struct pt_payload_fw_log_info_data *p_log_info_data)
{
	int rc = NVM_SUCCESS;
	COMMON_LOG_ENTRY();

	struct pt_input_payload_fw_error_log input;
	memset(&input, 0, sizeof (input));
	input.offset = 0;
	input.params = log_level | log_type
		| DEV_FW_ERR_LOG_RETRIEVE_INFO_DATA | DEV_FW_ERR_LOG_SMALL_PAYLOAD;
	input.request_count = 0;

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_GET_LOG;
	cmd.sub_opcode = SUBOP_ERROR_LOG;
	cmd.input_payload_size = sizeof (input);
	cmd.input_payload = &input;
	cmd.output_payload_size = sizeof (*p_log_info_data);
	cmd.output_payload = p_log_info_data;

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

// get firmware image info using a passthrough ioctl
int fw_get_fw_image_info(const NVM_UINT32 device_handle,
		struct pt_payload_fw_image_info *p_fw_image_info)
{
	COMMON_LOG_ENTRY();
	memset(p_fw_image_info, 0, sizeof (*p_fw_image_info));

	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct fw_cmd));
	cmd.device_handle = device_handle;
	cmd.opcode = PT_GET_LOG;
	cmd.sub_opcode = SUBOP_FW_IMAGE_INFO;
	cmd.output_payload_size = sizeof (*p_fw_image_info);
	cmd.output_payload = p_fw_image_info;
	int rc = ioctl_passthrough_cmd(&cmd);

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper function to get config data policy
 */
int fw_get_config_data_policy(unsigned int device_handle,
		struct pt_payload_get_config_data_policy *p_config_data)
{
	COMMON_LOG_ENTRY();

	struct fw_cmd fw_cmd;
	memset(&fw_cmd, 0, sizeof (fw_cmd));
	fw_cmd.device_handle = device_handle;
	fw_cmd.opcode = PT_GET_FEATURES;
	fw_cmd.sub_opcode = SUBOP_OPT_CONFIG_DATA_POLICY;
	fw_cmd.output_payload_size = sizeof (*p_config_data);
	fw_cmd.output_payload = p_config_data;

	int rc = ioctl_passthrough_cmd(&fw_cmd);
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Helper to retreve ARS long operation status
 */
int fw_get_status_for_long_op(NVM_NFIT_DEVICE_HANDLE dimm_handle,
		struct pt_payload_long_op_stat *payload)
{
	int rc = NVM_SUCCESS;
	struct fw_cmd fw_cmd;
	memset(&fw_cmd, 0, sizeof (fw_cmd));
	fw_cmd.device_handle = dimm_handle.handle;
	fw_cmd.opcode = PT_GET_LOG;
	fw_cmd.sub_opcode = SUBOP_LONG_OPERATION_STAT;
	fw_cmd.output_payload_size = sizeof (*payload);
	fw_cmd.output_payload = payload;
	rc = ioctl_passthrough_cmd(&fw_cmd);
	return rc;
}

/*
 * Helper function to set config data policy
 */
int fw_set_config_data_policy(unsigned int device_handle,
		struct pt_payload_set_config_data_policy *p_config_data)
{
	COMMON_LOG_ENTRY();

	struct fw_cmd fw_cmd;
	memset(&fw_cmd, 0, sizeof (fw_cmd));
	fw_cmd.device_handle = device_handle;
	fw_cmd.opcode = PT_SET_FEATURES;
	fw_cmd.sub_opcode = SUBOP_OPT_CONFIG_DATA_POLICY;
	fw_cmd.input_payload_size = sizeof (*p_config_data);
	fw_cmd.input_payload = p_config_data;

	int rc = ioctl_passthrough_cmd(&fw_cmd);
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

int fw_mb_err_to_nvm_lib_err(int status)
{
	COMMON_LOG_ENTRY();
	int ret = NVM_SUCCESS;

	COMMON_LOG_ERROR_F("firmware mail box error = 0x%x", DSM_EXTENDED_ERROR(status));
	switch (DSM_EXTENDED_ERROR(status))
	{
		case MB_SUCCESS:
			ret = NVM_SUCCESS;
			break;
		case MB_INVALID_CMD_PARAM :
			ret = NVM_ERR_INVALIDPARAMETER;
			break;
		case MB_DATA_XFER_ERR :
			ret = NVM_ERR_DATATRANSFERERROR;
			break;
		case MB_INTERNAL_DEV_ERR :
			ret = NVM_ERR_DEVICEERROR;
			break;
		case MB_UNSUPPORTED_CMD :
			ret = NVM_ERR_NOTSUPPORTED;
			break;
		case MB_DEVICE_BUSY :
			ret = NVM_ERR_DEVICEBUSY;
			break;
		case MB_INVALID_CREDENTIAL :
			ret = NVM_ERR_BADPASSPHRASE;
			break;
		case MB_SECURITY_CHK_FAIL :
			ret = NVM_ERR_BADFIRMWARE;
			break;
		case MB_INVALID_SECURITY_STATE :
			ret = NVM_ERR_BADSECURITYSTATE;
			break;
		case MB_SYSTEM_TIME_NOT_SET :
			ret = NVM_ERR_DEVICEERROR;
			break;
		case MB_DATA_NOT_SET :
			ret = NVM_ERR_DEVICEERROR;
			break;
		case MB_ABORTED :
			ret = NVM_ERR_DEVICEERROR;
			break;
		case MB_NO_NEW_FW :
			ret = NVM_ERR_BADFIRMWARE;
			break;
		case MB_REVISION_FAILURE :
			ret = NVM_ERR_BADFIRMWARE;
			break;
		case MB_INJECTION_DISABLED :
			ret = NVM_ERR_NOTSUPPORTED;
			break;
		case MB_CONFIG_LOCKED_COMMAND_INVALID :
			ret = NVM_ERR_NOTSUPPORTED;
			break;
		case MB_INVALID_ALIGNMENT :
			ret = NVM_ERR_DEVICEERROR;
			break;
		case MB_INCOMPATIBLE_DIMM :
			ret = NVM_ERR_NOTSUPPORTED;
			break;
		case MB_TIMED_OUT :
			ret = NVM_ERR_DEVICEBUSY;
			break;
		default :
			ret = NVM_ERR_DEVICEERROR;
	}
	COMMON_LOG_EXIT_RETURN_I(ret);
	return (ret);
}

int dsm_err_to_nvm_lib_err(unsigned int status)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (status)
	{
		COMMON_LOG_ERROR_F("DSM Vendor Error = %d", DSM_VENDOR_ERROR(status));
		switch (DSM_VENDOR_ERROR(status))
		{
			case DSM_VENDOR_ERR_NOT_SUPPORTED:
				rc = NVM_ERR_NOTSUPPORTED;
				break;
			case DSM_VENDOR_ERR_NONEXISTING:
				rc = NVM_ERR_BADDEVICE;
				break;
			case DSM_VENDOR_INVALID_INPUT:
				rc = NVM_ERR_UNKNOWN;
				break;
			case DSM_VENDOR_HW_ERR:
				rc = NVM_ERR_DEVICEERROR;
				break;
			case DSM_VENDOR_RETRY_SUGGESTED:
				rc = NVM_ERR_DEVICEERROR;
				break;
			case DSM_VENDOR_UNKNOWN:
				rc = NVM_ERR_UNKNOWN;
				break;
			case DSM_VENDOR_SPECIFIC_ERR:
				rc = fw_mb_err_to_nvm_lib_err(status);
				break;
			default:
				rc = NVM_ERR_DRIVERFAILED;
				break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

// function pointer, allowing callers to specify the actual passthrough function
static int (*p_ioctl_passthrough_cmd)(struct fw_cmd *p_cmd) = NULL;
void set_ioctl_passthrough_function(int (*f)(struct fw_cmd *p_cmd))
{
	p_ioctl_passthrough_cmd = f;
}
void unset_ioctl_passthrough_function()
{
	p_ioctl_passthrough_cmd = NULL;
}

int local_ioctl_passthrough_cmd(struct fw_cmd *p_cmd)
{
	COMMON_LOG_ENTRY();
	int rc;
	if (p_ioctl_passthrough_cmd)
	{
		rc = p_ioctl_passthrough_cmd(p_cmd);
	}
	else
	{
		rc = ioctl_passthrough_cmd(p_cmd);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
