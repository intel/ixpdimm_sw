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

#include <stdio.h>
#include <string.h>
#include "ixp_fis_properties.h"
#include <fw_commands.h>
#include <ixp.h>



struct ixp_fw_lookup_t
{
	int struct_offset;
	int size;
	char name[IXP_MAX_PROPERTY_NAME_SZ];
};

// Unfortunately we can't use named struct elements here (.fwcmd = ...)
// Visual Studio doesn't like it for some reason. Un-named it is then!
static const struct ixp_fw_lookup_t g_ixp_fw_lookup[] = {
	{offsetof(struct fwcmd_identify_dimm_data, vendor_id), 2, "VendorId"},
	{offsetof(struct fwcmd_identify_dimm_data, device_id), 2, "DeviceId"},
	{offsetof(struct fwcmd_identify_dimm_data, revision_id), 2, "RevisionId"},
	{offsetof(struct fwcmd_identify_dimm_data, interface_format_code), 2, "InterfaceFormatCode"},
	{offsetof(struct fwcmd_identify_dimm_data, firmware_revision), 5, "FirmwareRevision"},
	{offsetof(struct fwcmd_identify_dimm_data, reserved_old_api), 1, "ReservedOldApi"},
	{offsetof(struct fwcmd_identify_dimm_data, feature_sw_required_mask), 1, "FeatureSwRequiredMask"},
	{offsetof(struct fwcmd_identify_dimm_data, feature_sw_required_mask_invalidate_before_block_read), 1, "InvalidateBeforeBlockRead"},
	{offsetof(struct fwcmd_identify_dimm_data, feature_sw_required_mask_readback_of_bw_address_register_required_before_use), 1, "ReadbackOfBwAddressRegisterRequiredBeforeUse"},
	{offsetof(struct fwcmd_identify_dimm_data, number_of_block_windows), 2, "NumberOfBlockWindows"},
	{offsetof(struct fwcmd_identify_dimm_data, offset_of_block_mode_control_region), 4, "OffsetOfBlockModeControlRegion"},
	{offsetof(struct fwcmd_identify_dimm_data, raw_capacity), 4, "RawCapacity"},
	{offsetof(struct fwcmd_identify_dimm_data, manufacturer), 2, "Manufacturer"},
	{offsetof(struct fwcmd_identify_dimm_data, serial_number), 4, "SerialNumber"},
	{offsetof(struct fwcmd_identify_dimm_data, part_number), 20, "PartNumber"},
	{offsetof(struct fwcmd_identify_dimm_data, dimm_sku), 4, "DimmSku"},
	{offsetof(struct fwcmd_identify_dimm_data, dimm_sku_memory_mode_enabled), 1, "MemoryModeEnabled"},
	{offsetof(struct fwcmd_identify_dimm_data, dimm_sku_storage_mode_enabled), 1, "StorageModeEnabled"},
	{offsetof(struct fwcmd_identify_dimm_data, dimm_sku_app_direct_mode_enabled), 1, "AppDirectModeEnabled"},
	{offsetof(struct fwcmd_identify_dimm_data, dimm_sku_die_sparing_capable), 1, "DieSparingCapable"},
	{offsetof(struct fwcmd_identify_dimm_data, dimm_sku_soft_programmable_sku), 1, "SoftProgrammableSku"},
	{offsetof(struct fwcmd_identify_dimm_data, dimm_sku_encryption_enabled), 1, "EncryptionEnabled"},
	{offsetof(struct fwcmd_identify_dimm_data, interface_format_code_extra), 2, "InterfaceFormatCodeExtra"},
	{offsetof(struct fwcmd_identify_dimm_data, api_ver), 2, "ApiVer"},
	{offsetof(struct fwcmd_identify_dimm_characteristics_data, controller_temp_shutdown_threshold), 2, "ControllerTempShutdownThreshold"},
	{offsetof(struct fwcmd_identify_dimm_characteristics_data, media_temp_shutdown_threshold), 2, "MediaTempShutdownThreshold"},
	{offsetof(struct fwcmd_identify_dimm_characteristics_data, throttling_start_threshold), 2, "ThrottlingStartThreshold"},
	{offsetof(struct fwcmd_identify_dimm_characteristics_data, throttling_stop_threshold), 2, "ThrottlingStopThreshold"},
	{offsetof(struct fwcmd_get_security_state_data, security_state), 1, "SecurityState"},
	{offsetof(struct fwcmd_get_security_state_data, security_state_enabled), 1, "Enabled"},
	{offsetof(struct fwcmd_get_security_state_data, security_state_locked), 1, "Locked"},
	{offsetof(struct fwcmd_get_security_state_data, security_state_frozen), 1, "Frozen"},
	{offsetof(struct fwcmd_get_security_state_data, security_state_count_expired), 1, "CountExpired"},
	{offsetof(struct fwcmd_get_security_state_data, security_state_not_supported), 1, "NotSupported"},
	{offsetof(struct fwcmd_get_alarm_threshold_data, enable), 2, "Enable"},
	{offsetof(struct fwcmd_get_alarm_threshold_data, enable_spare_block), 1, "SpareBlock"},
	{offsetof(struct fwcmd_get_alarm_threshold_data, enable_media_temp), 1, "MediaTemp"},
	{offsetof(struct fwcmd_get_alarm_threshold_data, enable_controller_temp), 1, "ControllerTemp"},
	{offsetof(struct fwcmd_get_alarm_threshold_data, spare_block_threshold), 1, "SpareBlockThreshold"},
	{offsetof(struct fwcmd_get_alarm_threshold_data, media_temp_threshold), 2, "MediaTempThreshold"},
	{offsetof(struct fwcmd_get_alarm_threshold_data, controller_temp_threshold), 2, "ControllerTempThreshold"},
	{offsetof(struct fwcmd_power_management_policy_data, enable), 1, "Enable"},
	{offsetof(struct fwcmd_power_management_policy_data, peak_power_budget), 2, "PeakPowerBudget"},
	{offsetof(struct fwcmd_power_management_policy_data, average_power_budget), 2, "AveragePowerBudget"},
	{offsetof(struct fwcmd_power_management_policy_data, max_power), 1, "MaxPower"},
	{offsetof(struct fwcmd_die_sparing_policy_data, enable), 1, "Enable"},
	{offsetof(struct fwcmd_die_sparing_policy_data, aggressiveness), 1, "Aggressiveness"},
	{offsetof(struct fwcmd_die_sparing_policy_data, supported), 1, "Supported"},
	{offsetof(struct fwcmd_die_sparing_policy_data, supported_rank_0), 1, "Rank0"},
	{offsetof(struct fwcmd_die_sparing_policy_data, supported_rank_1), 1, "Rank1"},
	{offsetof(struct fwcmd_die_sparing_policy_data, supported_rank_2), 1, "Rank2"},
	{offsetof(struct fwcmd_die_sparing_policy_data, supported_rank_3), 1, "Rank3"},
	{offsetof(struct fwcmd_address_range_scrub_data, enable), 1, "Enable"},
	{offsetof(struct fwcmd_address_range_scrub_data, dpa_start_address), 8, "DpaStartAddress"},
	{offsetof(struct fwcmd_address_range_scrub_data, dpa_end_address), 8, "DpaEndAddress"},
	{offsetof(struct fwcmd_address_range_scrub_data, dpa_current_address), 8, "DpaCurrentAddress"},
	{offsetof(struct fwcmd_optional_configuration_data_policy_data, first_fast_refresh), 1, "FirstFastRefresh"},
	{offsetof(struct fwcmd_optional_configuration_data_policy_data, viral_policy_enabled), 1, "ViralPolicyEnabled"},
	{offsetof(struct fwcmd_optional_configuration_data_policy_data, viral_status), 1, "ViralStatus"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_retreive_mask), 2, "PmonRetreiveMask"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_0_counter), 4, "Pmon0Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_0_control), 4, "Pmon0Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_1_counter), 4, "Pmon1Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_1_control), 4, "Pmon1Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_2_counter), 4, "Pmon2Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_2_control), 4, "Pmon2Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_3_counter), 4, "Pmon3Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_3_control), 4, "Pmon3Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_4_counter), 4, "Pmon4Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_4_control), 4, "Pmon4Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_5_counter), 4, "Pmon5Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_5_control), 4, "Pmon5Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_6_counter), 4, "Pmon6Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_6_control), 4, "Pmon6Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_7_counter), 4, "Pmon7Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_7_control), 4, "Pmon7Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_8_counter), 4, "Pmon8Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_8_control), 4, "Pmon8Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_9_counter), 4, "Pmon9Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_9_control), 4, "Pmon9Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_10_counter), 4, "Pmon10Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_10_control), 4, "Pmon10Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_11_counter), 4, "Pmon11Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_11_control), 4, "Pmon11Control"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_14_counter), 4, "Pmon14Counter"},
	{offsetof(struct fwcmd_pmon_registers_data, pmon_14_control), 4, "Pmon14Control"},
	{offsetof(struct fwcmd_system_time_data, unix_time), 8, "UnixTime"},
	{offsetof(struct fwcmd_platform_config_data_data, signature), 4, "Signature"},
	{offsetof(struct fwcmd_platform_config_data_data, length), 4, "Length"},
	{offsetof(struct fwcmd_platform_config_data_data, revision), 1, "Revision"},
	{offsetof(struct fwcmd_platform_config_data_data, checksum), 1, "Checksum"},
	{offsetof(struct fwcmd_platform_config_data_data, oem_id), 6, "OemId"},
	{offsetof(struct fwcmd_platform_config_data_data, oem_table_id), 8, "OemTableId"},
	{offsetof(struct fwcmd_platform_config_data_data, oem_revision), 4, "OemRevision"},
	{offsetof(struct fwcmd_platform_config_data_data, creator_id), 4, "CreatorId"},
	{offsetof(struct fwcmd_platform_config_data_data, creator_revision), 4, "CreatorRevision"},
	{offsetof(struct fwcmd_platform_config_data_data, current_config_size), 4, "CurrentConfigSize"},
	{offsetof(struct fwcmd_platform_config_data_data, current_config_offset), 4, "CurrentConfigOffset"},
	{offsetof(struct fwcmd_platform_config_data_data, input_config_size), 4, "InputConfigSize"},
	{offsetof(struct fwcmd_platform_config_data_data, input_config_offset), 4, "InputConfigOffset"},
	{offsetof(struct fwcmd_platform_config_data_data, output_config_size), 4, "OutputConfigSize"},
	{offsetof(struct fwcmd_platform_config_data_data, output_config_offset), 4, "OutputConfigOffset"},
	{offsetof(struct fwcmd_dimm_partition_info_data, volatile_capacity), 4, "VolatileCapacity"},
	{offsetof(struct fwcmd_dimm_partition_info_data, volatile_start), 8, "VolatileStart"},
	{offsetof(struct fwcmd_dimm_partition_info_data, pm_capacity), 4, "PmCapacity"},
	{offsetof(struct fwcmd_dimm_partition_info_data, pm_start), 8, "PmStart"},
	{offsetof(struct fwcmd_dimm_partition_info_data, raw_capacity), 4, "RawCapacity"},
	{offsetof(struct fwcmd_dimm_partition_info_data, enabled_capacity), 4, "EnabledCapacity"},
	{offsetof(struct fwcmd_fw_debug_log_level_data, log_level), 1, "LogLevel"},
	{offsetof(struct fwcmd_fw_debug_log_level_data, logs), 1, "Logs"},
	{offsetof(struct fwcmd_fw_load_flag_data, load_flag), 1, "LoadFlag"},
	{offsetof(struct fwcmd_config_lockdown_data, locked), 1, "Locked"},
	{offsetof(struct fwcmd_ddrt_io_init_info_data, ddrt_io_info), 1, "DdrtIoInfo"},
	{offsetof(struct fwcmd_ddrt_io_init_info_data, ddrt_training_status), 1, "DdrtTrainingStatus"},
	{offsetof(struct fwcmd_get_supported_sku_features_data, dimm_sku), 4, "DimmSku"},
	{offsetof(struct fwcmd_enable_dimm_data, enable), 1, "Enable"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags), 4, "ValidationFlags"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_health_status), 1, "HealthStatus"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_spare_blocks), 1, "SpareBlocks"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_percent_used), 1, "PercentUsed"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_media_temp), 1, "MediaTemp"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_controller_temp), 1, "ControllerTemp"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_unsafe_shutdown_counter), 1, "UnsafeShutdownCounter"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_ait_dram_status), 1, "AitDramStatus"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_alarm_trips), 1, "AlarmTrips"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_last_shutdown_status), 1, "LastShutdownStatus"},
	{offsetof(struct fwcmd_smart_health_info_data, validation_flags_vendor_specific_data_size), 1, "VendorSpecificDataSize"},
	{offsetof(struct fwcmd_smart_health_info_data, health_status), 1, "HealthStatus"},
	{offsetof(struct fwcmd_smart_health_info_data, health_status_noncritical), 1, "Noncritical"},
	{offsetof(struct fwcmd_smart_health_info_data, health_status_critical), 1, "Critical"},
	{offsetof(struct fwcmd_smart_health_info_data, health_status_fatal), 1, "Fatal"},
	{offsetof(struct fwcmd_smart_health_info_data, spare_blocks), 1, "SpareBlocks"},
	{offsetof(struct fwcmd_smart_health_info_data, percent_used), 1, "PercentUsed"},
	{offsetof(struct fwcmd_smart_health_info_data, alarm_trips), 1, "AlarmTrips"},
	{offsetof(struct fwcmd_smart_health_info_data, alarm_trips_spare_block_trip), 1, "SpareBlockTrip"},
	{offsetof(struct fwcmd_smart_health_info_data, alarm_trips_media_temperature_trip), 1, "MediaTemperatureTrip"},
	{offsetof(struct fwcmd_smart_health_info_data, alarm_trips_controller_temperature_trip), 1, "ControllerTemperatureTrip"},
	{offsetof(struct fwcmd_smart_health_info_data, media_temp), 2, "MediaTemp"},
	{offsetof(struct fwcmd_smart_health_info_data, controller_temp), 2, "ControllerTemp"},
	{offsetof(struct fwcmd_smart_health_info_data, unsafe_shutdown_count), 4, "UnsafeShutdownCount"},
	{offsetof(struct fwcmd_smart_health_info_data, ait_dram_status), 1, "AitDramStatus"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status), 1, "LastShutdownStatus"},
	{offsetof(struct fwcmd_smart_health_info_data, vendor_specific_data_size), 4, "VendorSpecificDataSize"},
	{offsetof(struct fwcmd_smart_health_info_data, power_cycles), 8, "PowerCycles"},
	{offsetof(struct fwcmd_smart_health_info_data, power_on_time), 8, "PowerOnTime"},
	{offsetof(struct fwcmd_smart_health_info_data, uptime), 8, "Uptime"},
	{offsetof(struct fwcmd_smart_health_info_data, unsafe_shutdowns), 4, "UnsafeShutdowns"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_details), 1, "LastShutdownStatusDetails"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_details_pm_adr_command_received), 1, "PmAdrCommandReceived"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_details_pm_s3_received), 1, "PmS3Received"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_details_pm_s5_received), 1, "PmS5Received"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_details_ddrt_power_fail_command_received), 1, "DdrtPowerFailCommandReceived"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_details_pmic_12v_power_fail), 1, "Pmic12vPowerFail"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_details_pm_warm_reset_received), 1, "PmWarmResetReceived"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_details_thermal_shutdown_received), 1, "ThermalShutdownReceived"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_details_flush_complete), 1, "FlushComplete"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_time), 8, "LastShutdownTime"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_extended_details), 1, "LastShutdownStatusExtendedDetails"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_extended_details_viral_interrupt_received), 1, "ViralInterruptReceived"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_extended_details_surprise_clock_stop_interrupt_received), 1, "SurpriseClockStopInterruptReceived"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_extended_details_write_data_flush_complete), 1, "WriteDataFlushComplete"},
	{offsetof(struct fwcmd_smart_health_info_data, last_shutdown_status_extended_details_s4_power_state_received), 1, "S4PowerStateReceived"},
	{offsetof(struct fwcmd_smart_health_info_data, media_error_injections), 4, "MediaErrorInjections"},
	{offsetof(struct fwcmd_smart_health_info_data, non_media_error_injections), 4, "NonMediaErrorInjections"},
	{offsetof(struct fwcmd_firmware_image_info_data, firmware_revision), 5, "FirmwareRevision"},
	{offsetof(struct fwcmd_firmware_image_info_data, firmware_type), 1, "FirmwareType"},
	{offsetof(struct fwcmd_firmware_image_info_data, staged_fw_revision), 5, "StagedFwRevision"},
	{offsetof(struct fwcmd_firmware_image_info_data, last_fw_update_status), 1, "LastFwUpdateStatus"},
	{offsetof(struct fwcmd_firmware_image_info_data, commit_id), 40, "CommitId"},
	{offsetof(struct fwcmd_firmware_image_info_data, build_configuration), 16, "BuildConfiguration"},
	{offsetof(struct fwcmd_firmware_debug_log_data, log_size), 1, "LogSize"},
	{offsetof(struct fwcmd_memory_info_page_0_data, media_reads), 16, "MediaReads"},
	{offsetof(struct fwcmd_memory_info_page_0_data, media_writes), 16, "MediaWrites"},
	{offsetof(struct fwcmd_memory_info_page_0_data, read_requests), 16, "ReadRequests"},
	{offsetof(struct fwcmd_memory_info_page_0_data, write_requests), 16, "WriteRequests"},
	{offsetof(struct fwcmd_memory_info_page_0_data, block_read_requests), 16, "BlockReadRequests"},
	{offsetof(struct fwcmd_memory_info_page_0_data, block_write_requests), 16, "BlockWriteRequests"},
	{offsetof(struct fwcmd_memory_info_page_1_data, total_media_reads), 16, "TotalMediaReads"},
	{offsetof(struct fwcmd_memory_info_page_1_data, total_media_writes), 16, "TotalMediaWrites"},
	{offsetof(struct fwcmd_memory_info_page_1_data, total_read_requests), 16, "TotalReadRequests"},
	{offsetof(struct fwcmd_memory_info_page_1_data, total_write_requests), 16, "TotalWriteRequests"},
	{offsetof(struct fwcmd_memory_info_page_1_data, total_block_read_requests), 16, "TotalBlockReadRequests"},
	{offsetof(struct fwcmd_memory_info_page_1_data, total_block_write_requests), 16, "TotalBlockWriteRequests"},
	{offsetof(struct fwcmd_memory_info_page_3_data, error_injection_status), 4, "ErrorInjectionStatus"},
	{offsetof(struct fwcmd_memory_info_page_3_data, error_injection_status_error_injection_enabled), 1, "ErrorInjectionEnabled"},
	{offsetof(struct fwcmd_memory_info_page_3_data, error_injection_status_media_temperature_injection_enabled), 1, "MediaTemperatureInjectionEnabled"},
	{offsetof(struct fwcmd_memory_info_page_3_data, error_injection_status_software_triggers_enabled), 1, "SoftwareTriggersEnabled"},
	{offsetof(struct fwcmd_memory_info_page_3_data, poison_error_injections_counter), 4, "PoisonErrorInjectionsCounter"},
	{offsetof(struct fwcmd_memory_info_page_3_data, poison_error_clear_counter), 4, "PoisonErrorClearCounter"},
	{offsetof(struct fwcmd_memory_info_page_3_data, media_temperature_injections_counter), 4, "MediaTemperatureInjectionsCounter"},
	{offsetof(struct fwcmd_memory_info_page_3_data, software_triggers_counter), 4, "SoftwareTriggersCounter"},
	{offsetof(struct fwcmd_long_operation_status_data, command), 2, "Command"},
	{offsetof(struct fwcmd_long_operation_status_data, percent_complete), 2, "PercentComplete"},
	{offsetof(struct fwcmd_long_operation_status_data, estimate_time_to_completion), 4, "EstimateTimeToCompletion"},
	{offsetof(struct fwcmd_long_operation_status_data, status_code), 1, "StatusCode"},
	{offsetof(struct fwcmd_long_operation_status_data, command_specific_return_data), 119, "CommandSpecificReturnData"},
	{offsetof(struct fwcmd_bsr_data, major_checkpoint), 1, "MajorCheckpoint"},
	{offsetof(struct fwcmd_bsr_data, minor_checkpoint), 1, "MinorCheckpoint"},
	{offsetof(struct fwcmd_bsr_data, rest1), 4, "Rest1"},
	{offsetof(struct fwcmd_bsr_data, rest1_media_ready_1), 1, "MediaReady1"},
	{offsetof(struct fwcmd_bsr_data, rest1_media_ready_2), 1, "MediaReady2"},
	{offsetof(struct fwcmd_bsr_data, rest1_ddrt_io_init_complete), 1, "DdrtIoInitComplete"},
	{offsetof(struct fwcmd_bsr_data, rest1_pcr_lock), 1, "PcrLock"},
	{offsetof(struct fwcmd_bsr_data, rest1_mailbox_ready), 1, "MailboxReady"},
	{offsetof(struct fwcmd_bsr_data, rest1_watch_dog_status), 1, "WatchDogStatus"},
	{offsetof(struct fwcmd_bsr_data, rest1_first_fast_refresh_complete), 1, "FirstFastRefreshComplete"},
	{offsetof(struct fwcmd_bsr_data, rest1_credit_ready), 1, "CreditReady"},
	{offsetof(struct fwcmd_bsr_data, rest1_media_disabled), 1, "MediaDisabled"},
	{offsetof(struct fwcmd_bsr_data, rest1_opt_in_enabled), 1, "OptInEnabled"},
	{offsetof(struct fwcmd_bsr_data, rest1_opt_in_was_enabled), 1, "OptInWasEnabled"},
	{offsetof(struct fwcmd_bsr_data, rest1_assertion), 1, "Assertion"},
	{offsetof(struct fwcmd_bsr_data, rest1_mi_stall), 1, "MiStall"},
	{offsetof(struct fwcmd_bsr_data, rest1_ait_dram_ready), 1, "AitDramReady"},
	{offsetof(struct fwcmd_bsr_data, rest2), 2, "Rest2"},
};


// Get result from lower-level fwcmd and then populate any relevant properties
// in the props array
int get_fw_cmd(struct fwcmd_result (* fwcmd)(unsigned int),
void (* fwcmd_free)(struct fwcmd_result *),
unsigned int prop_key_first, unsigned int prop_key_length, unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props, const struct ixp_fw_lookup_t ixp_fw_lookup[])
{
	struct fwcmd_result result = (*fwcmd)(handle);
	if (!result.success)
	{
		// TODO: Convert fw error code into IXP error code
		return IXP_INTERNAL_ERROR;
	}

	struct ixp_prop_info * prop;
	for (int props_index = 0; props_index < num_props; props_index++)
	{
		prop = &(props[props_index]);

		if (prop->prop_key < prop_key_first ||
		prop->prop_key >= prop_key_first + prop_key_length)
		{
			continue;
		}

		prop->prop_value_size = ixp_fw_lookup[prop->prop_key].size;
		prop->prop_value = malloc(ixp_fw_lookup[prop->prop_key].size);
		if (NULL == prop->prop_value)
		{
			return IXP_NO_MEM_RESOURCES;
		}

		// TODO: Linux doesn't have memcpy_s??
		// Copy property data
		// dest, src, size
		memcpy(prop->prop_value,
		(char *)result.p_data + ixp_fw_lookup[prop->prop_key].struct_offset,
		ixp_fw_lookup[prop->prop_key].size);

		// Copy name string
		// TODO: Verify with Kelly/Dan this is the proper implementation
		// Use strcat?? https://stackoverflow.com/questions/1258550/why-should-you-use-strncpy-instead-of-strcpy
		if (snprintf(prop->prop_name, IXP_MAX_PROPERTY_NAME_SZ,
		"%s", ixp_fw_lookup[prop->prop_key].name) >= IXP_MAX_PROPERTY_NAME_SZ)
		{
			fwcmd_free(&result);
			// Pretty print string name is >= IXP_MAX_PROPERTY_NAME_SZ
			return IXP_INTERNAL_ERROR;
		}
	}

	fwcmd_free(&result);
	return IXP_SUCCESS;
}

void free_fw_cmd(unsigned int prop_key_first, unsigned int prop_key_length,
struct ixp_prop_info props[], unsigned int num_props)
{
	struct ixp_prop_info * prop;
	// Only need to free the data portion of struct
	for (int props_index = 0; props_index < num_props; props_index++)
	{
		prop = &(props[props_index]);

		if (prop->prop_key < prop_key_first ||
		prop->prop_key >= prop_key_first + prop_key_length ||
		prop->prop_value == NULL)
		{
			continue;
		}

		free(prop->prop_value);
		prop->prop_value = NULL;
		prop->prop_value_size = 0;
	}
}

int get_fis_identify_dimm_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_identify_dimm,
	(void (*)(struct fwcmd_result*))&fwcmd_free_identify_dimm,
	0, 24, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_identify_dimm_characteristics_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_identify_dimm_characteristics,
	(void (*)(struct fwcmd_result*))&fwcmd_free_identify_dimm_characteristics,
	24, 4, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_get_security_state_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_get_security_state,
	(void (*)(struct fwcmd_result*))&fwcmd_free_get_security_state,
	28, 6, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_get_alarm_threshold_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_get_alarm_threshold,
	(void (*)(struct fwcmd_result*))&fwcmd_free_get_alarm_threshold,
	34, 7, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_power_management_policy_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_power_management_policy,
	(void (*)(struct fwcmd_result*))&fwcmd_free_power_management_policy,
	41, 4, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_die_sparing_policy_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_die_sparing_policy,
	(void (*)(struct fwcmd_result*))&fwcmd_free_die_sparing_policy,
	45, 7, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_address_range_scrub_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_address_range_scrub,
	(void (*)(struct fwcmd_result*))&fwcmd_free_address_range_scrub,
	52, 4, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_optional_configuration_data_policy_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_optional_configuration_data_policy,
	(void (*)(struct fwcmd_result*))&fwcmd_free_optional_configuration_data_policy,
	56, 3, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_pmon_registers_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_pmon_registers,
	(void (*)(struct fwcmd_result*))&fwcmd_free_pmon_registers,
	59, 27, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_system_time_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_system_time,
	(void (*)(struct fwcmd_result*))&fwcmd_free_system_time,
	86, 1, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_platform_config_data_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_platform_config_data,
	(void (*)(struct fwcmd_result*))&fwcmd_free_platform_config_data,
	87, 15, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_namespace_labels_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_namespace_labels,
	(void (*)(struct fwcmd_result*))&fwcmd_free_namespace_labels,
	102, 0, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_dimm_partition_info_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_dimm_partition_info,
	(void (*)(struct fwcmd_result*))&fwcmd_free_dimm_partition_info,
	102, 6, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_fw_debug_log_level_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_fw_debug_log_level,
	(void (*)(struct fwcmd_result*))&fwcmd_free_fw_debug_log_level,
	108, 2, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_fw_load_flag_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_fw_load_flag,
	(void (*)(struct fwcmd_result*))&fwcmd_free_fw_load_flag,
	110, 1, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_config_lockdown_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_config_lockdown,
	(void (*)(struct fwcmd_result*))&fwcmd_free_config_lockdown,
	111, 1, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_ddrt_io_init_info_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_ddrt_io_init_info,
	(void (*)(struct fwcmd_result*))&fwcmd_free_ddrt_io_init_info,
	112, 2, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_get_supported_sku_features_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_get_supported_sku_features,
	(void (*)(struct fwcmd_result*))&fwcmd_free_get_supported_sku_features,
	114, 1, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_enable_dimm_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_enable_dimm,
	(void (*)(struct fwcmd_result*))&fwcmd_free_enable_dimm,
	115, 1, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_smart_health_info_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_smart_health_info,
	(void (*)(struct fwcmd_result*))&fwcmd_free_smart_health_info,
	116, 48, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_firmware_image_info_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_firmware_image_info,
	(void (*)(struct fwcmd_result*))&fwcmd_free_firmware_image_info,
	164, 6, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_firmware_debug_log_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_firmware_debug_log,
	(void (*)(struct fwcmd_result*))&fwcmd_free_firmware_debug_log,
	170, 1, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_memory_info_page_0_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_memory_info_page_0,
	(void (*)(struct fwcmd_result*))&fwcmd_free_memory_info_page_0,
	171, 6, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_memory_info_page_1_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_memory_info_page_1,
	(void (*)(struct fwcmd_result*))&fwcmd_free_memory_info_page_1,
	177, 6, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_memory_info_page_3_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_memory_info_page_3,
	(void (*)(struct fwcmd_result*))&fwcmd_free_memory_info_page_3,
	183, 8, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_long_operation_status_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_long_operation_status,
	(void (*)(struct fwcmd_result*))&fwcmd_free_long_operation_status,
	191, 5, handle, props, num_props, g_ixp_fw_lookup);
}

int get_fis_bsr_properties(unsigned int handle,
struct ixp_prop_info props[], unsigned int num_props)
{
	return get_fw_cmd(
	(struct fwcmd_result (*)(unsigned int))&fwcmd_alloc_bsr,
	(void (*)(struct fwcmd_result*))&fwcmd_free_bsr,
	196, 18, handle, props, num_props, g_ixp_fw_lookup);
}

void free_fis_identify_dimm_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(0, 24, props, num_props);
}

void free_fis_identify_dimm_characteristics_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(24, 4, props, num_props);
}

void free_fis_get_security_state_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(28, 6, props, num_props);
}

void free_fis_get_alarm_threshold_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(34, 7, props, num_props);
}

void free_fis_power_management_policy_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(41, 4, props, num_props);
}

void free_fis_die_sparing_policy_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(45, 7, props, num_props);
}

void free_fis_address_range_scrub_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(52, 4, props, num_props);
}

void free_fis_optional_configuration_data_policy_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(56, 3, props, num_props);
}

void free_fis_pmon_registers_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(59, 27, props, num_props);
}

void free_fis_system_time_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(86, 1, props, num_props);
}

void free_fis_platform_config_data_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(87, 15, props, num_props);
}

void free_fis_namespace_labels_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(102, 0, props, num_props);
}

void free_fis_dimm_partition_info_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(102, 6, props, num_props);
}

void free_fis_fw_debug_log_level_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(108, 2, props, num_props);
}

void free_fis_fw_load_flag_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(110, 1, props, num_props);
}

void free_fis_config_lockdown_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(111, 1, props, num_props);
}

void free_fis_ddrt_io_init_info_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(112, 2, props, num_props);
}

void free_fis_get_supported_sku_features_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(114, 1, props, num_props);
}

void free_fis_enable_dimm_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(115, 1, props, num_props);
}

void free_fis_smart_health_info_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(116, 48, props, num_props);
}

void free_fis_firmware_image_info_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(164, 6, props, num_props);
}

void free_fis_firmware_debug_log_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(170, 1, props, num_props);
}

void free_fis_memory_info_page_0_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(171, 6, props, num_props);
}

void free_fis_memory_info_page_1_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(177, 6, props, num_props);
}

void free_fis_memory_info_page_3_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(183, 8, props, num_props);
}

void free_fis_long_operation_status_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(191, 5, props, num_props);
}

void free_fis_bsr_properties(
struct ixp_prop_info props[], unsigned int num_props)
{
	free_fw_cmd(196, 18, props, num_props);
}

