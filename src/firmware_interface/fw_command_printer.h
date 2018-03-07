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
#ifndef CR_MGMT_FW_COMMAND_PRINTER_H
#define CR_MGMT_FW_COMMAND_PRINTER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "fw_commands.h"

void fwcmd_print_command_names();
void fwcmd_print_output_command_names();
void fwcmd_print_error(struct fwcmd_error_code error);
void print_tabs(int tab_count);


void fwcmd_identify_dimm_printer(const struct fwcmd_identify_dimm_data *p_value,
	int );

void fwcmd_identify_dimm_characteristics_printer(const struct fwcmd_identify_dimm_characteristics_data *p_value,
	int );

void fwcmd_get_security_state_printer(const struct fwcmd_get_security_state_data *p_value,
	int );

void fwcmd_get_alarm_threshold_printer(const struct fwcmd_get_alarm_threshold_data *p_value,
	int );

void fwcmd_power_management_policy_printer(const struct fwcmd_power_management_policy_data *p_value,
	int );

void fwcmd_die_sparing_policy_printer(const struct fwcmd_die_sparing_policy_data *p_value,
	int );

void fwcmd_address_range_scrub_printer(const struct fwcmd_address_range_scrub_data *p_value,
	int );

void fwcmd_optional_configuration_data_policy_printer(const struct fwcmd_optional_configuration_data_policy_data *p_value,
	int );

void fwcmd_pmon_registers_printer(const struct fwcmd_pmon_registers_data *p_value,
	int );

void fwcmd_system_time_printer(const struct fwcmd_system_time_data *p_value,
	int );

void fwcmd_device_identification_v1_printer(const struct fwcmd_device_identification_v1_data *p_value, 
	int );	
	
void fwcmd_device_identification_v2_printer(const struct fwcmd_device_identification_v2_data *p_value, 
	int );	
	
void fwcmd_id_info_table_printer(const struct fwcmd_id_info_table_data *p_value, 
	int );	
	
void fwcmd_interleave_information_table_printer(const struct fwcmd_interleave_information_table_data *p_value, 
	int );	
	
void fwcmd_partition_size_change_table_printer(const struct fwcmd_partition_size_change_table_data *p_value, 
	int );	
	
void fwcmd_current_config_table_printer(const struct fwcmd_current_config_table_data *p_value, 
	int );	
	
void fwcmd_config_input_table_printer(const struct fwcmd_config_input_table_data *p_value, 
	int );	
	
void fwcmd_config_output_table_printer(const struct fwcmd_config_output_table_data *p_value, 
	int );	
	
void fwcmd_platform_config_data_printer(const struct fwcmd_platform_config_data_data *p_value,
	int );

void fwcmd_ns_index_printer(const struct fwcmd_ns_index_data *p_value, 
	int );	
	
void fwcmd_ns_label_printer(const struct fwcmd_ns_label_data *p_value, 
	int );	
	
void fwcmd_ns_label_v1_1_printer(const struct fwcmd_ns_label_v1_1_data *p_value, 
	int );	
	
void fwcmd_ns_label_v1_2_printer(const struct fwcmd_ns_label_v1_2_data *p_value, 
	int );	
	
void fwcmd_namespace_labels_printer(const struct fwcmd_namespace_labels_data *p_value,
	int );

void fwcmd_dimm_partition_info_printer(const struct fwcmd_dimm_partition_info_data *p_value,
	int );

void fwcmd_fw_debug_log_level_printer(const struct fwcmd_fw_debug_log_level_data *p_value,
	int );

void fwcmd_fw_load_flag_printer(const struct fwcmd_fw_load_flag_data *p_value,
	int );

void fwcmd_config_lockdown_printer(const struct fwcmd_config_lockdown_data *p_value,
	int );

void fwcmd_ddrt_io_init_info_printer(const struct fwcmd_ddrt_io_init_info_data *p_value,
	int );

void fwcmd_get_supported_sku_features_printer(const struct fwcmd_get_supported_sku_features_data *p_value,
	int );

void fwcmd_enable_dimm_printer(const struct fwcmd_enable_dimm_data *p_value,
	int );

void fwcmd_smart_health_info_printer(const struct fwcmd_smart_health_info_data *p_value,
	int );

void fwcmd_firmware_image_info_printer(const struct fwcmd_firmware_image_info_data *p_value,
	int );

void fwcmd_firmware_debug_log_printer(const struct fwcmd_firmware_debug_log_data *p_value,
	int );

void fwcmd_memory_info_page_0_printer(const struct fwcmd_memory_info_page_0_data *p_value,
	int );

void fwcmd_memory_info_page_1_printer(const struct fwcmd_memory_info_page_1_data *p_value,
	int );

void fwcmd_memory_info_page_3_printer(const struct fwcmd_memory_info_page_3_data *p_value,
	int );

void fwcmd_long_operation_status_printer(const struct fwcmd_long_operation_status_data *p_value,
	int );

void fwcmd_bsr_printer(const struct fwcmd_bsr_data *p_value,
	int );

void fwcmd_identify_dimm_field_printer(const struct fwcmd_identify_dimm_data *p_value,
	int );

void fwcmd_identify_dimm_characteristics_field_printer(const struct fwcmd_identify_dimm_characteristics_data *p_value,
	int );

void fwcmd_get_security_state_field_printer(const struct fwcmd_get_security_state_data *p_value,
	int );

void fwcmd_get_alarm_threshold_field_printer(const struct fwcmd_get_alarm_threshold_data *p_value,
	int );

void fwcmd_power_management_policy_field_printer(const struct fwcmd_power_management_policy_data *p_value,
	int );

void fwcmd_die_sparing_policy_field_printer(const struct fwcmd_die_sparing_policy_data *p_value,
	int );

void fwcmd_address_range_scrub_field_printer(const struct fwcmd_address_range_scrub_data *p_value,
	int );

void fwcmd_optional_configuration_data_policy_field_printer(const struct fwcmd_optional_configuration_data_policy_data *p_value,
	int );

void fwcmd_pmon_registers_field_printer(const struct fwcmd_pmon_registers_data *p_value,
	int );

void fwcmd_system_time_field_printer(const struct fwcmd_system_time_data *p_value,
	int );

void fwcmd_device_identification_v1_field_printer(const struct fwcmd_device_identification_v1_data *p_value, 
	int );	
	
void fwcmd_device_identification_v2_field_printer(const struct fwcmd_device_identification_v2_data *p_value, 
	int );	
	
void fwcmd_id_info_table_field_printer(const struct fwcmd_id_info_table_data *p_value, 
	int );	
	
void fwcmd_interleave_information_table_field_printer(const struct fwcmd_interleave_information_table_data *p_value, 
	int );	
	
void fwcmd_partition_size_change_table_field_printer(const struct fwcmd_partition_size_change_table_data *p_value, 
	int );	
	
void fwcmd_current_config_table_field_printer(const struct fwcmd_current_config_table_data *p_value, 
	int );	
	
void fwcmd_config_input_table_field_printer(const struct fwcmd_config_input_table_data *p_value, 
	int );	
	
void fwcmd_config_output_table_field_printer(const struct fwcmd_config_output_table_data *p_value, 
	int );	
	
void fwcmd_platform_config_data_field_printer(const struct fwcmd_platform_config_data_data *p_value,
	int );

void fwcmd_ns_index_field_printer(const struct fwcmd_ns_index_data *p_value, 
	int );	
	
void fwcmd_ns_label_field_printer(const struct fwcmd_ns_label_data *p_value, 
	int );	
	
void fwcmd_ns_label_v1_1_field_printer(const struct fwcmd_ns_label_v1_1_data *p_value, 
	int );	
	
void fwcmd_ns_label_v1_2_field_printer(const struct fwcmd_ns_label_v1_2_data *p_value, 
	int );	
	
void fwcmd_namespace_labels_field_printer(const struct fwcmd_namespace_labels_data *p_value,
	int );

void fwcmd_dimm_partition_info_field_printer(const struct fwcmd_dimm_partition_info_data *p_value,
	int );

void fwcmd_fw_debug_log_level_field_printer(const struct fwcmd_fw_debug_log_level_data *p_value,
	int );

void fwcmd_fw_load_flag_field_printer(const struct fwcmd_fw_load_flag_data *p_value,
	int );

void fwcmd_config_lockdown_field_printer(const struct fwcmd_config_lockdown_data *p_value,
	int );

void fwcmd_ddrt_io_init_info_field_printer(const struct fwcmd_ddrt_io_init_info_data *p_value,
	int );

void fwcmd_get_supported_sku_features_field_printer(const struct fwcmd_get_supported_sku_features_data *p_value,
	int );

void fwcmd_enable_dimm_field_printer(const struct fwcmd_enable_dimm_data *p_value,
	int );

void fwcmd_smart_health_info_field_printer(const struct fwcmd_smart_health_info_data *p_value,
	int );

void fwcmd_firmware_image_info_field_printer(const struct fwcmd_firmware_image_info_data *p_value,
	int );

void fwcmd_firmware_debug_log_field_printer(const struct fwcmd_firmware_debug_log_data *p_value,
	int );

void fwcmd_memory_info_page_0_field_printer(const struct fwcmd_memory_info_page_0_data *p_value,
	int );

void fwcmd_memory_info_page_1_field_printer(const struct fwcmd_memory_info_page_1_data *p_value,
	int );

void fwcmd_memory_info_page_3_field_printer(const struct fwcmd_memory_info_page_3_data *p_value,
	int );

void fwcmd_long_operation_status_field_printer(const struct fwcmd_long_operation_status_data *p_value,
	int );

void fwcmd_bsr_field_printer(const struct fwcmd_bsr_data *p_value,
	int );

#ifdef __cplusplus
}
#endif


#endif //CR_MGMT_FW_COMMAND_PRINTER_H