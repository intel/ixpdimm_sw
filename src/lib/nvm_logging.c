/*
 * Copyright (c) 2015 2016, Intel Corporation
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

/*
 * This file contains the implementation of logging functions of the native API.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <common_types.h>

#include "nvm_management.h"
#include <persistence/schema.h>
#include <persistence/logging.h>
#include <persistence/lib_persistence.h>
#include <string/s_str.h>
#include "system.h"


/*
 * Determine if debug logging is enabled.
 */
int nvm_debug_logging_enabled()
{
	int rc = 0;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (get_current_log_level() > LOGGING_LEVEL_ERROR)
	{
		rc = 1;
	}
	return rc;
}

/*
 * Toggle log level on or off
 */
int nvm_toggle_debug_logging(const NVM_BOOL enabled)
{
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		int level = LOGGING_LEVEL_ERROR; // errors only
		if (enabled)
		{
			level = LOGGING_LEVEL_DEBUG; // all on
		}
		if (!set_current_log_level(level))
		{
			rc = NVM_ERR_UNKNOWN;
		}
	}

	return rc;
}

/*
 * Clear debug log entries from the database
 */
int nvm_purge_debug_log()
{
	int rc = NVM_SUCCESS;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		PersistentStore *p_store = get_lib_store();
		if (!p_store)
		{
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			// move all the logs to the database
			// ignore failures because the log only exists if it's not empty
			log_gather();

			// clear the table
			if (db_delete_all_logs(p_store) != DB_SUCCESS)
			{
				rc = NVM_ERR_UNKNOWN;
			}
		}
	}

	return rc;
}

/*
 * Retrieve the number of debug log entries in the database.
 */
int nvm_get_debug_log_count()
{
	int rc = 0;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		PersistentStore *p_store = get_lib_store();
		if (!p_store)
		{
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			// move all the logs to the database
			// ignore failures because the log only exists if it's not empty
			log_gather();

			if (db_get_log_count(p_store, &rc) != DB_SUCCESS)
			{
				rc = NVM_ERR_UNKNOWN;
				COMMON_LOG_ERROR("Failed to retrieve the log count from the database");
			}
		}
	}

	return rc;
}

/*
 * Retrieve a list of stored debug log entries from the database
 */
int nvm_get_debug_logs(struct log *p_logs, const NVM_UINT32 count)
{
	int rc = 0; // 0 logs

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (p_logs == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_logs is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else
	{
		PersistentStore *p_store = get_lib_store();
		if (!p_store)
		{
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			// move all the logs to the database
			// ignore failures because the log only exists if it's not empty
			log_gather();

			memset(p_logs, 0, (sizeof (struct log) * count));
			struct db_log db_logs[count];
			if ((rc = db_get_logs(p_store, db_logs, count)) != DB_ERR_FAILURE)
			{
				for (int i = 0; i < rc; i++)
				{
					p_logs[i].level = db_logs[i].level;
					p_logs[i].line_number = db_logs[i].line_number;
					s_strcpy(p_logs[i].file_name, db_logs[i].file_name, NVM_PATH_LEN);
					s_strcpy(p_logs[i].message, db_logs[i].message, NVM_LOG_MESSAGE_LEN);
					p_logs[i].time = db_logs[i].time;
				}
			}
			else
			{
				COMMON_LOG_ERROR("Failed to retrieve the logs from the database");
				rc = NVM_ERR_UNKNOWN;
			}
		}
	}

	return rc;
}
