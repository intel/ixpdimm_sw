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

#ifndef SRC_CORE_FIRMWARE_INTERFACE_FWCOMMANDSWRAPPER_H_
#define SRC_CORE_FIRMWARE_INTERFACE_FWCOMMANDSWRAPPER_H_

#include <nvm_types.h>
#include "firmware_interface/fw_commands.h"
#include <core/ExportCore.h>

namespace core
{
namespace firmware_interface
{

class  NVM_CORE_API FwCommandsWrapper
{
public:
	virtual ~FwCommandsWrapper();

	static FwCommandsWrapper &getFwWrapper();

	virtual struct fwcmd_identify_dimm_result FwcmdAllocIdentifyDimm(unsigned int handle) const;

	virtual void FwcmdFreeIdentifyDimm(struct fwcmd_identify_dimm_result *p_result) const;

	virtual struct fwcmd_identify_dimm_characteristics_result FwcmdAllocIdentifyDimmCharacteristics(unsigned int handle) const;

	virtual void FwcmdFreeIdentifyDimmCharacteristics(struct fwcmd_identify_dimm_characteristics_result *p_result) const;

	virtual struct fwcmd_get_security_state_result FwcmdAllocGetSecurityState(unsigned int handle) const;

	virtual void FwcmdFreeGetSecurityState(struct fwcmd_get_security_state_result *p_result) const;

	virtual struct fwcmd_set_passphrase_result FwcmdCallSetPassphrase(unsigned int handle,
	const char current_passphrase[33],
	const char new_passphrase[33]) const;

	virtual struct fwcmd_disable_passphrase_result FwcmdCallDisablePassphrase(unsigned int handle,
	const char current_passphrase[33]) const;

	virtual struct fwcmd_unlock_unit_result FwcmdCallUnlockUnit(unsigned int handle,
	const char current_passphrase[33]) const;

	virtual struct fwcmd_secure_erase_result FwcmdCallSecureErase(unsigned int handle,
	const char current_passphrase[33]) const;

	virtual struct fwcmd_freeze_lock_result FwcmdCallFreezeLock(unsigned int handle) const;

	virtual struct fwcmd_get_alarm_threshold_result FwcmdAllocGetAlarmThreshold(unsigned int handle) const;

	virtual void FwcmdFreeGetAlarmThreshold(struct fwcmd_get_alarm_threshold_result *p_result) const;

	virtual struct fwcmd_power_management_policy_result FwcmdAllocPowerManagementPolicy(unsigned int handle) const;

	virtual void FwcmdFreePowerManagementPolicy(struct fwcmd_power_management_policy_result *p_result) const;

	virtual struct fwcmd_die_sparing_policy_result FwcmdAllocDieSparingPolicy(unsigned int handle) const;

	virtual void FwcmdFreeDieSparingPolicy(struct fwcmd_die_sparing_policy_result *p_result) const;

	virtual struct fwcmd_address_range_scrub_result FwcmdAllocAddressRangeScrub(unsigned int handle) const;

	virtual void FwcmdFreeAddressRangeScrub(struct fwcmd_address_range_scrub_result *p_result) const;

	virtual struct fwcmd_optional_configuration_data_policy_result FwcmdAllocOptionalConfigurationDataPolicy(unsigned int handle) const;

	virtual void FwcmdFreeOptionalConfigurationDataPolicy(struct fwcmd_optional_configuration_data_policy_result *p_result) const;

	virtual struct fwcmd_pmon_registers_result FwcmdAllocPmonRegisters(unsigned int handle,
	const unsigned short pmon_retreive_mask) const;

	virtual void FwcmdFreePmonRegisters(struct fwcmd_pmon_registers_result *p_result) const;

	virtual struct fwcmd_set_alarm_threshold_result FwcmdCallSetAlarmThreshold(unsigned int handle,
	const unsigned char enable,
	const unsigned short peak_power_budget,
	const unsigned short avg_power_budget) const;

	virtual struct fwcmd_system_time_result FwcmdAllocSystemTime(unsigned int handle) const;

	virtual void FwcmdFreeSystemTime(struct fwcmd_system_time_result *p_result) const;

	virtual struct fwcmd_platform_config_data_result FwcmdAllocPlatformConfigData(unsigned int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset) const;

	virtual void FwcmdFreePlatformConfigData(struct fwcmd_platform_config_data_result *p_result) const;

	virtual struct fwcmd_namespace_labels_result FwcmdAllocNamespaceLabels(unsigned int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset) const;

	virtual void FwcmdFreeNamespaceLabels(struct fwcmd_namespace_labels_result *p_result) const;

	virtual struct fwcmd_dimm_partition_info_result FwcmdAllocDimmPartitionInfo(unsigned int handle) const;

	virtual void FwcmdFreeDimmPartitionInfo(struct fwcmd_dimm_partition_info_result *p_result) const;

	virtual struct fwcmd_fw_debug_log_level_result FwcmdAllocFwDebugLogLevel(unsigned int handle,
	const unsigned char log_id) const;

	virtual void FwcmdFreeFwDebugLogLevel(struct fwcmd_fw_debug_log_level_result *p_result) const;

	virtual struct fwcmd_fw_load_flag_result FwcmdAllocFwLoadFlag(unsigned int handle) const;

	virtual void FwcmdFreeFwLoadFlag(struct fwcmd_fw_load_flag_result *p_result) const;

	virtual struct fwcmd_config_lockdown_result FwcmdAllocConfigLockdown(unsigned int handle) const;

	virtual void FwcmdFreeConfigLockdown(struct fwcmd_config_lockdown_result *p_result) const;

	virtual struct fwcmd_ddrt_io_init_info_result FwcmdAllocDdrtIoInitInfo(unsigned int handle) const;

	virtual void FwcmdFreeDdrtIoInitInfo(struct fwcmd_ddrt_io_init_info_result *p_result) const;

	virtual struct fwcmd_get_supported_sku_features_result FwcmdAllocGetSupportedSkuFeatures(unsigned int handle) const;

	virtual void FwcmdFreeGetSupportedSkuFeatures(struct fwcmd_get_supported_sku_features_result *p_result) const;

	virtual struct fwcmd_enable_dimm_result FwcmdAllocEnableDimm(unsigned int handle) const;

	virtual void FwcmdFreeEnableDimm(struct fwcmd_enable_dimm_result *p_result) const;

	virtual struct fwcmd_smart_health_info_result FwcmdAllocSmartHealthInfo(unsigned int handle) const;

	virtual void FwcmdFreeSmartHealthInfo(struct fwcmd_smart_health_info_result *p_result) const;

	virtual struct fwcmd_firmware_image_info_result FwcmdAllocFirmwareImageInfo(unsigned int handle) const;

	virtual void FwcmdFreeFirmwareImageInfo(struct fwcmd_firmware_image_info_result *p_result) const;

	virtual struct fwcmd_firmware_debug_log_result FwcmdAllocFirmwareDebugLog(unsigned int handle,
	const unsigned char log_action,
	const unsigned int log_page_offset,
	const unsigned char log_id) const;

	virtual void FwcmdFreeFirmwareDebugLog(struct fwcmd_firmware_debug_log_result *p_result) const;

	virtual struct fwcmd_long_operation_status_result FwcmdAllocLongOperationStatus(unsigned int handle) const;

	virtual void FwcmdFreeLongOperationStatus(struct fwcmd_long_operation_status_result *p_result) const;

	virtual struct fwcmd_bsr_result FwcmdAllocBsr(unsigned int handle) const;

	virtual void FwcmdFreeBsr(struct fwcmd_bsr_result *p_result) const;

	virtual struct fwcmd_format_result FwcmdCallFormat(unsigned int handle,
	const unsigned char fill_pattern,
	const unsigned char preserve_pdas_write_count) const;

protected:
	FwCommandsWrapper();

};

}
}

#endif /* SRC_CORE_FIRMWARE_INTERFACE_FWCOMMANDSWRAPPER_H_ */