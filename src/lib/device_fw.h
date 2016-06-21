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

#ifndef	_DEVICE_FW_H
#define	_DEVICE_FW_H

#include <persistence/logging.h>
#include "nvm_types.h"
#include "fis_types.h"

int fw_mb_err_to_nvm_lib_err(int status);

int dsm_err_to_nvm_lib_err(unsigned int status);

void set_ioctl_passthrough_function(int (*f)(struct fw_cmd *p_cmd));
void unset_ioctl_passthrough_function();
/*
 * Compare the firmware API version to the supported version.
 * Because the firmware is backwards compatible need to make sure API version
 * is >= supported API.
 */
NVM_BOOL is_fw_api_version_supported(const unsigned int major_version,
	const unsigned int minor_version);

int fw_get_identify_dimm(const NVM_UINT32 device_handle,
		struct pt_payload_identify_dimm *p_id_dimm);

int fw_get_id_dimm_device_characteristics(unsigned int device_handle,
	struct pt_payload_device_characteristics *p_payload);

int fw_get_alarm_thresholds(NVM_UINT32 const device_handle,
	struct pt_payload_alarm_thresholds *p_thresholds);

int fw_set_alarm_thresholds(NVM_UINT32 const device_handle,
	struct pt_payload_alarm_thresholds *p_thresholds);

int fw_get_smart_health(const NVM_UINT32 device_handle,
	struct pt_payload_smart_health *p_dimm_smart);

int fw_get_memory_info_page(const NVM_UINT32 device_handle, const unsigned char page_num,
	void *p_memory_info_page, const unsigned int page_size);

int fw_get_fw_error_log_count(const NVM_UINT32 device_handle,
	const unsigned char log_level,
	const unsigned char log_type);

int fw_get_fw_error_logs(const NVM_UINT32 device_handle,
		const unsigned int error_count,
		NVM_UINT8 *large_buffer,
		const unsigned char log_level,
		const unsigned char log_type);

int fw_get_fw_error_log_info_data(const NVM_UINT32 device_handle,
		const unsigned char log_level,
		const unsigned char log_type,
		struct pt_payload_fw_log_info_data *p_log_info_data);

int fw_get_security_state(const NVM_UINT32 device_handle,
	struct pt_payload_get_security_state *p_security_state);

int fw_get_fw_image_info(const NVM_UINT32 device_handle,
	struct pt_payload_fw_image_info *p_fw_image_info);

int fw_get_config_data_policy(unsigned int device_handle,
	struct pt_payload_get_config_data_policy *payload);

int fw_get_status_for_long_op(NVM_NFIT_DEVICE_HANDLE dimm_handle,
        struct pt_payload_long_op_stat *payload);

int fw_set_config_data_policy(unsigned int device_handle,
	struct pt_payload_set_config_data_policy *p_config_data);

float fw_convert_fw_celsius_to_float(unsigned short fw_celsius);
unsigned short fw_convert_float_to_fw_celsius(float celsius);

#endif // device_fw.h
