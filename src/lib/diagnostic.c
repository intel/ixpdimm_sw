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
 * This file contains the implementation of the diagnostic entry point and common
 * diagnostic helper functions for the native API.
 */

#include "nvm_management.h"
#include "diagnostic.h"
#include <persistence/logging.h>
#include <persistence/lib_persistence.h>
#include <string/s_str.h>
#include <uid/uid.h>
#include "system.h"
#include "device_adapter.h"

/*
 * Run a diagnostic test on the device specified.
 */
int nvm_run_diagnostic(const NVM_UID device_uid,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	if (p_diagnostic == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_diagnostic is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (p_results == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_results is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else
	{
		switch (p_diagnostic->test)
		{
			case DIAG_TYPE_QUICK:
				rc = diag_quick_health_check(device_uid, p_diagnostic, p_results);
				break;
			case DIAG_TYPE_PLATFORM_CONFIG:
				rc = diag_platform_config_check(p_diagnostic, p_results);
				break;
			case DIAG_TYPE_PM_META:
				rc = diag_pm_metadata_check(p_results);
				break;
			case DIAG_TYPE_FW_CONSISTENCY:
				rc = diag_firmware_check(p_diagnostic, p_results);
				break;
			case DIAG_TYPE_SECURITY:
				rc = diag_security_check(p_diagnostic, p_results);
				break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Perform an integer value compare as part of a diagnostic test
 */
NVM_BOOL diag_check(const struct diagnostic *p_diagnostic,
		const NVM_UINT32 indicator, const NVM_UINT64 actual,
		NVM_UINT64 *p_expected, enum equality e)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	// is check excluded by caller
	if (p_diagnostic->excludes & indicator)
	{
		result = 1;
	}
	else
	{
		// is threshold specified by caller?
		for (int i = 0; i < p_diagnostic->overrides_len; i++)
		{
			if (p_diagnostic->p_overrides[i].type == indicator)
			{
				*p_expected  = p_diagnostic->p_overrides[i].threshold;
				break;
			}
		}

		// perform check
		switch (e)
		{
			case EQUALITY_LESSTHAN:
				result = (actual < (*p_expected));
				break;
			case EQUALITY_LESSTHANEQUAL:
				result = (actual <= (*p_expected));
				break;
			case EQUALITY_EQUAL:
				result = (actual == (*p_expected));
				break;
			case EQUALITY_GREATERTHANEQUAL:
				result = (actual >= (*p_expected));
				break;
			case EQUALITY_GREATHERTHAN:
				result = (actual > (*p_expected));
				break;
			case EQUALITY_NOTEQUAL:
				result = (actual != (*p_expected));
				break;
			default:
				break;
		}
	}

	return result;
}

NVM_BOOL diag_check_real(const struct diagnostic *p_diagnostic,
		const NVM_UINT32 indicator, const NVM_REAL32 actual,
		NVM_REAL32 *p_expected, enum equality e)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	// is check excluded by caller
	if (p_diagnostic->excludes & indicator)
	{
		result = 1;
	}
	else
	{
		// is threshold specified by caller?
		for (int i = 0; i < p_diagnostic->overrides_len; i++)
		{
			if (p_diagnostic->p_overrides[i].type == indicator)
			{
				*p_expected  = p_diagnostic->p_overrides[i].threshold;
				break;
			}
		}

		// perform check
		switch (e)
		{
			case EQUALITY_LESSTHAN:
				result = (actual < (*p_expected));
				break;
			case EQUALITY_LESSTHANEQUAL:
				result = (actual <= (*p_expected));
				break;
			case EQUALITY_EQUAL:
				result = (actual == (*p_expected));
				break;
			case EQUALITY_GREATERTHANEQUAL:
				result = (actual >= (*p_expected));
				break;
			case EQUALITY_GREATHERTHAN:
				result = (actual > (*p_expected));
				break;
			case EQUALITY_NOTEQUAL:
				result = (actual != (*p_expected));
				break;
			default:
				break;
		}
	}

	return result;
}

/*
 * Perform an string compare as part of a diagnostic test
 */
NVM_BOOL diag_check_str(const struct diagnostic *p_diagnostic,
		const NVM_UINT32 indicator, const char *actual,
		char *expected)
{
	COMMON_LOG_ENTRY();
	NVM_BOOL result = 0;

	// is check excluded by caller
	if (p_diagnostic->excludes & indicator)
	{
		result = 1;
	}
	else
	{
		// is threshold value specified by caller?
		for (int i = 0; i < p_diagnostic->overrides_len; i++)
		{
			if (p_diagnostic->p_overrides[i].type == indicator)
			{
				s_strcpy(expected, p_diagnostic->p_overrides[i].threshold_str,
						NVM_THRESHOLD_STR_LEN);
				break;
			}
		}

		// perform check
		result = (strncmp(expected, actual, NVM_THRESHOLD_STR_LEN) == 0);
	}
	return result;
}

/*
 * Clear existing diagnostic results for the specified diagnostic
 * and optionally for the specified device.
 */
void diag_clear_results(const enum diagnostic_test type,
		const NVM_BOOL clear_specific_device, const NVM_UID device_uid)
{
	int event_count;
	PersistentStore *p_store = get_lib_store();
	if (p_store)
	{
		db_get_event_count_by_event_type_type(p_store, type, &event_count);
		struct db_event events[event_count];
		db_get_events_by_event_type_type(p_store, type, events, event_count);
		for (int i = 0; i < event_count; i++)
		{
			NVM_BOOL matched = 1;
			// check for a matching uid is requested
			if (clear_specific_device)
			{
				COMMON_UID uid;
				uid_copy(events[i].uid, uid);
				if (uid_cmp(uid, device_uid) != 1)
				{
					matched = 0;
				}
			}
			if (matched)
			{
				db_delete_event_by_id(p_store, events[i].id);
			}
		}
	}
}
