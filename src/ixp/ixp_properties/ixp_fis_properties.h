/*
 * Copyright (c) 2018, Intel Corporation
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


#ifndef SRC_IXP_FIS_PROPERTIES_H_
#define SRC_IXP_FIS_PROPERTIES_H_

#include <ixp.h>

#ifdef __cplusplus
extern "C"
{
#endif

int get_fis_identify_dimm_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_identify_dimm_characteristics_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_get_security_state_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_get_alarm_threshold_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_power_management_policy_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_die_sparing_policy_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_address_range_scrub_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_optional_configuration_data_policy_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_pmon_registers_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_system_time_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_platform_config_data_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_namespace_labels_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_dimm_partition_info_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_fw_debug_log_level_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_fw_load_flag_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_config_lockdown_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_ddrt_io_init_info_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_get_supported_sku_features_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_enable_dimm_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_smart_health_info_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_firmware_image_info_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_firmware_debug_log_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_memory_info_page_0_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_memory_info_page_1_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_memory_info_page_3_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_long_operation_status_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

int get_fis_bsr_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_identify_dimm_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_identify_dimm_characteristics_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_get_security_state_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_get_alarm_threshold_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_power_management_policy_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_die_sparing_policy_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_address_range_scrub_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_optional_configuration_data_policy_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_pmon_registers_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_system_time_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_platform_config_data_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_namespace_labels_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_dimm_partition_info_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_fw_debug_log_level_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_fw_load_flag_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_config_lockdown_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_ddrt_io_init_info_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_get_supported_sku_features_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_enable_dimm_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_smart_health_info_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_firmware_image_info_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_firmware_debug_log_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_memory_info_page_0_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_memory_info_page_1_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_memory_info_page_3_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_long_operation_status_properties(
struct ixp_prop_info props[], unsigned int num_props);

void free_fis_bsr_properties(
struct ixp_prop_info props[], unsigned int num_props);

#ifdef __cplusplus
}
#endif

#endif /* SRC_IXP_FIS_PROPERTIES_H_ */