/*
 * Copyright (c) 2017 Intel Corporation
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

#include "FwCommandsWrapper.h"
#include <LogEnterExit.h>

namespace core
{
namespace firmware_interface
{

FwCommandsWrapper &FwCommandsWrapper::getFwWrapper()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	static FwCommandsWrapper *result = new FwCommandsWrapper();
	return *result;
}

FwCommandsWrapper::FwCommandsWrapper()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

FwCommandsWrapper::~FwCommandsWrapper()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

struct fwcmd_identify_dimm_result FwCommandsWrapper::FwcmdAllocIdentifyDimm(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_identify_dimm(handle);
}

void FwCommandsWrapper::FwcmdFreeIdentifyDimm(struct fwcmd_identify_dimm_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_identify_dimm(p_result);
}

struct fwcmd_identify_dimm_characteristics_result FwCommandsWrapper::FwcmdAllocIdentifyDimmCharacteristics(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_identify_dimm_characteristics(handle);
}

void FwCommandsWrapper::FwcmdFreeIdentifyDimmCharacteristics(struct fwcmd_identify_dimm_characteristics_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_identify_dimm_characteristics(p_result);
}

struct fwcmd_get_security_state_result FwCommandsWrapper::FwcmdAllocGetSecurityState(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_get_security_state(handle);
}

void FwCommandsWrapper::FwcmdFreeGetSecurityState(struct fwcmd_get_security_state_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_get_security_state(p_result);
}

struct fwcmd_set_passphrase_result FwCommandsWrapper::FwcmdCallSetPassphrase(unsigned int handle,
const char current_passphrase[33],
const char new_passphrase[33]) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_call_set_passphrase(handle,
	current_passphrase,
	new_passphrase);
}

struct fwcmd_disable_passphrase_result FwCommandsWrapper::FwcmdCallDisablePassphrase(unsigned int handle,
const char current_passphrase[33]) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_call_disable_passphrase(handle,
	current_passphrase);
}

struct fwcmd_unlock_unit_result FwCommandsWrapper::FwcmdCallUnlockUnit(unsigned int handle,
const char current_passphrase[33]) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_call_unlock_unit(handle,
	current_passphrase);
}

struct fwcmd_secure_erase_result FwCommandsWrapper::FwcmdCallSecureErase(unsigned int handle,
const char current_passphrase[33]) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_call_secure_erase(handle,
	current_passphrase);
}

struct fwcmd_freeze_lock_result FwCommandsWrapper::FwcmdCallFreezeLock(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_call_freeze_lock(handle);
}

struct fwcmd_get_alarm_threshold_result FwCommandsWrapper::FwcmdAllocGetAlarmThreshold(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_get_alarm_threshold(handle);
}

void FwCommandsWrapper::FwcmdFreeGetAlarmThreshold(struct fwcmd_get_alarm_threshold_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_get_alarm_threshold(p_result);
}

struct fwcmd_power_management_policy_result FwCommandsWrapper::FwcmdAllocPowerManagementPolicy(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_power_management_policy(handle);
}

void FwCommandsWrapper::FwcmdFreePowerManagementPolicy(struct fwcmd_power_management_policy_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_power_management_policy(p_result);
}

struct fwcmd_die_sparing_policy_result FwCommandsWrapper::FwcmdAllocDieSparingPolicy(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_die_sparing_policy(handle);
}

void FwCommandsWrapper::FwcmdFreeDieSparingPolicy(struct fwcmd_die_sparing_policy_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_die_sparing_policy(p_result);
}

struct fwcmd_address_range_scrub_result FwCommandsWrapper::FwcmdAllocAddressRangeScrub(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_address_range_scrub(handle);
}

void FwCommandsWrapper::FwcmdFreeAddressRangeScrub(struct fwcmd_address_range_scrub_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_address_range_scrub(p_result);
}

struct fwcmd_optional_configuration_data_policy_result FwCommandsWrapper::FwcmdAllocOptionalConfigurationDataPolicy(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_optional_configuration_data_policy(handle);
}

void FwCommandsWrapper::FwcmdFreeOptionalConfigurationDataPolicy(struct fwcmd_optional_configuration_data_policy_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_optional_configuration_data_policy(p_result);
}

struct fwcmd_pmon_registers_result FwCommandsWrapper::FwcmdAllocPmonRegisters(unsigned int handle,
const unsigned short pmon_retreive_mask) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_pmon_registers(handle,
	pmon_retreive_mask);
}

void FwCommandsWrapper::FwcmdFreePmonRegisters(struct fwcmd_pmon_registers_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_pmon_registers(p_result);
}

struct fwcmd_set_alarm_threshold_result FwCommandsWrapper::FwcmdCallSetAlarmThreshold(unsigned int handle,
const unsigned char enable,
const unsigned short peak_power_budget,
const unsigned short avg_power_budget) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_call_set_alarm_threshold(handle,
	enable,
	peak_power_budget,
	avg_power_budget);
}

struct fwcmd_system_time_result FwCommandsWrapper::FwcmdAllocSystemTime(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_system_time(handle);
}

void FwCommandsWrapper::FwcmdFreeSystemTime(struct fwcmd_system_time_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_system_time(p_result);
}

struct fwcmd_platform_config_data_result FwCommandsWrapper::FwcmdAllocPlatformConfigData(unsigned int handle,
const unsigned char partition_id,
const unsigned char command_option,
const unsigned int offset) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_platform_config_data(handle,
	partition_id,
	command_option,
	offset);
}

void FwCommandsWrapper::FwcmdFreePlatformConfigData(struct fwcmd_platform_config_data_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_platform_config_data(p_result);
}

struct fwcmd_namespace_labels_result FwCommandsWrapper::FwcmdAllocNamespaceLabels(unsigned int handle,
const unsigned char partition_id,
const unsigned char command_option,
const unsigned int offset) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_namespace_labels(handle,
	partition_id,
	command_option,
	offset);
}

void FwCommandsWrapper::FwcmdFreeNamespaceLabels(struct fwcmd_namespace_labels_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_namespace_labels(p_result);
}

struct fwcmd_dimm_partition_info_result FwCommandsWrapper::FwcmdAllocDimmPartitionInfo(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_dimm_partition_info(handle);
}

void FwCommandsWrapper::FwcmdFreeDimmPartitionInfo(struct fwcmd_dimm_partition_info_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_dimm_partition_info(p_result);
}

struct fwcmd_fw_debug_log_level_result FwCommandsWrapper::FwcmdAllocFwDebugLogLevel(unsigned int handle,
const unsigned char log_id) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_fw_debug_log_level(handle,
	log_id);
}

void FwCommandsWrapper::FwcmdFreeFwDebugLogLevel(struct fwcmd_fw_debug_log_level_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_fw_debug_log_level(p_result);
}

struct fwcmd_fw_load_flag_result FwCommandsWrapper::FwcmdAllocFwLoadFlag(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_fw_load_flag(handle);
}

void FwCommandsWrapper::FwcmdFreeFwLoadFlag(struct fwcmd_fw_load_flag_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_fw_load_flag(p_result);
}

struct fwcmd_config_lockdown_result FwCommandsWrapper::FwcmdAllocConfigLockdown(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_config_lockdown(handle);
}

void FwCommandsWrapper::FwcmdFreeConfigLockdown(struct fwcmd_config_lockdown_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_config_lockdown(p_result);
}

struct fwcmd_ddrt_io_init_info_result FwCommandsWrapper::FwcmdAllocDdrtIoInitInfo(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_ddrt_io_init_info(handle);
}

void FwCommandsWrapper::FwcmdFreeDdrtIoInitInfo(struct fwcmd_ddrt_io_init_info_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_ddrt_io_init_info(p_result);
}

struct fwcmd_get_supported_sku_features_result FwCommandsWrapper::FwcmdAllocGetSupportedSkuFeatures(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_get_supported_sku_features(handle);
}

void FwCommandsWrapper::FwcmdFreeGetSupportedSkuFeatures(struct fwcmd_get_supported_sku_features_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_get_supported_sku_features(p_result);
}

struct fwcmd_enable_dimm_result FwCommandsWrapper::FwcmdAllocEnableDimm(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_enable_dimm(handle);
}

void FwCommandsWrapper::FwcmdFreeEnableDimm(struct fwcmd_enable_dimm_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_enable_dimm(p_result);
}

struct fwcmd_smart_health_info_result FwCommandsWrapper::FwcmdAllocSmartHealthInfo(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_smart_health_info(handle);
}

void FwCommandsWrapper::FwcmdFreeSmartHealthInfo(struct fwcmd_smart_health_info_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_smart_health_info(p_result);
}

struct fwcmd_firmware_image_info_result FwCommandsWrapper::FwcmdAllocFirmwareImageInfo(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_firmware_image_info(handle);
}

void FwCommandsWrapper::FwcmdFreeFirmwareImageInfo(struct fwcmd_firmware_image_info_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_firmware_image_info(p_result);
}

struct fwcmd_firmware_debug_log_result FwCommandsWrapper::FwcmdAllocFirmwareDebugLog(unsigned int handle,
const unsigned char log_action,
const unsigned int log_page_offset,
const unsigned char log_id) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_firmware_debug_log(handle,
	log_action,
	log_page_offset,
	log_id);
}

void FwCommandsWrapper::FwcmdFreeFirmwareDebugLog(struct fwcmd_firmware_debug_log_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_firmware_debug_log(p_result);
}

struct fwcmd_memory_info_page_0_result FwCommandsWrapper::FwcmdAllocMemoryInfoPage0(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_memory_info_page_0(handle);
}

void FwCommandsWrapper::FwcmdFreeMemoryInfoPage0(struct fwcmd_memory_info_page_0_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_memory_info_page_0(p_result);
}

struct fwcmd_memory_info_page_1_result FwCommandsWrapper::FwcmdAllocMemoryInfoPage1(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_memory_info_page_1(handle);
}

void FwCommandsWrapper::FwcmdFreeMemoryInfoPage1(struct fwcmd_memory_info_page_1_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_memory_info_page_1(p_result);
}

struct fwcmd_memory_info_page_3_result FwCommandsWrapper::FwcmdAllocMemoryInfoPage3(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_memory_info_page_3(handle);
}

void FwCommandsWrapper::FwcmdFreeMemoryInfoPage3(struct fwcmd_memory_info_page_3_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_memory_info_page_3(p_result);
}

struct fwcmd_long_operation_status_result FwCommandsWrapper::FwcmdAllocLongOperationStatus(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_long_operation_status(handle);
}

void FwCommandsWrapper::FwcmdFreeLongOperationStatus(struct fwcmd_long_operation_status_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_long_operation_status(p_result);
}

struct fwcmd_bsr_result FwCommandsWrapper::FwcmdAllocBsr(unsigned int handle) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_alloc_bsr(handle);
}

void FwCommandsWrapper::FwcmdFreeBsr(struct fwcmd_bsr_result *p_result) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	fwcmd_free_bsr(p_result);
}

struct fwcmd_format_result FwCommandsWrapper::FwcmdCallFormat(unsigned int handle,
const unsigned char fill_pattern,
const unsigned char preserve_pdas_write_count) const
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	return fwcmd_call_format(handle,
	fill_pattern,
	preserve_pdas_write_count);
}

}
}