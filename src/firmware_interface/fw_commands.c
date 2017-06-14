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
#include "fw_commands.h"

#include <common/string/s_str.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* BEGIN identify_dimm */
struct fwcmd_identify_dimm_result fwcmd_alloc_identify_dimm(unsigned int handle)
{
	struct fwcmd_identify_dimm_result result;
	memset(&result, 0, sizeof (struct fwcmd_identify_dimm_result));

	struct pt_output_identify_dimm output_payload;
	unsigned int rc = fis_identify_dimm(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_identify_dimm_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_identify_dimm(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_identify_dimm_data(struct fwcmd_identify_dimm_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_identify_dimm(struct fwcmd_identify_dimm_result *p_result)
{
	fwcmd_free_identify_dimm_data(p_result->p_data);
	free(p_result->p_data);
}
/* END identify_dimm */

/* BEGIN identify_dimm_characteristics */
struct fwcmd_identify_dimm_characteristics_result fwcmd_alloc_identify_dimm_characteristics(unsigned int handle)
{
	struct fwcmd_identify_dimm_characteristics_result result;
	memset(&result, 0, sizeof (struct fwcmd_identify_dimm_characteristics_result));

	struct pt_output_identify_dimm_characteristics output_payload;
	unsigned int rc = fis_identify_dimm_characteristics(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_identify_dimm_characteristics_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_identify_dimm_characteristics(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_identify_dimm_characteristics_data(struct fwcmd_identify_dimm_characteristics_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_identify_dimm_characteristics(struct fwcmd_identify_dimm_characteristics_result *p_result)
{
	fwcmd_free_identify_dimm_characteristics_data(p_result->p_data);
	free(p_result->p_data);
}
/* END identify_dimm_characteristics */

/* BEGIN get_security_state */
struct fwcmd_get_security_state_result fwcmd_alloc_get_security_state(unsigned int handle)
{
	struct fwcmd_get_security_state_result result;
	memset(&result, 0, sizeof (struct fwcmd_get_security_state_result));

	struct pt_output_get_security_state output_payload;
	unsigned int rc = fis_get_security_state(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_get_security_state_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_get_security_state(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_get_security_state_data(struct fwcmd_get_security_state_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_get_security_state(struct fwcmd_get_security_state_result *p_result)
{
	fwcmd_free_get_security_state_data(p_result->p_data);
	free(p_result->p_data);
}
/* END get_security_state */

/* BEGIN set_passphrase */
struct fwcmd_set_passphrase_result fwcmd_call_set_passphrase(unsigned int handle,
	const char current_passphrase[33],
	const char new_passphrase[33])
{
	struct fwcmd_set_passphrase_result result;
	memset(&result, 0, sizeof (struct fwcmd_set_passphrase_result));

	struct pt_input_set_passphrase input_payload;
	memmove(input_payload.current_passphrase, current_passphrase, 32);
	memmove(input_payload.new_passphrase, new_passphrase, 32);
	unsigned int rc = fis_set_passphrase(handle,
		&input_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.success = 1;
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
/* END set_passphrase */

/* BEGIN disable_passphrase */
struct fwcmd_disable_passphrase_result fwcmd_call_disable_passphrase(unsigned int handle,
	const char current_passphrase[33])
{
	struct fwcmd_disable_passphrase_result result;
	memset(&result, 0, sizeof (struct fwcmd_disable_passphrase_result));

	struct pt_input_disable_passphrase input_payload;
	memmove(input_payload.current_passphrase, current_passphrase, 32);
	unsigned int rc = fis_disable_passphrase(handle,
		&input_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.success = 1;
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
/* END disable_passphrase */

/* BEGIN unlock_unit */
struct fwcmd_unlock_unit_result fwcmd_call_unlock_unit(unsigned int handle,
	const char current_passphrase[33])
{
	struct fwcmd_unlock_unit_result result;
	memset(&result, 0, sizeof (struct fwcmd_unlock_unit_result));

	struct pt_input_unlock_unit input_payload;
	memmove(input_payload.current_passphrase, current_passphrase, 32);
	unsigned int rc = fis_unlock_unit(handle,
		&input_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.success = 1;
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
/* END unlock_unit */

/* BEGIN secure_erase */
struct fwcmd_secure_erase_result fwcmd_call_secure_erase(unsigned int handle,
	const char current_passphrase[33])
{
	struct fwcmd_secure_erase_result result;
	memset(&result, 0, sizeof (struct fwcmd_secure_erase_result));

	struct pt_input_secure_erase input_payload;
	memmove(input_payload.current_passphrase, current_passphrase, 32);
	unsigned int rc = fis_secure_erase(handle,
		&input_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.success = 1;
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
/* END secure_erase */

/* BEGIN freeze_lock */
struct fwcmd_freeze_lock_result fwcmd_call_freeze_lock(unsigned int handle)
{
	struct fwcmd_freeze_lock_result result;
	memset(&result, 0, sizeof (struct fwcmd_freeze_lock_result));

	unsigned int rc = fis_freeze_lock(handle);

	if (PT_IS_SUCCESS(rc))
	{
		result.success = 1;
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
/* END freeze_lock */

/* BEGIN get_alarm_threshold */
struct fwcmd_get_alarm_threshold_result fwcmd_alloc_get_alarm_threshold(unsigned int handle)
{
	struct fwcmd_get_alarm_threshold_result result;
	memset(&result, 0, sizeof (struct fwcmd_get_alarm_threshold_result));

	struct pt_output_get_alarm_threshold output_payload;
	unsigned int rc = fis_get_alarm_threshold(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_get_alarm_threshold_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_get_alarm_threshold(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_get_alarm_threshold_data(struct fwcmd_get_alarm_threshold_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_get_alarm_threshold(struct fwcmd_get_alarm_threshold_result *p_result)
{
	fwcmd_free_get_alarm_threshold_data(p_result->p_data);
	free(p_result->p_data);
}
/* END get_alarm_threshold */

/* BEGIN power_management_policy */
struct fwcmd_power_management_policy_result fwcmd_alloc_power_management_policy(unsigned int handle)
{
	struct fwcmd_power_management_policy_result result;
	memset(&result, 0, sizeof (struct fwcmd_power_management_policy_result));

	struct pt_output_power_management_policy output_payload;
	unsigned int rc = fis_power_management_policy(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_power_management_policy_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_power_management_policy(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_power_management_policy_data(struct fwcmd_power_management_policy_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_power_management_policy(struct fwcmd_power_management_policy_result *p_result)
{
	fwcmd_free_power_management_policy_data(p_result->p_data);
	free(p_result->p_data);
}
/* END power_management_policy */

/* BEGIN die_sparing_policy */
struct fwcmd_die_sparing_policy_result fwcmd_alloc_die_sparing_policy(unsigned int handle)
{
	struct fwcmd_die_sparing_policy_result result;
	memset(&result, 0, sizeof (struct fwcmd_die_sparing_policy_result));

	struct pt_output_die_sparing_policy output_payload;
	unsigned int rc = fis_die_sparing_policy(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_die_sparing_policy_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_die_sparing_policy(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_die_sparing_policy_data(struct fwcmd_die_sparing_policy_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_die_sparing_policy(struct fwcmd_die_sparing_policy_result *p_result)
{
	fwcmd_free_die_sparing_policy_data(p_result->p_data);
	free(p_result->p_data);
}
/* END die_sparing_policy */

/* BEGIN address_range_scrub */
struct fwcmd_address_range_scrub_result fwcmd_alloc_address_range_scrub(unsigned int handle)
{
	struct fwcmd_address_range_scrub_result result;
	memset(&result, 0, sizeof (struct fwcmd_address_range_scrub_result));

	struct pt_output_address_range_scrub output_payload;
	unsigned int rc = fis_address_range_scrub(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_address_range_scrub_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_address_range_scrub(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_address_range_scrub_data(struct fwcmd_address_range_scrub_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_address_range_scrub(struct fwcmd_address_range_scrub_result *p_result)
{
	fwcmd_free_address_range_scrub_data(p_result->p_data);
	free(p_result->p_data);
}
/* END address_range_scrub */

/* BEGIN optional_configuration_data_policy */
struct fwcmd_optional_configuration_data_policy_result fwcmd_alloc_optional_configuration_data_policy(unsigned int handle)
{
	struct fwcmd_optional_configuration_data_policy_result result;
	memset(&result, 0, sizeof (struct fwcmd_optional_configuration_data_policy_result));

	struct pt_output_optional_configuration_data_policy output_payload;
	unsigned int rc = fis_optional_configuration_data_policy(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_optional_configuration_data_policy_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_optional_configuration_data_policy(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_optional_configuration_data_policy_data(struct fwcmd_optional_configuration_data_policy_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_optional_configuration_data_policy(struct fwcmd_optional_configuration_data_policy_result *p_result)
{
	fwcmd_free_optional_configuration_data_policy_data(p_result->p_data);
	free(p_result->p_data);
}
/* END optional_configuration_data_policy */

/* BEGIN pmon_registers */
struct fwcmd_pmon_registers_result fwcmd_alloc_pmon_registers(unsigned int handle,
	const unsigned short pmon_retreive_mask)
{
	struct fwcmd_pmon_registers_result result;
	memset(&result, 0, sizeof (struct fwcmd_pmon_registers_result));

	struct pt_input_pmon_registers input_payload;
	input_payload.pmon_retreive_mask = pmon_retreive_mask;
	struct pt_output_pmon_registers output_payload;
	unsigned int rc = fis_pmon_registers(handle,
		&input_payload,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_pmon_registers_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_pmon_registers(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_pmon_registers_data(struct fwcmd_pmon_registers_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_pmon_registers(struct fwcmd_pmon_registers_result *p_result)
{
	fwcmd_free_pmon_registers_data(p_result->p_data);
	free(p_result->p_data);
}
/* END pmon_registers */

/* BEGIN set_alarm_threshold */
struct fwcmd_set_alarm_threshold_result fwcmd_call_set_alarm_threshold(unsigned int handle,
	const unsigned char enable,
	const unsigned short peak_power_budget,
	const unsigned short avg_power_budget)
{
	struct fwcmd_set_alarm_threshold_result result;
	memset(&result, 0, sizeof (struct fwcmd_set_alarm_threshold_result));

	struct pt_input_set_alarm_threshold input_payload;
	input_payload.enable = enable;
	input_payload.peak_power_budget = peak_power_budget;
	input_payload.avg_power_budget = avg_power_budget;
	unsigned int rc = fis_set_alarm_threshold(handle,
		&input_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.success = 1;
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
/* END set_alarm_threshold */

/* BEGIN system_time */
struct fwcmd_system_time_result fwcmd_alloc_system_time(unsigned int handle)
{
	struct fwcmd_system_time_result result;
	memset(&result, 0, sizeof (struct fwcmd_system_time_result));

	struct pt_output_system_time output_payload;
	unsigned int rc = fis_system_time(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_system_time_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_system_time(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_system_time_data(struct fwcmd_system_time_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_system_time(struct fwcmd_system_time_result *p_result)
{
	fwcmd_free_system_time_data(p_result->p_data);
	free(p_result->p_data);
}
/* END system_time */

/* BEGIN platform_config_data_configuration_header_table */
struct fwcmd_platform_config_data_configuration_header_table_result fwcmd_alloc_platform_config_data_configuration_header_table(unsigned int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset)
{
	struct fwcmd_platform_config_data_configuration_header_table_result result;
	memset(&result, 0, sizeof (struct fwcmd_platform_config_data_configuration_header_table_result));

	struct pt_input_platform_config_data_configuration_header_table input_payload;
	input_payload.partition_id = partition_id;
	input_payload.command_option = command_option;
	input_payload.offset = offset;
	struct pt_output_platform_config_data_configuration_header_table output_payload;
	unsigned int rc = fis_platform_config_data_configuration_header_table(handle,
		&input_payload,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_platform_config_data_configuration_header_table_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_platform_config_data_configuration_header_table(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_platform_config_data_identification_information_table_data(struct fwcmd_platform_config_data_identification_information_table_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_platform_config_data_interleave_information_table_data(struct fwcmd_platform_config_data_interleave_information_table_data *p_data)
{
	if (p_data)
	{
		for (int i = 0; i < p_data->platform_config_data_identification_information_table_count; i++)
		{
			fwcmd_free_platform_config_data_identification_information_table_data(&p_data->platform_config_data_identification_information_table[i]);
		}
		free(p_data->platform_config_data_identification_information_table);

	}
}

void fwcmd_free_platform_config_data_partition_size_change_table_data(struct fwcmd_platform_config_data_partition_size_change_table_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_platform_config_data_current_config_table_data(struct fwcmd_platform_config_data_current_config_table_data *p_data)
{
	if (p_data)
	{
		for (int i = 0; i < p_data->platform_config_data_interleave_information_table_count; i++)
		{
			fwcmd_free_platform_config_data_interleave_information_table_data(&p_data->platform_config_data_interleave_information_table[i]);
		}
		free(p_data->platform_config_data_interleave_information_table);

	}
}

void fwcmd_free_platform_config_data_config_input_table_data(struct fwcmd_platform_config_data_config_input_table_data *p_data)
{
	if (p_data)
	{
		for (int i = 0; i < p_data->platform_config_data_interleave_information_table_count; i++)
		{
			fwcmd_free_platform_config_data_interleave_information_table_data(&p_data->platform_config_data_interleave_information_table[i]);
		}
		free(p_data->platform_config_data_interleave_information_table);

		for (int i = 0; i < p_data->platform_config_data_partition_size_change_table_count; i++)
		{
			fwcmd_free_platform_config_data_partition_size_change_table_data(&p_data->platform_config_data_partition_size_change_table[i]);
		}
		free(p_data->platform_config_data_partition_size_change_table);

	}
}

void fwcmd_free_platform_config_data_config_output_table_data(struct fwcmd_platform_config_data_config_output_table_data *p_data)
{
	if (p_data)
	{
		for (int i = 0; i < p_data->platform_config_data_interleave_information_table_count; i++)
		{
			fwcmd_free_platform_config_data_interleave_information_table_data(&p_data->platform_config_data_interleave_information_table[i]);
		}
		free(p_data->platform_config_data_interleave_information_table);

		for (int i = 0; i < p_data->platform_config_data_partition_size_change_table_count; i++)
		{
			fwcmd_free_platform_config_data_partition_size_change_table_data(&p_data->platform_config_data_partition_size_change_table[i]);
		}
		free(p_data->platform_config_data_partition_size_change_table);

	}
}

void fwcmd_free_platform_config_data_configuration_header_table_data(struct fwcmd_platform_config_data_configuration_header_table_data *p_data)
{
	if (p_data)
	{
		fwcmd_free_platform_config_data_current_config_table_data(&p_data->platform_config_data_current_config_table);
		fwcmd_free_platform_config_data_config_input_table_data(&p_data->platform_config_data_config_input_table);
		fwcmd_free_platform_config_data_config_output_table_data(&p_data->platform_config_data_config_output_table);
	}
}

void fwcmd_free_platform_config_data_configuration_header_table(struct fwcmd_platform_config_data_configuration_header_table_result *p_result)
{
	fwcmd_free_platform_config_data_configuration_header_table_data(p_result->p_data);
	free(p_result->p_data);
}
/* END platform_config_data_configuration_header_table */

/* BEGIN namespace_labels */
struct fwcmd_namespace_labels_result fwcmd_alloc_namespace_labels(unsigned int handle,
	const unsigned char partition_id,
	const unsigned char command_option,
	const unsigned int offset)
{
	struct fwcmd_namespace_labels_result result;
	memset(&result, 0, sizeof (struct fwcmd_namespace_labels_result));

	struct pt_input_namespace_labels input_payload;
	input_payload.partition_id = partition_id;
	input_payload.command_option = command_option;
	input_payload.offset = offset;
	struct pt_output_namespace_labels output_payload;
	unsigned int rc = fis_namespace_labels(handle,
		&input_payload,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_namespace_labels_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_namespace_labels(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_ns_index_data(struct fwcmd_ns_index_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_ns_label_data(struct fwcmd_ns_label_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_namespace_labels_data(struct fwcmd_namespace_labels_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_namespace_labels(struct fwcmd_namespace_labels_result *p_result)
{
	fwcmd_free_namespace_labels_data(p_result->p_data);
	free(p_result->p_data);
}
/* END namespace_labels */

/* BEGIN dimm_partition_info */
struct fwcmd_dimm_partition_info_result fwcmd_alloc_dimm_partition_info(unsigned int handle)
{
	struct fwcmd_dimm_partition_info_result result;
	memset(&result, 0, sizeof (struct fwcmd_dimm_partition_info_result));

	struct pt_output_dimm_partition_info output_payload;
	unsigned int rc = fis_dimm_partition_info(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_dimm_partition_info_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_dimm_partition_info(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_dimm_partition_info_data(struct fwcmd_dimm_partition_info_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_dimm_partition_info(struct fwcmd_dimm_partition_info_result *p_result)
{
	fwcmd_free_dimm_partition_info_data(p_result->p_data);
	free(p_result->p_data);
}
/* END dimm_partition_info */

/* BEGIN fw_debug_log_level */
struct fwcmd_fw_debug_log_level_result fwcmd_alloc_fw_debug_log_level(unsigned int handle,
	const unsigned char log_id)
{
	struct fwcmd_fw_debug_log_level_result result;
	memset(&result, 0, sizeof (struct fwcmd_fw_debug_log_level_result));

	struct pt_input_fw_debug_log_level input_payload;
	input_payload.log_id = log_id;
	struct pt_output_fw_debug_log_level output_payload;
	unsigned int rc = fis_fw_debug_log_level(handle,
		&input_payload,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_fw_debug_log_level_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_fw_debug_log_level(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_fw_debug_log_level_data(struct fwcmd_fw_debug_log_level_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_fw_debug_log_level(struct fwcmd_fw_debug_log_level_result *p_result)
{
	fwcmd_free_fw_debug_log_level_data(p_result->p_data);
	free(p_result->p_data);
}
/* END fw_debug_log_level */

/* BEGIN fw_load_flag */
struct fwcmd_fw_load_flag_result fwcmd_alloc_fw_load_flag(unsigned int handle)
{
	struct fwcmd_fw_load_flag_result result;
	memset(&result, 0, sizeof (struct fwcmd_fw_load_flag_result));

	struct pt_output_fw_load_flag output_payload;
	unsigned int rc = fis_fw_load_flag(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_fw_load_flag_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_fw_load_flag(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_fw_load_flag_data(struct fwcmd_fw_load_flag_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_fw_load_flag(struct fwcmd_fw_load_flag_result *p_result)
{
	fwcmd_free_fw_load_flag_data(p_result->p_data);
	free(p_result->p_data);
}
/* END fw_load_flag */

/* BEGIN config_lockdown */
struct fwcmd_config_lockdown_result fwcmd_alloc_config_lockdown(unsigned int handle)
{
	struct fwcmd_config_lockdown_result result;
	memset(&result, 0, sizeof (struct fwcmd_config_lockdown_result));

	struct pt_output_config_lockdown output_payload;
	unsigned int rc = fis_config_lockdown(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_config_lockdown_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_config_lockdown(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_config_lockdown_data(struct fwcmd_config_lockdown_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_config_lockdown(struct fwcmd_config_lockdown_result *p_result)
{
	fwcmd_free_config_lockdown_data(p_result->p_data);
	free(p_result->p_data);
}
/* END config_lockdown */

/* BEGIN ddrt_io_init_info */
struct fwcmd_ddrt_io_init_info_result fwcmd_alloc_ddrt_io_init_info(unsigned int handle)
{
	struct fwcmd_ddrt_io_init_info_result result;
	memset(&result, 0, sizeof (struct fwcmd_ddrt_io_init_info_result));

	struct pt_output_ddrt_io_init_info output_payload;
	unsigned int rc = fis_ddrt_io_init_info(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_ddrt_io_init_info_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_ddrt_io_init_info(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_ddrt_io_init_info_data(struct fwcmd_ddrt_io_init_info_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_ddrt_io_init_info(struct fwcmd_ddrt_io_init_info_result *p_result)
{
	fwcmd_free_ddrt_io_init_info_data(p_result->p_data);
	free(p_result->p_data);
}
/* END ddrt_io_init_info */

/* BEGIN get_supported_sku_features */
struct fwcmd_get_supported_sku_features_result fwcmd_alloc_get_supported_sku_features(unsigned int handle)
{
	struct fwcmd_get_supported_sku_features_result result;
	memset(&result, 0, sizeof (struct fwcmd_get_supported_sku_features_result));

	struct pt_output_get_supported_sku_features output_payload;
	unsigned int rc = fis_get_supported_sku_features(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_get_supported_sku_features_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_get_supported_sku_features(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_get_supported_sku_features_data(struct fwcmd_get_supported_sku_features_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_get_supported_sku_features(struct fwcmd_get_supported_sku_features_result *p_result)
{
	fwcmd_free_get_supported_sku_features_data(p_result->p_data);
	free(p_result->p_data);
}
/* END get_supported_sku_features */

/* BEGIN enable_dimm */
struct fwcmd_enable_dimm_result fwcmd_alloc_enable_dimm(unsigned int handle)
{
	struct fwcmd_enable_dimm_result result;
	memset(&result, 0, sizeof (struct fwcmd_enable_dimm_result));

	struct pt_output_enable_dimm output_payload;
	unsigned int rc = fis_enable_dimm(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_enable_dimm_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_enable_dimm(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_enable_dimm_data(struct fwcmd_enable_dimm_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_enable_dimm(struct fwcmd_enable_dimm_result *p_result)
{
	fwcmd_free_enable_dimm_data(p_result->p_data);
	free(p_result->p_data);
}
/* END enable_dimm */

/* BEGIN smart_health_info */
struct fwcmd_smart_health_info_result fwcmd_alloc_smart_health_info(unsigned int handle)
{
	struct fwcmd_smart_health_info_result result;
	memset(&result, 0, sizeof (struct fwcmd_smart_health_info_result));

	struct pt_output_smart_health_info output_payload;
	unsigned int rc = fis_smart_health_info(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_smart_health_info_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_smart_health_info(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_smart_health_info_data(struct fwcmd_smart_health_info_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_smart_health_info(struct fwcmd_smart_health_info_result *p_result)
{
	fwcmd_free_smart_health_info_data(p_result->p_data);
	free(p_result->p_data);
}
/* END smart_health_info */

/* BEGIN firmware_image_info */
struct fwcmd_firmware_image_info_result fwcmd_alloc_firmware_image_info(unsigned int handle)
{
	struct fwcmd_firmware_image_info_result result;
	memset(&result, 0, sizeof (struct fwcmd_firmware_image_info_result));

	struct pt_output_firmware_image_info output_payload;
	unsigned int rc = fis_firmware_image_info(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_firmware_image_info_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_firmware_image_info(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_firmware_image_info_data(struct fwcmd_firmware_image_info_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_firmware_image_info(struct fwcmd_firmware_image_info_result *p_result)
{
	fwcmd_free_firmware_image_info_data(p_result->p_data);
	free(p_result->p_data);
}
/* END firmware_image_info */

/* BEGIN firmware_debug_log */
struct fwcmd_firmware_debug_log_result fwcmd_alloc_firmware_debug_log(unsigned int handle,
	const unsigned char log_action,
	const unsigned int log_page_offset,
	const unsigned char log_id)
{
	struct fwcmd_firmware_debug_log_result result;
	memset(&result, 0, sizeof (struct fwcmd_firmware_debug_log_result));

	struct pt_input_firmware_debug_log input_payload;
	input_payload.log_action = log_action;
	input_payload.log_page_offset = log_page_offset;
	input_payload.log_id = log_id;
	struct pt_output_firmware_debug_log output_payload;
	unsigned int rc = fis_firmware_debug_log(handle,
		&input_payload,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_firmware_debug_log_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_firmware_debug_log(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_firmware_debug_log_data(struct fwcmd_firmware_debug_log_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_firmware_debug_log(struct fwcmd_firmware_debug_log_result *p_result)
{
	fwcmd_free_firmware_debug_log_data(p_result->p_data);
	free(p_result->p_data);
}
/* END firmware_debug_log */

/* BEGIN long_operation_status */
struct fwcmd_long_operation_status_result fwcmd_alloc_long_operation_status(unsigned int handle)
{
	struct fwcmd_long_operation_status_result result;
	memset(&result, 0, sizeof (struct fwcmd_long_operation_status_result));

	struct pt_output_long_operation_status output_payload;
	unsigned int rc = fis_long_operation_status(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_long_operation_status_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_long_operation_status(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_long_operation_status_data(struct fwcmd_long_operation_status_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_long_operation_status(struct fwcmd_long_operation_status_result *p_result)
{
	fwcmd_free_long_operation_status_data(p_result->p_data);
	free(p_result->p_data);
}
/* END long_operation_status */

/* BEGIN bsr */
struct fwcmd_bsr_result fwcmd_alloc_bsr(unsigned int handle)
{
	struct fwcmd_bsr_result result;
	memset(&result, 0, sizeof (struct fwcmd_bsr_result));

	struct pt_output_bsr output_payload;
	unsigned int rc = fis_bsr(handle,
		&output_payload);

	if (PT_IS_SUCCESS(rc))
	{
		result.p_data = (struct fwcmd_bsr_data *)malloc(sizeof(*result.p_data));
		if (result.p_data)
		{
			rc = fis_parse_bsr(&output_payload, result.p_data);
			if (FWCMD_PARSE_SUCCESS(rc))
			{
				result.success = 1;
			}
			else
			{
				result.error_code.type = FWCMD_ERROR_TYPE_PARSE;
				result.error_code.code = rc;
			}
		}
		else
		{
			result.error_code.code = FWCMD_ERR_NOMEMORY;
		}
	}
	else
	{
		result.error_code.type = FWCMD_ERROR_TYPE_PT;
        result.error_code.code = rc;
	}
	return result;
}
void fwcmd_free_bsr_data(struct fwcmd_bsr_data *p_data)
{
	if (p_data)
	{
	}
}

void fwcmd_free_bsr(struct fwcmd_bsr_result *p_result)
{
	fwcmd_free_bsr_data(p_result->p_data);
	free(p_result->p_data);
}
/* END bsr */

/*
 * helper functions
 */
int fwcmd_is_command_name(const char * cmd_name)
{
	int exists = 0;

	if (s_strncmpi(cmd_name, "identify_dimm", sizeof ("identify_dimm")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "identify_dimm_characteristics", sizeof ("identify_dimm_characteristics")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "get_security_state", sizeof ("get_security_state")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "set_passphrase", sizeof ("set_passphrase")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "disable_passphrase", sizeof ("disable_passphrase")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "unlock_unit", sizeof ("unlock_unit")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "secure_erase", sizeof ("secure_erase")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "freeze_lock", sizeof ("freeze_lock")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "get_alarm_threshold", sizeof ("get_alarm_threshold")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "power_management_policy", sizeof ("power_management_policy")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "die_sparing_policy", sizeof ("die_sparing_policy")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "address_range_scrub", sizeof ("address_range_scrub")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "optional_configuration_data_policy", sizeof ("optional_configuration_data_policy")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "pmon_registers", sizeof ("pmon_registers")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "set_alarm_threshold", sizeof ("set_alarm_threshold")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "system_time", sizeof ("system_time")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "platform_config_data_configuration_header_table", sizeof ("platform_config_data_configuration_header_table")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "namespace_labels", sizeof ("namespace_labels")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "dimm_partition_info", sizeof ("dimm_partition_info")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "fw_debug_log_level", sizeof ("fw_debug_log_level")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "fw_load_flag", sizeof ("fw_load_flag")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "config_lockdown", sizeof ("config_lockdown")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "ddrt_io_init_info", sizeof ("ddrt_io_init_info")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "get_supported_sku_features", sizeof ("get_supported_sku_features")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "enable_dimm", sizeof ("enable_dimm")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "smart_health_info", sizeof ("smart_health_info")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "firmware_image_info", sizeof ("firmware_image_info")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "firmware_debug_log", sizeof ("firmware_debug_log")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "long_operation_status", sizeof ("long_operation_status")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "bsr", sizeof ("bsr")) == 0)
	{
		exists = 1;
	}
	return exists;
}

int fwcmd_is_output_command_name(const char * cmd_name)
{
	int exists = 0;

	if (s_strncmpi(cmd_name, "identify_dimm", sizeof ("identify_dimm")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "identify_dimm_characteristics", sizeof ("identify_dimm_characteristics")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "get_security_state", sizeof ("get_security_state")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "get_alarm_threshold", sizeof ("get_alarm_threshold")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "power_management_policy", sizeof ("power_management_policy")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "die_sparing_policy", sizeof ("die_sparing_policy")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "address_range_scrub", sizeof ("address_range_scrub")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "optional_configuration_data_policy", sizeof ("optional_configuration_data_policy")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "pmon_registers", sizeof ("pmon_registers")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "system_time", sizeof ("system_time")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "platform_config_data_configuration_header_table", sizeof ("platform_config_data_configuration_header_table")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "namespace_labels", sizeof ("namespace_labels")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "dimm_partition_info", sizeof ("dimm_partition_info")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "fw_debug_log_level", sizeof ("fw_debug_log_level")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "fw_load_flag", sizeof ("fw_load_flag")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "config_lockdown", sizeof ("config_lockdown")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "ddrt_io_init_info", sizeof ("ddrt_io_init_info")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "get_supported_sku_features", sizeof ("get_supported_sku_features")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "enable_dimm", sizeof ("enable_dimm")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "smart_health_info", sizeof ("smart_health_info")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "firmware_image_info", sizeof ("firmware_image_info")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "firmware_debug_log", sizeof ("firmware_debug_log")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "long_operation_status", sizeof ("long_operation_status")) == 0)
	{
		exists = 1;
	}
	if (s_strncmpi(cmd_name, "bsr", sizeof ("bsr")) == 0)
	{
		exists = 1;
	}
	return exists;
}
