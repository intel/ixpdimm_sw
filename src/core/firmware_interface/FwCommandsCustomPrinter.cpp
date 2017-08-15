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
#include "FwCommands.h"

#include <sstream>

namespace core
{
namespace firmware_interface
{

void addInterleaveInfoTable(FwCommands *p_fwCmd, std::stringstream &result,
	int pDataCount, const struct fwcmd_interleave_information_table_data *pData, int revision)
{
	for (int i = 0; i < pDataCount; i++)
	{
		result << p_fwCmd->fwPayloadFieldsToString_InterleaveInformationTable(&pData[i]);

		for (int j = 0; j < pData[i].id_info_table_count; j++)
		{
			fwcmd_id_info_table_data *pInfoTableData = &pData->id_info_table[j];
			result << "\nPlatform Config Data Identification Information Table:" << "\n";

			if (revision == 1)
			{
				fwcmd_device_identification_v1_data &v1Data = pInfoTableData->device_identification.device_identification_v1;
				result << "ManufacturerId: " << v1Data.manufacturer_id << "\n";
				result << "SerialNumber: " << v1Data.serial_number << "\n";
				result << "ModelNumber: " << v1Data.model_number << "\n";
			}
			else if (revision == 2)
			{
				fwcmd_device_identification_v2_data &v2Data = pInfoTableData->device_identification.device_identification_v2;
				result << "Uid: ";
				for (int u = 0; u < 9; u++)
				{
					result << std::hex << (int)v2Data.uid[u];
				}
				result << "\n";
			}

			result << "PartitionOffset: " << pInfoTableData->partition_offset << "\n";
			result << "PartitionSize: " << pInfoTableData->partition_size << "\n";
		}
	}
}

void addPartitionSizeChangeTable(FwCommands *pCmds, std::stringstream &result,
	fwcmd_partition_size_change_table_data *table_data, int tableCount)
{
	for (int j = 0; j < tableCount; j++)
	{
		result << pCmds->fwPayloadFieldsToString_PartitionSizeChangeTable(&table_data[j]);
	}
}

std::string FwCommands::fwPayloadToString_Custom_PlatformConfigData(
	fwcmd_platform_config_data_data *pData)
{
	std::stringstream result;
	result << fwPayloadFieldsToString_PlatformConfigData(pData);

	// Add Current
	fwcmd_current_config_table_data *pCurrent = &(pData->current_config_table);
	result << fwPayloadFieldsToString_CurrentConfigTable(pCurrent);
	addInterleaveInfoTable(this, result,
		pCurrent->interleave_information_table_count,
		pCurrent->interleave_information_table,
		pCurrent->revision);

	// Add Input
	fwcmd_config_input_table_data *pInput = &(pData->config_input_table);
	result << fwPayloadFieldsToString_ConfigInputTable(pInput);
	addInterleaveInfoTable(this, result,
		pInput->interleave_information_table_count,
		pInput->interleave_information_table,
		pInput->revision);
	addPartitionSizeChangeTable(this, result,
		pInput->partition_size_change_table,
		pInput->partition_size_change_table_count);

	// Add Output
	fwcmd_config_output_table_data *pOutput = &(pData->config_output_table);
	result << fwPayloadFieldsToString_ConfigOutputTable(pOutput);
	addInterleaveInfoTable(this, result,
		pOutput->interleave_information_table_count,
		pOutput->interleave_information_table,
		pOutput->revision);
	addPartitionSizeChangeTable(this, result,
		pOutput->partition_size_change_table,
		pOutput->partition_size_change_table_count);

	return result.str();
}

}
}