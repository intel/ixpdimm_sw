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
 * This file contains the implementation of the PM metadata diagnostic
 * for the native API.
 */

#include "nvm_management.h"
#include "diagnostic.h"
#include "device_adapter.h"
#include <persistence/logging.h>
#include <persistence/event.h>
#include "device_utilities.h"
#include "capabilities.h"

NVM_UINT16 get_pm_metadata_code(struct health_event event);
void get_pm_metadata_entity_uid(struct health_event event, NVM_UID result);
enum diagnostic_result get_pm_metadata_result(struct health_event event);
enum event_severity get_pm_metadata_severity(struct health_event event);

/*
 * Call into the driver to run a metadata check. Store results into the event table
 */
int diag_pm_metadata_check(NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc;

	*p_results = 0;

	diag_clear_results(EVENT_TYPE_DIAG_PM_META, 0, NULL);

	if ((rc = IS_NVM_FEATURE_SUPPORTED(pm_metadata_diagnostic)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("The persistent metadata diagnostic is not supported.");
	}
	else
	{
		rc = get_test_result_count(DRIVER_DIAGNOSTIC_PM_METADATA_CHECK);
		if (rc > 0)
		{
			NVM_UINT32 result_count = (NVM_UINT32)rc;
			struct health_event results[result_count];
			if ((rc = run_test(DRIVER_DIAGNOSTIC_PM_METADATA_CHECK, result_count, results))
				== NVM_SUCCESS)
			{
				// store results in db
				COMMON_LOG_DEBUG_F("Got %d results from driver", result_count);
				for (NVM_UINT32 i = 0; i < result_count && i < MAX_DIAGNOSTIC_RESULTS; i ++)
				{
					NVM_UID uid;
					get_pm_metadata_entity_uid(results[i], uid);
					store_event_by_parts(
							EVENT_TYPE_DIAG_PM_META,
							get_pm_metadata_severity(results[i]),
							get_pm_metadata_code(results[i]),
							uid,
							0, // action is not required
							NULL,
							NULL,
							NULL,
							get_pm_metadata_result(results[i]));
				}
				*p_results = result_count;
			}
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Driver metadata check diagnostic:
 * Turn a driver diagnostic result into a diagnostic result code.
 */
enum diagnostic_result get_pm_metadata_result(struct health_event event)
{
	enum diagnostic_result result = DIAGNOSTIC_RESULT_FAILED;

	if ((event.event_type == HEALTH_EVENT_TYPE_NAMESPACE &&
		event.health.namespace_event.health_flag == NAMESPACE_HEALTH_RESULT_OK) ||
		(event.event_type == HEALTH_EVENT_TYPE_LABEL_AREA &&
			event.health.label_area_event.health_flag == LABEL_AREA_HEALTH_RESULT_OK))
	{
		result = DIAGNOSTIC_RESULT_OK;
	}

	return result;
}

/*
 * Driver metadata check diagnostic:
 * Turn a diagnostic result into an event code.
 */
NVM_UINT16 get_pm_metadata_code(struct health_event event)
{
	NVM_UINT16 result;
	if (event.event_type == HEALTH_EVENT_TYPE_NAMESPACE)
	{
		switch (event.health.namespace_event.health_flag)
		{
			case NAMESPACE_HEALTH_RESULT_OK:
				result = EVENT_CODE_DIAG_PM_META_NS_OK;
				break;
			case NAMESPACE_HEALTH_RESULT_MISSING_LABEL:
				result = EVENT_CODE_DIAG_PM_META_NS_MISSING_LABEL;
				break;
			case NAMESPACE_HEALTH_RESULT_CORRUPT_INTERLEAVE_SET:
				result = EVENT_CODE_DIAG_PM_META_NS_CORRUPT_INTERLEAVE_SET;
				break;
			case NAMESPACE_HEALTH_RESULT_INCONSISTENT_LABELS:
				result = EVENT_CODE_DIAG_PM_META_NS_INCONSISTENT_LABELS;
				break;
			case NAMESPACE_HEALTH_RESULT_INVALID_BLOCK_SIZE:
				result = EVENT_CODE_DIAG_PM_META_NS_INVALID_BLOCK_SIZE;
				break;
			case NAMESPACE_HEALTH_RESULT_CORRUPT_BTT_METADATA:
				result = EVENT_CODE_DIAG_PM_META_NS_CORRUPT_BTT_METADATA;
				break;
			default:
				result = EVENT_CODE_DIAG_PM_META_UNKNOWN;
		}
	}
	else if (event.event_type == HEALTH_EVENT_TYPE_LABEL_AREA)
	{
		switch (event.health.label_area_event.health_flag)
		{
			case LABEL_AREA_HEALTH_RESULT_OK:
				result = EVENT_CODE_DIAG_PM_META_LABEL_OK;
				break;
			case LABEL_AREA_HEALTH_RESULT_MISSING_PCD:
				result = EVENT_CODE_DIAG_PM_META_LABEL_MISSING_PCD;
				break;
			case LABEL_AREA_HEALTH_RESULT_MISSING_VALID_INDEX_BLOCK:
				result = EVENT_CODE_DIAG_PM_META_LABEL_MISSING_VALID_INDEX_BLOCK;
				break;
			case LABEL_AREA_HEALTH_RESULT_INSUFFICIENT_PERSISTENT_MEMORY:
				result = EVENT_CODE_DIAG_PM_META_LABEL_INSUFFICIENT_PM;
				break;
			case LABEL_AREA_HEALTH_RESULT_LABELS_OVERLAP:
				result = EVENT_CODE_DIAG_PM_META_LABEL_LABELS_OVERLAP;
				break;
			default:
				result = EVENT_CODE_DIAG_PM_META_UNKNOWN;
		}
	}
	else
	{
		COMMON_LOG_ERROR_F("Driver check metatdata health event type is unknown: %d",
			(int)event.event_type);
		result = EVENT_CODE_DIAG_PM_META_UNKNOWN;
	}
	return result;
}

/*
 * Driver metadata check diagnostic:
 * Populate result with the appropriate namespace or device uid, depending on the type
 * of event.
 */
void get_pm_metadata_entity_uid(struct health_event event, NVM_UID result)
{
	if (event.event_type == HEALTH_EVENT_TYPE_NAMESPACE)
	{
		memmove(result, event.health.namespace_event.namespace_uid, NVM_MAX_UID_LEN);
	}
	else if (event.event_type == HEALTH_EVENT_TYPE_LABEL_AREA)
	{
		struct device_discovery discovery;
		NVM_NFIT_DEVICE_HANDLE handle;
		handle.handle = event.health.label_area_event.device_handle;
		if (lookup_dev_handle(handle, &discovery) == NVM_SUCCESS)
		{
			memmove(result, discovery.uid, NVM_MAX_UID_LEN);
		}
	}
	else
	{
		COMMON_LOG_ERROR_F("Driver check metatdata health event type is unknown: %d",
		(int)event.event_type);
	}
}

/*
 * Given a PM metadata diagnostic result, return the severity of the event.
 */
enum event_severity get_pm_metadata_severity(struct health_event event)
{
	enum event_severity severity = EVENT_SEVERITY_INFO;
	if ((event.event_type == HEALTH_EVENT_TYPE_NAMESPACE &&
			event.health.namespace_event.health_flag != NAMESPACE_HEALTH_RESULT_OK) ||
		(event.event_type == HEALTH_EVENT_TYPE_LABEL_AREA &&
			event.health.label_area_event.health_flag != LABEL_AREA_HEALTH_RESULT_OK))
	{
		severity = EVENT_SEVERITY_WARN;
	}
	return severity;
}
