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

#include "fis_commands.h"
#include "fis_parser.h"
#include "fw_command_dump.h"
#include "fw_command_printer.h"

#include <common/string/s_str.h>
#include <stdio.h>


#define	COMMAND_NAME_BUFFER_SIZE 256

int fwcmd_dump(const char *command_name, unsigned int handle, const char *filename)
{
	int rc = FWCMD_DUMP_RESULT_SUCCESS;
	if (s_strncmpi(command_name, "identify_dimm",
		sizeof ("identify_dimm")) == 0)
	{
		rc = fwcmd_dump_identify_dimm(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "identify_dimm_characteristics",
		sizeof ("identify_dimm_characteristics")) == 0)
	{
		rc = fwcmd_dump_identify_dimm_characteristics(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "get_security_state",
		sizeof ("get_security_state")) == 0)
	{
		rc = fwcmd_dump_get_security_state(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "get_alarm_threshold",
		sizeof ("get_alarm_threshold")) == 0)
	{
		rc = fwcmd_dump_get_alarm_threshold(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "power_management_policy",
		sizeof ("power_management_policy")) == 0)
	{
		rc = fwcmd_dump_power_management_policy(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "die_sparing_policy",
		sizeof ("die_sparing_policy")) == 0)
	{
		rc = fwcmd_dump_die_sparing_policy(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "address_range_scrub",
		sizeof ("address_range_scrub")) == 0)
	{
		rc = fwcmd_dump_address_range_scrub(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "optional_configuration_data_policy",
		sizeof ("optional_configuration_data_policy")) == 0)
	{
		rc = fwcmd_dump_optional_configuration_data_policy(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "pmon_registers",
		sizeof ("pmon_registers")) == 0)
	{
		rc = fwcmd_dump_pmon_registers(handle,
			0,
			filename);
	}
	else if (s_strncmpi(command_name, "system_time",
		sizeof ("system_time")) == 0)
	{
		rc = fwcmd_dump_system_time(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "platform_config_data",
		sizeof ("platform_config_data")) == 0)
	{
		rc = fwcmd_dump_platform_config_data(handle,
			1,
			0,
			0,
			filename);
	}
	else if (s_strncmpi(command_name, "namespace_labels",
		sizeof ("namespace_labels")) == 0)
	{
		rc = fwcmd_dump_namespace_labels(handle,
			2,
			0,
			0,
			filename);
	}
	else if (s_strncmpi(command_name, "dimm_partition_info",
		sizeof ("dimm_partition_info")) == 0)
	{
		rc = fwcmd_dump_dimm_partition_info(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "fw_debug_log_level",
		sizeof ("fw_debug_log_level")) == 0)
	{
		rc = fwcmd_dump_fw_debug_log_level(handle,
			0,
			filename);
	}
	else if (s_strncmpi(command_name, "fw_load_flag",
		sizeof ("fw_load_flag")) == 0)
	{
		rc = fwcmd_dump_fw_load_flag(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "config_lockdown",
		sizeof ("config_lockdown")) == 0)
	{
		rc = fwcmd_dump_config_lockdown(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "ddrt_io_init_info",
		sizeof ("ddrt_io_init_info")) == 0)
	{
		rc = fwcmd_dump_ddrt_io_init_info(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "get_supported_sku_features",
		sizeof ("get_supported_sku_features")) == 0)
	{
		rc = fwcmd_dump_get_supported_sku_features(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "enable_dimm",
		sizeof ("enable_dimm")) == 0)
	{
		rc = fwcmd_dump_enable_dimm(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "smart_health_info",
		sizeof ("smart_health_info")) == 0)
	{
		rc = fwcmd_dump_smart_health_info(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "firmware_image_info",
		sizeof ("firmware_image_info")) == 0)
	{
		rc = fwcmd_dump_firmware_image_info(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "firmware_debug_log",
		sizeof ("firmware_debug_log")) == 0)
	{
		rc = fwcmd_dump_firmware_debug_log(handle,
			0,
			0,
			0,
			filename);
	}
	else if (s_strncmpi(command_name, "long_operation_status",
		sizeof ("long_operation_status")) == 0)
	{
		rc = fwcmd_dump_long_operation_status(handle,
			filename);
	}
	else if (s_strncmpi(command_name, "bsr",
		sizeof ("bsr")) == 0)
	{
		rc = fwcmd_dump_bsr(handle,
			filename);
	}
	else
	{
		printf("Command \"%s\" not recognized. Available commands: \n", command_name);
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
		printf("\tplatform_config_data\n");
		printf("\tnamespace_labels\n");
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
		rc = FWCMD_DUMP_RESULT_ERR;
	}
	return rc;
}

void fwcmd_read_and_print(const char *filename)
{
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
		unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);
		pFile = NULL;

		if (bytes_read == fsize)
		{
			char command_name[COMMAND_NAME_BUFFER_SIZE];
			s_strcpy(command_name, (char *)buffer, COMMAND_NAME_BUFFER_SIZE);
            unsigned char *p_payload = buffer + COMMAND_NAME_BUFFER_SIZE;

            if (s_strncmpi(command_name, "identify_dimm",
            		sizeof ("identify_dimm")) == 0)
            {
            	struct fwcmd_identify_dimm_data data;

				fis_parse_identify_dimm((struct pt_output_identify_dimm *)p_payload, &data);
				fwcmd_identify_dimm_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "identify_dimm_characteristics",
            		sizeof ("identify_dimm_characteristics")) == 0)
            {
            	struct fwcmd_identify_dimm_characteristics_data data;

				fis_parse_identify_dimm_characteristics((struct pt_output_identify_dimm_characteristics *)p_payload, &data);
				fwcmd_identify_dimm_characteristics_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "get_security_state",
            		sizeof ("get_security_state")) == 0)
            {
            	struct fwcmd_get_security_state_data data;

				fis_parse_get_security_state((struct pt_output_get_security_state *)p_payload, &data);
				fwcmd_get_security_state_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "get_alarm_threshold",
            		sizeof ("get_alarm_threshold")) == 0)
            {
            	struct fwcmd_get_alarm_threshold_data data;

				fis_parse_get_alarm_threshold((struct pt_output_get_alarm_threshold *)p_payload, &data);
				fwcmd_get_alarm_threshold_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "power_management_policy",
            		sizeof ("power_management_policy")) == 0)
            {
            	struct fwcmd_power_management_policy_data data;

				fis_parse_power_management_policy((struct pt_output_power_management_policy *)p_payload, &data);
				fwcmd_power_management_policy_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "die_sparing_policy",
            		sizeof ("die_sparing_policy")) == 0)
            {
            	struct fwcmd_die_sparing_policy_data data;

				fis_parse_die_sparing_policy((struct pt_output_die_sparing_policy *)p_payload, &data);
				fwcmd_die_sparing_policy_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "address_range_scrub",
            		sizeof ("address_range_scrub")) == 0)
            {
            	struct fwcmd_address_range_scrub_data data;

				fis_parse_address_range_scrub((struct pt_output_address_range_scrub *)p_payload, &data);
				fwcmd_address_range_scrub_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "optional_configuration_data_policy",
            		sizeof ("optional_configuration_data_policy")) == 0)
            {
            	struct fwcmd_optional_configuration_data_policy_data data;

				fis_parse_optional_configuration_data_policy((struct pt_output_optional_configuration_data_policy *)p_payload, &data);
				fwcmd_optional_configuration_data_policy_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "pmon_registers",
            		sizeof ("pmon_registers")) == 0)
            {
            	struct fwcmd_pmon_registers_data data;

				fis_parse_pmon_registers((struct pt_output_pmon_registers *)p_payload, &data);
				fwcmd_pmon_registers_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "system_time",
            		sizeof ("system_time")) == 0)
            {
            	struct fwcmd_system_time_data data;

				fis_parse_system_time((struct pt_output_system_time *)p_payload, &data);
				fwcmd_system_time_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "platform_config_data",
            		sizeof ("platform_config_data")) == 0)
            {
            	struct fwcmd_platform_config_data_data data;

				fis_parse_platform_config_data((struct pt_output_platform_config_data *)p_payload, &data);
				fwcmd_platform_config_data_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "namespace_labels",
            		sizeof ("namespace_labels")) == 0)
            {
            	struct fwcmd_namespace_labels_data data;

				fis_parse_namespace_labels((struct pt_output_namespace_labels *)p_payload, &data);
				fwcmd_namespace_labels_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "dimm_partition_info",
            		sizeof ("dimm_partition_info")) == 0)
            {
            	struct fwcmd_dimm_partition_info_data data;

				fis_parse_dimm_partition_info((struct pt_output_dimm_partition_info *)p_payload, &data);
				fwcmd_dimm_partition_info_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "fw_debug_log_level",
            		sizeof ("fw_debug_log_level")) == 0)
            {
            	struct fwcmd_fw_debug_log_level_data data;

				fis_parse_fw_debug_log_level((struct pt_output_fw_debug_log_level *)p_payload, &data);
				fwcmd_fw_debug_log_level_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "fw_load_flag",
            		sizeof ("fw_load_flag")) == 0)
            {
            	struct fwcmd_fw_load_flag_data data;

				fis_parse_fw_load_flag((struct pt_output_fw_load_flag *)p_payload, &data);
				fwcmd_fw_load_flag_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "config_lockdown",
            		sizeof ("config_lockdown")) == 0)
            {
            	struct fwcmd_config_lockdown_data data;

				fis_parse_config_lockdown((struct pt_output_config_lockdown *)p_payload, &data);
				fwcmd_config_lockdown_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "ddrt_io_init_info",
            		sizeof ("ddrt_io_init_info")) == 0)
            {
            	struct fwcmd_ddrt_io_init_info_data data;

				fis_parse_ddrt_io_init_info((struct pt_output_ddrt_io_init_info *)p_payload, &data);
				fwcmd_ddrt_io_init_info_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "get_supported_sku_features",
            		sizeof ("get_supported_sku_features")) == 0)
            {
            	struct fwcmd_get_supported_sku_features_data data;

				fis_parse_get_supported_sku_features((struct pt_output_get_supported_sku_features *)p_payload, &data);
				fwcmd_get_supported_sku_features_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "enable_dimm",
            		sizeof ("enable_dimm")) == 0)
            {
            	struct fwcmd_enable_dimm_data data;

				fis_parse_enable_dimm((struct pt_output_enable_dimm *)p_payload, &data);
				fwcmd_enable_dimm_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "smart_health_info",
            		sizeof ("smart_health_info")) == 0)
            {
            	struct fwcmd_smart_health_info_data data;

				fis_parse_smart_health_info((struct pt_output_smart_health_info *)p_payload, &data);
				fwcmd_smart_health_info_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "firmware_image_info",
            		sizeof ("firmware_image_info")) == 0)
            {
            	struct fwcmd_firmware_image_info_data data;

				fis_parse_firmware_image_info((struct pt_output_firmware_image_info *)p_payload, &data);
				fwcmd_firmware_image_info_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "firmware_debug_log",
            		sizeof ("firmware_debug_log")) == 0)
            {
            	struct fwcmd_firmware_debug_log_data data;

				fis_parse_firmware_debug_log((struct pt_output_firmware_debug_log *)p_payload, &data);
				fwcmd_firmware_debug_log_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "long_operation_status",
            		sizeof ("long_operation_status")) == 0)
            {
            	struct fwcmd_long_operation_status_data data;

				fis_parse_long_operation_status((struct pt_output_long_operation_status *)p_payload, &data);
				fwcmd_long_operation_status_printer(&data, 0);
            }
            else if (s_strncmpi(command_name, "bsr",
            		sizeof ("bsr")) == 0)
            {
            	struct fwcmd_bsr_data data;

				fis_parse_bsr((struct pt_output_bsr *)p_payload, &data);
				fwcmd_bsr_printer(&data, 0);
            }

		}
		else
		{
			printf("Issue reading file.\n");
		}

        free(buffer);
	}
	else
	{
		printf("Issue opening file.\n");
	}
}

int fwcmd_dump_identify_dimm(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_identify_dimm output_payload;

		rc = fis_identify_dimm(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "identify_dimm", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_identify_dimm_result fwcmd_read_identify_dimm(const char *filename)
{
	struct fwcmd_identify_dimm_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_identify_dimm_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_identify_dimm((const struct pt_output_identify_dimm*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_identify_dimm_characteristics(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_identify_dimm_characteristics output_payload;

		rc = fis_identify_dimm_characteristics(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "identify_dimm_characteristics", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_identify_dimm_characteristics_result fwcmd_read_identify_dimm_characteristics(const char *filename)
{
	struct fwcmd_identify_dimm_characteristics_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_identify_dimm_characteristics_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_identify_dimm_characteristics((const struct pt_output_identify_dimm_characteristics*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_get_security_state(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_get_security_state output_payload;

		rc = fis_get_security_state(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "get_security_state", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_get_security_state_result fwcmd_read_get_security_state(const char *filename)
{
	struct fwcmd_get_security_state_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_get_security_state_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_get_security_state((const struct pt_output_get_security_state*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_get_alarm_threshold(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_get_alarm_threshold output_payload;

		rc = fis_get_alarm_threshold(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "get_alarm_threshold", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_get_alarm_threshold_result fwcmd_read_get_alarm_threshold(const char *filename)
{
	struct fwcmd_get_alarm_threshold_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_get_alarm_threshold_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_get_alarm_threshold((const struct pt_output_get_alarm_threshold*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_power_management_policy(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_power_management_policy output_payload;

		rc = fis_power_management_policy(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "power_management_policy", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_power_management_policy_result fwcmd_read_power_management_policy(const char *filename)
{
	struct fwcmd_power_management_policy_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

        size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_power_management_policy_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_power_management_policy((const struct pt_output_power_management_policy*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_die_sparing_policy(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_die_sparing_policy output_payload;

		rc = fis_die_sparing_policy(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "die_sparing_policy", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_die_sparing_policy_result fwcmd_read_die_sparing_policy(const char *filename)
{
	struct fwcmd_die_sparing_policy_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_die_sparing_policy_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_die_sparing_policy((const struct pt_output_die_sparing_policy*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_address_range_scrub(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_address_range_scrub output_payload;

		rc = fis_address_range_scrub(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "address_range_scrub", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_address_range_scrub_result fwcmd_read_address_range_scrub(const char *filename)
{
	struct fwcmd_address_range_scrub_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_address_range_scrub_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_address_range_scrub((const struct pt_output_address_range_scrub*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_optional_configuration_data_policy(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_optional_configuration_data_policy output_payload;

		rc = fis_optional_configuration_data_policy(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "optional_configuration_data_policy", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_optional_configuration_data_policy_result fwcmd_read_optional_configuration_data_policy(const char *filename)
{
	struct fwcmd_optional_configuration_data_policy_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_optional_configuration_data_policy_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_optional_configuration_data_policy((const struct pt_output_optional_configuration_data_policy*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_pmon_registers(const int handle,
	const unsigned short pmon_retreive_mask,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_pmon_registers output_payload;

		struct pt_input_pmon_registers input_payload;
		input_payload.pmon_retreive_mask = pmon_retreive_mask;
		rc = fis_pmon_registers(handle,
			&input_payload,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "pmon_registers", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_pmon_registers_result fwcmd_read_pmon_registers(const char *filename)
{
	struct fwcmd_pmon_registers_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_pmon_registers_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_pmon_registers((const struct pt_output_pmon_registers*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_system_time(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_system_time output_payload;

		rc = fis_system_time(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "system_time", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_system_time_result fwcmd_read_system_time(const char *filename)
{
	struct fwcmd_system_time_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_system_time_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_system_time((const struct pt_output_system_time*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_platform_config_data(const int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_platform_config_data output_payload;

		struct pt_input_platform_config_data input_payload;
		input_payload.partition_id = partition_id;
		input_payload.command_option = command_option;
		input_payload.offset = offset;
		rc = fis_platform_config_data(handle,
			&input_payload,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "platform_config_data", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_platform_config_data_result fwcmd_read_platform_config_data(const char *filename)
{
	struct fwcmd_platform_config_data_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_platform_config_data_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_platform_config_data((const struct pt_output_platform_config_data*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_namespace_labels(const int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_namespace_labels output_payload;

		struct pt_input_namespace_labels input_payload;
		input_payload.partition_id = partition_id;
		input_payload.command_option = command_option;
		input_payload.offset = offset;
		rc = fis_namespace_labels(handle,
			&input_payload,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "namespace_labels", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_namespace_labels_result fwcmd_read_namespace_labels(const char *filename)
{
	struct fwcmd_namespace_labels_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_namespace_labels_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_namespace_labels((const struct pt_output_namespace_labels*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_dimm_partition_info(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_dimm_partition_info output_payload;

		rc = fis_dimm_partition_info(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "dimm_partition_info", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_dimm_partition_info_result fwcmd_read_dimm_partition_info(const char *filename)
{
	struct fwcmd_dimm_partition_info_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_dimm_partition_info_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_dimm_partition_info((const struct pt_output_dimm_partition_info*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_fw_debug_log_level(const int handle,
	const unsigned char log_id,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_fw_debug_log_level output_payload;

		struct pt_input_fw_debug_log_level input_payload;
		input_payload.log_id = log_id;
		rc = fis_fw_debug_log_level(handle,
			&input_payload,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "fw_debug_log_level", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_fw_debug_log_level_result fwcmd_read_fw_debug_log_level(const char *filename)
{
	struct fwcmd_fw_debug_log_level_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_fw_debug_log_level_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_fw_debug_log_level((const struct pt_output_fw_debug_log_level*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_fw_load_flag(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_fw_load_flag output_payload;

		rc = fis_fw_load_flag(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "fw_load_flag", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_fw_load_flag_result fwcmd_read_fw_load_flag(const char *filename)
{
	struct fwcmd_fw_load_flag_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_fw_load_flag_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_fw_load_flag((const struct pt_output_fw_load_flag*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_config_lockdown(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_config_lockdown output_payload;

		rc = fis_config_lockdown(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "config_lockdown", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_config_lockdown_result fwcmd_read_config_lockdown(const char *filename)
{
	struct fwcmd_config_lockdown_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_config_lockdown_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_config_lockdown((const struct pt_output_config_lockdown*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_ddrt_io_init_info(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_ddrt_io_init_info output_payload;

		rc = fis_ddrt_io_init_info(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "ddrt_io_init_info", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_ddrt_io_init_info_result fwcmd_read_ddrt_io_init_info(const char *filename)
{
	struct fwcmd_ddrt_io_init_info_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_ddrt_io_init_info_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_ddrt_io_init_info((const struct pt_output_ddrt_io_init_info*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_get_supported_sku_features(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_get_supported_sku_features output_payload;

		rc = fis_get_supported_sku_features(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "get_supported_sku_features", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_get_supported_sku_features_result fwcmd_read_get_supported_sku_features(const char *filename)
{
	struct fwcmd_get_supported_sku_features_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_get_supported_sku_features_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_get_supported_sku_features((const struct pt_output_get_supported_sku_features*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_enable_dimm(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_enable_dimm output_payload;

		rc = fis_enable_dimm(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "enable_dimm", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_enable_dimm_result fwcmd_read_enable_dimm(const char *filename)
{
	struct fwcmd_enable_dimm_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_enable_dimm_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_enable_dimm((const struct pt_output_enable_dimm*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_smart_health_info(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_smart_health_info output_payload;

		rc = fis_smart_health_info(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "smart_health_info", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_smart_health_info_result fwcmd_read_smart_health_info(const char *filename)
{
	struct fwcmd_smart_health_info_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_smart_health_info_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_smart_health_info((const struct pt_output_smart_health_info*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_firmware_image_info(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_firmware_image_info output_payload;

		rc = fis_firmware_image_info(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "firmware_image_info", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_firmware_image_info_result fwcmd_read_firmware_image_info(const char *filename)
{
	struct fwcmd_firmware_image_info_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_firmware_image_info_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_firmware_image_info((const struct pt_output_firmware_image_info*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_firmware_debug_log(const int handle,
	const unsigned char log_action,
	const unsigned int log_page_offset,
	const unsigned char log_id,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_firmware_debug_log output_payload;

		struct pt_input_firmware_debug_log input_payload;
		input_payload.log_action = log_action;
		input_payload.log_page_offset = log_page_offset;
		input_payload.log_id = log_id;
		rc = fis_firmware_debug_log(handle,
			&input_payload,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "firmware_debug_log", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_firmware_debug_log_result fwcmd_read_firmware_debug_log(const char *filename)
{
	struct fwcmd_firmware_debug_log_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_firmware_debug_log_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_firmware_debug_log((const struct pt_output_firmware_debug_log*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_long_operation_status(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_long_operation_status output_payload;

		rc = fis_long_operation_status(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "long_operation_status", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_long_operation_status_result fwcmd_read_long_operation_status(const char *filename)
{
	struct fwcmd_long_operation_status_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_long_operation_status_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_long_operation_status((const struct pt_output_long_operation_status*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
int fwcmd_dump_bsr(const int handle,
	const char * filename)
{
	FILE *pFile = fopen(filename, "wb");
	int rc = 0;
	if (pFile)
	{
		struct pt_output_bsr output_payload;

		rc = fis_bsr(handle,
			&output_payload);
		if (rc == 0)
		{
			size_t bytes_written = 0;
			char name[COMMAND_NAME_BUFFER_SIZE] = {0};
			s_strcpy(name, "bsr", COMMAND_NAME_BUFFER_SIZE);
			bytes_written = fwrite(name, 1, COMMAND_NAME_BUFFER_SIZE, pFile);
			if (bytes_written != COMMAND_NAME_BUFFER_SIZE)
			{
                rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
			}
			else
			{
				unsigned char *p_buffer = (unsigned char *) (&output_payload);
				bytes_written = fwrite(p_buffer, 1, sizeof(output_payload), pFile);
				if (bytes_written != sizeof(output_payload))
				{
                	rc = FWCMD_DUMP_RESULT_ERR_FILE_WRITE;
				}
			}
			fclose(pFile);
		}
	}
	else
	{
		rc = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return rc;
}

struct fwcmd_bsr_result fwcmd_read_bsr(const char *filename)
{
	struct fwcmd_bsr_result result = {0,};
	FILE *pFile = fopen(filename, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		long fsize = ftell(pFile);
		rewind(pFile);
        unsigned char *buffer = malloc(fsize);
        if (NULL == buffer)
        {
            printf("Internal Error\n");
            return result;
        }

		size_t bytes_read = fread(buffer, 1, fsize, pFile);
		fclose(pFile);

		if (bytes_read == fsize)
		{
			result.p_data = (struct fwcmd_bsr_data *)malloc(sizeof(*result.p_data));
			int parse_result = fis_parse_bsr((const struct pt_output_bsr*) buffer, result.p_data);
			if (FWCMD_PARSE_SUCCESS(parse_result))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = parse_result;
			}
		}
		else
		{
			result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
			result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_READ;
		}

        free(buffer);
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_DUMP;
		result.error_code.code = FWCMD_DUMP_RESULT_ERR_FILE_OPEN;
	}

	return result;
}
