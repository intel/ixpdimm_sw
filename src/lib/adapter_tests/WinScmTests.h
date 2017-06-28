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

#ifndef CR_MGMT_WINSCMTESTS_H
#define CR_MGMT_WINSCMTESTS_H

#include <gtest/gtest.h>
#include <firmware_interface/fis_commands.h>
#include <lib/win_scm2_ioctl_passthrough.h>
#include <lib/win_scm2_version_info.h>
#include <lib/win_scm2_capabilities.h>
#include <lib/win_scm2_adapter.h>
#include <lib/win_scm2_passthrough.h>
#include <firmware_interface/fw_commands.h>
#include <firmware_interface/fis_parser.h>
#include <firmware_interface/fw_command_printer.h>

bool g_print = false;

static const std::string ARG_PRINT = "print";

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);

	for (int i = 1; i < argc; ++i)
	{
		if(argv[i] == ARG_PRINT)
		{
			g_print = true;
		}
	}

	return RUN_ALL_TESTS();
}

class WinScmTests : public ::testing::Test
{

};

TEST_F(WinScmTests, get_security_state)
{
	struct pt_output_get_security_state output_payload;
	struct fw_cmd cmd;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = 1;
	cmd.opcode = 0x02;
	cmd.sub_opcode = 0x00;
	cmd.output_payload_size = sizeof (struct pt_output_get_security_state);
	cmd.output_payload = &output_payload;

	unsigned int dsm_status;
	int scm_status = win_scm2_passthrough(&cmd, &dsm_status);

	if (scm_status != 0)
	{
		FAIL() << "SCM ERROR: " << scm_status;
	}
	else if (dsm_status != 0)
	{
		FAIL() << "DSM ERROR: " << dsm_status;
	}
}


TEST_F(WinScmTests, get_platform_config_from_fw_interface)
{
	const fwcmd_platform_config_data_configuration_header_table_result &result =
			fwcmd_alloc_platform_config_data_configuration_header_table(1, 1, 0, 0);

	if(result.success)
	{
		fwcmd_platform_config_data_configuration_header_table_printer(result.p_data, 0);
	}
	else
	{
		FAIL() << "ERROR: " << result.error_code.code;
	}
}

void read_pcd()
{
	struct pt_input_platform_config_data_configuration_header_table input_payload;
	memset(&input_payload, 0, sizeof (input_payload));
	input_payload.partition_id = 1;
	input_payload.command_option = 0;
	input_payload.offset = 0;

	struct pt_output_platform_config_data_configuration_header_table large_output_payload;
	memset(&large_output_payload, 0, sizeof (large_output_payload));

	unsigned char output_payload[128];
	memset(output_payload, 0, 128);

	struct fw_cmd cmd;
	unsigned int dsm_status;
	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = 1;
	cmd.opcode = 0x06;
	cmd.sub_opcode = 0x01;
	cmd.large_output_payload_size = sizeof (struct pt_output_platform_config_data_configuration_header_table);
	cmd.large_output_payload = &large_output_payload;
	cmd.output_payload = &output_payload;
	cmd.output_payload_size = 128;
	cmd.input_payload = &input_payload;
	cmd.input_payload_size = sizeof (struct pt_input_platform_config_data_configuration_header_table);
	int scm_status = win_scm2_passthrough(&cmd, &dsm_status);

	if (scm_status != 0)
	{
		FAIL() << "SCM ERROR: " << scm_status;
	}
	else if (dsm_status != 0)
	{
		FAIL() << "DSM ERROR: " << dsm_status;
	}
	else
	{
		struct fwcmd_platform_config_data_configuration_header_table_data parsed;
		int parse_status = fis_parse_platform_config_data_configuration_header_table(&large_output_payload, &parsed);
		if (!FWCMD_PARSE_SUCCESS(parse_status))
		{
			FAIL() << "Parse failed: " << parse_status;
		}
		else
		{
			fwcmd_platform_config_data_configuration_header_table_printer(&parsed, 0);
		}
	}
}

TEST_F(WinScmTests, pcd_passthrough)
{
	read_pcd();
}

TEST_F(WinScmTests, identify_dimm_passthrough)
{
	struct fw_cmd cmd;
	unsigned int dsm_status;


	memset(&cmd, 0, sizeof (struct pt_fw_cmd));
	cmd.device_handle = 1;
	cmd.opcode = 0x01;
	cmd.sub_opcode = 0x00;
	cmd.output_payload_size = sizeof (struct pt_output_identify_dimm);
	struct pt_output_identify_dimm p_output_payload;
	cmd.output_payload = &p_output_payload;

	int scm_status = win_scm2_passthrough(&cmd, &dsm_status);

	if (scm_status != 0)
	{
		FAIL() << "SCM ERROR: " << scm_status;


	}
	else if (dsm_status != 0)
	{
		FAIL() << "DSM ERROR: " << dsm_status;
	}

}

TEST_F(WinScmTests, large_payload)
{
	pt_input_platform_config_data_configuration_header_table pcd_input;
	memset(&pcd_input, 0, sizeof (pcd_input));
	pcd_input.partition_id = 1;

	int buffer_size = 128;
	unsigned char buffer[buffer_size];

	unsigned int dsm_status;
	int rc = win_scm2_ioctl_passthrough_cmd(1, // handle
			6, 1, // op codes
			&pcd_input, sizeof (pcd_input), // input
			buffer, buffer_size, // output
			&dsm_status);

	if (rc != 0)
	{
		FAIL() << "RC: " << rc;
	}
	else if(dsm_status != 0)
	{
		FAIL() << "DSM STATUS: " << dsm_status;
	}
	else
	{
		int pl_size = 128;
		char pl[pl_size];
		memset(pl, 0, pl_size);

		rc = win_scm2_ioctl_passthrough_cmd(1, // handle
				PT_BIOS_EMULATED_COMMAND, PT_SUBOP_GET_PAYLOAD_SIZE, // op codes
				buffer, buffer_size, // input
				&pl, pl_size, // output
				&dsm_status);
		if (rc != 0)
		{
			FAIL() << "RC: " << rc;
		}
		else if(dsm_status != 0)
		{
			FAIL() << "DSM STATUS: " << dsm_status;
		}
		else if (g_print)
		{
			printf("[%s:%d]RYON> success\n", __FILE__, __LINE__);
			printf("[%s:%d]RYON> input_payload_size: %u\n", __FILE__, __LINE__, ((struct pt_bios_get_size *)pl)->large_input_payload_size);
			printf("[%s:%d]RYON> output_payload_size: %u\n", __FILE__, __LINE__, ((struct pt_bios_get_size *)pl)->large_output_payload_size);
			printf("[%s:%d]RYON> rw_size: %u\n", __FILE__, __LINE__, ((struct pt_bios_get_size *)pl)->rw_size);
		}
	}
}

TEST_F(WinScmTests, identify_dimm_passthrough_ioctl)
{
	pt_output_identify_dimm id_dimm;
	unsigned int dsm_status;
	int rc = win_scm2_ioctl_passthrough_cmd(1, 1, 0, (void *) NULL, 0, &id_dimm, sizeof(id_dimm),
			&dsm_status);

	if (rc != 0)
	{
		FAIL() << "RC: " << rc;
	}
	else if(dsm_status != 0)
	{
		FAIL() << "DSM STATUS: " << dsm_status;
	}
	else if (g_print)
	{
		printf("vendor_id: 0x%x\n", id_dimm.vendor_id);
		printf("device_id: 0x%x\n", id_dimm.device_id);
		printf("revision_id: 0x%x\n", id_dimm.revision_id);
		printf("interface_format_code: 0x%x\n", id_dimm.interface_format_code);
		printf("reserved_old_api: 0x%x\n", id_dimm.reserved_old_api);
		printf("feature_sw_required_mask: 0x%x\n", id_dimm.feature_sw_required_mask);
		printf("number_of_block_windows: 0x%x\n", id_dimm.number_of_block_windows);
		printf("offset_of_block_mode_control_region: 0x%x\n",
				id_dimm.offset_of_block_mode_control_region);
		printf("raw_capacity: 0x%x\n", id_dimm.raw_capacity);
		printf("manufacturer: 0x%x\n", id_dimm.manufacturer);
		printf("serial_number: 0x%x\n", id_dimm.serial_number);
		printf("dimm_sku: 0x%x\n", id_dimm.dimm_sku);
		printf("interface_format_code_extra: 0x%x\n", id_dimm.interface_format_code_extra);
		printf("api_ver: 0x%x\n", id_dimm.api_ver);
	}
}

TEST_F(WinScmTests, get_version_info)
{
	GET_INTERFACE_VERSION_OUTPUT_PAYLOAD version_info;
	int rc = win_scm2_version_info(1, &version_info);

	if (rc != 0)
	{
		FAIL() << "RC: " << rc;
	}
	else if (g_print)
	{
		printf("version_info.MajorVersion: 0x%lx\n", version_info.MajorVersion);
		printf("version_info.MinorVersion: 0x%lx\n", version_info.MinorVersion);
	}
}


TEST_F(WinScmTests, get_capabilities)
{
	DRIVER_CAPABILITIES capabilities;
	int rc = win_scm2_ioctl_get_driver_capabilities(1, &capabilities);
	if (rc != 0)
	{
		FAIL() << "RC: " << rc;
	}
	else if (g_print)
	{
		printf("capabilities.NumBlockSizes: %d\n", (int) capabilities.NumBlockSizes);
		for (int i = 0; i < MAX_NUMBER_OF_BLOCK_SIZES; i++)
		{
			printf("capabilities.BlockSizes[%d]: %d\n", i, (int) capabilities.BlockSizes[i]);
		}
		printf("capabilities.MinNamespaceSize: %d\n", (int) capabilities.MinNamespaceSize);
		printf("capabilities.NamespaceAlignment: %d\n", (int) capabilities.NamespaceAlignment);


		printf("capabilities.SupportedFeatures.GetTopology: %d\n",
				(int) capabilities.SupportedFeatures.GetTopology);
		printf("capabilities.SupportedFeatures.GetInterleave: %d\n",
				(int) capabilities.SupportedFeatures.GetInterleave);
		printf("capabilities.SupportedFeatures.GetDimmDetail: %d\n",
				(int) capabilities.SupportedFeatures.GetDimmDetail);
		printf("capabilities.SupportedFeatures.GetNamespaces: %d\n",
				(int) capabilities.SupportedFeatures.GetNamespaces);
		printf("capabilities.SupportedFeatures.GetNamespaceDetail: %d\n",
				(int) capabilities.SupportedFeatures.GetNamespaceDetail);
		printf("capabilities.SupportedFeatures.GetPowerData: %d\n",
				(int) capabilities.SupportedFeatures.GetPowerData);
		printf("capabilities.SupportedFeatures.CreateNamespace: %d\n",
				(int) capabilities.SupportedFeatures.CreateNamespace);
		printf("capabilities.SupportedFeatures.RenameNamespace: %d\n",
				(int) capabilities.SupportedFeatures.RenameNamespace);
		printf("capabilities.SupportedFeatures.DeleteNamespace: %d\n",
				(int) capabilities.SupportedFeatures.DeleteNamespace);
		printf("capabilities.SupportedFeatures.RunDiagnostic: %d\n",
				(int) capabilities.SupportedFeatures.RunDiagnostic);
		printf("capabilities.SupportedFeatures.SendPassthru: %d\n",
				(int) capabilities.SupportedFeatures.SendPassthru);
		printf("capabilities.SupportedFeatures.HealthInfoReporting: %d\n",
				(int) capabilities.SupportedFeatures.HealthInfoReporting);
		printf("capabilities.SupportedFeatures.AcpiNfitTableUpdateNotification: %d\n",
				(int) capabilities.SupportedFeatures.AcpiNfitTableUpdateNotification);
		printf("capabilities.SupportedFeatures.AcpiNfitHealthEventNotification: %d\n",
				(int) capabilities.SupportedFeatures.AcpiNfitHealthEventNotification);
		printf("capabilities.SupportedFeatures.CommandEffects: %d\n",
				(int) capabilities.SupportedFeatures.CommandEffects);
		printf("capabilities.SupportedFeatures.PolledSmartDetection: %d\n",
				(int) capabilities.SupportedFeatures.PolledSmartDetection);
		printf("capabilities.SupportedFeatures.DeviceStateChanges: %d\n",
				(int) capabilities.SupportedFeatures.DeviceStateChanges);
		printf("capabilities.SupportedFeatures.BlockNamespace: %d\n",
				(int) capabilities.SupportedFeatures.BlockNamespace);
		printf("capabilities.SupportedFeatures.PmemNamespace: %d\n",
				(int) capabilities.SupportedFeatures.PmemNamespace);
		printf("capabilities.SupportedFeatures.EnumerateDimm: %d\n",
				(int) capabilities.SupportedFeatures.EnumerateDimm);
		printf("capabilities.SupportedFeatures.ScmDriverImplementation: %d\n",
				(int) capabilities.SupportedFeatures.ScmDriverImplementation);
	}
}


#endif //CR_MGMT_WINSCMTESTS_H
