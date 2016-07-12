/*
 * Copyright (c) 2016, Intel Corporation
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

#include "cleanup_support_events.h"
#include <persistence/logging.h>
#include <string/s_str.h>
#include <uid/uid.h>
#include "device_utilities.h"

int get_db_events_alloc(PersistentStore *p_ps, struct db_event **pp_events)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	int event_count = 0;
	if (db_get_event_count(p_ps, &event_count) == DB_SUCCESS)
	{
		*pp_events = calloc(event_count, sizeof (struct db_event));
		if (*pp_events == NULL)
		{
			COMMON_LOG_ERROR("Could not allocate memory for event structures");
			rc = NVM_ERR_NOMEMORY;
		}
		else if (db_get_events(p_ps, *pp_events, event_count) == event_count)
		{
			rc = event_count;
		}
		else
		{
			COMMON_LOG_ERROR("Failed to retrieve events");
			rc = NVM_ERR_UNKNOWN;
			free(*pp_events);
			*pp_events = NULL;
		}
	}
	else
	{
		COMMON_LOG_ERROR("Failed to retrieve event count");
		rc = NVM_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int get_db_events_of_type_alloc(PersistentStore *p_ps,
		const enum event_type event_type, struct db_event **pp_events)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	int event_count = 0;
	if (db_get_event_count_by_event_type_type(p_ps, event_type, &event_count)
			== DB_SUCCESS)
	{
		*pp_events = calloc(event_count, sizeof (struct db_event));
		if (*pp_events == NULL)
		{
			COMMON_LOG_ERROR("Could not allocate memory for event structures");
			rc = NVM_ERR_NOMEMORY;
		}
		else if (db_get_events_by_event_type_type(p_ps, event_type, *pp_events, event_count)
				== DB_SUCCESS)
		{
			rc = event_count;
		}
		else
		{
			COMMON_LOG_ERROR("Failed to retrieve events by type");
			rc = NVM_ERR_UNKNOWN;
			free(*pp_events);
			*pp_events = NULL;
		}
	}
	else
	{
		COMMON_LOG_ERROR("Failed to retrieve event count by type");
		rc = NVM_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void fill_event_arg_with_cleared_serial_number(NVM_EVENT_ARG arg)
{
	s_strcpy(arg, "MISSING", NVM_EVENT_ARG_LEN);
}

void clear_serial_num_in_db_event(struct db_event *p_event,
		const struct event_field_metadata *p_metadata)
{
	COMMON_LOG_ENTRY();
	enum event_arg_type serial_type = EVENT_ARG_TYPE_SERIAL_NUM;

	if (EVENT_METADATA_HAS_TYPE_IN_ARG((*p_metadata), serial_type, 1))
	{
		fill_event_arg_with_cleared_serial_number(p_event->arg1);
	}

	if (EVENT_METADATA_HAS_TYPE_IN_ARG((*p_metadata), serial_type, 2))
	{
		fill_event_arg_with_cleared_serial_number(p_event->arg2);
	}

	if (EVENT_METADATA_HAS_TYPE_IN_ARG((*p_metadata), serial_type, 3))
	{
		fill_event_arg_with_cleared_serial_number(p_event->arg3);
	}

	COMMON_LOG_EXIT();
}

int change_serial_num_in_events_in_db(PersistentStore* p_ps, enum event_type event_type,
		unsigned int event_code)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct event_field_metadata metadata =
			get_event_field_metadata(event_type, event_code);
	if (event_field_metadata_includes_arg_type(&metadata, EVENT_ARG_TYPE_SERIAL_NUM))
	{
		struct db_event *p_events = NULL;
		rc = get_db_events_of_type_alloc(p_ps, event_type, &p_events);
		if (rc > 0)
		{
			int count = rc;
			rc = NVM_SUCCESS;
			for (int i = 0; i < count; i++)
			{
				if (p_events[i].code == event_code)
				{
					clear_serial_num_in_db_event(&(p_events[i]), &metadata);
					if (db_update_event_by_id(p_ps, p_events[i].id, &(p_events[i])) != DB_SUCCESS)
					{
						COMMON_LOG_ERROR("failed to update event");
						rc = NVM_ERR_DEVICEERROR;
						break;
					}
				}
			}
		}
		free(p_events);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int change_serial_num_in_events(PersistentStore *p_ps,
		enum event_type event_type, unsigned int event_code)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_ps != NULL)
	{
		rc = change_serial_num_in_events_in_db(p_ps, event_type, event_code);
	}
	else
	{
		COMMON_LOG_ERROR("Database is invalid");
		rc = NVM_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void generate_obfuscated_uid_for_device(const struct device_discovery *p_device,
		NVM_UID obfuscated_uid)
{
	COMMON_LOG_ENTRY();

	struct device_discovery device_copy;
	memmove(&device_copy, p_device, sizeof (struct device_discovery));

	// Regenerate UID with 4-byte handle in place of serial number
	memmove(device_copy.serial_number, &(device_copy.device_handle),
			NVM_SERIAL_LEN);
	calculate_device_uid(&device_copy);
	uid_copy(device_copy.uid, obfuscated_uid);

	COMMON_LOG_EXIT();
}

void generate_obfuscated_uid_for_missing_device(NVM_UID obfuscated_uid)
{
	COMMON_LOG_ENTRY();

	uid_copy("MISSING", obfuscated_uid);

	COMMON_LOG_EXIT();
}

int generate_obfuscated_dimm_uid(const NVM_UID real_uid,
		NVM_UID obfuscated_uid)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (real_uid == NULL || obfuscated_uid == NULL)
	{
		COMMON_LOG_ERROR("UID was NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		struct device_discovery device;
		rc = nvm_get_device_discovery(real_uid, &device);
		if (rc == NVM_SUCCESS)
		{
			generate_obfuscated_uid_for_device(&device,
					obfuscated_uid);
		}
		else
		{
			generate_obfuscated_uid_for_missing_device(obfuscated_uid);
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

void append_obfuscated_uid_to_string(const char *real_uid,
		char *str_buf, const size_t str_buf_size)
{
	COMMON_LOG_ENTRY();

	NVM_UID original_uid;
	uid_copy(real_uid, original_uid);
	NVM_UID obfuscated_uid;
	generate_obfuscated_dimm_uid(original_uid, obfuscated_uid);
	s_strcat(str_buf, str_buf_size, obfuscated_uid);

	COMMON_LOG_EXIT();
}

void build_obfuscated_dimm_uid_list_from_real_uid_list(
		const char *original_list, const size_t original_list_size,
		char *obfuscated_list, const size_t obfuscated_list_size)
{
	COMMON_LOG_ENTRY();

	memset(obfuscated_list, 0, obfuscated_list_size);

	// Mutable list for parsing
	char original_list_copy[original_list_size];
	s_strcpy(original_list_copy, original_list, original_list_size);

	const char *delimiter = ", ";
	char *p_ch = strtok(original_list_copy, delimiter);
	int count = 0;
	while (p_ch != NULL)
	{
		if (count > 0)
		{
			s_strcat(obfuscated_list, obfuscated_list_size, delimiter);
		}

		append_obfuscated_uid_to_string(p_ch,
				obfuscated_list, obfuscated_list_size);

		p_ch = strtok(NULL, delimiter);
		count++;
	}

	COMMON_LOG_EXIT();
}

void replace_dimm_uids_with_obfuscated_uids(char *uid_buffer, size_t buffer_size)
{
	COMMON_LOG_ENTRY();

	NVM_EVENT_ARG original_uid_arg;
	s_strcpy(original_uid_arg, uid_buffer, sizeof (original_uid_arg));

	build_obfuscated_dimm_uid_list_from_real_uid_list(
			original_uid_arg, sizeof (original_uid_arg), uid_buffer, buffer_size);

	COMMON_LOG_EXIT();
}

NVM_BOOL event_uid_should_be_obfuscated(enum event_uid_type uid_type)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = (uid_type == EVENT_UID_TYPE_DEVICE);

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

NVM_BOOL event_arg_should_be_obfuscated(enum event_arg_type arg_type)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = (arg_type == EVENT_ARG_TYPE_DEVICE_UID) ||
			(arg_type == EVENT_ARG_TYPE_DEVICE_UID_LIST);

	COMMON_LOG_EXIT_RETURN_I(result);
	return result;
}

void obfuscate_dimm_uids_in_event_arg(enum event_arg_type arg_type,
		char *arg, const size_t arg_len)
{
	if (event_arg_should_be_obfuscated(arg_type))
	{
		replace_dimm_uids_with_obfuscated_uids(arg, arg_len);
	}
}

void obfuscate_dimm_uid_in_event_uid(enum event_uid_type uid_type,
		char *uid, const size_t uid_len)
{
	if (event_uid_should_be_obfuscated(uid_type))
	{
		replace_dimm_uids_with_obfuscated_uids(uid, uid_len);
	}
}

void obfuscate_dimm_uid_in_db_event(struct db_event *p_event,
		const struct event_field_metadata *p_metadata)
{
	COMMON_LOG_ENTRY();

	obfuscate_dimm_uid_in_event_uid(p_metadata->uid_type, p_event->uid, EVENT_UID_LEN);

	obfuscate_dimm_uids_in_event_arg(p_metadata->arg1_type, p_event->arg1, EVENT_ARG1_LEN);
	obfuscate_dimm_uids_in_event_arg(p_metadata->arg2_type, p_event->arg2, EVENT_ARG2_LEN);
	obfuscate_dimm_uids_in_event_arg(p_metadata->arg3_type, p_event->arg3, EVENT_ARG3_LEN);

	COMMON_LOG_EXIT();
}

NVM_BOOL event_contains_device_uid(struct event_field_metadata *p_metadata)
{
	return p_metadata->uid_type == EVENT_UID_TYPE_DEVICE ||
			event_field_metadata_includes_arg_type(p_metadata, EVENT_ARG_TYPE_DEVICE_UID);
}

int change_dimm_uid_in_db_event(PersistentStore *p_ps, struct db_event *p_event)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	struct event_field_metadata metadata = get_event_field_metadata(
			p_event->type, p_event->code);
	if (event_contains_device_uid(&metadata))
	{
		obfuscate_dimm_uid_in_db_event(p_event, &metadata);
		if (db_update_event_by_id(p_ps, p_event->id, p_event)
					!= DB_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Couldn't update event ID %u in DB",
					p_event->id);
			rc = NVM_ERR_UNKNOWN;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int change_dimm_uid_in_db_events(PersistentStore *p_ps,
		struct db_event *p_events, const int event_count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	for (int i = 0; i < event_count; i++)
	{
		rc = change_dimm_uid_in_db_event(p_ps, &(p_events[i]));
		if (rc != NVM_SUCCESS)
		{
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int change_dimm_uid_in_events(PersistentStore *p_ps)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_ps == NULL)
	{
		COMMON_LOG_ERROR("Database is invalid");
		rc = NVM_ERR_UNKNOWN;
	}
	else
	{
		struct db_event *p_events = NULL;
		rc = get_db_events_alloc(p_ps, &p_events);
		if (rc > 0)
		{
			int event_count = rc;
			rc = change_dimm_uid_in_db_events(p_ps,
					p_events, event_count);
		}
		free(p_events);
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
