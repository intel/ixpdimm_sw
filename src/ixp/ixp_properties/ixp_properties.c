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

#include <string.h>
#include <ixp.h>
#include <ixp_prv.h>
#include "ixp_fis_properties.h"
#include "ixp_properties.h"

// TODO: Add more types of entries in here, not just fw
// TODO: Change passing a handle to passing ... something else (not context)
//       Will probably require a conversion function
// An array indexed by ixp_prop_key properties that contains useful information
// defined in the ixp_lookup_t struct
static struct ixp_lookup_t g_ixp_lookup[] = {
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "VendorId"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "DeviceId"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "RevisionId"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "InterfaceFormatCode"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "FirmwareRevision"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "ReservedOldApi"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "FeatureSwRequiredMask"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "InvalidateBeforeBlockRead"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "ReadbackOfBwAddressRegisterRequiredBeforeUse"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "NumberOfBlockWindows"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "OffsetOfBlockModeControlRegion"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "RawCapacity"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "Manufacturer"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "SerialNumber"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "PartNumber"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "DimmSku"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "MemoryModeEnabled"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "StorageModeEnabled"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "AppDirectModeEnabled"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "DieSparingCapable"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "SoftProgrammableSku"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "EncryptionEnabled"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "InterfaceFormatCodeExtra"},
	{get_fis_identify_dimm_properties, free_fis_identify_dimm_properties, "ApiVer"},
	{get_fis_identify_dimm_characteristics_properties, free_fis_identify_dimm_characteristics_properties, "ControllerTempShutdownThreshold"},
	{get_fis_identify_dimm_characteristics_properties, free_fis_identify_dimm_characteristics_properties, "MediaTempShutdownThreshold"},
	{get_fis_identify_dimm_characteristics_properties, free_fis_identify_dimm_characteristics_properties, "ThrottlingStartThreshold"},
	{get_fis_identify_dimm_characteristics_properties, free_fis_identify_dimm_characteristics_properties, "ThrottlingStopThreshold"},
	{get_fis_get_security_state_properties, free_fis_get_security_state_properties, "SecurityState"},
	{get_fis_get_security_state_properties, free_fis_get_security_state_properties, "Enabled"},
	{get_fis_get_security_state_properties, free_fis_get_security_state_properties, "Locked"},
	{get_fis_get_security_state_properties, free_fis_get_security_state_properties, "Frozen"},
	{get_fis_get_security_state_properties, free_fis_get_security_state_properties, "CountExpired"},
	{get_fis_get_security_state_properties, free_fis_get_security_state_properties, "NotSupported"},
	{get_fis_get_alarm_threshold_properties, free_fis_get_alarm_threshold_properties, "Enable"},
	{get_fis_get_alarm_threshold_properties, free_fis_get_alarm_threshold_properties, "SpareBlock"},
	{get_fis_get_alarm_threshold_properties, free_fis_get_alarm_threshold_properties, "MediaTemp"},
	{get_fis_get_alarm_threshold_properties, free_fis_get_alarm_threshold_properties, "ControllerTemp"},
	{get_fis_get_alarm_threshold_properties, free_fis_get_alarm_threshold_properties, "SpareBlockThreshold"},
	{get_fis_get_alarm_threshold_properties, free_fis_get_alarm_threshold_properties, "MediaTempThreshold"},
	{get_fis_get_alarm_threshold_properties, free_fis_get_alarm_threshold_properties, "ControllerTempThreshold"},
	{get_fis_power_management_policy_properties, free_fis_power_management_policy_properties, "Enable"},
	{get_fis_power_management_policy_properties, free_fis_power_management_policy_properties, "PeakPowerBudget"},
	{get_fis_power_management_policy_properties, free_fis_power_management_policy_properties, "AveragePowerBudget"},
	{get_fis_power_management_policy_properties, free_fis_power_management_policy_properties, "MaxPower"},
	{get_fis_die_sparing_policy_properties, free_fis_die_sparing_policy_properties, "Enable"},
	{get_fis_die_sparing_policy_properties, free_fis_die_sparing_policy_properties, "Aggressiveness"},
	{get_fis_die_sparing_policy_properties, free_fis_die_sparing_policy_properties, "Supported"},
	{get_fis_die_sparing_policy_properties, free_fis_die_sparing_policy_properties, "Rank0"},
	{get_fis_die_sparing_policy_properties, free_fis_die_sparing_policy_properties, "Rank1"},
	{get_fis_die_sparing_policy_properties, free_fis_die_sparing_policy_properties, "Rank2"},
	{get_fis_die_sparing_policy_properties, free_fis_die_sparing_policy_properties, "Rank3"},
	{get_fis_address_range_scrub_properties, free_fis_address_range_scrub_properties, "Enable"},
	{get_fis_address_range_scrub_properties, free_fis_address_range_scrub_properties, "DpaStartAddress"},
	{get_fis_address_range_scrub_properties, free_fis_address_range_scrub_properties, "DpaEndAddress"},
	{get_fis_address_range_scrub_properties, free_fis_address_range_scrub_properties, "DpaCurrentAddress"},
	{get_fis_optional_configuration_data_policy_properties, free_fis_optional_configuration_data_policy_properties, "FirstFastRefresh"},
	{get_fis_optional_configuration_data_policy_properties, free_fis_optional_configuration_data_policy_properties, "ViralPolicyEnabled"},
	{get_fis_optional_configuration_data_policy_properties, free_fis_optional_configuration_data_policy_properties, "ViralStatus"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "PmonRetreiveMask"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon0Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon0Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon1Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon1Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon2Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon2Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon3Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon3Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon4Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon4Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon5Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon5Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon6Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon6Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon7Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon7Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon8Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon8Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon9Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon9Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon10Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon10Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon11Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon11Control"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon14Counter"},
	{get_fis_pmon_registers_properties, free_fis_pmon_registers_properties, "Pmon14Control"},
	{get_fis_system_time_properties, free_fis_system_time_properties, "UnixTime"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "Signature"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "Length"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "Revision"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "Checksum"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "OemId"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "OemTableId"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "OemRevision"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "CreatorId"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "CreatorRevision"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "CurrentConfigSize"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "CurrentConfigOffset"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "InputConfigSize"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "InputConfigOffset"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "OutputConfigSize"},
	{get_fis_platform_config_data_properties, free_fis_platform_config_data_properties, "OutputConfigOffset"},
	{get_fis_dimm_partition_info_properties, free_fis_dimm_partition_info_properties, "VolatileCapacity"},
	{get_fis_dimm_partition_info_properties, free_fis_dimm_partition_info_properties, "VolatileStart"},
	{get_fis_dimm_partition_info_properties, free_fis_dimm_partition_info_properties, "PmCapacity"},
	{get_fis_dimm_partition_info_properties, free_fis_dimm_partition_info_properties, "PmStart"},
	{get_fis_dimm_partition_info_properties, free_fis_dimm_partition_info_properties, "RawCapacity"},
	{get_fis_dimm_partition_info_properties, free_fis_dimm_partition_info_properties, "EnabledCapacity"},
	{get_fis_fw_debug_log_level_properties, free_fis_fw_debug_log_level_properties, "LogLevel"},
	{get_fis_fw_debug_log_level_properties, free_fis_fw_debug_log_level_properties, "Logs"},
	{get_fis_fw_load_flag_properties, free_fis_fw_load_flag_properties, "LoadFlag"},
	{get_fis_config_lockdown_properties, free_fis_config_lockdown_properties, "Locked"},
	{get_fis_ddrt_io_init_info_properties, free_fis_ddrt_io_init_info_properties, "DdrtIoInfo"},
	{get_fis_ddrt_io_init_info_properties, free_fis_ddrt_io_init_info_properties, "DdrtTrainingStatus"},
	{get_fis_get_supported_sku_features_properties, free_fis_get_supported_sku_features_properties, "DimmSku"},
	{get_fis_enable_dimm_properties, free_fis_enable_dimm_properties, "Enable"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "ValidationFlags"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "HealthStatus"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "SpareBlocks"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "PercentUsed"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "MediaTemp"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "ControllerTemp"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "UnsafeShutdownCounter"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "AitDramStatus"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "AlarmTrips"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "LastShutdownStatus"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "VendorSpecificDataSize"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "HealthStatus"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "Noncritical"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "Critical"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "Fatal"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "SpareBlocks"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "PercentUsed"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "AlarmTrips"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "SpareBlockTrip"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "MediaTemperatureTrip"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "ControllerTemperatureTrip"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "MediaTemp"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "ControllerTemp"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "UnsafeShutdownCount"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "AitDramStatus"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "LastShutdownStatus"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "VendorSpecificDataSize"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "PowerCycles"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "PowerOnTime"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "Uptime"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "UnsafeShutdowns"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "LastShutdownStatusDetails"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "PmAdrCommandReceived"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "PmS3Received"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "PmS5Received"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "DdrtPowerFailCommandReceived"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "Pmic12vPowerFail"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "PmWarmResetReceived"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "ThermalShutdownReceived"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "FlushComplete"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "LastShutdownTime"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "LastShutdownStatusExtendedDetails"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "ViralInterruptReceived"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "SurpriseClockStopInterruptReceived"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "WriteDataFlushComplete"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "S4PowerStateReceived"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "MediaErrorInjections"},
	{get_fis_smart_health_info_properties, free_fis_smart_health_info_properties, "NonMediaErrorInjections"},
	{get_fis_firmware_image_info_properties, free_fis_firmware_image_info_properties, "FirmwareRevision"},
	{get_fis_firmware_image_info_properties, free_fis_firmware_image_info_properties, "FirmwareType"},
	{get_fis_firmware_image_info_properties, free_fis_firmware_image_info_properties, "StagedFwRevision"},
	{get_fis_firmware_image_info_properties, free_fis_firmware_image_info_properties, "LastFwUpdateStatus"},
	{get_fis_firmware_image_info_properties, free_fis_firmware_image_info_properties, "CommitId"},
	{get_fis_firmware_image_info_properties, free_fis_firmware_image_info_properties, "BuildConfiguration"},
	{get_fis_firmware_debug_log_properties, free_fis_firmware_debug_log_properties, "LogSize"},
	{get_fis_memory_info_page_0_properties, free_fis_memory_info_page_0_properties, "MediaReads"},
	{get_fis_memory_info_page_0_properties, free_fis_memory_info_page_0_properties, "MediaWrites"},
	{get_fis_memory_info_page_0_properties, free_fis_memory_info_page_0_properties, "ReadRequests"},
	{get_fis_memory_info_page_0_properties, free_fis_memory_info_page_0_properties, "WriteRequests"},
	{get_fis_memory_info_page_0_properties, free_fis_memory_info_page_0_properties, "BlockReadRequests"},
	{get_fis_memory_info_page_0_properties, free_fis_memory_info_page_0_properties, "BlockWriteRequests"},
	{get_fis_memory_info_page_1_properties, free_fis_memory_info_page_1_properties, "TotalMediaReads"},
	{get_fis_memory_info_page_1_properties, free_fis_memory_info_page_1_properties, "TotalMediaWrites"},
	{get_fis_memory_info_page_1_properties, free_fis_memory_info_page_1_properties, "TotalReadRequests"},
	{get_fis_memory_info_page_1_properties, free_fis_memory_info_page_1_properties, "TotalWriteRequests"},
	{get_fis_memory_info_page_1_properties, free_fis_memory_info_page_1_properties, "TotalBlockReadRequests"},
	{get_fis_memory_info_page_1_properties, free_fis_memory_info_page_1_properties, "TotalBlockWriteRequests"},
	{get_fis_memory_info_page_3_properties, free_fis_memory_info_page_3_properties, "ErrorInjectionStatus"},
	{get_fis_memory_info_page_3_properties, free_fis_memory_info_page_3_properties, "ErrorInjectionEnabled"},
	{get_fis_memory_info_page_3_properties, free_fis_memory_info_page_3_properties, "MediaTemperatureInjectionEnabled"},
	{get_fis_memory_info_page_3_properties, free_fis_memory_info_page_3_properties, "SoftwareTriggersEnabled"},
	{get_fis_memory_info_page_3_properties, free_fis_memory_info_page_3_properties, "PoisonErrorInjectionsCounter"},
	{get_fis_memory_info_page_3_properties, free_fis_memory_info_page_3_properties, "PoisonErrorClearCounter"},
	{get_fis_memory_info_page_3_properties, free_fis_memory_info_page_3_properties, "MediaTemperatureInjectionsCounter"},
	{get_fis_memory_info_page_3_properties, free_fis_memory_info_page_3_properties, "SoftwareTriggersCounter"},
	{get_fis_long_operation_status_properties, free_fis_long_operation_status_properties, "Command"},
	{get_fis_long_operation_status_properties, free_fis_long_operation_status_properties, "PercentComplete"},
	{get_fis_long_operation_status_properties, free_fis_long_operation_status_properties, "EstimateTimeToCompletion"},
	{get_fis_long_operation_status_properties, free_fis_long_operation_status_properties, "StatusCode"},
	{get_fis_long_operation_status_properties, free_fis_long_operation_status_properties, "CommandSpecificReturnData"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "MajorCheckpoint"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "MinorCheckpoint"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "Rest1"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "MediaReady1"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "MediaReady2"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "DdrtIoInitComplete"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "PcrLock"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "MailboxReady"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "WatchDogStatus"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "FirstFastRefreshComplete"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "CreditReady"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "MediaDisabled"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "OptInEnabled"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "OptInWasEnabled"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "Assertion"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "MiStall"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "AitDramReady"},
	{get_fis_bsr_properties, free_fis_bsr_properties, "Rest2"},
};

int ixp_init_prop(struct ixp_prop_info *prop, IXP_PROP_KEY prop_key)
{
	if (!prop)
	{
		return IXP_NULL_INPUT_PARAM;
	}
	prop->prop_key = prop_key;
	prop->prop_value = NULL;
	prop->prop_value_size = 0;
	//todo: pprop_info->prop_name
	return IXP_SUCCESS;
}

int ixp_get_prop_value(struct ixp_prop_info *prop, void **prop_value, unsigned int *prop_value_size)
{
	if(!prop || !prop_value || !*prop_value || !prop_value_size)
		return IXP_NULL_INPUT_PARAM;

	*prop_value = prop->prop_value;
	*prop_value_size = prop->prop_value_size;
	return IXP_SUCCESS;
}

int ixp_get_prop_name(struct ixp_prop_info *prop, char **prop_name)
{
	if (!prop || !prop_name || !*prop_name)
	{
		return IXP_NULL_INPUT_PARAM;
	}

	*prop_name = prop->prop_name;
	return IXP_SUCCESS;
}

int ixp_get_prop(struct ixp_context *ctx, struct ixp_prop_info *prop)
{
	if (!ctx || !prop)
	{
		return IXP_NULL_INPUT_PARAM;
	}
	return ixp_get_props(ctx, prop, 1);
}

int ixp_free_prop(struct ixp_prop_info *prop)
{
	if (!prop)
	{
		return IXP_NULL_INPUT_PARAM;
	}
	return ixp_free_props(prop, 1);
}

int ixp_get_props(struct ixp_context *ctx, struct ixp_prop_info *props, unsigned int num_props)
{
	int status;

	if (!ctx || !props || !props)
	{
		return IXP_NULL_INPUT_PARAM;
	}

	// Check all properties first for an invalid prop_key as callees iterate
	// through the full props array before this function does
	for (int index = 0; index < num_props; index++)
	{
		if (!PROP_KEY_VALID(props[index].prop_key))
		{
			return IXP_INVALID_PROP_KEY;
		}
	}

	for (int index = 0; index < num_props; index++)
	{
		// Check if already populated
		if (NULL != props[index].prop_value)
		{
			continue;
		}

		// If property value is uninitialized, call the associated function to populate it
		// NOTE: The callee will populate all possible properties in props
		if (IXP_SUCCESS != (status = (g_ixp_lookup[props[index].prop_key].f_populate)(
			(unsigned int)ctx->handle.handle, props, num_props)))
		{
			// TODO: How should we deal with errors? Continue on? Free all and return?
			ixp_free_props(props, num_props);
			return status;
		}
	}
	return IXP_SUCCESS;
}

int ixp_free_props(struct ixp_prop_info *props, unsigned int num_props)
{
	if (!props)
	{
		return IXP_NULL_INPUT_PARAM;
	}

	for (int index = 0; index < num_props; index++)
	{
		// Check if already freed
		if (NULL == props[index].prop_value)
		{
			continue;
		}

		g_ixp_lookup[props[index].prop_key].f_free(props, num_props);
	}

	return IXP_SUCCESS;
}

int ixp_get_prop_key_by_name(char * name, unsigned int length, IXP_PROP_KEY * key)
{
	// TODO: Can optimize by making large string of all props and running
	//       strstr on it
	// TODO: There are duplicate key user string names still. Need to make
	//       them appropriately unique
	// TODO: Currently only doing mem info page keys to avoid duplicating
	//       properties already done by normal C++ show dimm flow
	for (int i = FIS_MEMORY_INFO_PAGE_0_MEDIA_READS;
		i < FIS_MEMORY_INFO_PAGE_3_SOFTWARE_TRIGGERS_COUNTER; i++)
	{
		if (length == strlen(g_ixp_lookup[i].prop_name) &&
		strncmp(g_ixp_lookup[i].prop_name, name, length) == 0)
		{
			*key = i;
			return IXP_SUCCESS;
		}
	}
	return IXP_INVALID_PROP_KEY;
}

int ixp_get_g_ixp_lookup_entry(IXP_PROP_KEY key, struct ixp_lookup_t * entry)
{
	if (!PROP_KEY_VALID(key))
	{
		return IXP_INVALID_PROP_KEY;
	}
	*entry = g_ixp_lookup[key];
	return IXP_SUCCESS;
}

int ixp_set_g_ixp_lookup_entry(IXP_PROP_KEY key, struct ixp_lookup_t entry)
{
	g_ixp_lookup[key] = entry;
	return IXP_SUCCESS;
}