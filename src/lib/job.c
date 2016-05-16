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
 * Describes the interface for the job functions for the Native API.
 */

#include "nvm_management.h"
#include <persistence/logging.h>
#include <persistence/lib_persistence.h>
#include "device_adapter.h"
#include <string.h>
#include "system.h"

int nvm_get_job_count()
{
	COMMON_LOG_ENTRY();
	int rc = NVM_ERR_UNKNOWN;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else
	{
		rc = get_job_count();
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

int nvm_get_jobs(struct job *p_jobs, const NVM_UINT32 count)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;
	int job_index = 0;

	if (check_caller_permissions() != NVM_SUCCESS)
	{
		rc = NVM_ERR_INVALIDPERMISSIONS;
	}
	else if (!is_supported_driver_available())
	{
		rc = NVM_ERR_BADDRIVER;
	}
	else if (p_jobs == NULL)
	{
		COMMON_LOG_ERROR("Invalid parameter, p_jobs is NULL");
		rc = NVM_ERR_INVALIDPARAMETER;
	}
	else if ((rc = get_job_count()) < 0)
	{
		COMMON_LOG_ERROR("Failed to retrieve job count.");
	}
	else if (rc > 0)
	{
		// clear the structure
		memset(p_jobs, 0, sizeof (struct job) * count);

		if ((rc = nvm_get_device_count()) >= 0)
		{
			int device_count = rc;
			struct device_discovery devices[rc];
			if ((rc = nvm_get_devices(devices, device_count)) > 0)
			{
				// iterate through all devices and add up the capacities
				for (int i = 0; i < device_count; i++)
				{
					if (job_index >= count)
					{
						rc = NVM_ERR_ARRAYTOOSMALL;
						COMMON_LOG_ERROR("Invalid parameter, "
								"count is smaller than number of jobs");
						break;
					}

					if (devices[i].manageability == MANAGEMENT_VALIDCONFIG)
					{
						struct pt_payload_sanitize_dimm_status sanitize_status;
						struct fw_cmd cmd;
						memset(&cmd, 0, sizeof (struct fw_cmd));
						cmd.device_handle = devices[i].device_handle.handle;
						cmd.opcode = PT_GET_SEC_INFO;
						cmd.sub_opcode = SUBOP_GET_SAN_STATE;
						cmd.output_payload_size = sizeof (sanitize_status);
						cmd.output_payload = &sanitize_status;
						if (NVM_SUCCESS != ioctl_passthrough_cmd(&cmd))
						{
							COMMON_LOG_ERROR_F(
									"Unable to get sanitize status for handle: [%d]",
									devices[i].device_handle.handle);
						}
						else
						{
							if (sanitize_status.state != SAN_IDLE)
							{
								if (sanitize_status.state == SAN_INPROGRESS)
								{
									p_jobs[job_index].status = NVM_JOB_STATUS_RUNNING;
								}
								else if (sanitize_status.state ==  SAN_COMPLETED)
								{
									p_jobs[job_index].status = NVM_JOB_STATUS_COMPLETE;
								}
								else
								{
									p_jobs[job_index].status = NVM_JOB_STATUS_UNKNOWN;
								}

								p_jobs[job_index].type = NVM_JOB_TYPE_SANITIZE;
								p_jobs[job_index].percent_complete = sanitize_status.progress;
								memmove(p_jobs[job_index].uid, devices[i].uid, NVM_MAX_UID_LEN);
								memmove(p_jobs[job_index].affected_element,
									devices[i].uid, NVM_MAX_UID_LEN);
								p_jobs[job_index].result = NULL;
								job_index++;
							}
						}
					}
					rc = job_index;
				}
			}
			else
			{
				COMMON_LOG_ERROR_F("Unable to get device discovery: rc = %d", rc);
			}
		}
		else
		{
			COMMON_LOG_ERROR_F("Unable to get device count: rc = %d", rc);
		}
	}
	return rc;
}
