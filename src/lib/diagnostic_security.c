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
 * This file contains the implementation of the security diagnostic
 * for the native API.
 */

#include "nvm_management.h"
#include "diagnostic.h"
#include <persistence/logging.h>
#include <persistence/event.h>
#include <string/s_str.h>
#include "capabilities.h"

/*
 * Used in security check diagnostic to display lock states enum as strings
 */
static const char *lock_state_strings[] =
{ "Unknown", "Disabled", "Unlocked", "Locked", "Frozen", "Exceeded",
		"Busy", "Idle", "Completed" };

/*
 * Run the security check diagnostic algorithm
 */
int diag_security_check(const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	int dev_count = 0;
	*p_results = 0;

	// clear previous results
	diag_clear_results(EVENT_TYPE_DIAG_SECURITY, 0, NULL);

	if ((rc = IS_NVM_FEATURE_SUPPORTED(security_diagnostic)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("The security diagnostic is not supported.");
	}
	else
	{
		dev_count = nvm_get_device_count();
		if (dev_count == 0)
		{
			store_event_by_parts(EVENT_TYPE_DIAG_SECURITY, EVENT_SEVERITY_WARN,
					EVENT_CODE_DIAG_SECURITY_NO_DIMMS, NULL, 0, NULL, NULL, NULL,
					DIAGNOSTIC_RESULT_ABORTED);
			(*p_results)++;
		}
		else if (dev_count > 0)
		{
			// get device_discovery information of all dimms
			struct device_discovery dimms[dev_count];
			int manageable_dev_count = 0;
			dev_count = nvm_get_devices(dimms, dev_count);
			if (dev_count > 0)
			{
				rc = NVM_SUCCESS;
				// count the number of dimms in each security state.
				int security_state_count = (int)LOCK_STATE_NOT_SUPPORTED;

				int count[security_state_count+1];
				memset(count, 0, sizeof (int) * (security_state_count + 1));
				for (int i = 0; i < dev_count; i++)
				{
					// only take dimms that are manageable into account
					if (dimms[i].manageability == MANAGEMENT_VALIDCONFIG)
						{
							count[dimms[i].lock_state]++;
							manageable_dev_count++;
						}
				}

				// check if all manageable dimms are security not supported
				if ((count[LOCK_STATE_NOT_SUPPORTED] == manageable_dev_count) &&
						(!(p_diagnostic->excludes &
								DIAG_THRESHOLD_SECURITY_ALL_NOTSUPPORTED)))
				{
					store_event_by_parts(EVENT_TYPE_DIAG_SECURITY,
							EVENT_SEVERITY_WARN,
							EVENT_CODE_DIAG_SECURITY_ALL_NOTSUPPORTED, NULL, 0, NULL,
							NULL, NULL, DIAGNOSTIC_RESULT_FAILED);
					(*p_results)++;
				}
				// check if all manageable dimms are disabled
				else if ((count[LOCK_STATE_DISABLED] == manageable_dev_count) &&
					(!(p_diagnostic->excludes & DIAG_THRESHOLD_SECURITY_ALL_DISABLED)))
				{
					store_event_by_parts(EVENT_TYPE_DIAG_SECURITY,
							EVENT_SEVERITY_WARN,
							EVENT_CODE_DIAG_SECURITY_ALL_DISABLED, NULL, 0, NULL,
							NULL, NULL, DIAGNOSTIC_RESULT_FAILED);
					(*p_results)++;
				}

				if (*p_results == 0)
				{
					// check if all manageable dimms have the same security state
					if (!(p_diagnostic->excludes & DIAG_THRESHOLD_SECURITY_CONSISTENT))
					{
						char inconsistent_security_state_event_arg_str[NVM_EVENT_ARG_LEN];
						NVM_BOOL inconsistent_flag = 0;
						for (int j = 0; j < security_state_count; j++)
						{
							// check if security settings are inconsistent
							if (count[j] > 0 && count[j] != manageable_dev_count)
							{
								// appending to the argument
								if (inconsistent_flag)
								{
									s_strcat(inconsistent_security_state_event_arg_str,
											NVM_EVENT_ARG_LEN, ", ");
								}
								char arg_str_security_state[NVM_EVENT_ARG_LEN];
								s_snprintf(arg_str_security_state, NVM_EVENT_ARG_LEN,
										"%d %s", count[j], lock_state_strings[j]);
								s_strcat(inconsistent_security_state_event_arg_str,
										NVM_EVENT_ARG_LEN, arg_str_security_state);
								inconsistent_flag = 1;
							}
						}

						if (inconsistent_flag)
						{
							store_event_by_parts(EVENT_TYPE_DIAG_SECURITY,
									EVENT_SEVERITY_WARN,
									EVENT_CODE_DIAG_SECURITY_INCONSISTENT, NULL, 0,
									inconsistent_security_state_event_arg_str, NULL,
									NULL, DIAGNOSTIC_RESULT_FAILED);
							(*p_results)++;
						}
					}
				}

			} // nvm_get_devices failed
			else
			{
				rc = dev_count;
			}
		} // nvm_get_device_count failed
		else
		{
			rc = dev_count;
		}

		// add success message
		if ((rc == NVM_SUCCESS) && (*p_results == 0)) // No errors/warnings
		{
			// store success event
			store_event_by_parts(
				EVENT_TYPE_DIAG_SECURITY,
				EVENT_SEVERITY_INFO,
				EVENT_CODE_DIAG_SECURITY_SUCCESS,
				NULL,
				0,
				NULL,
				NULL,
				NULL,
				DIAGNOSTIC_RESULT_OK);
			(*p_results)++;
		}

	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
