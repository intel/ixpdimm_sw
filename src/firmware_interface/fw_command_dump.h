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
#ifndef CR_MGMT_FW_COMMAND_DUMP_H
#define CR_MGMT_FW_COMMAND_DUMP_H

#ifdef __cplusplus
extern "C"
{
#endif

enum FWCMD_DUMP_RESULT
{
	FWCMD_DUMP_RESULT_SUCCESS = 0,
	FWCMD_DUMP_RESULT_ERR = 1,
	FWCMD_DUMP_RESULT_ERR_FILE_OPEN = 2,
	FWCMD_DUMP_RESULT_ERR_FILE_READ = 3,
	FWCMD_DUMP_RESULT_ERR_FILE_WRITE = 4,

};

int fwcmd_dump(const char *command_name, unsigned int handle, const char *filename);

void fwcmd_read_and_print(const char *filename);


int fwcmd_dump_identify_dimm(const int handle,
	const char * filename);

struct fwcmd_identify_dimm_result fwcmd_read_identify_dimm(const char *filename);

int fwcmd_dump_identify_dimm_characteristics(const int handle,
	const char * filename);

struct fwcmd_identify_dimm_characteristics_result fwcmd_read_identify_dimm_characteristics(const char *filename);

int fwcmd_dump_get_security_state(const int handle,
	const char * filename);

struct fwcmd_get_security_state_result fwcmd_read_get_security_state(const char *filename);

int fwcmd_dump_get_alarm_threshold(const int handle,
	const char * filename);

struct fwcmd_get_alarm_threshold_result fwcmd_read_get_alarm_threshold(const char *filename);

int fwcmd_dump_power_management_policy(const int handle,
	const char * filename);

struct fwcmd_power_management_policy_result fwcmd_read_power_management_policy(const char *filename);

int fwcmd_dump_die_sparing_policy(const int handle,
	const char * filename);

struct fwcmd_die_sparing_policy_result fwcmd_read_die_sparing_policy(const char *filename);

int fwcmd_dump_address_range_scrub(const int handle,
	const char * filename);

struct fwcmd_address_range_scrub_result fwcmd_read_address_range_scrub(const char *filename);

int fwcmd_dump_optional_configuration_data_policy(const int handle,
	const char * filename);

struct fwcmd_optional_configuration_data_policy_result fwcmd_read_optional_configuration_data_policy(const char *filename);

int fwcmd_dump_pmon_registers(const int handle,
	const unsigned short pmon_retreive_mask,
	const char * filename);

struct fwcmd_pmon_registers_result fwcmd_read_pmon_registers(const char *filename);

int fwcmd_dump_system_time(const int handle,
	const char * filename);

struct fwcmd_system_time_result fwcmd_read_system_time(const char *filename);

int fwcmd_dump_platform_config_data_configuration_header_table(const int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset,
	const char * filename);

struct fwcmd_platform_config_data_configuration_header_table_result fwcmd_read_platform_config_data_configuration_header_table(const char *filename);

int fwcmd_dump_namespace_labels(const int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset,
	const char * filename);

struct fwcmd_namespace_labels_result fwcmd_read_namespace_labels(const char *filename);

int fwcmd_dump_dimm_partition_info(const int handle,
	const char * filename);

struct fwcmd_dimm_partition_info_result fwcmd_read_dimm_partition_info(const char *filename);

int fwcmd_dump_fw_debug_log_level(const int handle,
	const unsigned char log_id,
	const char * filename);

struct fwcmd_fw_debug_log_level_result fwcmd_read_fw_debug_log_level(const char *filename);

int fwcmd_dump_fw_load_flag(const int handle,
	const char * filename);

struct fwcmd_fw_load_flag_result fwcmd_read_fw_load_flag(const char *filename);

int fwcmd_dump_config_lockdown(const int handle,
	const char * filename);

struct fwcmd_config_lockdown_result fwcmd_read_config_lockdown(const char *filename);

int fwcmd_dump_ddrt_io_init_info(const int handle,
	const char * filename);

struct fwcmd_ddrt_io_init_info_result fwcmd_read_ddrt_io_init_info(const char *filename);

int fwcmd_dump_get_supported_sku_features(const int handle,
	const char * filename);

struct fwcmd_get_supported_sku_features_result fwcmd_read_get_supported_sku_features(const char *filename);

int fwcmd_dump_enable_dimm(const int handle,
	const char * filename);

struct fwcmd_enable_dimm_result fwcmd_read_enable_dimm(const char *filename);

int fwcmd_dump_smart_health_info(const int handle,
	const char * filename);

struct fwcmd_smart_health_info_result fwcmd_read_smart_health_info(const char *filename);

int fwcmd_dump_firmware_image_info(const int handle,
	const char * filename);

struct fwcmd_firmware_image_info_result fwcmd_read_firmware_image_info(const char *filename);

int fwcmd_dump_firmware_debug_log(const int handle,
	const unsigned char log_action,
	const unsigned int log_page_offset,
	const unsigned char log_id,
	const char * filename);

struct fwcmd_firmware_debug_log_result fwcmd_read_firmware_debug_log(const char *filename);

int fwcmd_dump_long_operation_status(const int handle,
	const char * filename);

struct fwcmd_long_operation_status_result fwcmd_read_long_operation_status(const char *filename);

int fwcmd_dump_bsr(const int handle,
	const char * filename);

struct fwcmd_bsr_result fwcmd_read_bsr(const char *filename);

#ifdef __cplusplus
}
#endif



#endif //CR_MGMT_FW_COMMAND_DUMP_H