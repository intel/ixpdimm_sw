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

#ifndef FWCOMMANDS_H_
#define FWCOMMANDS_H_

#include "FwCommandsWrapper.h"
#include "string"
#include <nvm_types.h>
#include "firmware_interface/fw_commands.h"
#include <core/ExportCore.h>

namespace core
{
namespace firmware_interface
{

class NVM_CORE_API FwCommands
{

public:

FwCommands(const FwCommandsWrapper &m_wrapper = core::firmware_interface::FwCommandsWrapper::getFwWrapper());
FwCommands(const FwCommands &other);
virtual ~FwCommands();

FwCommands &operator=(const FwCommands &other);
static FwCommands &getFwCommands();

virtual std::string fwPayloadToString_Custom_PlatformConfigData(fwcmd_platform_config_data_data *pData);
virtual enum return_code fwGetPayload_IdentifyDimm(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_IdentifyDimmCharacteristics(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_GetSecurityState(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_SetPassphrase(unsigned int handle,
	const char current_passphrase[33],
	const char new_passphrase[33],
	std::string &resultString);

virtual enum return_code fwGetPayload_DisablePassphrase(unsigned int handle,
	const char current_passphrase[33],
	std::string &resultString);

virtual enum return_code fwGetPayload_UnlockUnit(unsigned int handle,
	const char current_passphrase[33],
	std::string &resultString);

virtual enum return_code fwGetPayload_SecureErase(unsigned int handle,
	const char current_passphrase[33],
	std::string &resultString);

virtual enum return_code fwGetPayload_FreezeLock(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_GetAlarmThreshold(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_PowerManagementPolicy(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_DieSparingPolicy(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_AddressRangeScrub(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_OptionalConfigurationDataPolicy(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_PmonRegisters(unsigned int handle,
	const unsigned short pmon_retreive_mask,
	std::string &resultString);

virtual enum return_code fwGetPayload_SetAlarmThreshold(unsigned int handle,
	const unsigned char enable,
	const unsigned short peak_power_budget,
	const unsigned short avg_power_budget,
	std::string &resultString);

virtual enum return_code fwGetPayload_SystemTime(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_PlatformConfigData(unsigned int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset,
	std::string &resultString);

virtual enum return_code fwGetPayload_NamespaceLabels(unsigned int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset,
	std::string &resultString);

virtual enum return_code fwGetPayload_DimmPartitionInfo(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_FwDebugLogLevel(unsigned int handle,
	const unsigned char log_id,
	std::string &resultString);

virtual enum return_code fwGetPayload_FwLoadFlag(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_ConfigLockdown(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_DdrtIoInitInfo(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_GetSupportedSkuFeatures(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_EnableDimm(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_SmartHealthInfo(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_FirmwareImageInfo(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_FirmwareDebugLog(unsigned int handle,
	const unsigned char log_action,
	const unsigned int log_page_offset,
	const unsigned char log_id,
	std::string &resultString);

virtual enum return_code fwGetPayload_MemoryInfoPage0(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_MemoryInfoPage1(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_MemoryInfoPage3(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_LongOperationStatus(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_Bsr(unsigned int handle,
	std::string &resultString);

virtual enum return_code fwGetPayload_Format(unsigned int handle,
	const unsigned char fill_pattern,
	const unsigned char preserve_pdas_write_count,
	std::string &resultString);

virtual std::string fwPayloadToString_IdentifyDimm(
	const struct fwcmd_identify_dimm_data *p_data);

virtual std::string fwPayloadToString_IdentifyDimmCharacteristics(
	const struct fwcmd_identify_dimm_characteristics_data *p_data);

virtual std::string fwPayloadToString_GetSecurityState(
	const struct fwcmd_get_security_state_data *p_data);

virtual std::string fwPayloadToString_GetAlarmThreshold(
	const struct fwcmd_get_alarm_threshold_data *p_data);

virtual std::string fwPayloadToString_PowerManagementPolicy(
	const struct fwcmd_power_management_policy_data *p_data);

virtual std::string fwPayloadToString_DieSparingPolicy(
	const struct fwcmd_die_sparing_policy_data *p_data);

virtual std::string fwPayloadToString_AddressRangeScrub(
	const struct fwcmd_address_range_scrub_data *p_data);

virtual std::string fwPayloadToString_OptionalConfigurationDataPolicy(
	const struct fwcmd_optional_configuration_data_policy_data *p_data);

virtual std::string fwPayloadToString_PmonRegisters(
	const struct fwcmd_pmon_registers_data *p_data);

virtual std::string fwPayloadToString_SystemTime(
	const struct fwcmd_system_time_data *p_data);

virtual std::string fwPayloadToString_DeviceIdentificationV1(
	const struct fwcmd_device_identification_v1_data *p_data);

virtual std::string fwPayloadToString_DeviceIdentificationV2(
	const struct fwcmd_device_identification_v2_data *p_data);

virtual std::string fwPayloadToString_IdInfoTable(
	const struct fwcmd_id_info_table_data *p_data);

virtual std::string fwPayloadToString_InterleaveInformationTable(
	const struct fwcmd_interleave_information_table_data *p_data);

virtual std::string fwPayloadToString_PartitionSizeChangeTable(
	const struct fwcmd_partition_size_change_table_data *p_data);

virtual std::string fwPayloadToString_CurrentConfigTable(
	const struct fwcmd_current_config_table_data *p_data);

virtual std::string fwPayloadToString_ConfigInputTable(
	const struct fwcmd_config_input_table_data *p_data);

virtual std::string fwPayloadToString_ConfigOutputTable(
	const struct fwcmd_config_output_table_data *p_data);

virtual std::string fwPayloadToString_PlatformConfigData(
	const struct fwcmd_platform_config_data_data *p_data);

virtual std::string fwPayloadToString_NsIndex(
	const struct fwcmd_ns_index_data *p_data);

virtual std::string fwPayloadToString_NsLabel(
	const struct fwcmd_ns_label_data *p_data);

virtual std::string fwPayloadToString_NsLabelV11(
	const struct fwcmd_ns_label_v1_1_data *p_data);

virtual std::string fwPayloadToString_NsLabelV12(
	const struct fwcmd_ns_label_v1_2_data *p_data);

virtual std::string fwPayloadToString_NamespaceLabels(
	const struct fwcmd_namespace_labels_data *p_data);

virtual std::string fwPayloadToString_DimmPartitionInfo(
	const struct fwcmd_dimm_partition_info_data *p_data);

virtual std::string fwPayloadToString_FwDebugLogLevel(
	const struct fwcmd_fw_debug_log_level_data *p_data);

virtual std::string fwPayloadToString_FwLoadFlag(
	const struct fwcmd_fw_load_flag_data *p_data);

virtual std::string fwPayloadToString_ConfigLockdown(
	const struct fwcmd_config_lockdown_data *p_data);

virtual std::string fwPayloadToString_DdrtIoInitInfo(
	const struct fwcmd_ddrt_io_init_info_data *p_data);

virtual std::string fwPayloadToString_GetSupportedSkuFeatures(
	const struct fwcmd_get_supported_sku_features_data *p_data);

virtual std::string fwPayloadToString_EnableDimm(
	const struct fwcmd_enable_dimm_data *p_data);

virtual std::string fwPayloadToString_SmartHealthInfo(
	const struct fwcmd_smart_health_info_data *p_data);

virtual std::string fwPayloadToString_FirmwareImageInfo(
	const struct fwcmd_firmware_image_info_data *p_data);

virtual std::string fwPayloadToString_FirmwareDebugLog(
	const struct fwcmd_firmware_debug_log_data *p_data);

virtual std::string fwPayloadToString_MemoryInfoPage0(
	const struct fwcmd_memory_info_page_0_data *p_data);

virtual std::string fwPayloadToString_MemoryInfoPage1(
	const struct fwcmd_memory_info_page_1_data *p_data);

virtual std::string fwPayloadToString_MemoryInfoPage3(
	const struct fwcmd_memory_info_page_3_data *p_data);

virtual std::string fwPayloadToString_LongOperationStatus(
	const struct fwcmd_long_operation_status_data *p_data);

virtual std::string fwPayloadToString_Bsr(
	const struct fwcmd_bsr_data *p_data);

virtual std::string fwPayloadFieldsToString_IdentifyDimm(
	const struct fwcmd_identify_dimm_data *p_data);

virtual std::string fwPayloadFieldsToString_IdentifyDimmCharacteristics(
	const struct fwcmd_identify_dimm_characteristics_data *p_data);

virtual std::string fwPayloadFieldsToString_GetSecurityState(
	const struct fwcmd_get_security_state_data *p_data);

virtual std::string fwPayloadFieldsToString_GetAlarmThreshold(
	const struct fwcmd_get_alarm_threshold_data *p_data);

virtual std::string fwPayloadFieldsToString_PowerManagementPolicy(
	const struct fwcmd_power_management_policy_data *p_data);

virtual std::string fwPayloadFieldsToString_DieSparingPolicy(
	const struct fwcmd_die_sparing_policy_data *p_data);

virtual std::string fwPayloadFieldsToString_AddressRangeScrub(
	const struct fwcmd_address_range_scrub_data *p_data);

virtual std::string fwPayloadFieldsToString_OptionalConfigurationDataPolicy(
	const struct fwcmd_optional_configuration_data_policy_data *p_data);

virtual std::string fwPayloadFieldsToString_PmonRegisters(
	const struct fwcmd_pmon_registers_data *p_data);

virtual std::string fwPayloadFieldsToString_SystemTime(
	const struct fwcmd_system_time_data *p_data);

virtual std::string fwPayloadFieldsToString_DeviceIdentificationV1(
	const struct fwcmd_device_identification_v1_data *p_data);

virtual std::string fwPayloadFieldsToString_DeviceIdentificationV2(
	const struct fwcmd_device_identification_v2_data *p_data);

virtual std::string fwPayloadFieldsToString_IdInfoTable(
	const struct fwcmd_id_info_table_data *p_data);

virtual std::string fwPayloadFieldsToString_InterleaveInformationTable(
	const struct fwcmd_interleave_information_table_data *p_data);

virtual std::string fwPayloadFieldsToString_PartitionSizeChangeTable(
	const struct fwcmd_partition_size_change_table_data *p_data);

virtual std::string fwPayloadFieldsToString_CurrentConfigTable(
	const struct fwcmd_current_config_table_data *p_data);

virtual std::string fwPayloadFieldsToString_ConfigInputTable(
	const struct fwcmd_config_input_table_data *p_data);

virtual std::string fwPayloadFieldsToString_ConfigOutputTable(
	const struct fwcmd_config_output_table_data *p_data);

virtual std::string fwPayloadFieldsToString_PlatformConfigData(
	const struct fwcmd_platform_config_data_data *p_data);

virtual std::string fwPayloadFieldsToString_NsIndex(
	const struct fwcmd_ns_index_data *p_data);

virtual std::string fwPayloadFieldsToString_NsLabel(
	const struct fwcmd_ns_label_data *p_data);

virtual std::string fwPayloadFieldsToString_NsLabelV11(
	const struct fwcmd_ns_label_v1_1_data *p_data);

virtual std::string fwPayloadFieldsToString_NsLabelV12(
	const struct fwcmd_ns_label_v1_2_data *p_data);

virtual std::string fwPayloadFieldsToString_NamespaceLabels(
	const struct fwcmd_namespace_labels_data *p_data);

virtual std::string fwPayloadFieldsToString_DimmPartitionInfo(
	const struct fwcmd_dimm_partition_info_data *p_data);

virtual std::string fwPayloadFieldsToString_FwDebugLogLevel(
	const struct fwcmd_fw_debug_log_level_data *p_data);

virtual std::string fwPayloadFieldsToString_FwLoadFlag(
	const struct fwcmd_fw_load_flag_data *p_data);

virtual std::string fwPayloadFieldsToString_ConfigLockdown(
	const struct fwcmd_config_lockdown_data *p_data);

virtual std::string fwPayloadFieldsToString_DdrtIoInitInfo(
	const struct fwcmd_ddrt_io_init_info_data *p_data);

virtual std::string fwPayloadFieldsToString_GetSupportedSkuFeatures(
	const struct fwcmd_get_supported_sku_features_data *p_data);

virtual std::string fwPayloadFieldsToString_EnableDimm(
	const struct fwcmd_enable_dimm_data *p_data);

virtual std::string fwPayloadFieldsToString_SmartHealthInfo(
	const struct fwcmd_smart_health_info_data *p_data);

virtual std::string fwPayloadFieldsToString_FirmwareImageInfo(
	const struct fwcmd_firmware_image_info_data *p_data);

virtual std::string fwPayloadFieldsToString_FirmwareDebugLog(
	const struct fwcmd_firmware_debug_log_data *p_data);

virtual std::string fwPayloadFieldsToString_MemoryInfoPage0(
	const struct fwcmd_memory_info_page_0_data *p_data);

virtual std::string fwPayloadFieldsToString_MemoryInfoPage1(
	const struct fwcmd_memory_info_page_1_data *p_data);

virtual std::string fwPayloadFieldsToString_MemoryInfoPage3(
	const struct fwcmd_memory_info_page_3_data *p_data);

virtual std::string fwPayloadFieldsToString_LongOperationStatus(
	const struct fwcmd_long_operation_status_data *p_data);

virtual std::string fwPayloadFieldsToString_Bsr(
	const struct fwcmd_bsr_data *p_data);

virtual enum return_code dsm_err_to_nvm_lib_err(pt_result result);

virtual enum return_code fw_mb_err_to_nvm_lib_err(int extended_status);

virtual enum return_code convertFwcmdErrorCodeToNvmErrorCode(struct fwcmd_error_code error);

private:
	const FwCommandsWrapper &m_wrapper;


};

} /* namespace firmware_interface */
} /* namespace core */


#endif /* FWCOMMANDS_H_ */