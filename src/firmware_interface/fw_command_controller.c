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

#include <stdio.h>
#include <common/string/s_str.h>

#include "fw_command_controller.h"
#include "fw_commands.h"
#include "fw_command_printer.h"

int to_int(char * value)
{
	return atoi(value);
}

char *to_str(char *value)
{
	return value;
}

void fwcmd_print_all(unsigned int handle)
{
	printf("Printing identify_dimm ... \n");
	fwcmd_run_identify_dimm(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing identify_dimm_characteristics ... \n");
	fwcmd_run_identify_dimm_characteristics(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing get_security_state ... \n");
	fwcmd_run_get_security_state(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing get_alarm_threshold ... \n");
	fwcmd_run_get_alarm_threshold(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing power_management_policy ... \n");
	fwcmd_run_power_management_policy(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing die_sparing_policy ... \n");
	fwcmd_run_die_sparing_policy(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing address_range_scrub ... \n");
	fwcmd_run_address_range_scrub(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing optional_configuration_data_policy ... \n");
	fwcmd_run_optional_configuration_data_policy(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing pmon_registers ... \n");
	fwcmd_run_pmon_registers(handle
		, 0
		);
	printf("--------------------------------------------\n");
	printf("Printing system_time ... \n");
	fwcmd_run_system_time(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing platform_config_data_configuration_header_table ... \n");
	fwcmd_run_platform_config_data_configuration_header_table(handle
		, 1
		, 0
		, 0
		);
	printf("--------------------------------------------\n");
	printf("Printing dimm_partition_info ... \n");
	fwcmd_run_dimm_partition_info(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing fw_debug_log_level ... \n");
	fwcmd_run_fw_debug_log_level(handle
		, 0
		);
	printf("--------------------------------------------\n");
	printf("Printing fw_load_flag ... \n");
	fwcmd_run_fw_load_flag(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing config_lockdown ... \n");
	fwcmd_run_config_lockdown(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing ddrt_io_init_info ... \n");
	fwcmd_run_ddrt_io_init_info(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing get_supported_sku_features ... \n");
	fwcmd_run_get_supported_sku_features(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing enable_dimm ... \n");
	fwcmd_run_enable_dimm(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing smart_health_info ... \n");
	fwcmd_run_smart_health_info(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing firmware_image_info ... \n");
	fwcmd_run_firmware_image_info(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing firmware_debug_log ... \n");
	fwcmd_run_firmware_debug_log(handle
		, 0
		, 0
		, 0
		);
	printf("--------------------------------------------\n");
	printf("Printing long_operation_status ... \n");
	fwcmd_run_long_operation_status(handle
		);
	printf("--------------------------------------------\n");
	printf("Printing bsr ... \n");
	fwcmd_run_bsr(handle
		);
	printf("--------------------------------------------\n");
}

void fwcmd_run(const char *command_name,
	unsigned int handle,
	fwcmd_args *p_args)
{
	if (s_strncmpi(command_name, "identify_dimm",
		sizeof ("identify_dimm")) == 0)
	{
		fwcmd_run_identify_dimm(handle
		);

	}
	else if (s_strncmpi(command_name, "identify_dimm_characteristics",
		sizeof ("identify_dimm_characteristics")) == 0)
	{
		fwcmd_run_identify_dimm_characteristics(handle
		);

	}
	else if (s_strncmpi(command_name, "get_security_state",
		sizeof ("get_security_state")) == 0)
	{
		fwcmd_run_get_security_state(handle
		);

	}
	else if (s_strncmpi(command_name, "set_passphrase",
		sizeof ("set_passphrase")) == 0)
	{
		char * current_passphrase_value = find_arg(p_args, "current_passphrase");
		if (!current_passphrase_value)
		{
			printf("Required argument 'current_passphrase' not found\n");
			return;
		}
		char * new_passphrase_value = find_arg(p_args, "new_passphrase");
		if (!new_passphrase_value)
		{
			printf("Required argument 'new_passphrase' not found\n");
			return;
		}
		fwcmd_run_set_passphrase(handle
			, current_passphrase_value
			, new_passphrase_value
		);

	}
	else if (s_strncmpi(command_name, "disable_passphrase",
		sizeof ("disable_passphrase")) == 0)
	{
		char * current_passphrase_value = find_arg(p_args, "current_passphrase");
		if (!current_passphrase_value)
		{
			printf("Required argument 'current_passphrase' not found\n");
			return;
		}
		fwcmd_run_disable_passphrase(handle
			, current_passphrase_value
		);

	}
	else if (s_strncmpi(command_name, "unlock_unit",
		sizeof ("unlock_unit")) == 0)
	{
		char * current_passphrase_value = find_arg(p_args, "current_passphrase");
		if (!current_passphrase_value)
		{
			printf("Required argument 'current_passphrase' not found\n");
			return;
		}
		fwcmd_run_unlock_unit(handle
			, current_passphrase_value
		);

	}
	else if (s_strncmpi(command_name, "secure_erase",
		sizeof ("secure_erase")) == 0)
	{
		char * current_passphrase_value = find_arg(p_args, "current_passphrase");
		if (!current_passphrase_value)
		{
			printf("Required argument 'current_passphrase' not found\n");
			return;
		}
		fwcmd_run_secure_erase(handle
			, current_passphrase_value
		);

	}
	else if (s_strncmpi(command_name, "freeze_lock",
		sizeof ("freeze_lock")) == 0)
	{
		fwcmd_run_freeze_lock(handle
		);

	}
	else if (s_strncmpi(command_name, "get_alarm_threshold",
		sizeof ("get_alarm_threshold")) == 0)
	{
		fwcmd_run_get_alarm_threshold(handle
		);

	}
	else if (s_strncmpi(command_name, "power_management_policy",
		sizeof ("power_management_policy")) == 0)
	{
		fwcmd_run_power_management_policy(handle
		);

	}
	else if (s_strncmpi(command_name, "die_sparing_policy",
		sizeof ("die_sparing_policy")) == 0)
	{
		fwcmd_run_die_sparing_policy(handle
		);

	}
	else if (s_strncmpi(command_name, "address_range_scrub",
		sizeof ("address_range_scrub")) == 0)
	{
		fwcmd_run_address_range_scrub(handle
		);

	}
	else if (s_strncmpi(command_name, "optional_configuration_data_policy",
		sizeof ("optional_configuration_data_policy")) == 0)
	{
		fwcmd_run_optional_configuration_data_policy(handle
		);

	}
	else if (s_strncmpi(command_name, "pmon_registers",
		sizeof ("pmon_registers")) == 0)
	{
		char * pmon_retreive_mask_value = find_arg(p_args, "pmon_retreive_mask");
		if (!pmon_retreive_mask_value)
		{
			pmon_retreive_mask_value = "0";
		}
		fwcmd_run_pmon_registers(handle
			, to_int(pmon_retreive_mask_value)
		);

	}
	else if (s_strncmpi(command_name, "set_alarm_threshold",
		sizeof ("set_alarm_threshold")) == 0)
	{
		char * enable_value = find_arg(p_args, "enable");
		if (!enable_value)
		{
			printf("Required argument 'enable' not found\n");
			return;
		}
		char * peak_power_budget_value = find_arg(p_args, "peak_power_budget");
		if (!peak_power_budget_value)
		{
			printf("Required argument 'peak_power_budget' not found\n");
			return;
		}
		char * avg_power_budget_value = find_arg(p_args, "avg_power_budget");
		if (!avg_power_budget_value)
		{
			printf("Required argument 'avg_power_budget' not found\n");
			return;
		}
		fwcmd_run_set_alarm_threshold(handle
			, to_int(enable_value)
			, to_int(peak_power_budget_value)
			, to_int(avg_power_budget_value)
		);

	}
	else if (s_strncmpi(command_name, "system_time",
		sizeof ("system_time")) == 0)
	{
		fwcmd_run_system_time(handle
		);

	}
	else if (s_strncmpi(command_name, "platform_config_data_configuration_header_table",
		sizeof ("platform_config_data_configuration_header_table")) == 0)
	{
		char * partition_id_value = find_arg(p_args, "partition_id");
		if (!partition_id_value)
		{
			partition_id_value = "1";
		}
		char * command_option_value = find_arg(p_args, "command_option");
		if (!command_option_value)
		{
			command_option_value = "0";
		}
		char * offset_value = find_arg(p_args, "offset");
		if (!offset_value)
		{
			offset_value = "0";
		}
		fwcmd_run_platform_config_data_configuration_header_table(handle
			, to_int(partition_id_value)
			, to_int(command_option_value)
			, to_int(offset_value)
		);

	}
	else if (s_strncmpi(command_name, "dimm_partition_info",
		sizeof ("dimm_partition_info")) == 0)
	{
		fwcmd_run_dimm_partition_info(handle
		);

	}
	else if (s_strncmpi(command_name, "fw_debug_log_level",
		sizeof ("fw_debug_log_level")) == 0)
	{
		char * log_id_value = find_arg(p_args, "log_id");
		if (!log_id_value)
		{
			log_id_value = "0";
		}
		fwcmd_run_fw_debug_log_level(handle
			, to_int(log_id_value)
		);

	}
	else if (s_strncmpi(command_name, "fw_load_flag",
		sizeof ("fw_load_flag")) == 0)
	{
		fwcmd_run_fw_load_flag(handle
		);

	}
	else if (s_strncmpi(command_name, "config_lockdown",
		sizeof ("config_lockdown")) == 0)
	{
		fwcmd_run_config_lockdown(handle
		);

	}
	else if (s_strncmpi(command_name, "ddrt_io_init_info",
		sizeof ("ddrt_io_init_info")) == 0)
	{
		fwcmd_run_ddrt_io_init_info(handle
		);

	}
	else if (s_strncmpi(command_name, "get_supported_sku_features",
		sizeof ("get_supported_sku_features")) == 0)
	{
		fwcmd_run_get_supported_sku_features(handle
		);

	}
	else if (s_strncmpi(command_name, "enable_dimm",
		sizeof ("enable_dimm")) == 0)
	{
		fwcmd_run_enable_dimm(handle
		);

	}
	else if (s_strncmpi(command_name, "smart_health_info",
		sizeof ("smart_health_info")) == 0)
	{
		fwcmd_run_smart_health_info(handle
		);

	}
	else if (s_strncmpi(command_name, "firmware_image_info",
		sizeof ("firmware_image_info")) == 0)
	{
		fwcmd_run_firmware_image_info(handle
		);

	}
	else if (s_strncmpi(command_name, "firmware_debug_log",
		sizeof ("firmware_debug_log")) == 0)
	{
		char * log_action_value = find_arg(p_args, "log_action");
		if (!log_action_value)
		{
			log_action_value = "0";
		}
		char * log_page_offset_value = find_arg(p_args, "log_page_offset");
		if (!log_page_offset_value)
		{
			log_page_offset_value = "0";
		}
		char * log_id_value = find_arg(p_args, "log_id");
		if (!log_id_value)
		{
			log_id_value = "0";
		}
		fwcmd_run_firmware_debug_log(handle
			, to_int(log_action_value)
			, to_int(log_page_offset_value)
			, to_int(log_id_value)
		);

	}
	else if (s_strncmpi(command_name, "long_operation_status",
		sizeof ("long_operation_status")) == 0)
	{
		fwcmd_run_long_operation_status(handle
		);

	}
	else if (s_strncmpi(command_name, "bsr",
		sizeof ("bsr")) == 0)
	{
		fwcmd_run_bsr(handle
		);

	}
	else
	{
		printf("Command \"%s\" not recognized. Available commands: \n", command_name);
		printf("\tidentify_dimm\n");
		printf("\tidentify_dimm_characteristics\n");
		printf("\tget_security_state\n");
		printf("\tset_passphrase\n");
		printf("\tdisable_passphrase\n");
		printf("\tunlock_unit\n");
		printf("\tsecure_erase\n");
		printf("\tfreeze_lock\n");
		printf("\tget_alarm_threshold\n");
		printf("\tpower_management_policy\n");
		printf("\tdie_sparing_policy\n");
		printf("\taddress_range_scrub\n");
		printf("\toptional_configuration_data_policy\n");
		printf("\tpmon_registers\n");
		printf("\tset_alarm_threshold\n");
		printf("\tsystem_time\n");
		printf("\tplatform_config_data_configuration_header_table\n");
		printf("\tdimm_partition_info\n");
		printf("\tfw_debug_log_level\n");
		printf("\tfw_load_flag\n");
		printf("\tconfig_lockdown\n");
		printf("\tddrt_io_init_info\n");
		printf("\tget_supported_sku_features\n");
		printf("\tenable_dimm\n");
		printf("\tsmart_health_info\n");
		printf("\tfirmware_image_info\n");
		printf("\tfirmware_debug_log\n");
		printf("\tlong_operation_status\n");
		printf("\tbsr\n");
	}
}

void fwcmd_run_identify_dimm(unsigned int handle)
{
	struct fwcmd_identify_dimm_result result = fwcmd_alloc_identify_dimm(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_identify_dimm_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing identify_dimm. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_identify_dimm(&result);
}

void fwcmd_run_identify_dimm_characteristics(unsigned int handle)
{
	struct fwcmd_identify_dimm_characteristics_result result = fwcmd_alloc_identify_dimm_characteristics(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_identify_dimm_characteristics_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing identify_dimm_characteristics. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_identify_dimm_characteristics(&result);
}

void fwcmd_run_get_security_state(unsigned int handle)
{
	struct fwcmd_get_security_state_result result = fwcmd_alloc_get_security_state(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_get_security_state_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing get_security_state. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_get_security_state(&result);
}

void fwcmd_run_set_passphrase(unsigned int handle, const char current_passphrase[33], const char new_passphrase[33])
{
	struct fwcmd_set_passphrase_result result = fwcmd_call_set_passphrase(handle, current_passphrase, new_passphrase);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
	}
	else
	{
		printf("There was an issue executing set_passphrase. \n");
		fwcmd_print_error(result.error_code);
	}
}

void fwcmd_run_disable_passphrase(unsigned int handle, const char current_passphrase[33])
{
	struct fwcmd_disable_passphrase_result result = fwcmd_call_disable_passphrase(handle, current_passphrase);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
	}
	else
	{
		printf("There was an issue executing disable_passphrase. \n");
		fwcmd_print_error(result.error_code);
	}
}

void fwcmd_run_unlock_unit(unsigned int handle, const char current_passphrase[33])
{
	struct fwcmd_unlock_unit_result result = fwcmd_call_unlock_unit(handle, current_passphrase);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
	}
	else
	{
		printf("There was an issue executing unlock_unit. \n");
		fwcmd_print_error(result.error_code);
	}
}

void fwcmd_run_secure_erase(unsigned int handle, const char current_passphrase[33])
{
	struct fwcmd_secure_erase_result result = fwcmd_call_secure_erase(handle, current_passphrase);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
	}
	else
	{
		printf("There was an issue executing secure_erase. \n");
		fwcmd_print_error(result.error_code);
	}
}

void fwcmd_run_freeze_lock(unsigned int handle)
{
	struct fwcmd_freeze_lock_result result = fwcmd_call_freeze_lock(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
	}
	else
	{
		printf("There was an issue executing freeze_lock. \n");
		fwcmd_print_error(result.error_code);
	}
}

void fwcmd_run_get_alarm_threshold(unsigned int handle)
{
	struct fwcmd_get_alarm_threshold_result result = fwcmd_alloc_get_alarm_threshold(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_get_alarm_threshold_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing get_alarm_threshold. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_get_alarm_threshold(&result);
}

void fwcmd_run_power_management_policy(unsigned int handle)
{
	struct fwcmd_power_management_policy_result result = fwcmd_alloc_power_management_policy(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_power_management_policy_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing power_management_policy. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_power_management_policy(&result);
}

void fwcmd_run_die_sparing_policy(unsigned int handle)
{
	struct fwcmd_die_sparing_policy_result result = fwcmd_alloc_die_sparing_policy(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_die_sparing_policy_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing die_sparing_policy. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_die_sparing_policy(&result);
}

void fwcmd_run_address_range_scrub(unsigned int handle)
{
	struct fwcmd_address_range_scrub_result result = fwcmd_alloc_address_range_scrub(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_address_range_scrub_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing address_range_scrub. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_address_range_scrub(&result);
}

void fwcmd_run_optional_configuration_data_policy(unsigned int handle)
{
	struct fwcmd_optional_configuration_data_policy_result result = fwcmd_alloc_optional_configuration_data_policy(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_optional_configuration_data_policy_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing optional_configuration_data_policy. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_optional_configuration_data_policy(&result);
}

void fwcmd_run_pmon_registers(unsigned int handle, const unsigned short pmon_retreive_mask)
{
	struct fwcmd_pmon_registers_result result = fwcmd_alloc_pmon_registers(handle, pmon_retreive_mask);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_pmon_registers_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing pmon_registers. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_pmon_registers(&result);
}

void fwcmd_run_set_alarm_threshold(unsigned int handle, const unsigned char enable, const unsigned short peak_power_budget, const unsigned short avg_power_budget)
{
	struct fwcmd_set_alarm_threshold_result result = fwcmd_call_set_alarm_threshold(handle, enable, peak_power_budget, avg_power_budget);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
	}
	else
	{
		printf("There was an issue executing set_alarm_threshold. \n");
		fwcmd_print_error(result.error_code);
	}
}

void fwcmd_run_system_time(unsigned int handle)
{
	struct fwcmd_system_time_result result = fwcmd_alloc_system_time(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_system_time_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing system_time. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_system_time(&result);
}

void fwcmd_run_platform_config_data_configuration_header_table(unsigned int handle, const unsigned char partition_id, const unsigned char command_option, const unsigned int offset)
{
	struct fwcmd_platform_config_data_configuration_header_table_result result = fwcmd_alloc_platform_config_data_configuration_header_table(handle, partition_id, command_option, offset);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_platform_config_data_configuration_header_table_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing platform_config_data_configuration_header_table. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_platform_config_data_configuration_header_table(&result);
}

void fwcmd_run_dimm_partition_info(unsigned int handle)
{
	struct fwcmd_dimm_partition_info_result result = fwcmd_alloc_dimm_partition_info(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_dimm_partition_info_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing dimm_partition_info. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_dimm_partition_info(&result);
}

void fwcmd_run_fw_debug_log_level(unsigned int handle, const unsigned char log_id)
{
	struct fwcmd_fw_debug_log_level_result result = fwcmd_alloc_fw_debug_log_level(handle, log_id);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_fw_debug_log_level_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing fw_debug_log_level. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_fw_debug_log_level(&result);
}

void fwcmd_run_fw_load_flag(unsigned int handle)
{
	struct fwcmd_fw_load_flag_result result = fwcmd_alloc_fw_load_flag(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_fw_load_flag_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing fw_load_flag. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_fw_load_flag(&result);
}

void fwcmd_run_config_lockdown(unsigned int handle)
{
	struct fwcmd_config_lockdown_result result = fwcmd_alloc_config_lockdown(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_config_lockdown_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing config_lockdown. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_config_lockdown(&result);
}

void fwcmd_run_ddrt_io_init_info(unsigned int handle)
{
	struct fwcmd_ddrt_io_init_info_result result = fwcmd_alloc_ddrt_io_init_info(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_ddrt_io_init_info_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing ddrt_io_init_info. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_ddrt_io_init_info(&result);
}

void fwcmd_run_get_supported_sku_features(unsigned int handle)
{
	struct fwcmd_get_supported_sku_features_result result = fwcmd_alloc_get_supported_sku_features(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_get_supported_sku_features_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing get_supported_sku_features. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_get_supported_sku_features(&result);
}

void fwcmd_run_enable_dimm(unsigned int handle)
{
	struct fwcmd_enable_dimm_result result = fwcmd_alloc_enable_dimm(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_enable_dimm_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing enable_dimm. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_enable_dimm(&result);
}

void fwcmd_run_smart_health_info(unsigned int handle)
{
	struct fwcmd_smart_health_info_result result = fwcmd_alloc_smart_health_info(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_smart_health_info_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing smart_health_info. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_smart_health_info(&result);
}

void fwcmd_run_firmware_image_info(unsigned int handle)
{
	struct fwcmd_firmware_image_info_result result = fwcmd_alloc_firmware_image_info(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_firmware_image_info_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing firmware_image_info. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_firmware_image_info(&result);
}

void fwcmd_run_firmware_debug_log(unsigned int handle, const unsigned char log_action, const unsigned int log_page_offset, const unsigned char log_id)
{
	struct fwcmd_firmware_debug_log_result result = fwcmd_alloc_firmware_debug_log(handle, log_action, log_page_offset, log_id);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_firmware_debug_log_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing firmware_debug_log. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_firmware_debug_log(&result);
}

void fwcmd_run_long_operation_status(unsigned int handle)
{
	struct fwcmd_long_operation_status_result result = fwcmd_alloc_long_operation_status(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_long_operation_status_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing long_operation_status. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_long_operation_status(&result);
}

void fwcmd_run_bsr(unsigned int handle)
{
	struct fwcmd_bsr_result result = fwcmd_alloc_bsr(handle);

	if (result.success)
	{
		printf("0x%x: Success!\n", handle);
		fwcmd_bsr_printer(result.p_data, 0);
	}
	else
	{
		printf("There was an issue executing bsr. \n");
		fwcmd_print_error(result.error_code);
	}
	fwcmd_free_bsr(&result);
}

