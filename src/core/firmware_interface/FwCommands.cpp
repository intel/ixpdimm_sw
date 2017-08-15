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

#include "FwCommands.h"
#include "firmware_interface/fis_commands.h"
#include "firmware_interface/fw_command_printer.h"
#include "fis_types.h"
#include "sstream"
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <iomanip>
#include <LogEnterExit.h>

namespace core
{
namespace firmware_interface
{

FwCommands &FwCommands::getFwCommands()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	// Creating the singleton on class init as a static class member
	// can lead to static initialization order issues.
	// This is a thread-safe form of lazy initialization.
	static FwCommands *result = new FwCommands();
	return *result;
}

FwCommands::FwCommands(const FwCommandsWrapper &wrapper) : m_wrapper(wrapper)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

FwCommands::FwCommands(const FwCommands &other) : m_wrapper(other.m_wrapper)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

FwCommands &FwCommands::operator=(const FwCommands &other)
{
	return *this;
}

FwCommands::~FwCommands()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

enum return_code FwCommands::fwGetPayload_IdentifyDimm(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_identify_dimm_result result = m_wrapper.FwcmdAllocIdentifyDimm(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_IdentifyDimm(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeIdentifyDimm(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_IdentifyDimmCharacteristics(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_identify_dimm_characteristics_result result = m_wrapper.FwcmdAllocIdentifyDimmCharacteristics(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_IdentifyDimmCharacteristics(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeIdentifyDimmCharacteristics(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_GetSecurityState(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_get_security_state_result result = m_wrapper.FwcmdAllocGetSecurityState(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_GetSecurityState(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeGetSecurityState(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_SetPassphrase(unsigned int handle, const char current_passphrase[33], const char new_passphrase[33], std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_set_passphrase_result result = m_wrapper.FwcmdCallSetPassphrase(handle, current_passphrase, new_passphrase);

	if (result.success)
	{
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	return rc;
}

enum return_code FwCommands::fwGetPayload_DisablePassphrase(unsigned int handle, const char current_passphrase[33], std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_disable_passphrase_result result = m_wrapper.FwcmdCallDisablePassphrase(handle, current_passphrase);

	if (result.success)
	{
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	return rc;
}

enum return_code FwCommands::fwGetPayload_UnlockUnit(unsigned int handle, const char current_passphrase[33], std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_unlock_unit_result result = m_wrapper.FwcmdCallUnlockUnit(handle, current_passphrase);

	if (result.success)
	{
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	return rc;
}

enum return_code FwCommands::fwGetPayload_SecureErase(unsigned int handle, const char current_passphrase[33], std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_secure_erase_result result = m_wrapper.FwcmdCallSecureErase(handle, current_passphrase);

	if (result.success)
	{
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	return rc;
}

enum return_code FwCommands::fwGetPayload_FreezeLock(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_freeze_lock_result result = m_wrapper.FwcmdCallFreezeLock(handle);

	if (result.success)
	{
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	return rc;
}

enum return_code FwCommands::fwGetPayload_GetAlarmThreshold(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_get_alarm_threshold_result result = m_wrapper.FwcmdAllocGetAlarmThreshold(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_GetAlarmThreshold(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeGetAlarmThreshold(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_PowerManagementPolicy(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_power_management_policy_result result = m_wrapper.FwcmdAllocPowerManagementPolicy(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_PowerManagementPolicy(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreePowerManagementPolicy(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_DieSparingPolicy(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_die_sparing_policy_result result = m_wrapper.FwcmdAllocDieSparingPolicy(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_DieSparingPolicy(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeDieSparingPolicy(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_AddressRangeScrub(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_address_range_scrub_result result = m_wrapper.FwcmdAllocAddressRangeScrub(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_AddressRangeScrub(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeAddressRangeScrub(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_OptionalConfigurationDataPolicy(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_optional_configuration_data_policy_result result = m_wrapper.FwcmdAllocOptionalConfigurationDataPolicy(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_OptionalConfigurationDataPolicy(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeOptionalConfigurationDataPolicy(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_PmonRegisters(unsigned int handle, const unsigned short pmon_retreive_mask, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_pmon_registers_result result = m_wrapper.FwcmdAllocPmonRegisters(handle, pmon_retreive_mask);

	if (result.success)
	{
		resultString += fwPayloadToString_PmonRegisters(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreePmonRegisters(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_SetAlarmThreshold(unsigned int handle, const unsigned char enable, const unsigned short peak_power_budget, const unsigned short avg_power_budget, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_set_alarm_threshold_result result = m_wrapper.FwcmdCallSetAlarmThreshold(handle, enable, peak_power_budget, avg_power_budget);

	if (result.success)
	{
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	return rc;
}

enum return_code FwCommands::fwGetPayload_SystemTime(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_system_time_result result = m_wrapper.FwcmdAllocSystemTime(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_SystemTime(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeSystemTime(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_PlatformConfigData(unsigned int handle, const unsigned char partition_id, const unsigned char command_option, const unsigned int offset, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_platform_config_data_result result = m_wrapper.FwcmdAllocPlatformConfigData(handle, partition_id, command_option, offset);

	if (result.success)
	{
		resultString += fwPayloadToString_Custom_PlatformConfigData(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreePlatformConfigData(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_NamespaceLabels(unsigned int handle, const unsigned char partition_id, const unsigned char command_option, const unsigned int offset, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_namespace_labels_result result = m_wrapper.FwcmdAllocNamespaceLabels(handle, partition_id, command_option, offset);

	if (result.success)
	{
		resultString += fwPayloadToString_NamespaceLabels(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeNamespaceLabels(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_DimmPartitionInfo(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_dimm_partition_info_result result = m_wrapper.FwcmdAllocDimmPartitionInfo(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_DimmPartitionInfo(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeDimmPartitionInfo(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_FwDebugLogLevel(unsigned int handle, const unsigned char log_id, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_fw_debug_log_level_result result = m_wrapper.FwcmdAllocFwDebugLogLevel(handle, log_id);

	if (result.success)
	{
		resultString += fwPayloadToString_FwDebugLogLevel(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeFwDebugLogLevel(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_FwLoadFlag(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_fw_load_flag_result result = m_wrapper.FwcmdAllocFwLoadFlag(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_FwLoadFlag(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeFwLoadFlag(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_ConfigLockdown(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_config_lockdown_result result = m_wrapper.FwcmdAllocConfigLockdown(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_ConfigLockdown(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeConfigLockdown(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_DdrtIoInitInfo(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_ddrt_io_init_info_result result = m_wrapper.FwcmdAllocDdrtIoInitInfo(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_DdrtIoInitInfo(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeDdrtIoInitInfo(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_GetSupportedSkuFeatures(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_get_supported_sku_features_result result = m_wrapper.FwcmdAllocGetSupportedSkuFeatures(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_GetSupportedSkuFeatures(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeGetSupportedSkuFeatures(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_EnableDimm(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_enable_dimm_result result = m_wrapper.FwcmdAllocEnableDimm(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_EnableDimm(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeEnableDimm(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_SmartHealthInfo(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_smart_health_info_result result = m_wrapper.FwcmdAllocSmartHealthInfo(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_SmartHealthInfo(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeSmartHealthInfo(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_FirmwareImageInfo(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_firmware_image_info_result result = m_wrapper.FwcmdAllocFirmwareImageInfo(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_FirmwareImageInfo(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeFirmwareImageInfo(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_FirmwareDebugLog(unsigned int handle, const unsigned char log_action, const unsigned int log_page_offset, const unsigned char log_id, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_firmware_debug_log_result result = m_wrapper.FwcmdAllocFirmwareDebugLog(handle, log_action, log_page_offset, log_id);

	if (result.success)
	{
		resultString += fwPayloadToString_FirmwareDebugLog(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeFirmwareDebugLog(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_LongOperationStatus(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_long_operation_status_result result = m_wrapper.FwcmdAllocLongOperationStatus(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_LongOperationStatus(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeLongOperationStatus(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_Bsr(unsigned int handle, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_bsr_result result = m_wrapper.FwcmdAllocBsr(handle);

	if (result.success)
	{
		resultString += fwPayloadToString_Bsr(result.p_data);
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	m_wrapper.FwcmdFreeBsr(&result);
	return rc;
}

enum return_code FwCommands::fwGetPayload_Format(unsigned int handle, const unsigned char fill_pattern, const unsigned char preserve_pdas_write_count, std::string &resultString)
{
	enum return_code rc = NVM_SUCCESS;

	struct fwcmd_format_result result = m_wrapper.FwcmdCallFormat(handle, fill_pattern, preserve_pdas_write_count);

	if (result.success)
	{
	}
	else
	{
		rc = convertFwcmdErrorCodeToNvmErrorCode(result.error_code);
	}

	return rc;
}

std::string FwCommands::fwPayloadToString_IdentifyDimm(const struct fwcmd_identify_dimm_data *p_data)
{
	std::stringstream result;
	result << "\nIdentify Dimm:" << "\n";
	result << fwPayloadFieldsToString_IdentifyDimm(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_IdentifyDimmCharacteristics(const struct fwcmd_identify_dimm_characteristics_data *p_data)
{
	std::stringstream result;
	result << "\nIdentify Dimm Characteristics:" << "\n";
	result << fwPayloadFieldsToString_IdentifyDimmCharacteristics(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_GetSecurityState(const struct fwcmd_get_security_state_data *p_data)
{
	std::stringstream result;
	result << "\nGet Security State:" << "\n";
	result << fwPayloadFieldsToString_GetSecurityState(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_GetAlarmThreshold(const struct fwcmd_get_alarm_threshold_data *p_data)
{
	std::stringstream result;
	result << "\nGet Alarm Threshold:" << "\n";
	result << fwPayloadFieldsToString_GetAlarmThreshold(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_PowerManagementPolicy(const struct fwcmd_power_management_policy_data *p_data)
{
	std::stringstream result;
	result << "\nPower Management Policy:" << "\n";
	result << fwPayloadFieldsToString_PowerManagementPolicy(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_DieSparingPolicy(const struct fwcmd_die_sparing_policy_data *p_data)
{
	std::stringstream result;
	result << "\nDie Sparing Policy:" << "\n";
	result << fwPayloadFieldsToString_DieSparingPolicy(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_AddressRangeScrub(const struct fwcmd_address_range_scrub_data *p_data)
{
	std::stringstream result;
	result << "\nAddress Range Scrub:" << "\n";
	result << fwPayloadFieldsToString_AddressRangeScrub(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_OptionalConfigurationDataPolicy(const struct fwcmd_optional_configuration_data_policy_data *p_data)
{
	std::stringstream result;
	result << "\nOptional Configuration Data Policy:" << "\n";
	result << fwPayloadFieldsToString_OptionalConfigurationDataPolicy(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_PmonRegisters(const struct fwcmd_pmon_registers_data *p_data)
{
	std::stringstream result;
	result << "\nPmon Registers:" << "\n";
	result << fwPayloadFieldsToString_PmonRegisters(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_SystemTime(const struct fwcmd_system_time_data *p_data)
{
	std::stringstream result;
	result << "\nSystem Time:" << "\n";
	result << fwPayloadFieldsToString_SystemTime(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_DeviceIdentificationV1(const struct fwcmd_device_identification_v1_data *p_data)
{
	std::stringstream result;
	result << "\nDevice Identification V1:" << "\n";
	result << fwPayloadFieldsToString_DeviceIdentificationV1(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_DeviceIdentificationV2(const struct fwcmd_device_identification_v2_data *p_data)
{
	std::stringstream result;
	result << "\nDevice Identification V2:" << "\n";
	result << fwPayloadFieldsToString_DeviceIdentificationV2(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_IdInfoTable(const struct fwcmd_id_info_table_data *p_data)
{
	std::stringstream result;
	result << "\nId Info Table:" << "\n";
	result << fwPayloadFieldsToString_IdInfoTable(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_InterleaveInformationTable(const struct fwcmd_interleave_information_table_data *p_data)
{
	std::stringstream result;
	result << "\nInterleave Information Table:" << "\n";
	result << fwPayloadFieldsToString_InterleaveInformationTable(p_data);
	
	for (int i = 0; i < p_data->id_info_table_count; i++)
	{
		result << fwPayloadToString_IdInfoTable(&p_data->id_info_table[i]);
	}

	return result.str();
}

std::string FwCommands::fwPayloadToString_PartitionSizeChangeTable(const struct fwcmd_partition_size_change_table_data *p_data)
{
	std::stringstream result;
	result << "\nPartition Size Change Table:" << "\n";
	result << fwPayloadFieldsToString_PartitionSizeChangeTable(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_CurrentConfigTable(const struct fwcmd_current_config_table_data *p_data)
{
	std::stringstream result;
	result << "\nCurrent Config Table:" << "\n";
	result << fwPayloadFieldsToString_CurrentConfigTable(p_data);
	
	for (int i = 0; i < p_data->interleave_information_table_count; i++)
	{
		result << fwPayloadToString_InterleaveInformationTable(&p_data->interleave_information_table[i]);
	}
	return result.str();
}

std::string FwCommands::fwPayloadToString_ConfigInputTable(const struct fwcmd_config_input_table_data *p_data)
{
	std::stringstream result;
	result << "\nConfig Input Table:" << "\n";
	result << fwPayloadFieldsToString_ConfigInputTable(p_data);
	
	for (int i = 0; i < p_data->interleave_information_table_count; i++)
	{
		result << fwPayloadToString_InterleaveInformationTable(&p_data->interleave_information_table[i]);
	}
	for (int i = 0; i < p_data->partition_size_change_table_count; i++)
	{
		result << fwPayloadToString_PartitionSizeChangeTable(&p_data->partition_size_change_table[i]);
	}
	return result.str();
}

std::string FwCommands::fwPayloadToString_ConfigOutputTable(const struct fwcmd_config_output_table_data *p_data)
{
	std::stringstream result;
	result << "\nConfig Output Table:" << "\n";
	result << fwPayloadFieldsToString_ConfigOutputTable(p_data);
	
	for (int i = 0; i < p_data->interleave_information_table_count; i++)
	{
		result << fwPayloadToString_InterleaveInformationTable(&p_data->interleave_information_table[i]);
	}
	for (int i = 0; i < p_data->partition_size_change_table_count; i++)
	{
		result << fwPayloadToString_PartitionSizeChangeTable(&p_data->partition_size_change_table[i]);
	}
	return result.str();
}

std::string FwCommands::fwPayloadToString_PlatformConfigData(const struct fwcmd_platform_config_data_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data:" << "\n";
	result << fwPayloadFieldsToString_PlatformConfigData(p_data);
	
	result << fwPayloadToString_CurrentConfigTable(&p_data->current_config_table);
	result << fwPayloadToString_ConfigInputTable(&p_data->config_input_table);
	result << fwPayloadToString_ConfigOutputTable(&p_data->config_output_table);
	return result.str();
}

std::string FwCommands::fwPayloadToString_NsIndex(const struct fwcmd_ns_index_data *p_data)
{
	std::stringstream result;
	result << "\nNs Index:" << "\n";
	result << fwPayloadFieldsToString_NsIndex(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_NsLabel(const struct fwcmd_ns_label_data *p_data)
{
	std::stringstream result;
	result << "\nNs Label:" << "\n";
	result << fwPayloadFieldsToString_NsLabel(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_NsLabelV11(const struct fwcmd_ns_label_v1_1_data *p_data)
{
	std::stringstream result;
	result << "\nNs Label V1 1:" << "\n";
	result << fwPayloadFieldsToString_NsLabelV11(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_NsLabelV12(const struct fwcmd_ns_label_v1_2_data *p_data)
{
	std::stringstream result;
	result << "\nNs Label V1 2:" << "\n";
	result << fwPayloadFieldsToString_NsLabelV12(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_NamespaceLabels(const struct fwcmd_namespace_labels_data *p_data)
{
	std::stringstream result;
	result << "\nNamespace Labels:" << "\n";
	result << fwPayloadFieldsToString_NamespaceLabels(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_DimmPartitionInfo(const struct fwcmd_dimm_partition_info_data *p_data)
{
	std::stringstream result;
	result << "\nDimm Partition Info:" << "\n";
	result << fwPayloadFieldsToString_DimmPartitionInfo(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_FwDebugLogLevel(const struct fwcmd_fw_debug_log_level_data *p_data)
{
	std::stringstream result;
	result << "\nFw Debug Log Level:" << "\n";
	result << fwPayloadFieldsToString_FwDebugLogLevel(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_FwLoadFlag(const struct fwcmd_fw_load_flag_data *p_data)
{
	std::stringstream result;
	result << "\nFw Load Flag:" << "\n";
	result << fwPayloadFieldsToString_FwLoadFlag(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_ConfigLockdown(const struct fwcmd_config_lockdown_data *p_data)
{
	std::stringstream result;
	result << "\nConfig Lockdown:" << "\n";
	result << fwPayloadFieldsToString_ConfigLockdown(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_DdrtIoInitInfo(const struct fwcmd_ddrt_io_init_info_data *p_data)
{
	std::stringstream result;
	result << "\nDdrt Io Init Info:" << "\n";
	result << fwPayloadFieldsToString_DdrtIoInitInfo(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_GetSupportedSkuFeatures(const struct fwcmd_get_supported_sku_features_data *p_data)
{
	std::stringstream result;
	result << "\nGet Supported Sku Features:" << "\n";
	result << fwPayloadFieldsToString_GetSupportedSkuFeatures(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_EnableDimm(const struct fwcmd_enable_dimm_data *p_data)
{
	std::stringstream result;
	result << "\nEnable Dimm:" << "\n";
	result << fwPayloadFieldsToString_EnableDimm(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_SmartHealthInfo(const struct fwcmd_smart_health_info_data *p_data)
{
	std::stringstream result;
	result << "\nSmart Health Info:" << "\n";
	result << fwPayloadFieldsToString_SmartHealthInfo(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_FirmwareImageInfo(const struct fwcmd_firmware_image_info_data *p_data)
{
	std::stringstream result;
	result << "\nFirmware Image Info:" << "\n";
	result << fwPayloadFieldsToString_FirmwareImageInfo(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_FirmwareDebugLog(const struct fwcmd_firmware_debug_log_data *p_data)
{
	std::stringstream result;
	result << "\nFirmware Debug Log:" << "\n";
	result << fwPayloadFieldsToString_FirmwareDebugLog(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_LongOperationStatus(const struct fwcmd_long_operation_status_data *p_data)
{
	std::stringstream result;
	result << "\nLong Operation Status:" << "\n";
	result << fwPayloadFieldsToString_LongOperationStatus(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadToString_Bsr(const struct fwcmd_bsr_data *p_data)
{
	std::stringstream result;
	result << "\nBsr:" << "\n";
	result << fwPayloadFieldsToString_Bsr(p_data);
	
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_IdentifyDimm(const struct fwcmd_identify_dimm_data *p_data)
{
	std::stringstream result;
	result << "\nIdentify Dimm:" << "\n";
	result << "VendorId: " << p_data->vendor_id << "\n";
	result << "DeviceId: " << p_data->device_id << "\n";
	result << "RevisionId: " << p_data->revision_id << "\n";
	result << "InterfaceFormatCode: " << p_data->interface_format_code << "\n";
	result << "FirmwareRevision: " << p_data->firmware_revision << "\n";
	result << "ReservedOldApi: " << (int) p_data->reserved_old_api << "\n";
	result << "FeatureSwRequiredMask: " << (int) p_data->feature_sw_required_mask << "\n";	result << "InvalidateBeforeBlockRead: " << p_data->feature_sw_required_mask_invalidate_before_block_read << "\n";
	result << "ReadbackOfBwAddressRegisterRequiredBeforeUse: " << p_data->feature_sw_required_mask_readback_of_bw_address_register_required_before_use << "\n";

	result << "NumberOfBlockWindows: " << p_data->number_of_block_windows << "\n";
	result << "OffsetOfBlockModeControlRegion: " << p_data->offset_of_block_mode_control_region << "\n";
	result << "RawCapacity: " << p_data->raw_capacity << "\n";
	result << "Manufacturer: " << p_data->manufacturer << "\n";
	result << "SerialNumber: " << p_data->serial_number << "\n";
	result << "PartNumber: " << p_data->part_number << "\n";
	result << "DimmSku: " << p_data->dimm_sku << "\n";	result << "MemoryModeEnabled: " << p_data->dimm_sku_memory_mode_enabled << "\n";
	result << "StorageModeEnabled: " << p_data->dimm_sku_storage_mode_enabled << "\n";
	result << "AppDirectModeEnabled: " << p_data->dimm_sku_app_direct_mode_enabled << "\n";
	result << "DieSparingCapable: " << p_data->dimm_sku_die_sparing_capable << "\n";
	result << "SoftProgrammableSku: " << p_data->dimm_sku_soft_programmable_sku << "\n";
	result << "EncryptionEnabled: " << p_data->dimm_sku_encryption_enabled << "\n";

	result << "InterfaceFormatCodeExtra: " << p_data->interface_format_code_extra << "\n";
	result << "ApiVer: " << p_data->api_ver << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_IdentifyDimmCharacteristics(const struct fwcmd_identify_dimm_characteristics_data *p_data)
{
	std::stringstream result;
	result << "\nIdentify Dimm Characteristics:" << "\n";
	result << "ControllerTempShutdownThreshold: " << p_data->controller_temp_shutdown_threshold << "\n";
	result << "MediaTempShutdownThreshold: " << p_data->media_temp_shutdown_threshold << "\n";
	result << "ThrottlingStartThreshold: " << p_data->throttling_start_threshold << "\n";
	result << "ThrottlingStopThreshold: " << p_data->throttling_stop_threshold << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_GetSecurityState(const struct fwcmd_get_security_state_data *p_data)
{
	std::stringstream result;
	result << "\nGet Security State:" << "\n";
	result << "SecurityState: " << (int) p_data->security_state << "\n";	result << "Enabled: " << p_data->security_state_enabled << "\n";
	result << "Locked: " << p_data->security_state_locked << "\n";
	result << "Frozen: " << p_data->security_state_frozen << "\n";
	result << "CountExpired: " << p_data->security_state_count_expired << "\n";
	result << "NotSupported: " << p_data->security_state_not_supported << "\n";

	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_GetAlarmThreshold(const struct fwcmd_get_alarm_threshold_data *p_data)
{
	std::stringstream result;
	result << "\nGet Alarm Threshold:" << "\n";
	result << "Enable: " << p_data->enable << "\n";	result << "SpareBlock: " << p_data->enable_spare_block << "\n";
	result << "MediaTemp: " << p_data->enable_media_temp << "\n";
	result << "ControllerTemp: " << p_data->enable_controller_temp << "\n";

	result << "SpareBlockThreshold: " << (int) p_data->spare_block_threshold << "\n";
	result << "MediaTempThreshold: " << p_data->media_temp_threshold << "\n";
	result << "ControllerTempThreshold: " << p_data->controller_temp_threshold << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_PowerManagementPolicy(const struct fwcmd_power_management_policy_data *p_data)
{
	std::stringstream result;
	result << "\nPower Management Policy:" << "\n";
	result << "Enable: " << (int) p_data->enable << "\n";
	result << "PeakPowerBudget: " << p_data->peak_power_budget << "\n";
	result << "AveragePowerBudget: " << p_data->average_power_budget << "\n";
	result << "MaxPower: " << (int) p_data->max_power << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_DieSparingPolicy(const struct fwcmd_die_sparing_policy_data *p_data)
{
	std::stringstream result;
	result << "\nDie Sparing Policy:" << "\n";
	result << "Enable: " << (int) p_data->enable << "\n";
	result << "Aggressiveness: " << (int) p_data->aggressiveness << "\n";
	result << "Supported: " << (int) p_data->supported << "\n";	result << "Rank0: " << p_data->supported_rank_0 << "\n";
	result << "Rank1: " << p_data->supported_rank_1 << "\n";
	result << "Rank2: " << p_data->supported_rank_2 << "\n";
	result << "Rank3: " << p_data->supported_rank_3 << "\n";

	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_AddressRangeScrub(const struct fwcmd_address_range_scrub_data *p_data)
{
	std::stringstream result;
	result << "\nAddress Range Scrub:" << "\n";
	result << "Enable: " << (int) p_data->enable << "\n";
	result << "DpaStartAddress: " << p_data->dpa_start_address << "\n";
	result << "DpaEndAddress: " << p_data->dpa_end_address << "\n";
	result << "DpaCurrentAddress: " << p_data->dpa_current_address << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_OptionalConfigurationDataPolicy(const struct fwcmd_optional_configuration_data_policy_data *p_data)
{
	std::stringstream result;
	result << "\nOptional Configuration Data Policy:" << "\n";
	result << "FirstFastRefresh: " << (int) p_data->first_fast_refresh << "\n";
	result << "ViralPolicyEnabled: " << (int) p_data->viral_policy_enabled << "\n";
	result << "ViralStatus: " << (int) p_data->viral_status << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_PmonRegisters(const struct fwcmd_pmon_registers_data *p_data)
{
	std::stringstream result;
	result << "\nPmon Registers:" << "\n";
	result << "PmonRetreiveMask: " << p_data->pmon_retreive_mask << "\n";
	result << "Pmon0Counter: " << p_data->pmon_0_counter << "\n";
	result << "Pmon0Control: " << p_data->pmon_0_control << "\n";
	result << "Pmon1Counter: " << p_data->pmon_1_counter << "\n";
	result << "Pmon1Control: " << p_data->pmon_1_control << "\n";
	result << "Pmon2Counter: " << p_data->pmon_2_counter << "\n";
	result << "Pmon2Control: " << p_data->pmon_2_control << "\n";
	result << "Pmon3Counter: " << p_data->pmon_3_counter << "\n";
	result << "Pmon3Control: " << p_data->pmon_3_control << "\n";
	result << "Pmon4Counter: " << p_data->pmon_4_counter << "\n";
	result << "Pmon4Control: " << p_data->pmon_4_control << "\n";
	result << "Pmon5Counter: " << p_data->pmon_5_counter << "\n";
	result << "Pmon5Control: " << p_data->pmon_5_control << "\n";
	result << "Pmon6Counter: " << p_data->pmon_6_counter << "\n";
	result << "Pmon6Control: " << p_data->pmon_6_control << "\n";
	result << "Pmon7Counter: " << p_data->pmon_7_counter << "\n";
	result << "Pmon7Control: " << p_data->pmon_7_control << "\n";
	result << "Pmon8Counter: " << p_data->pmon_8_counter << "\n";
	result << "Pmon8Control: " << p_data->pmon_8_control << "\n";
	result << "Pmon9Counter: " << p_data->pmon_9_counter << "\n";
	result << "Pmon9Control: " << p_data->pmon_9_control << "\n";
	result << "Pmon10Counter: " << p_data->pmon_10_counter << "\n";
	result << "Pmon10Control: " << p_data->pmon_10_control << "\n";
	result << "Pmon11Counter: " << p_data->pmon_11_counter << "\n";
	result << "Pmon11Control: " << p_data->pmon_11_control << "\n";
	result << "Pmon14Counter: " << p_data->pmon_14_counter << "\n";
	result << "Pmon14Control: " << p_data->pmon_14_control << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_SystemTime(const struct fwcmd_system_time_data *p_data)
{
	std::stringstream result;
	result << "\nSystem Time:" << "\n";
	result << "UnixTime: " << p_data->unix_time << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_DeviceIdentificationV1(const struct fwcmd_device_identification_v1_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data Device Identification V1:" << "\n";
	result << "ManufacturerId: " << p_data->manufacturer_id << "\n";
	result << "SerialNumber: " << p_data->serial_number << "\n";
	result << "ModelNumber: " << p_data->model_number << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_DeviceIdentificationV2(const struct fwcmd_device_identification_v2_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data Device Identification V2:" << "\n";
	result << "Uid: " << p_data->uid << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_IdInfoTable(const struct fwcmd_id_info_table_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data Identification Information Table:" << "\n";
	result << fwPayloadToString_DeviceIdentificationV1(&p_data->device_identification.device_identification_v1);
	result << fwPayloadToString_DeviceIdentificationV2(&p_data->device_identification.device_identification_v2);

	result << "PartitionOffset: " << p_data->partition_offset << "\n";
	result << "PartitionSize: " << p_data->partition_size << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_InterleaveInformationTable(const struct fwcmd_interleave_information_table_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data Interleave Information Table:" << "\n";
	result << "Type: " << p_data->type << "\n";
	result << "Length: " << p_data->length << "\n";
	result << "Index: " << p_data->index << "\n";
	result << "NumberOfDimms: " << (int) p_data->number_of_dimms << "\n";
	result << "MemoryType: " << (int) p_data->memory_type << "\n";
	result << "Format: " << p_data->format << "\n";
	result << "MirrorEnabled: " << (int) p_data->mirror_enabled << "\n";
	result << "ChangeStatus: " << (int) p_data->change_status << "\n";
	result << "MemorySpare: " << (int) p_data->memory_spare << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_PartitionSizeChangeTable(const struct fwcmd_partition_size_change_table_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data Partition Size Change Table:" << "\n";
	result << "Type: " << p_data->type << "\n";
	result << "Length: " << p_data->length << "\n";
	result << "PlatformConfigDataPartitionSizeChangeTable: " << p_data->platform_config_data_partition_size_change_table << "\n";
	result << "PersistentMemoryPartitionSize: " << p_data->persistent_memory_partition_size << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_CurrentConfigTable(const struct fwcmd_current_config_table_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data Current Config Table:" << "\n";
	result << "Signature: " << p_data->signature << "\n";
	result << "Length: " << p_data->length << "\n";
	result << "Revision: " << (int) p_data->revision << "\n";
	result << "Checksum: " << (int) p_data->checksum << "\n";
	result << "OemId: " << p_data->oem_id << "\n";
	result << "OemTableId: " << p_data->oem_table_id << "\n";
	result << "OemRevision: " << p_data->oem_revision << "\n";
	result << "CreatorId: " << p_data->creator_id << "\n";
	result << "CreatorRevision: " << p_data->creator_revision << "\n";
	result << "ConfigStatus: " << p_data->config_status << "\n";
	result << "VolatileMemorySize: " << p_data->volatile_memory_size << "\n";
	result << "PersistentMemorySize: " << p_data->persistent_memory_size << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_ConfigInputTable(const struct fwcmd_config_input_table_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data Config Input Table:" << "\n";
	result << "Signature: " << p_data->signature << "\n";
	result << "Length: " << p_data->length << "\n";
	result << "Revision: " << (int) p_data->revision << "\n";
	result << "Checksum: " << (int) p_data->checksum << "\n";
	result << "OemId: " << p_data->oem_id << "\n";
	result << "OemTableId: " << p_data->oem_table_id << "\n";
	result << "OemRevision: " << p_data->oem_revision << "\n";
	result << "CreatorId: " << p_data->creator_id << "\n";
	result << "CreatorRevision: " << p_data->creator_revision << "\n";
	result << "SequenceNumber: " << p_data->sequence_number << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_ConfigOutputTable(const struct fwcmd_config_output_table_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data Config Output Table:" << "\n";
	result << "Signature: " << p_data->signature << "\n";
	result << "Length: " << p_data->length << "\n";
	result << "Revision: " << (int) p_data->revision << "\n";
	result << "Checksum: " << (int) p_data->checksum << "\n";
	result << "OemId: " << p_data->oem_id << "\n";
	result << "OemTableId: " << p_data->oem_table_id << "\n";
	result << "OemRevision: " << p_data->oem_revision << "\n";
	result << "CreatorId: " << p_data->creator_id << "\n";
	result << "CreatorRevision: " << p_data->creator_revision << "\n";
	result << "SequenceNumber: " << p_data->sequence_number << "\n";
	result << "ValidationStatus: " << (int) p_data->validation_status << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_PlatformConfigData(const struct fwcmd_platform_config_data_data *p_data)
{
	std::stringstream result;
	result << "\nPlatform Config Data Configuration Header Table:" << "\n";
	result << "Signature: " << p_data->signature << "\n";
	result << "Length: " << p_data->length << "\n";
	result << "Revision: " << (int) p_data->revision << "\n";
	result << "Checksum: " << (int) p_data->checksum << "\n";
	result << "OemId: " << p_data->oem_id << "\n";
	result << "OemTableId: " << p_data->oem_table_id << "\n";
	result << "OemRevision: " << p_data->oem_revision << "\n";
	result << "CreatorId: " << p_data->creator_id << "\n";
	result << "CreatorRevision: " << p_data->creator_revision << "\n";
	result << "CurrentConfigSize: " << p_data->current_config_size << "\n";
	result << "CurrentConfigOffset: " << p_data->current_config_offset << "\n";
	result << "InputConfigSize: " << p_data->input_config_size << "\n";
	result << "InputConfigOffset: " << p_data->input_config_offset << "\n";
	result << "OutputConfigSize: " << p_data->output_config_size << "\n";
	result << "OutputConfigOffset: " << p_data->output_config_offset << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_NsIndex(const struct fwcmd_ns_index_data *p_data)
{
	std::stringstream result;
	result << "\nNs Index:" << "\n";
	result << "Signature: " << p_data->signature << "\n";
	result << "Flags: " << p_data->flags << "\n";
	result << "Sequence: " << p_data->sequence << "\n";
	result << "MyOffset: " << p_data->my_offset << "\n";
	result << "MySize: " << p_data->my_size << "\n";
	result << "OtherOffset: " << p_data->other_offset << "\n";
	result << "LabelOffset: " << p_data->label_offset << "\n";
	result << "Nlabel: " << p_data->nlabel << "\n";
	result << "LabelMajorVersion: " << p_data->label_major_version << "\n";
	result << "LabelMinorVersion: " << p_data->label_minor_version << "\n";
	result << "Checksum: " << p_data->checksum << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_NsLabel(const struct fwcmd_ns_label_data *p_data)
{
	std::stringstream result;
	result << "\nNs Label:" << "\n";
	result << "Uuid: " << p_data->uuid << "\n";
	result << "Name: " << p_data->name << "\n";
	result << "Flags: " << p_data->flags << "\n";	result << "ReadOnly: " << p_data->flags_read_only << "\n";
	result << "Local: " << p_data->flags_local << "\n";
	result << "Updating: " << p_data->flags_updating << "\n";

	result << "Nlabel: " << p_data->nlabel << "\n";
	result << "Position: " << p_data->position << "\n";
	result << "IsetCookie: " << p_data->iset_cookie << "\n";
	result << "LbaSize: " << p_data->lba_size << "\n";
	result << "Dpa: " << p_data->dpa << "\n";
	result << "Rawsize: " << p_data->rawsize << "\n";
	result << "Slot: " << p_data->slot << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_NsLabelV11(const struct fwcmd_ns_label_v1_1_data *p_data)
{
	std::stringstream result;
	result << "\nNs Label V1 1:" << "\n";
	result << fwPayloadToString_NsLabel(&p_data->label);

	result << "Unused: " << p_data->unused << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_NsLabelV12(const struct fwcmd_ns_label_v1_2_data *p_data)
{
	std::stringstream result;
	result << "\nNs Label V1 2:" << "\n";
	result << fwPayloadToString_NsLabel(&p_data->label);

	result << "Alignment: " << (int) p_data->alignment << "\n";
	result << "Reserved: " << p_data->reserved << "\n";
	result << "TypeGuid: " << p_data->type_guid << "\n";
	result << "AddressAbstractionGuid: " << p_data->address_abstraction_guid << "\n";
	result << "Reserved1: " << p_data->reserved1 << "\n";
	result << "Checksum: " << p_data->checksum << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_NamespaceLabels(const struct fwcmd_namespace_labels_data *p_data)
{
	std::stringstream result;
	result << "\nNamespace Labels:" << "\n";
	result << fwPayloadToString_NsIndex(&p_data->index1);

	result << fwPayloadToString_NsIndex(&p_data->index2);

	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_DimmPartitionInfo(const struct fwcmd_dimm_partition_info_data *p_data)
{
	std::stringstream result;
	result << "\nDimm Partition Info:" << "\n";
	result << "VolatileCapacity: " << p_data->volatile_capacity << "\n";
	result << "VolatileStart: " << p_data->volatile_start << "\n";
	result << "PmCapacity: " << p_data->pm_capacity << "\n";
	result << "PmStart: " << p_data->pm_start << "\n";
	result << "RawCapacity: " << p_data->raw_capacity << "\n";
	result << "EnabledCapacity: " << p_data->enabled_capacity << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_FwDebugLogLevel(const struct fwcmd_fw_debug_log_level_data *p_data)
{
	std::stringstream result;
	result << "\nFw Debug Log Level:" << "\n";
	result << "LogLevel: " << (int) p_data->log_level << "\n";
	result << "Logs: " << (int) p_data->logs << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_FwLoadFlag(const struct fwcmd_fw_load_flag_data *p_data)
{
	std::stringstream result;
	result << "\nFw Load Flag:" << "\n";
	result << "LoadFlag: " << (int) p_data->load_flag << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_ConfigLockdown(const struct fwcmd_config_lockdown_data *p_data)
{
	std::stringstream result;
	result << "\nConfig Lockdown:" << "\n";
	result << "Locked: " << (int) p_data->locked << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_DdrtIoInitInfo(const struct fwcmd_ddrt_io_init_info_data *p_data)
{
	std::stringstream result;
	result << "\nDdrt Io Init Info:" << "\n";
	result << "DdrtIoInfo: " << (int) p_data->ddrt_io_info << "\n";
	result << "DdrtTrainingComplete: " << (int) p_data->ddrt_training_complete << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_GetSupportedSkuFeatures(const struct fwcmd_get_supported_sku_features_data *p_data)
{
	std::stringstream result;
	result << "\nGet Supported Sku Features:" << "\n";
	result << "DimmSku: " << p_data->dimm_sku << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_EnableDimm(const struct fwcmd_enable_dimm_data *p_data)
{
	std::stringstream result;
	result << "\nEnable Dimm:" << "\n";
	result << "Enable: " << (int) p_data->enable << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_SmartHealthInfo(const struct fwcmd_smart_health_info_data *p_data)
{
	std::stringstream result;
	result << "\nSmart Health Info:" << "\n";
	result << "ValidationFlags: " << p_data->validation_flags << "\n";	result << "HealthStatus: " << p_data->validation_flags_health_status << "\n";
	result << "SpareBlocks: " << p_data->validation_flags_spare_blocks << "\n";
	result << "PercentUsed: " << p_data->validation_flags_percent_used << "\n";
	result << "MediaTemp: " << p_data->validation_flags_media_temp << "\n";
	result << "ControllerTemp: " << p_data->validation_flags_controller_temp << "\n";
	result << "UnsafeShutdownCounter: " << p_data->validation_flags_unsafe_shutdown_counter << "\n";
	result << "AitDramStatus: " << p_data->validation_flags_ait_dram_status << "\n";
	result << "AlarmTrips: " << p_data->validation_flags_alarm_trips << "\n";
	result << "LastShutdownStatus: " << p_data->validation_flags_last_shutdown_status << "\n";
	result << "VendorSpecificDataSize: " << p_data->validation_flags_vendor_specific_data_size << "\n";

	result << "HealthStatus: " << (int) p_data->health_status << "\n";	result << "Noncritical: " << p_data->health_status_noncritical << "\n";
	result << "Critical: " << p_data->health_status_critical << "\n";
	result << "Fatal: " << p_data->health_status_fatal << "\n";

	result << "SpareBlocks: " << (int) p_data->spare_blocks << "\n";
	result << "PercentUsed: " << (int) p_data->percent_used << "\n";
	result << "AlarmTrips: " << (int) p_data->alarm_trips << "\n";	result << "SpareBlockTrip: " << p_data->alarm_trips_spare_block_trip << "\n";
	result << "MediaTemperatureTrip: " << p_data->alarm_trips_media_temperature_trip << "\n";
	result << "ControllerTemperatureTrip: " << p_data->alarm_trips_controller_temperature_trip << "\n";

	result << "MediaTemp: " << p_data->media_temp << "\n";
	result << "ControllerTemp: " << p_data->controller_temp << "\n";
	result << "UnsafeShutdownCount: " << p_data->unsafe_shutdown_count << "\n";
	result << "AitDramStatus: " << (int) p_data->ait_dram_status << "\n";
	result << "LastShutdownStatus: " << (int) p_data->last_shutdown_status << "\n";
	result << "VendorSpecificDataSize: " << p_data->vendor_specific_data_size << "\n";
	result << "PowerCycles: " << p_data->power_cycles << "\n";
	result << "PowerOnTime: " << p_data->power_on_time << "\n";
	result << "Uptime: " << p_data->uptime << "\n";
	result << "UnsafeShutdowns: " << p_data->unsafe_shutdowns << "\n";
	result << "LastShutdownStatusDetails: " << (int) p_data->last_shutdown_status_details << "\n";	result << "PmAdrCommandReceived: " << p_data->last_shutdown_status_details_pm_adr_command_received << "\n";
	result << "PmS3Received: " << p_data->last_shutdown_status_details_pm_s3_received << "\n";
	result << "PmS5Received: " << p_data->last_shutdown_status_details_pm_s5_received << "\n";
	result << "DdrtPowerFailCommandReceived: " << p_data->last_shutdown_status_details_ddrt_power_fail_command_received << "\n";
	result << "Pmic12vPowerFail: " << p_data->last_shutdown_status_details_pmic_12v_power_fail << "\n";
	result << "PmWarmResetReceived: " << p_data->last_shutdown_status_details_pm_warm_reset_received << "\n";
	result << "ThermalShutdownReceived: " << p_data->last_shutdown_status_details_thermal_shutdown_received << "\n";
	result << "FlushComplete: " << p_data->last_shutdown_status_details_flush_complete << "\n";

	result << "LastShutdownTime: " << p_data->last_shutdown_time << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_FirmwareImageInfo(const struct fwcmd_firmware_image_info_data *p_data)
{
	std::stringstream result;
	result << "\nFirmware Image Info:" << "\n";
	result << "FirmwareRevision: " << p_data->firmware_revision << "\n";
	result << "FirmwareType: " << (int) p_data->firmware_type << "\n";
	result << "StagedFwRevision: " << p_data->staged_fw_revision << "\n";
	result << "LastFwUpdateStatus: " << (int) p_data->last_fw_update_status << "\n";
	result << "CommitId: " << p_data->commit_id << "\n";
	result << "BuildConfiguration: " << p_data->build_configuration << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_FirmwareDebugLog(const struct fwcmd_firmware_debug_log_data *p_data)
{
	std::stringstream result;
	result << "\nFirmware Debug Log:" << "\n";
	result << "LogSize: " << (int) p_data->log_size << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_LongOperationStatus(const struct fwcmd_long_operation_status_data *p_data)
{
	std::stringstream result;
	result << "\nLong Operation Status:" << "\n";
	result << "Command: " << p_data->command << "\n";
	result << "PercentComplete: " << p_data->percent_complete << "\n";
	result << "EstimateTimeToCompletion: " << p_data->estimate_time_to_completion << "\n";
	result << "StatusCode: " << (int) p_data->status_code << "\n";
	result << "CommandSpecificReturnData: " << p_data->command_specific_return_data << "\n";
	return result.str();
}

std::string FwCommands::fwPayloadFieldsToString_Bsr(const struct fwcmd_bsr_data *p_data)
{
	std::stringstream result;
	result << "\nBsr:" << "\n";
	result << "MajorCheckpoint: " << (int) p_data->major_checkpoint << "\n";
	result << "MinorCheckpoint: " << (int) p_data->minor_checkpoint << "\n";
	result << "Rest1: " << p_data->rest1 << "\n";	result << "MediaReady1: " << p_data->rest1_media_ready_1 << "\n";
	result << "MediaReady2: " << p_data->rest1_media_ready_2 << "\n";
	result << "DdrtIoInitComplete: " << p_data->rest1_ddrt_io_init_complete << "\n";
	result << "PcrLock: " << p_data->rest1_pcr_lock << "\n";
	result << "MailboxReady: " << p_data->rest1_mailbox_ready << "\n";
	result << "WatchDogStatus: " << p_data->rest1_watch_dog_status << "\n";
	result << "FirstFastRefreshComplete: " << p_data->rest1_first_fast_refresh_complete << "\n";
	result << "CreditReady: " << p_data->rest1_credit_ready << "\n";
	result << "MediaDisabled: " << p_data->rest1_media_disabled << "\n";
	result << "OptInEnabled: " << p_data->rest1_opt_in_enabled << "\n";
	result << "OptInWasEnabled: " << p_data->rest1_opt_in_was_enabled << "\n";
	result << "Assertion: " << p_data->rest1_assertion << "\n";
	result << "MiStall: " << p_data->rest1_mi_stall << "\n";
	result << "AitDramReady: " << p_data->rest1_ait_dram_ready << "\n";

	result << "Rest2: " << p_data->rest2 << "\n";
	return result.str();
}

enum return_code FwCommands::dsm_err_to_nvm_lib_err(pt_result result)
{
	enum return_code rc = NVM_SUCCESS;

	if (result.fw_status)
	{
		switch (result.fw_status)
		{
		case DSM_VENDOR_ERR_NOT_SUPPORTED:
			rc = NVM_ERR_NOTSUPPORTED;
			break;
		case DSM_VENDOR_ERR_NONEXISTING:
			rc = NVM_ERR_BADDEVICE;
			break;
		case DSM_VENDOR_INVALID_INPUT:
			rc = NVM_ERR_UNKNOWN;
			break;
		case DSM_VENDOR_HW_ERR:
			rc = NVM_ERR_DEVICEERROR;
			break;
		case DSM_VENDOR_RETRY_SUGGESTED:
        	rc = NVM_ERR_TIMEOUT;
			break;
		case DSM_VENDOR_UNKNOWN:
			rc = NVM_ERR_UNKNOWN;
			break;
		case DSM_VENDOR_SPECIFIC_ERR:
			rc = fw_mb_err_to_nvm_lib_err(result.fw_ext_status);
			break;
		default:
			rc = NVM_ERR_DRIVERFAILED;
			break;
		}
	}
	else if (result.fw_ext_status)
	{
		rc = fw_mb_err_to_nvm_lib_err(result.fw_ext_status);
	}

	return rc;
}

enum return_code FwCommands::fw_mb_err_to_nvm_lib_err(int extended_status)
{
	enum return_code rc = NVM_SUCCESS;

	switch (extended_status)
	{
		case MB_SUCCESS:
			rc = NVM_SUCCESS;
			break;
		case MB_INVALID_CMD_PARAM :
			rc = NVM_ERR_INVALIDPARAMETER;
			break;
		case MB_DATA_XFER_ERR :
			rc = NVM_ERR_DATATRANSFERERROR;
			break;
		case MB_INTERNAL_DEV_ERR :
			rc = NVM_ERR_DEVICEERROR;
			break;
		case MB_UNSUPPORTED_CMD :
			rc = NVM_ERR_NOTSUPPORTED;
			break;
		case MB_DEVICE_BUSY :
			rc = NVM_ERR_DEVICEBUSY;
			break;
		case MB_INVALID_CREDENTIAL :
			rc = NVM_ERR_BADPASSPHRASE;
			break;
		case MB_SECURITY_CHK_FAIL :
			rc = NVM_ERR_BADFIRMWARE;
			break;
		case MB_INVALID_SECURITY_STATE :
			rc = NVM_ERR_BADSECURITYSTATE;
			break;
		case MB_SYSTEM_TIME_NOT_SET :
			rc = NVM_ERR_DEVICEERROR;
			break;
		case MB_DATA_NOT_SET :
			rc = NVM_ERR_DEVICEERROR;
			break;
		case MB_ABORTED :
			rc = NVM_ERR_DEVICEERROR;
			break;
		case MB_NO_NEW_FW :
			rc = NVM_ERR_BADFIRMWARE;
			break;
		case MB_REVISION_FAILURE :
			rc = NVM_ERR_BADFIRMWARE;
			break;
		case MB_INJECTION_DISABLED :
			rc = NVM_ERR_NOTSUPPORTED;
			break;
		case MB_CONFIG_LOCKED_COMMAND_INVALID :
			rc = NVM_ERR_NOTSUPPORTED;
			break;
		case MB_INVALID_ALIGNMENT :
			rc = NVM_ERR_DEVICEERROR;
			break;
		case MB_INCOMPATIBLE_DIMM :
			rc = NVM_ERR_NOTSUPPORTED;
			break;
		case MB_TIMED_OUT :
			rc = NVM_ERR_DEVICEBUSY;
			break;
		case MB_MEDIA_DISABLED :
			rc = NVM_ERR_DEVICEERROR;
			break;
		case MB_FW_UPDATE_ALREADY_OCCURED :
			rc = NVM_ERR_FWALREADYSTAGED;
			break;
		case MB_NO_RESOURCES_AVAILABLE :
			rc = NVM_ERR_DEVICEERROR;
			break;
		default :
			rc = NVM_ERR_DEVICEERROR;
			break;
	}

	return rc;
}

enum return_code FwCommands::convertFwcmdErrorCodeToNvmErrorCode(struct fwcmd_error_code error)
{
	enum return_code rc = NVM_SUCCESS;

	switch (error.type)
	{
	case FWCMD_ERROR_TYPE_DRIVER:
		rc = NVM_ERR_DRIVERFAILED;
		break;
	case FWCMD_ERROR_TYPE_PT:
	{
		pt_result result;
		PT_RESULT_DECODE(error.code, result);

		if ((enum pt_ioctl_result)result.func != PT_SUCCESS)
		{
			switch((enum pt_ioctl_result)result.func)
			{
			case PT_SUCCESS:
			case PT_ERR_UNKNOWN:
				rc = NVM_ERR_UNKNOWN;
				break;
			case PT_ERR_BADDEVICEHANDLE:
			case PT_ERR_BADDEVICE:
				rc = NVM_ERR_BADDEVICE;
				break;
			case PT_ERR_NOMEMORY:
				rc = NVM_ERR_NOMEMORY;
				break;
			case PT_ERR_DRIVERFAILED:
				rc = NVM_ERR_DRIVERFAILED;
				break;
			case PT_ERR_BADSECURITY:
				rc = NVM_ERR_BADSECURITYSTATE;
				break;
			case PT_ERR_DEVICEBUSY:
				rc = NVM_ERR_DEVICEBUSY;
				break;
			case PT_ERR_INVALIDPERMISSIONS:
				rc = NVM_ERR_INVALIDPERMISSIONS;
				break;
			default:
				rc = NVM_ERR_UNKNOWN;
				break;
			}
		}
		else
		{
			rc = dsm_err_to_nvm_lib_err(result);
			break;
		}
		break;
	}
	case FWCMD_ERROR_TYPE_PARSE:
		rc = (enum return_code) error.code;
		break;
	default:
		rc = NVM_ERR_UNKNOWN;
		break;
	}

	return rc;
}

} /* namespace firmware_interface */
} /* namespace core */