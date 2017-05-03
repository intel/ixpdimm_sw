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

#include "fw_command_printer.h"
#include <driver_interface/passthrough.h>
#include <stdio.h>
#include <string.h>


void print_tabs(int tab_count)
{
	for(int i = 0; i < tab_count; i++)
	{
		printf("\t");
	}
}

void fwcmd_print_error(struct fwcmd_error_code error)
{
	switch (error.type)
	{
		case FWCMD_ERROR_TYPE_DRIVER:
    		printf("Driver error: 0x%x\n", error.code);
			break;
    	case FWCMD_ERROR_TYPE_PT:
		{
			pt_result result;
			PT_RESULT_DECODE(error.code, result);
			if(result.fw_ext_status)
			{
				char fis_message[1024];
				fis_get_error_message(result.fw_ext_status, fis_message, 1024);
				printf("FW Error: 0x%x - '%s'\n", result.fw_ext_status, fis_message);
			}
			else
			{
				printf("Passthrough Error: 0x%x\n", error.code);
				char error_message[1024];
				pt_get_error_message(error.code, error_message, 1024);
				printf("%s", error_message);
			}

			break;
		}
    	case FWCMD_ERROR_TYPE_PARSE:
    		printf("Parsing error: 0x%x\n", error.code);
			break;
    	case FWCMD_ERROR_TYPE_DUMP:
    		printf("Error: 0x%x\n", error.code);
			break;
		default:
			printf("Unknown error type: %d\n", error.type);
	}
}

void fwcmd_print_command_names()
{
	printf("Commands: \n");
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

void fwcmd_print_output_command_names()
{
	printf("Commands with output: \n");

	printf("\tidentify_dimm\n");
	printf("\tidentify_dimm_characteristics\n");
	printf("\tget_security_state\n");
	printf("\tget_alarm_threshold\n");
	printf("\tpower_management_policy\n");
	printf("\tdie_sparing_policy\n");
	printf("\taddress_range_scrub\n");
	printf("\toptional_configuration_data_policy\n");
	printf("\tpmon_registers\n");
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


void fwcmd_identify_dimm_printer(const struct fwcmd_identify_dimm_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("IdentifyDimm:\n");
	print_tabs(indent_count + 1);
	printf("VendorId: 0x%x\n", p_value->vendor_id);
	print_tabs(indent_count + 1);
	printf("DeviceId: 0x%x\n", p_value->device_id);
	print_tabs(indent_count + 1);
	printf("RevisionId: 0x%x\n", p_value->revision_id);
	print_tabs(indent_count + 1);
	printf("InterfaceFormatCode: 0x%x\n", p_value->interface_format_code);
	print_tabs(indent_count + 1);
	printf("FirmwareRevision: %.5s\n", p_value->firmware_revision);
	print_tabs(indent_count + 1);
	printf("ReservedOldApi: 0x%x\n", p_value->reserved_old_api);
	print_tabs(indent_count + 1);
	printf("FeatureSwRequiredMask: 0x%x\n", p_value->feature_sw_required_mask);
	print_tabs(indent_count + 2);
	printf("InvalidateBeforeBlockRead: %d\n", p_value->feature_sw_required_mask_invalidate_before_block_read);
	print_tabs(indent_count + 2);
	printf("ReadbackOfBwAddressRegisterRequiredBeforeUse: %d\n", p_value->feature_sw_required_mask_readback_of_bw_address_register_required_before_use);
	print_tabs(indent_count + 1);
	printf("NumberOfBlockWindows: 0x%x\n", p_value->number_of_block_windows);
	print_tabs(indent_count + 1);
	printf("OffsetOfBlockModeControlRegion: 0x%x\n", p_value->offset_of_block_mode_control_region);
	print_tabs(indent_count + 1);
	printf("RawCapacity: 0x%x\n", p_value->raw_capacity);
	print_tabs(indent_count + 1);
	printf("Manufacturer: 0x%x\n", p_value->manufacturer);
	print_tabs(indent_count + 1);
	printf("SerialNumber: 0x%x\n", p_value->serial_number);
	print_tabs(indent_count + 1);
	printf("PartNumber: %s\n", p_value->part_number);
	print_tabs(indent_count + 1);
	printf("DimmSku: 0x%x\n", p_value->dimm_sku);
	print_tabs(indent_count + 2);
	printf("MemoryModeEnabled: %d\n", p_value->dimm_sku_memory_mode_enabled);
	print_tabs(indent_count + 2);
	printf("StorageModeEnabled: %d\n", p_value->dimm_sku_storage_mode_enabled);
	print_tabs(indent_count + 2);
	printf("AppDirectModeEnabled: %d\n", p_value->dimm_sku_app_direct_mode_enabled);
	print_tabs(indent_count + 2);
	printf("DieSparingCapable: %d\n", p_value->dimm_sku_die_sparing_capable);
	print_tabs(indent_count + 2);
	printf("SoftProgrammableSku: %d\n", p_value->dimm_sku_soft_programmable_sku);
	print_tabs(indent_count + 2);
	printf("EncryptionEnabled: %d\n", p_value->dimm_sku_encryption_enabled);
	print_tabs(indent_count + 1);
	printf("InterfaceFormatCodeExtra: 0x%x\n", p_value->interface_format_code_extra);
	print_tabs(indent_count + 1);
	printf("ApiVer: 0x%x\n", p_value->api_ver);
}

void fwcmd_identify_dimm_characteristics_printer(const struct fwcmd_identify_dimm_characteristics_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("IdentifyDimmCharacteristics:\n");
	print_tabs(indent_count + 1);
	printf("ControllerTempShutdownThreshold: 0x%x\n", p_value->controller_temp_shutdown_threshold);
	print_tabs(indent_count + 1);
	printf("MediaTempShutdownThreshold: 0x%x\n", p_value->media_temp_shutdown_threshold);
	print_tabs(indent_count + 1);
	printf("ThrottlingStartThreshold: 0x%x\n", p_value->throttling_start_threshold);
	print_tabs(indent_count + 1);
	printf("ThrottlingStopThreshold: 0x%x\n", p_value->throttling_stop_threshold);
}

void fwcmd_get_security_state_printer(const struct fwcmd_get_security_state_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("GetSecurityState:\n");
	print_tabs(indent_count + 1);
	printf("SecurityState: 0x%x\n", p_value->security_state);
	print_tabs(indent_count + 2);
	printf("Enabled: %d\n", p_value->security_state_enabled);
	print_tabs(indent_count + 2);
	printf("Locked: %d\n", p_value->security_state_locked);
	print_tabs(indent_count + 2);
	printf("Frozen: %d\n", p_value->security_state_frozen);
	print_tabs(indent_count + 2);
	printf("CountExpired: %d\n", p_value->security_state_count_expired);
	print_tabs(indent_count + 2);
	printf("NotSupported: %d\n", p_value->security_state_not_supported);
}

void fwcmd_get_alarm_threshold_printer(const struct fwcmd_get_alarm_threshold_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("GetAlarmThreshold:\n");
	print_tabs(indent_count + 1);
	printf("Enable: 0x%x\n", p_value->enable);
	print_tabs(indent_count + 2);
	printf("SpareBlock: %d\n", p_value->enable_spare_block);
	print_tabs(indent_count + 2);
	printf("MediaTemp: %d\n", p_value->enable_media_temp);
	print_tabs(indent_count + 2);
	printf("ControllerTemp: %d\n", p_value->enable_controller_temp);
	print_tabs(indent_count + 1);
	printf("SpareBlockThreshold: 0x%x\n", p_value->spare_block_threshold);
	print_tabs(indent_count + 1);
	printf("MediaTempThreshold: 0x%x\n", p_value->media_temp_threshold);
	print_tabs(indent_count + 1);
	printf("ControllerTempThreshold: 0x%x\n", p_value->controller_temp_threshold);
}

void fwcmd_power_management_policy_printer(const struct fwcmd_power_management_policy_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("PowerManagementPolicy:\n");
	print_tabs(indent_count + 1);
	printf("Enable: 0x%x\n", p_value->enable);
	print_tabs(indent_count + 1);
	printf("PeakPowerBudget: 0x%x\n", p_value->peak_power_budget);
	print_tabs(indent_count + 1);
	printf("AveragePowerBudget: 0x%x\n", p_value->average_power_budget);
	print_tabs(indent_count + 1);
	printf("MaxPower: 0x%x\n", p_value->max_power);
}

void fwcmd_die_sparing_policy_printer(const struct fwcmd_die_sparing_policy_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("DieSparingPolicy:\n");
	print_tabs(indent_count + 1);
	printf("Enable: 0x%x\n", p_value->enable);
	print_tabs(indent_count + 1);
	printf("Aggressiveness: 0x%x\n", p_value->aggressiveness);
	print_tabs(indent_count + 1);
	printf("Supported: 0x%x\n", p_value->supported);
	print_tabs(indent_count + 2);
	printf("Rank0: %d\n", p_value->supported_rank_0);
	print_tabs(indent_count + 2);
	printf("Rank1: %d\n", p_value->supported_rank_1);
	print_tabs(indent_count + 2);
	printf("Rank2: %d\n", p_value->supported_rank_2);
	print_tabs(indent_count + 2);
	printf("Rank3: %d\n", p_value->supported_rank_3);
}

void fwcmd_address_range_scrub_printer(const struct fwcmd_address_range_scrub_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("AddressRangeScrub:\n");
	print_tabs(indent_count + 1);
	printf("Enable: 0x%x\n", p_value->enable);
	print_tabs(indent_count + 1);
	printf("DpaStartAddress: 0x%llx\n", p_value->dpa_start_address);
	print_tabs(indent_count + 1);
	printf("DpaEndAddress: 0x%llx\n", p_value->dpa_end_address);
	print_tabs(indent_count + 1);
	printf("DpaCurrentAddress: 0x%llx\n", p_value->dpa_current_address);
}

void fwcmd_optional_configuration_data_policy_printer(const struct fwcmd_optional_configuration_data_policy_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("OptionalConfigurationDataPolicy:\n");
	print_tabs(indent_count + 1);
	printf("FirstFastRefresh: 0x%x\n", p_value->first_fast_refresh);
	print_tabs(indent_count + 1);
	printf("ViralPolicyEnabled: 0x%x\n", p_value->viral_policy_enabled);
	print_tabs(indent_count + 1);
	printf("ViralStatus: 0x%x\n", p_value->viral_status);
}

void fwcmd_pmon_registers_printer(const struct fwcmd_pmon_registers_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("PmonRegisters:\n");
	print_tabs(indent_count + 1);
	printf("PmonRetreiveMask: 0x%x\n", p_value->pmon_retreive_mask);
	print_tabs(indent_count + 1);
	printf("Pmon0Counter: 0x%x\n", p_value->pmon_0_counter);
	print_tabs(indent_count + 1);
	printf("Pmon0Control: 0x%x\n", p_value->pmon_0_control);
	print_tabs(indent_count + 1);
	printf("Pmon1Counter: 0x%x\n", p_value->pmon_1_counter);
	print_tabs(indent_count + 1);
	printf("Pmon1Control: 0x%x\n", p_value->pmon_1_control);
	print_tabs(indent_count + 1);
	printf("Pmon2Counter: 0x%x\n", p_value->pmon_2_counter);
	print_tabs(indent_count + 1);
	printf("Pmon2Control: 0x%x\n", p_value->pmon_2_control);
	print_tabs(indent_count + 1);
	printf("Pmon3Counter: 0x%x\n", p_value->pmon_3_counter);
	print_tabs(indent_count + 1);
	printf("Pmon3Control: 0x%x\n", p_value->pmon_3_control);
	print_tabs(indent_count + 1);
	printf("Pmon4Counter: 0x%x\n", p_value->pmon_4_counter);
	print_tabs(indent_count + 1);
	printf("Pmon4Control: 0x%x\n", p_value->pmon_4_control);
	print_tabs(indent_count + 1);
	printf("Pmon5Counter: 0x%x\n", p_value->pmon_5_counter);
	print_tabs(indent_count + 1);
	printf("Pmon5Control: 0x%x\n", p_value->pmon_5_control);
	print_tabs(indent_count + 1);
	printf("Pmon6Counter: 0x%x\n", p_value->pmon_6_counter);
	print_tabs(indent_count + 1);
	printf("Pmon6Control: 0x%x\n", p_value->pmon_6_control);
	print_tabs(indent_count + 1);
	printf("Pmon7Counter: 0x%x\n", p_value->pmon_7_counter);
	print_tabs(indent_count + 1);
	printf("Pmon7Control: 0x%x\n", p_value->pmon_7_control);
	print_tabs(indent_count + 1);
	printf("Pmon8Counter: 0x%x\n", p_value->pmon_8_counter);
	print_tabs(indent_count + 1);
	printf("Pmon8Control: 0x%x\n", p_value->pmon_8_control);
	print_tabs(indent_count + 1);
	printf("Pmon9Counter: 0x%x\n", p_value->pmon_9_counter);
	print_tabs(indent_count + 1);
	printf("Pmon9Control: 0x%x\n", p_value->pmon_9_control);
	print_tabs(indent_count + 1);
	printf("Pmon10Counter: 0x%x\n", p_value->pmon_10_counter);
	print_tabs(indent_count + 1);
	printf("Pmon10Control: 0x%x\n", p_value->pmon_10_control);
	print_tabs(indent_count + 1);
	printf("Pmon11Counter: 0x%x\n", p_value->pmon_11_counter);
	print_tabs(indent_count + 1);
	printf("Pmon11Control: 0x%x\n", p_value->pmon_11_control);
	print_tabs(indent_count + 1);
	printf("Pmon14Counter: 0x%x\n", p_value->pmon_14_counter);
	print_tabs(indent_count + 1);
	printf("Pmon14Control: 0x%x\n", p_value->pmon_14_control);
}

void fwcmd_system_time_printer(const struct fwcmd_system_time_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("SystemTime:\n");
	print_tabs(indent_count + 1);
	printf("UnixTime: 0x%llx\n", p_value->unix_time);
}

void fwcmd_platform_config_data_identification_information_table_printer(const struct fwcmd_platform_config_data_identification_information_table_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("PlatformConfigDataIdentificationInformationTable:\n");
	print_tabs(indent_count + 1);
	printf("ManufacturerId: 0x%x\n", p_value->manufacturer_id);
	print_tabs(indent_count + 1);
	printf("SerialNumber: 0x%x\n", p_value->serial_number);
	print_tabs(indent_count + 1);
	printf("ModelNumber: %s\n", p_value->model_number);
	print_tabs(indent_count + 1);
	printf("PartitionOffset: 0x%llx\n", p_value->partition_offset);
	print_tabs(indent_count + 1);
	printf("PartitionSize: 0x%llx\n", p_value->partition_size);
}

void fwcmd_platform_config_data_interleave_information_table_printer(const struct fwcmd_platform_config_data_interleave_information_table_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("PlatformConfigDataInterleaveInformationTable:\n");
	print_tabs(indent_count + 1);
	printf("Type: 0x%x\n", p_value->type);
	print_tabs(indent_count + 1);
	printf("Length: 0x%x\n", p_value->length);
	print_tabs(indent_count + 1);
	printf("Index: 0x%x\n", p_value->index);
	print_tabs(indent_count + 1);
	printf("NumberOfDimms: 0x%x\n", p_value->number_of_dimms);
	print_tabs(indent_count + 1);
	printf("MemoryType: 0x%x\n", p_value->memory_type);
	print_tabs(indent_count + 1);
	printf("Format: 0x%x\n", p_value->format);
	print_tabs(indent_count + 1);
	printf("MirrorEnabled: 0x%x\n", p_value->mirror_enabled);
	print_tabs(indent_count + 1);
	printf("ChangeStatus: 0x%x\n", p_value->change_status);
	print_tabs(indent_count + 1);
	printf("MemorySpare: 0x%x\n", p_value->memory_spare);
	for (size_t i = 0; i < p_value->platform_config_data_identification_information_table_count; i++)
	{
		fwcmd_platform_config_data_identification_information_table_printer(&p_value->platform_config_data_identification_information_table[i], indent_count + 1);
	}

}

void fwcmd_platform_config_data_partition_size_change_table_printer(const struct fwcmd_platform_config_data_partition_size_change_table_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("PlatformConfigDataPartitionSizeChangeTable:\n");
	print_tabs(indent_count + 1);
	printf("Type: 0x%x\n", p_value->type);
	print_tabs(indent_count + 1);
	printf("Length: 0x%x\n", p_value->length);
	print_tabs(indent_count + 1);
	printf("PlatformConfigDataPartitionSizeChangeTable: 0x%x\n", p_value->platform_config_data_partition_size_change_table);
	print_tabs(indent_count + 1);
	printf("PersistentMemoryPartitionSize: 0x%llx\n", p_value->persistent_memory_partition_size);
}

void fwcmd_platform_config_data_current_config_table_printer(const struct fwcmd_platform_config_data_current_config_table_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("PlatformConfigDataCurrentConfigTable:\n");
	print_tabs(indent_count + 1);
	printf("Signature: %s\n", p_value->signature);
	print_tabs(indent_count + 1);
	printf("Length: 0x%x\n", p_value->length);
	print_tabs(indent_count + 1);
	printf("Revision: 0x%x\n", p_value->revision);
	print_tabs(indent_count + 1);
	printf("Checksum: 0x%x\n", p_value->checksum);
	print_tabs(indent_count + 1);
	printf("OemId: %s\n", p_value->oem_id);
	print_tabs(indent_count + 1);
	printf("OemTableId: %s\n", p_value->oem_table_id);
	print_tabs(indent_count + 1);
	printf("OemRevision: 0x%x\n", p_value->oem_revision);
	print_tabs(indent_count + 1);
	printf("CreatorId: 0x%x\n", p_value->creator_id);
	print_tabs(indent_count + 1);
	printf("CreatorRevision: 0x%x\n", p_value->creator_revision);
	print_tabs(indent_count + 1);
	printf("ConfigStatus: 0x%x\n", p_value->config_status);
	print_tabs(indent_count + 1);
	printf("VolatileMemorySize: 0x%llx\n", p_value->volatile_memory_size);
	print_tabs(indent_count + 1);
	printf("PersistentMemorySize: 0x%llx\n", p_value->persistent_memory_size);
	for (size_t i = 0; i < p_value->platform_config_data_interleave_information_table_count; i++)
	{
		fwcmd_platform_config_data_interleave_information_table_printer(&p_value->platform_config_data_interleave_information_table[i], indent_count + 1);
	}
}

void fwcmd_platform_config_data_config_input_table_printer(const struct fwcmd_platform_config_data_config_input_table_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("PlatformConfigDataConfigInputTable:\n");
	print_tabs(indent_count + 1);
	printf("Signature: %s\n", p_value->signature);
	print_tabs(indent_count + 1);
	printf("Length: 0x%x\n", p_value->length);
	print_tabs(indent_count + 1);
	printf("Revision: 0x%x\n", p_value->revision);
	print_tabs(indent_count + 1);
	printf("Checksum: 0x%x\n", p_value->checksum);
	print_tabs(indent_count + 1);
	printf("OemId: %s\n", p_value->oem_id);
	print_tabs(indent_count + 1);
	printf("OemTableId: %s\n", p_value->oem_table_id);
	print_tabs(indent_count + 1);
	printf("OemRevision: 0x%x\n", p_value->oem_revision);
	print_tabs(indent_count + 1);
	printf("CreatorId: 0x%x\n", p_value->creator_id);
	print_tabs(indent_count + 1);
	printf("CreatorRevision: 0x%x\n", p_value->creator_revision);
	print_tabs(indent_count + 1);
	printf("SequenceNumber: 0x%x\n", p_value->sequence_number);
	for (size_t i = 0; i < p_value->platform_config_data_interleave_information_table_count; i++)
	{
		fwcmd_platform_config_data_interleave_information_table_printer(&p_value->platform_config_data_interleave_information_table[i], indent_count + 1);
	}
	for (size_t i = 0; i < p_value->platform_config_data_partition_size_change_table_count; i++)
	{
		fwcmd_platform_config_data_partition_size_change_table_printer(&p_value->platform_config_data_partition_size_change_table[i], indent_count + 1);
	}
}

void fwcmd_platform_config_data_config_output_table_printer(const struct fwcmd_platform_config_data_config_output_table_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("PlatformConfigDataConfigOutputTable:\n");
	print_tabs(indent_count + 1);
	printf("Signature: %s\n", p_value->signature);
	print_tabs(indent_count + 1);
	printf("Length: 0x%x\n", p_value->length);
	print_tabs(indent_count + 1);
	printf("Revision: 0x%x\n", p_value->revision);
	print_tabs(indent_count + 1);
	printf("Checksum: 0x%x\n", p_value->checksum);
	print_tabs(indent_count + 1);
	printf("OemId: %s\n", p_value->oem_id);
	print_tabs(indent_count + 1);
	printf("OemTableId: %s\n", p_value->oem_table_id);
	print_tabs(indent_count + 1);
	printf("OemRevision: 0x%x\n", p_value->oem_revision);
	print_tabs(indent_count + 1);
	printf("CreatorId: 0x%x\n", p_value->creator_id);
	print_tabs(indent_count + 1);
	printf("CreatorRevision: 0x%x\n", p_value->creator_revision);
	print_tabs(indent_count + 1);
	printf("SequenceNumber: 0x%x\n", p_value->sequence_number);
	print_tabs(indent_count + 1);
	printf("ValidationStatus: 0x%x\n", p_value->validation_status);
	for (size_t i = 0; i < p_value->platform_config_data_interleave_information_table_count; i++)
	{
		fwcmd_platform_config_data_interleave_information_table_printer(&p_value->platform_config_data_interleave_information_table[i], indent_count + 1);
	}
	for (size_t i = 0; i < p_value->platform_config_data_partition_size_change_table_count; i++)
	{
		fwcmd_platform_config_data_partition_size_change_table_printer(&p_value->platform_config_data_partition_size_change_table[i], indent_count + 1);
	}
}

void fwcmd_platform_config_data_configuration_header_table_printer(const struct fwcmd_platform_config_data_configuration_header_table_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("PlatformConfigDataConfigurationHeaderTable:\n");
	print_tabs(indent_count + 1);
	printf("Signature: %s\n", p_value->signature);
	print_tabs(indent_count + 1);
	printf("Length: 0x%x\n", p_value->length);
	print_tabs(indent_count + 1);
	printf("Revision: 0x%x\n", p_value->revision);
	print_tabs(indent_count + 1);
	printf("Checksum: 0x%x\n", p_value->checksum);
	print_tabs(indent_count + 1);
	printf("OemId: %s\n", p_value->oem_id);
	print_tabs(indent_count + 1);
	printf("OemTableId: %s\n", p_value->oem_table_id);
	print_tabs(indent_count + 1);
	printf("OemRevision: 0x%x\n", p_value->oem_revision);
	print_tabs(indent_count + 1);
	printf("CreatorId: 0x%x\n", p_value->creator_id);
	print_tabs(indent_count + 1);
	printf("CreatorRevision: 0x%x\n", p_value->creator_revision);
	print_tabs(indent_count + 1);
	printf("CurrentConfigSize: 0x%x\n", p_value->current_config_size);
	print_tabs(indent_count + 1);
	printf("CurrentConfigOffset: 0x%x\n", p_value->current_config_offset);
	print_tabs(indent_count + 1);
	printf("InputConfigSize: 0x%x\n", p_value->input_config_size);
	print_tabs(indent_count + 1);
	printf("InputConfigOffset: 0x%x\n", p_value->input_config_offset);
	print_tabs(indent_count + 1);
	printf("OutputConfigSize: 0x%x\n", p_value->output_config_size);
	print_tabs(indent_count + 1);
	printf("OutputConfigOffset: 0x%x\n", p_value->output_config_offset);
	fwcmd_platform_config_data_current_config_table_printer(&p_value->platform_config_data_current_config_table, indent_count + 1);
	fwcmd_platform_config_data_config_input_table_printer(&p_value->platform_config_data_config_input_table, indent_count + 1);
	fwcmd_platform_config_data_config_output_table_printer(&p_value->platform_config_data_config_output_table, indent_count + 1);
}

void fwcmd_dimm_partition_info_printer(const struct fwcmd_dimm_partition_info_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("DimmPartitionInfo:\n");
	print_tabs(indent_count + 1);
	printf("VolatileCapacity: 0x%x\n", p_value->volatile_capacity);
	print_tabs(indent_count + 1);
	printf("VolatileStart: 0x%llx\n", p_value->volatile_start);
	print_tabs(indent_count + 1);
	printf("PmCapacity: 0x%x\n", p_value->pm_capacity);
	print_tabs(indent_count + 1);
	printf("PmStart: 0x%llx\n", p_value->pm_start);
	print_tabs(indent_count + 1);
	printf("RawCapacity: 0x%x\n", p_value->raw_capacity);
	print_tabs(indent_count + 1);
	printf("EnabledCapacity: 0x%x\n", p_value->enabled_capacity);
}

void fwcmd_fw_debug_log_level_printer(const struct fwcmd_fw_debug_log_level_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("FwDebugLogLevel:\n");
	print_tabs(indent_count + 1);
	printf("LogLevel: 0x%x\n", p_value->log_level);
	print_tabs(indent_count + 1);
	printf("Logs: 0x%x\n", p_value->logs);
}

void fwcmd_fw_load_flag_printer(const struct fwcmd_fw_load_flag_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("FwLoadFlag:\n");
	print_tabs(indent_count + 1);
	printf("LoadFlag: 0x%x\n", p_value->load_flag);
}

void fwcmd_config_lockdown_printer(const struct fwcmd_config_lockdown_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("ConfigLockdown:\n");
	print_tabs(indent_count + 1);
	printf("Locked: 0x%x\n", p_value->locked);
}

void fwcmd_ddrt_io_init_info_printer(const struct fwcmd_ddrt_io_init_info_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("DdrtIoInitInfo:\n");
	print_tabs(indent_count + 1);
	printf("DdrtIoInfo: 0x%x\n", p_value->ddrt_io_info);
	print_tabs(indent_count + 1);
	printf("DdrtTrainingComplete: 0x%x\n", p_value->ddrt_training_complete);
}

void fwcmd_get_supported_sku_features_printer(const struct fwcmd_get_supported_sku_features_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("GetSupportedSkuFeatures:\n");
	print_tabs(indent_count + 1);
	printf("DimmSku: 0x%x\n", p_value->dimm_sku);
}

void fwcmd_enable_dimm_printer(const struct fwcmd_enable_dimm_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("EnableDimm:\n");
	print_tabs(indent_count + 1);
	printf("Enable: 0x%x\n", p_value->enable);
}

void fwcmd_smart_health_info_printer(const struct fwcmd_smart_health_info_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("SmartHealthInfo:\n");
	print_tabs(indent_count + 1);
	printf("ValidationFlags: 0x%x\n", p_value->validation_flags);
	print_tabs(indent_count + 2);
	printf("HealthStatus: %d\n", p_value->validation_flags_health_status);
	print_tabs(indent_count + 2);
	printf("SpareBlocks: %d\n", p_value->validation_flags_spare_blocks);
	print_tabs(indent_count + 2);
	printf("PercentUsed: %d\n", p_value->validation_flags_percent_used);
	print_tabs(indent_count + 2);
	printf("MediaTemp: %d\n", p_value->validation_flags_media_temp);
	print_tabs(indent_count + 2);
	printf("ControllerTemp: %d\n", p_value->validation_flags_controller_temp);
	print_tabs(indent_count + 2);
	printf("UnsafeShutdownCounter: %d\n", p_value->validation_flags_unsafe_shutdown_counter);
	print_tabs(indent_count + 2);
	printf("AitDramStatus: %d\n", p_value->validation_flags_ait_dram_status);
	print_tabs(indent_count + 2);
	printf("AlarmTrips: %d\n", p_value->validation_flags_alarm_trips);
	print_tabs(indent_count + 2);
	printf("LastShutdownStatus: %d\n", p_value->validation_flags_last_shutdown_status);
	print_tabs(indent_count + 2);
	printf("VendorSpecificDataSize: %d\n", p_value->validation_flags_vendor_specific_data_size);
	print_tabs(indent_count + 1);
	printf("HealthStatus: 0x%x\n", p_value->health_status);
	print_tabs(indent_count + 2);
	printf("Noncritical: %d\n", p_value->health_status_noncritical);
	print_tabs(indent_count + 2);
	printf("Critical: %d\n", p_value->health_status_critical);
	print_tabs(indent_count + 2);
	printf("Fatal: %d\n", p_value->health_status_fatal);
	print_tabs(indent_count + 1);
	printf("SpareBlocks: 0x%x\n", p_value->spare_blocks);
	print_tabs(indent_count + 1);
	printf("PercentUsed: 0x%x\n", p_value->percent_used);
	print_tabs(indent_count + 1);
	printf("AlarmTrips: 0x%x\n", p_value->alarm_trips);
	print_tabs(indent_count + 2);
	printf("SpareBlockTrip: %d\n", p_value->alarm_trips_spare_block_trip);
	print_tabs(indent_count + 2);
	printf("MediaTemperatureTrip: %d\n", p_value->alarm_trips_media_temperature_trip);
	print_tabs(indent_count + 2);
	printf("ControllerTemperatureTrip: %d\n", p_value->alarm_trips_controller_temperature_trip);
	print_tabs(indent_count + 1);
	printf("MediaTemp: 0x%x\n", p_value->media_temp);
	print_tabs(indent_count + 1);
	printf("ControllerTemp: 0x%x\n", p_value->controller_temp);
	print_tabs(indent_count + 1);
	printf("UnsafeShutdownCount: 0x%x\n", p_value->unsafe_shutdown_count);
	print_tabs(indent_count + 1);
	printf("AitDramStatus: 0x%x\n", p_value->ait_dram_status);
	print_tabs(indent_count + 1);
	printf("LastShutdownStatus: 0x%x\n", p_value->last_shutdown_status);
	print_tabs(indent_count + 1);
	printf("VendorSpecificDataSize: 0x%x\n", p_value->vendor_specific_data_size);
	print_tabs(indent_count + 1);
	printf("PowerCycles: 0x%llx\n", p_value->power_cycles);
	print_tabs(indent_count + 1);
	printf("PowerOnTime: 0x%llx\n", p_value->power_on_time);
	print_tabs(indent_count + 1);
	printf("Uptime: 0x%llx\n", p_value->uptime);
	print_tabs(indent_count + 1);
	printf("UnsafeShutdowns: 0x%x\n", p_value->unsafe_shutdowns);
	print_tabs(indent_count + 1);
	printf("LastShutdownStatusDetails: 0x%x\n", p_value->last_shutdown_status_details);
	print_tabs(indent_count + 2);
	printf("PmAdrCommandReceived: %d\n", p_value->last_shutdown_status_details_pm_adr_command_received);
	print_tabs(indent_count + 2);
	printf("PmS3Received: %d\n", p_value->last_shutdown_status_details_pm_s3_received);
	print_tabs(indent_count + 2);
	printf("PmS5Received: %d\n", p_value->last_shutdown_status_details_pm_s5_received);
	print_tabs(indent_count + 2);
	printf("DdrtPowerFailCommandReceived: %d\n", p_value->last_shutdown_status_details_ddrt_power_fail_command_received);
	print_tabs(indent_count + 2);
	printf("Pmic12vPowerFail: %d\n", p_value->last_shutdown_status_details_pmic_12v_power_fail);
	print_tabs(indent_count + 2);
	printf("PmWarmResetReceived: %d\n", p_value->last_shutdown_status_details_pm_warm_reset_received);
	print_tabs(indent_count + 2);
	printf("ThermalShutdownReceived: %d\n", p_value->last_shutdown_status_details_thermal_shutdown_received);
	print_tabs(indent_count + 2);
	printf("FlushComplete: %d\n", p_value->last_shutdown_status_details_flush_complete);
	print_tabs(indent_count + 1);
	printf("LastShutdownTime: 0x%llx\n", p_value->last_shutdown_time);
}

void fwcmd_firmware_image_info_printer(const struct fwcmd_firmware_image_info_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("FirmwareImageInfo:\n");
	print_tabs(indent_count + 1);
	printf("FirmwareRevision: %.5s\n", p_value->firmware_revision);
	print_tabs(indent_count + 1);
	printf("FirmwareType: 0x%x\n", p_value->firmware_type);
	print_tabs(indent_count + 1);
	printf("StagedFwRevision: %.5s\n", p_value->staged_fw_revision);
	print_tabs(indent_count + 1);
	printf("LastFwUpdateStatus: 0x%x\n", p_value->last_fw_update_status);
	print_tabs(indent_count + 1);
	printf("CommitId: %s\n", p_value->commit_id);
	print_tabs(indent_count + 1);
	printf("BuildConfiguration: %s\n", p_value->build_configuration);
}

void fwcmd_firmware_debug_log_printer(const struct fwcmd_firmware_debug_log_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("FirmwareDebugLog:\n");
	print_tabs(indent_count + 1);
	printf("LogSize: 0x%x\n", p_value->log_size);
}

void fwcmd_long_operation_status_printer(const struct fwcmd_long_operation_status_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("LongOperationStatus:\n");
	print_tabs(indent_count + 1);
	printf("Command: 0x%x\n", p_value->command);
	print_tabs(indent_count + 1);
	printf("PercentComplete: 0x%x\n", p_value->percent_complete);
	print_tabs(indent_count + 1);
	printf("EstimateTimeToCompletion: 0x%x\n", p_value->estimate_time_to_completion);
	print_tabs(indent_count + 1);
	printf("StatusCode: 0x%x\n", p_value->status_code);
	print_tabs(indent_count + 1);
	printf("CommandSpecificReturnData: %.119s\n", p_value->command_specific_return_data);
}

void fwcmd_bsr_printer(const struct fwcmd_bsr_data *p_value, int indent_count)
{
	print_tabs(indent_count);
	printf("Bsr:\n");
	print_tabs(indent_count + 1);
	printf("MajorCheckpoint: 0x%x\n", p_value->major_checkpoint);
	print_tabs(indent_count + 1);
	printf("MinorCheckpoint: 0x%x\n", p_value->minor_checkpoint);
	print_tabs(indent_count + 1);
	printf("Rest1: 0x%x\n", p_value->rest1);
	print_tabs(indent_count + 2);
	printf("MediaReady1: %d\n", p_value->rest1_media_ready_1);
	print_tabs(indent_count + 2);
	printf("MediaReady2: %d\n", p_value->rest1_media_ready_2);
	print_tabs(indent_count + 2);
	printf("DdrtIoInitComplete: %d\n", p_value->rest1_ddrt_io_init_complete);
	print_tabs(indent_count + 2);
	printf("PcrLock: %d\n", p_value->rest1_pcr_lock);
	print_tabs(indent_count + 2);
	printf("MailboxReady: %d\n", p_value->rest1_mailbox_ready);
	print_tabs(indent_count + 2);
	printf("WatchDogStatus: %d\n", p_value->rest1_watch_dog_status);
	print_tabs(indent_count + 2);
	printf("FirstFastRefreshComplete: %d\n", p_value->rest1_first_fast_refresh_complete);
	print_tabs(indent_count + 2);
	printf("CreditReady: %d\n", p_value->rest1_credit_ready);
	print_tabs(indent_count + 2);
	printf("MediaDisabled: %d\n", p_value->rest1_media_disabled);
	print_tabs(indent_count + 2);
	printf("OptInEnabled: %d\n", p_value->rest1_opt_in_enabled);
	print_tabs(indent_count + 2);
	printf("OptInWasEnabled: %d\n", p_value->rest1_opt_in_was_enabled);
	print_tabs(indent_count + 2);
	printf("Assertion: %d\n", p_value->rest1_assertion);
	print_tabs(indent_count + 2);
	printf("MiStall: %d\n", p_value->rest1_mi_stall);
	print_tabs(indent_count + 2);
	printf("AitDramReady: %d\n", p_value->rest1_ait_dram_ready);
	print_tabs(indent_count + 1);
	printf("Rest2: 0x%x\n", p_value->rest2);
}

