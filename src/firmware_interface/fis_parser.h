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
#ifndef CR_MGMT_FIS_PARSER_H
#define CR_MGMT_FIS_PARSER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "fis_commands.h"
#include "fw_commands.h"

#define FWCMD_PARSE_SUCCESS(rc) (rc == FIS_PARSER_CODES_SUCCESS)

enum fis_parser_codes
{
	FIS_PARSER_CODES_SUCCESS = 0,
	FIS_PARSER_CODES_PARSING_WRONG_OFFSET = 1,
	FIS_PARSER_CODES_PARSING_TYPE_NOT_FOUND = 2,
};

enum fis_parser_codes fis_parse_identify_dimm(
	const struct pt_output_identify_dimm *p_output_payload,
	struct fwcmd_identify_dimm_data *p_data);

enum fis_parser_codes fis_parse_identify_dimm_characteristics(
	const struct pt_output_identify_dimm_characteristics *p_output_payload,
	struct fwcmd_identify_dimm_characteristics_data *p_data);

enum fis_parser_codes fis_parse_get_security_state(
	const struct pt_output_get_security_state *p_output_payload,
	struct fwcmd_get_security_state_data *p_data);

enum fis_parser_codes fis_parse_get_alarm_threshold(
	const struct pt_output_get_alarm_threshold *p_output_payload,
	struct fwcmd_get_alarm_threshold_data *p_data);

enum fis_parser_codes fis_parse_power_management_policy(
	const struct pt_output_power_management_policy *p_output_payload,
	struct fwcmd_power_management_policy_data *p_data);

enum fis_parser_codes fis_parse_die_sparing_policy(
	const struct pt_output_die_sparing_policy *p_output_payload,
	struct fwcmd_die_sparing_policy_data *p_data);

enum fis_parser_codes fis_parse_address_range_scrub(
	const struct pt_output_address_range_scrub *p_output_payload,
	struct fwcmd_address_range_scrub_data *p_data);

enum fis_parser_codes fis_parse_optional_configuration_data_policy(
	const struct pt_output_optional_configuration_data_policy *p_output_payload,
	struct fwcmd_optional_configuration_data_policy_data *p_data);

enum fis_parser_codes fis_parse_pmon_registers(
	const struct pt_output_pmon_registers *p_output_payload,
	struct fwcmd_pmon_registers_data *p_data);

enum fis_parser_codes fis_parse_system_time(
	const struct pt_output_system_time *p_output_payload,
	struct fwcmd_system_time_data *p_data);

enum fis_parser_codes fis_parse_platform_config_data(
	const struct pt_output_platform_config_data *p_output_payload,
	struct fwcmd_platform_config_data_data *p_data);

enum fis_parser_codes fis_parse_namespace_labels(
	const struct pt_output_namespace_labels *p_output_payload,
	struct fwcmd_namespace_labels_data *p_data);

enum fis_parser_codes fis_parse_dimm_partition_info(
	const struct pt_output_dimm_partition_info *p_output_payload,
	struct fwcmd_dimm_partition_info_data *p_data);

enum fis_parser_codes fis_parse_fw_debug_log_level(
	const struct pt_output_fw_debug_log_level *p_output_payload,
	struct fwcmd_fw_debug_log_level_data *p_data);

enum fis_parser_codes fis_parse_fw_load_flag(
	const struct pt_output_fw_load_flag *p_output_payload,
	struct fwcmd_fw_load_flag_data *p_data);

enum fis_parser_codes fis_parse_config_lockdown(
	const struct pt_output_config_lockdown *p_output_payload,
	struct fwcmd_config_lockdown_data *p_data);

enum fis_parser_codes fis_parse_ddrt_io_init_info(
	const struct pt_output_ddrt_io_init_info *p_output_payload,
	struct fwcmd_ddrt_io_init_info_data *p_data);

enum fis_parser_codes fis_parse_get_supported_sku_features(
	const struct pt_output_get_supported_sku_features *p_output_payload,
	struct fwcmd_get_supported_sku_features_data *p_data);

enum fis_parser_codes fis_parse_enable_dimm(
	const struct pt_output_enable_dimm *p_output_payload,
	struct fwcmd_enable_dimm_data *p_data);

enum fis_parser_codes fis_parse_smart_health_info(
	const struct pt_output_smart_health_info *p_output_payload,
	struct fwcmd_smart_health_info_data *p_data);

enum fis_parser_codes fis_parse_firmware_image_info(
	const struct pt_output_firmware_image_info *p_output_payload,
	struct fwcmd_firmware_image_info_data *p_data);

enum fis_parser_codes fis_parse_firmware_debug_log(
	const struct pt_output_firmware_debug_log *p_output_payload,
	struct fwcmd_firmware_debug_log_data *p_data);

enum fis_parser_codes fis_parse_long_operation_status(
	const struct pt_output_long_operation_status *p_output_payload,
	struct fwcmd_long_operation_status_data *p_data);

enum fis_parser_codes fis_parse_bsr(
	const struct pt_output_bsr *p_output_payload,
	struct fwcmd_bsr_data *p_data);

#ifdef __cplusplus
}
#endif

#endif //CR_MGMT_FIS_PARSER_H