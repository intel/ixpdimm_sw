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
#ifndef CR_MGMT_FW_PRINT_CONTROLLER_H
#define CR_MGMT_FW_PRINT_CONTROLLER_H


#ifdef __cplusplus
extern "C"
{
#endif

#include "fwcmd_args.h"

#include <string.h>

void fwcmd_print_all(unsigned int handle);

void fwcmd_run(const char *command_name, unsigned int handle, fwcmd_args *p_args);

void fwcmd_run_identify_dimm(unsigned int handle);

void fwcmd_run_identify_dimm_characteristics(unsigned int handle);

void fwcmd_run_get_security_state(unsigned int handle);

void fwcmd_run_set_passphrase(unsigned int handle, const char current_passphrase[33], const char new_passphrase[33]);

void fwcmd_run_disable_passphrase(unsigned int handle, const char current_passphrase[33]);

void fwcmd_run_unlock_unit(unsigned int handle, const char current_passphrase[33]);

void fwcmd_run_secure_erase(unsigned int handle, const char current_passphrase[33]);

void fwcmd_run_freeze_lock(unsigned int handle);

void fwcmd_run_get_alarm_threshold(unsigned int handle);

void fwcmd_run_power_management_policy(unsigned int handle);

void fwcmd_run_die_sparing_policy(unsigned int handle);

void fwcmd_run_address_range_scrub(unsigned int handle);

void fwcmd_run_optional_configuration_data_policy(unsigned int handle);

void fwcmd_run_pmon_registers(unsigned int handle, const unsigned short pmon_retreive_mask);

void fwcmd_run_set_alarm_threshold(unsigned int handle, const unsigned char enable, const unsigned short peak_power_budget, const unsigned short avg_power_budget);

void fwcmd_run_system_time(unsigned int handle);

void fwcmd_run_platform_config_data_configuration_header_table(unsigned int handle, const unsigned char partition_id, const unsigned char command_option, const unsigned int offset);

void fwcmd_run_namespace_labels(unsigned int handle, const unsigned char partition_id, const unsigned char command_option, const unsigned int offset);

void fwcmd_run_dimm_partition_info(unsigned int handle);

void fwcmd_run_fw_debug_log_level(unsigned int handle, const unsigned char log_id);

void fwcmd_run_fw_load_flag(unsigned int handle);

void fwcmd_run_config_lockdown(unsigned int handle);

void fwcmd_run_ddrt_io_init_info(unsigned int handle);

void fwcmd_run_get_supported_sku_features(unsigned int handle);

void fwcmd_run_enable_dimm(unsigned int handle);

void fwcmd_run_smart_health_info(unsigned int handle);

void fwcmd_run_firmware_image_info(unsigned int handle);

void fwcmd_run_firmware_debug_log(unsigned int handle, const unsigned char log_action, const unsigned int log_page_offset, const unsigned char log_id);

void fwcmd_run_long_operation_status(unsigned int handle);

void fwcmd_run_bsr(unsigned int handle);

void fwcmd_run_format(unsigned int handle, const unsigned char fill_pattern, const unsigned char preserve_pdas_write_count);

#ifdef __cplusplus
}
#endif

#endif //CR_MGMT_FW_PRINT_CONTROLLER_H