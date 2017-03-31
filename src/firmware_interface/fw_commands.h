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
#ifndef CR_MGMT_FW_COMMANDS_H
#define CR_MGMT_FW_COMMANDS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Data Structures for identify_dimm
 */
struct fwcmd_identify_dimm_data
{
	unsigned short vendor_id;
	unsigned short device_id;
	unsigned short revision_id;
	unsigned short interface_format_code;
	char firmware_revision[5];
	unsigned char reserved_old_api;
	unsigned char feature_sw_required_mask;
	unsigned char feature_sw_required_mask_invalidate_before_block_read;
	unsigned char feature_sw_required_mask_readback_of_bw_address_register_required_before_use;
	unsigned short number_of_block_windows;
	unsigned int offset_of_block_mode_control_region;
	unsigned int raw_capacity;
	unsigned short manufacturer;
	unsigned int serial_number;
	char part_number[21];
	unsigned int dimm_sku;
	unsigned char dimm_sku_memory_mode_enabled;
	unsigned char dimm_sku_storage_mode_enabled;
	unsigned char dimm_sku_app_direct_mode_enabled;
	unsigned char dimm_sku_die_sparing_capable;
	unsigned char dimm_sku_soft_programmable_sku;
	unsigned char dimm_sku_encryption_enabled;
	unsigned short interface_format_code_extra;
	unsigned short api_ver;

};
struct fwcmd_identify_dimm_result
{
	int success;
	int error_code;
	struct fwcmd_identify_dimm_data *p_data;
};

/*
 * Firmware Command Function identify_dimm
 */
struct fwcmd_identify_dimm_result fwcmd_alloc_identify_dimm(unsigned int handle);

void fwcmd_free_identify_dimm(struct fwcmd_identify_dimm_result *p_result);
/*
 * Data Structures for identify_dimm_characteristics
 */
struct fwcmd_identify_dimm_characteristics_data
{
	unsigned short controller_temp_shutdown_threshold;
	unsigned short media_temp_shutdown_threshold;
	unsigned short throttling_start_threshold;
	unsigned short throttling_stop_threshold;

};
struct fwcmd_identify_dimm_characteristics_result
{
	int success;
	int error_code;
	struct fwcmd_identify_dimm_characteristics_data *p_data;
};

/*
 * Firmware Command Function identify_dimm_characteristics
 */
struct fwcmd_identify_dimm_characteristics_result fwcmd_alloc_identify_dimm_characteristics(unsigned int handle);

void fwcmd_free_identify_dimm_characteristics(struct fwcmd_identify_dimm_characteristics_result *p_result);
/*
 * Data Structures for get_security_state
 */
struct fwcmd_get_security_state_data
{
	unsigned char security_state;
	unsigned char security_state_enabled;
	unsigned char security_state_locked;
	unsigned char security_state_frozen;
	unsigned char security_state_count_expired;
	unsigned char security_state_not_supported;

};
struct fwcmd_get_security_state_result
{
	int success;
	int error_code;
	struct fwcmd_get_security_state_data *p_data;
};

/*
 * Firmware Command Function get_security_state
 */
struct fwcmd_get_security_state_result fwcmd_alloc_get_security_state(unsigned int handle);

void fwcmd_free_get_security_state(struct fwcmd_get_security_state_result *p_result);
/*
 * Data Structures for set_passphrase
 */
struct fwcmd_set_passphrase_result
{
	int success;
	int error_code;
};

/*
 * Firmware Command Function set_passphrase
 */
struct fwcmd_set_passphrase_result fwcmd_call_set_passphrase(unsigned int handle,
	const char current_passphrase[33],
	const char new_passphrase[33]);

/*
 * Data Structures for disable_passphrase
 */
struct fwcmd_disable_passphrase_result
{
	int success;
	int error_code;
};

/*
 * Firmware Command Function disable_passphrase
 */
struct fwcmd_disable_passphrase_result fwcmd_call_disable_passphrase(unsigned int handle,
	const char current_passphrase[33]);

/*
 * Data Structures for unlock_unit
 */
struct fwcmd_unlock_unit_result
{
	int success;
	int error_code;
};

/*
 * Firmware Command Function unlock_unit
 */
struct fwcmd_unlock_unit_result fwcmd_call_unlock_unit(unsigned int handle,
	const char current_passphrase[33]);

/*
 * Data Structures for secure_erase
 */
struct fwcmd_secure_erase_result
{
	int success;
	int error_code;
};

/*
 * Firmware Command Function secure_erase
 */
struct fwcmd_secure_erase_result fwcmd_call_secure_erase(unsigned int handle,
	const char current_passphrase[33]);

/*
 * Data Structures for freeze_lock
 */
struct fwcmd_freeze_lock_result
{
	int success;
	int error_code;
};

/*
 * Firmware Command Function freeze_lock
 */
struct fwcmd_freeze_lock_result fwcmd_call_freeze_lock(unsigned int handle);

/*
 * Data Structures for get_alarm_threshold
 */
struct fwcmd_get_alarm_threshold_data
{
	unsigned short enable;
	unsigned char enable_spare_block;
	unsigned char enable_media_temp;
	unsigned char enable_controller_temp;
	unsigned char spare_block_threshold;
	unsigned short media_temp_threshold;
	unsigned short controller_temp_threshold;

};
struct fwcmd_get_alarm_threshold_result
{
	int success;
	int error_code;
	struct fwcmd_get_alarm_threshold_data *p_data;
};

/*
 * Firmware Command Function get_alarm_threshold
 */
struct fwcmd_get_alarm_threshold_result fwcmd_alloc_get_alarm_threshold(unsigned int handle);

void fwcmd_free_get_alarm_threshold(struct fwcmd_get_alarm_threshold_result *p_result);
/*
 * Data Structures for power_management_policy
 */
struct fwcmd_power_management_policy_data
{
	unsigned char enable;
	unsigned short peak_power_budget;
	unsigned short average_power_budget;
	unsigned char max_power;

};
struct fwcmd_power_management_policy_result
{
	int success;
	int error_code;
	struct fwcmd_power_management_policy_data *p_data;
};

/*
 * Firmware Command Function power_management_policy
 */
struct fwcmd_power_management_policy_result fwcmd_alloc_power_management_policy(unsigned int handle);

void fwcmd_free_power_management_policy(struct fwcmd_power_management_policy_result *p_result);
/*
 * Data Structures for die_sparing_policy
 */
struct fwcmd_die_sparing_policy_data
{
	unsigned char enable;
	unsigned char aggressiveness;
	unsigned char supported;
	unsigned char supported_rank_0;
	unsigned char supported_rank_1;
	unsigned char supported_rank_2;
	unsigned char supported_rank_3;

};
struct fwcmd_die_sparing_policy_result
{
	int success;
	int error_code;
	struct fwcmd_die_sparing_policy_data *p_data;
};

/*
 * Firmware Command Function die_sparing_policy
 */
struct fwcmd_die_sparing_policy_result fwcmd_alloc_die_sparing_policy(unsigned int handle);

void fwcmd_free_die_sparing_policy(struct fwcmd_die_sparing_policy_result *p_result);
/*
 * Data Structures for address_range_scrub
 */
struct fwcmd_address_range_scrub_data
{
	unsigned char enable;
	unsigned long long dpa_start_address;
	unsigned long long dpa_end_address;
	unsigned long long dpa_current_address;

};
struct fwcmd_address_range_scrub_result
{
	int success;
	int error_code;
	struct fwcmd_address_range_scrub_data *p_data;
};

/*
 * Firmware Command Function address_range_scrub
 */
struct fwcmd_address_range_scrub_result fwcmd_alloc_address_range_scrub(unsigned int handle);

void fwcmd_free_address_range_scrub(struct fwcmd_address_range_scrub_result *p_result);
/*
 * Data Structures for optional_configuration_data_policy
 */
struct fwcmd_optional_configuration_data_policy_data
{
	unsigned char first_fast_refresh;
	unsigned char viral_policy_enabled;
	unsigned char viral_status;

};
struct fwcmd_optional_configuration_data_policy_result
{
	int success;
	int error_code;
	struct fwcmd_optional_configuration_data_policy_data *p_data;
};

/*
 * Firmware Command Function optional_configuration_data_policy
 */
struct fwcmd_optional_configuration_data_policy_result fwcmd_alloc_optional_configuration_data_policy(unsigned int handle);

void fwcmd_free_optional_configuration_data_policy(struct fwcmd_optional_configuration_data_policy_result *p_result);
/*
 * Data Structures for pmon_registers
 */
struct fwcmd_pmon_registers_data
{
	unsigned short pmon_retreive_mask;
	unsigned int pmon_0_counter;
	unsigned int pmon_0_control;
	unsigned int pmon_1_counter;
	unsigned int pmon_1_control;
	unsigned int pmon_2_counter;
	unsigned int pmon_2_control;
	unsigned int pmon_3_counter;
	unsigned int pmon_3_control;
	unsigned int pmon_4_counter;
	unsigned int pmon_4_control;
	unsigned int pmon_5_counter;
	unsigned int pmon_5_control;
	unsigned int pmon_6_counter;
	unsigned int pmon_6_control;
	unsigned int pmon_7_counter;
	unsigned int pmon_7_control;
	unsigned int pmon_8_counter;
	unsigned int pmon_8_control;
	unsigned int pmon_9_counter;
	unsigned int pmon_9_control;
	unsigned int pmon_10_counter;
	unsigned int pmon_10_control;
	unsigned int pmon_11_counter;
	unsigned int pmon_11_control;
	unsigned int pmon_14_counter;
	unsigned int pmon_14_control;

};
struct fwcmd_pmon_registers_result
{
	int success;
	int error_code;
	struct fwcmd_pmon_registers_data *p_data;
};

/*
 * Firmware Command Function pmon_registers
 */
struct fwcmd_pmon_registers_result fwcmd_alloc_pmon_registers(unsigned int handle,
	const unsigned short pmon_retreive_mask);

void fwcmd_free_pmon_registers(struct fwcmd_pmon_registers_result *p_result);
/*
 * Data Structures for set_alarm_threshold
 */
struct fwcmd_set_alarm_threshold_result
{
	int success;
	int error_code;
};

/*
 * Firmware Command Function set_alarm_threshold
 */
struct fwcmd_set_alarm_threshold_result fwcmd_call_set_alarm_threshold(unsigned int handle,
	const unsigned char enable,
	const unsigned short peak_power_budget,
	const unsigned short avg_power_budget);

/*
 * Data Structures for system_time
 */
struct fwcmd_system_time_data
{
	unsigned long long unix_time;

};
struct fwcmd_system_time_result
{
	int success;
	int error_code;
	struct fwcmd_system_time_data *p_data;
};

/*
 * Firmware Command Function system_time
 */
struct fwcmd_system_time_result fwcmd_alloc_system_time(unsigned int handle);

void fwcmd_free_system_time(struct fwcmd_system_time_result *p_result);
/*
 * Data Structures for platform_config_data
 */
struct fwcmd_dimm_info_for_interleave_set_data
{
	unsigned short manufacturer_id;
	unsigned int serial_number;
	char model_number[21];
	unsigned long long partition_offset;
	unsigned long long partition_size;

};
struct fwcmd_dimm_interleave_information_data
{
	unsigned short type;
	unsigned short length;
	unsigned short index;
	unsigned char number_of_dimms;
	unsigned char memory_type;
	unsigned int format;
	unsigned char mirror_enabled;
	unsigned char change_status;
	unsigned char memory_spare;
	int dimm_info_for_interleave_set_count;
	struct fwcmd_dimm_info_for_interleave_set_data *dimm_info_for_interleave_set;


};
struct fwcmd_dimm_partition_size_change_data
{
	unsigned short type;
	unsigned short length;
	unsigned int partition_size_change_status;
	unsigned long long persistent_memory_partition_size;

};
struct fwcmd_current_config_data
{
	char signature[5];
	unsigned int length;
	unsigned char revision;
	unsigned char checksum;
	char oem_id[7];
	char oem_table_id[9];
	unsigned int oem_revision;
	unsigned int creator_id;
	unsigned int creator_revision;
	unsigned short config_status;
	unsigned long long volatile_memory_size;
	unsigned long long persistent_memory_size;
	int dimm_interleave_information_count;
	struct fwcmd_dimm_interleave_information_data *dimm_interleave_information;


};
struct fwcmd_input_config_data
{
	char signature[5];
	unsigned int length;
	unsigned char revision;
	unsigned char checksum;
	char oem_id[7];
	char oem_table_id[9];
	unsigned int oem_revision;
	unsigned int creator_id;
	unsigned int creator_revision;
	unsigned int sequence_number;
	int dimm_interleave_information_count;
	struct fwcmd_dimm_interleave_information_data *dimm_interleave_information;

	int dimm_partition_size_change_count;
	struct fwcmd_dimm_partition_size_change_data *dimm_partition_size_change;


};
struct fwcmd_output_config_data
{
	char signature[5];
	unsigned int length;
	unsigned char revision;
	unsigned char checksum;
	char oem_id[7];
	char oem_table_id[9];
	unsigned int oem_revision;
	unsigned int creator_id;
	unsigned int creator_revision;
	unsigned int sequence_number;
	unsigned char validation_status;
	int dimm_interleave_information_count;
	struct fwcmd_dimm_interleave_information_data *dimm_interleave_information;

	int dimm_partition_size_change_count;
	struct fwcmd_dimm_partition_size_change_data *dimm_partition_size_change;


};
struct fwcmd_platform_config_data_data
{
	char signature[5];
	unsigned int length;
	unsigned char revision;
	unsigned char checksum;
	char oem_id[7];
	char oem_table_id[9];
	unsigned int oem_revision;
	unsigned int creator_id;
	unsigned int creator_revision;
	unsigned int current_config_size;
	unsigned int current_config_offset;
	unsigned int input_config_size;
	unsigned int input_config_offset;
	unsigned int output_config_size;
	unsigned int output_config_offset;
	struct fwcmd_current_config_data current_config;
	struct fwcmd_input_config_data input_config;
	struct fwcmd_output_config_data output_config;

};
struct fwcmd_platform_config_data_result
{
	int success;
	int error_code;
	struct fwcmd_platform_config_data_data *p_data;
};

/*
 * Firmware Command Function platform_config_data
 */
struct fwcmd_platform_config_data_result fwcmd_alloc_platform_config_data(unsigned int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset);

void fwcmd_free_platform_config_data(struct fwcmd_platform_config_data_result *p_result);
/*
 * Data Structures for dimm_partition_info
 */
struct fwcmd_dimm_partition_info_data
{
	unsigned int volatile_capacity;
	unsigned long long volatile_start;
	unsigned int pm_capacity;
	unsigned long long pm_start;
	unsigned int raw_capacity;
	unsigned int enabled_capacity;

};
struct fwcmd_dimm_partition_info_result
{
	int success;
	int error_code;
	struct fwcmd_dimm_partition_info_data *p_data;
};

/*
 * Firmware Command Function dimm_partition_info
 */
struct fwcmd_dimm_partition_info_result fwcmd_alloc_dimm_partition_info(unsigned int handle);

void fwcmd_free_dimm_partition_info(struct fwcmd_dimm_partition_info_result *p_result);
/*
 * Data Structures for fw_debug_log_level
 */
struct fwcmd_fw_debug_log_level_data
{
	unsigned char log_level;
	unsigned char logs;

};
struct fwcmd_fw_debug_log_level_result
{
	int success;
	int error_code;
	struct fwcmd_fw_debug_log_level_data *p_data;
};

/*
 * Firmware Command Function fw_debug_log_level
 */
struct fwcmd_fw_debug_log_level_result fwcmd_alloc_fw_debug_log_level(unsigned int handle,
	const unsigned char log_id);

void fwcmd_free_fw_debug_log_level(struct fwcmd_fw_debug_log_level_result *p_result);
/*
 * Data Structures for fw_load_flag
 */
struct fwcmd_fw_load_flag_data
{
	unsigned char load_flag;

};
struct fwcmd_fw_load_flag_result
{
	int success;
	int error_code;
	struct fwcmd_fw_load_flag_data *p_data;
};

/*
 * Firmware Command Function fw_load_flag
 */
struct fwcmd_fw_load_flag_result fwcmd_alloc_fw_load_flag(unsigned int handle);

void fwcmd_free_fw_load_flag(struct fwcmd_fw_load_flag_result *p_result);
/*
 * Data Structures for config_lockdown
 */
struct fwcmd_config_lockdown_data
{
	unsigned char locked;

};
struct fwcmd_config_lockdown_result
{
	int success;
	int error_code;
	struct fwcmd_config_lockdown_data *p_data;
};

/*
 * Firmware Command Function config_lockdown
 */
struct fwcmd_config_lockdown_result fwcmd_alloc_config_lockdown(unsigned int handle);

void fwcmd_free_config_lockdown(struct fwcmd_config_lockdown_result *p_result);
/*
 * Data Structures for ddrt_io_init_info
 */
struct fwcmd_ddrt_io_init_info_data
{
	unsigned char ddrt_io_info;
	unsigned char ddrt_training_complete;

};
struct fwcmd_ddrt_io_init_info_result
{
	int success;
	int error_code;
	struct fwcmd_ddrt_io_init_info_data *p_data;
};

/*
 * Firmware Command Function ddrt_io_init_info
 */
struct fwcmd_ddrt_io_init_info_result fwcmd_alloc_ddrt_io_init_info(unsigned int handle);

void fwcmd_free_ddrt_io_init_info(struct fwcmd_ddrt_io_init_info_result *p_result);
/*
 * Data Structures for get_supported_sku_features
 */
struct fwcmd_get_supported_sku_features_data
{
	unsigned int dimm_sku;

};
struct fwcmd_get_supported_sku_features_result
{
	int success;
	int error_code;
	struct fwcmd_get_supported_sku_features_data *p_data;
};

/*
 * Firmware Command Function get_supported_sku_features
 */
struct fwcmd_get_supported_sku_features_result fwcmd_alloc_get_supported_sku_features(unsigned int handle);

void fwcmd_free_get_supported_sku_features(struct fwcmd_get_supported_sku_features_result *p_result);
/*
 * Data Structures for enable_dimm
 */
struct fwcmd_enable_dimm_data
{
	unsigned char enable;

};
struct fwcmd_enable_dimm_result
{
	int success;
	int error_code;
	struct fwcmd_enable_dimm_data *p_data;
};

/*
 * Firmware Command Function enable_dimm
 */
struct fwcmd_enable_dimm_result fwcmd_alloc_enable_dimm(unsigned int handle);

void fwcmd_free_enable_dimm(struct fwcmd_enable_dimm_result *p_result);
/*
 * Data Structures for smart_health_info
 */
struct fwcmd_smart_health_info_data
{
	unsigned int validation_flags;
	unsigned char validation_flags_health_status;
	unsigned char validation_flags_spare_blocks;
	unsigned char validation_flags_percent_used;
	unsigned char validation_flags_media_temp;
	unsigned char validation_flags_controller_temp;
	unsigned char validation_flags_unsafe_shutdown_counter;
	unsigned char validation_flags_ait_dram_status;
	unsigned char validation_flags_alarm_trips;
	unsigned char validation_flags_last_shutdown_status;
	unsigned char validation_flags_vendor_specific_data_size;
	unsigned char health_status;
	unsigned char health_status_noncritical;
	unsigned char health_status_critical;
	unsigned char health_status_fatal;
	unsigned char spare_blocks;
	unsigned char percent_used;
	unsigned char alarm_trips;
	unsigned char alarm_trips_spare_block_trip;
	unsigned char alarm_trips_media_temperature_trip;
	unsigned char alarm_trips_controller_temperature_trip;
	unsigned short media_temp;
	unsigned short controller_temp;
	unsigned int unsafe_shutdown_count;
	unsigned char ait_dram_status;
	unsigned char last_shutdown_status;
	unsigned int vendor_specific_data_size;
	unsigned long long power_cycles;
	unsigned long long power_on_time;
	unsigned long long uptime;
	unsigned int unsafe_shutdowns;
	unsigned char last_shutdown_status_details;
	unsigned char last_shutdown_status_details_pm_adr_command_received;
	unsigned char last_shutdown_status_details_pm_s3_received;
	unsigned char last_shutdown_status_details_pm_s5_received;
	unsigned char last_shutdown_status_details_ddrt_power_fail_command_received;
	unsigned char last_shutdown_status_details_pmic_12v_power_fail;
	unsigned char last_shutdown_status_details_pm_warm_reset_received;
	unsigned char last_shutdown_status_details_thermal_shutdown_received;
	unsigned char last_shutdown_status_details_flush_complete;
	unsigned long long last_shutdown_time;

};
struct fwcmd_smart_health_info_result
{
	int success;
	int error_code;
	struct fwcmd_smart_health_info_data *p_data;
};

/*
 * Firmware Command Function smart_health_info
 */
struct fwcmd_smart_health_info_result fwcmd_alloc_smart_health_info(unsigned int handle);

void fwcmd_free_smart_health_info(struct fwcmd_smart_health_info_result *p_result);
/*
 * Data Structures for firmware_image_info
 */
struct fwcmd_firmware_image_info_data
{
	char firmware_revision[5];
	unsigned char firmware_type;
	char staged_fw_revision[5];
	unsigned char last_fw_update_status;
	char commit_id[41];
	char build_configuration[17];

};
struct fwcmd_firmware_image_info_result
{
	int success;
	int error_code;
	struct fwcmd_firmware_image_info_data *p_data;
};

/*
 * Firmware Command Function firmware_image_info
 */
struct fwcmd_firmware_image_info_result fwcmd_alloc_firmware_image_info(unsigned int handle);

void fwcmd_free_firmware_image_info(struct fwcmd_firmware_image_info_result *p_result);
/*
 * Data Structures for firmware_debug_log
 */
struct fwcmd_firmware_debug_log_data
{
	unsigned char log_size;

};
struct fwcmd_firmware_debug_log_result
{
	int success;
	int error_code;
	struct fwcmd_firmware_debug_log_data *p_data;
};

/*
 * Firmware Command Function firmware_debug_log
 */
struct fwcmd_firmware_debug_log_result fwcmd_alloc_firmware_debug_log(unsigned int handle,
	const unsigned char log_action,
	const unsigned int log_page_offset,
	const unsigned char log_id);

void fwcmd_free_firmware_debug_log(struct fwcmd_firmware_debug_log_result *p_result);
/*
 * Data Structures for long_operation_status
 */
struct fwcmd_long_operation_status_data
{
	unsigned short command;
	unsigned short percent_complete;
	unsigned int estimate_time_to_completion;
	unsigned char status_code;
	char command_specific_return_data[119];

};
struct fwcmd_long_operation_status_result
{
	int success;
	int error_code;
	struct fwcmd_long_operation_status_data *p_data;
};

/*
 * Firmware Command Function long_operation_status
 */
struct fwcmd_long_operation_status_result fwcmd_alloc_long_operation_status(unsigned int handle);

void fwcmd_free_long_operation_status(struct fwcmd_long_operation_status_result *p_result);
/*
 * Data Structures for bsr
 */
struct fwcmd_bsr_data
{
	unsigned char major_checkpoint;
	unsigned char minor_checkpoint;
	unsigned int rest1;
	unsigned char rest1_media_ready_1;
	unsigned char rest1_media_ready_2;
	unsigned char rest1_ddrt_io_init_complete;
	unsigned char rest1_pcr_lock;
	unsigned char rest1_mailbox_ready;
	unsigned char rest1_watch_dog_status;
	unsigned char rest1_first_fast_refresh_complete;
	unsigned char rest1_credit_ready;
	unsigned char rest1_media_disabled;
	unsigned char rest1_opt_in_enabled;
	unsigned char rest1_opt_in_was_enabled;
	unsigned char rest1_assertion;
	unsigned char rest1_mi_stall;
	unsigned char rest1_ait_dram_ready;
	unsigned short rest2;

};
struct fwcmd_bsr_result
{
	int success;
	int error_code;
	struct fwcmd_bsr_data *p_data;
};

/*
 * Firmware Command Function bsr
 */
struct fwcmd_bsr_result fwcmd_alloc_bsr(unsigned int handle);

void fwcmd_free_bsr(struct fwcmd_bsr_result *p_result);
int fwcmd_is_command_name(const char * cmd_name);

int fwcmd_is_output_command_name(const char * cmd_name);

#ifdef __cplusplus
}
#endif

#endif //CR_MGMT_FW_COMMANDS_H