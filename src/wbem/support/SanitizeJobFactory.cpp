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
 * This file contains the provider for the SanitizeJob instances which
 * provide security santize operations on an NVM DIMM.
 */

#include <string.h>
#include <nvm_management.h>
#include <LogEnterExit.h>
#include <libinvm-cim/ObjectPath.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <server/BaseServerFactory.h>
#include <uid/uid.h>
#include "SanitizeJobFactory.h"

#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include <core/device/Device.h>
#include <core/device/DeviceHelper.h>

wbem::support::SanitizeJobFactory::SanitizeJobFactory()
	throw (wbem::framework::Exception)
{
}

wbem::support::SanitizeJobFactory::~SanitizeJobFactory()
{
}

void wbem::support::SanitizeJobFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(NAME_KEY);
	attributes.push_back(OPERATIONALSTATUS_KEY);
	attributes.push_back(JOBSTATE_KEY);
	attributes.push_back(PERCENTCOMPLETE_KEY);
	attributes.push_back(DELETEONCOMPLETION_KEY);
	attributes.push_back(TIMEBEFOREREMOVAL_KEY);
}

wbem::framework::instance_names_t* wbem::support::SanitizeJobFactory::getInstanceNames()
				throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	int rc = NVM_SUCCESS;
	framework::instance_names_t *pNames = new framework::instance_names_t();

	try
	{
		// Get the host server name
		std::string hostName = wbem::server::getHostName();

		if ((rc = nvm_get_job_count()) >= NVM_SUCCESS)
		{
			struct job jobs[rc];
			memset(jobs, 0, sizeof(jobs));
			if ((rc = nvm_get_jobs(jobs, rc)) >= NVM_SUCCESS)
			{
				for (int i = 0; i < rc; i++)
				{
					if (jobs[i].type == NVM_JOB_TYPE_SANITIZE)
					{
						framework::attributes_t keys;
						NVM_UID job_uid_str;

						uid_copy(jobs[i].uid, job_uid_str);
						std::string instanceIdStr = job_uid_str;

						keys[INSTANCEID_KEY] = framework::Attribute(instanceIdStr, true);

						framework::ObjectPath jobPath(hostName, NVM_NAMESPACE,
								SANITIZEJOB_CREATIONCLASSNAME, keys);
						pNames->push_back(jobPath);
					}
				}
			}
			else
			{
				throw exception::NvmExceptionLibError(rc);
			}
		}
		else
		{
			throw exception::NvmExceptionLibError(rc);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	return pNames;
}

wbem::framework::Instance* wbem::support::SanitizeJobFactory::getInstance(wbem::framework::ObjectPath &path,
	wbem::framework::attribute_names_t &attributes) throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		int jobCount, rc;
		int foundIndex = -1;

		// Verify InstanceID
		framework::Attribute attribute = path.getKeyValue(INSTANCEID_KEY);
		std::string instanceUid = attribute.stringValue();

		if (!core::device::isUidValid(instanceUid))
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}

		checkAttributes(attributes);

		if ((jobCount = nvm_get_job_count()) < NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(jobCount);
		}
		struct job jobs[jobCount];
		memset(jobs, 0, sizeof(jobs));
		if ((rc = nvm_get_jobs(jobs, jobCount)) < NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}

		for (int i = 0; i < jobCount; i++)
		{
			NVM_UID jobUidStr;
			uid_copy(jobs[i].uid, jobUidStr);

			if ((instanceUid.compare(jobUidStr) == 0) &&
					(jobs[i].type == NVM_JOB_TYPE_SANITIZE))
			{
				foundIndex = i;
				break;
			}
		}

		// Could not find the job
		if (foundIndex == -1)
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}

		// InstanceID - the job uid
		framework::Attribute a(instanceUid, true);
		pInstance->setAttribute(INSTANCEID_KEY, a, attributes);

		// Name - the type of job e.g. Sanitize
		if (containsAttribute(NAME_KEY, attributes))
		{
			std::string name;
			if (jobs[foundIndex].type == NVM_JOB_TYPE_SANITIZE)
			{
				name = NVM_JOB_TYPE_SANITIZE_NAME;
			}

			framework::Attribute a(name, false);
			pInstance->setAttribute(NAME_KEY, a, attributes);
		}

		// OperationalStatus - always "OK"
		if (containsAttribute(OPERATIONALSTATUS_KEY, attributes))
		{
			std::string operationalStatus = NVM_STATUS_OK;

			framework::Attribute a(operationalStatus, false);
			pInstance->setAttribute(OPERATIONALSTATUS_KEY, a, attributes);
		}

		// JobState - status of the job from the job struct
		if (containsAttribute(JOBSTATE_KEY, attributes))
		{
			std::string jobState;
			if (jobs[foundIndex].status == NVM_JOB_STATUS_UNKNOWN)
			{
				jobState = "Unknown";
			}
			else if (jobs[foundIndex].status == NVM_JOB_STATUS_UNKNOWN)
			{
				jobState = "Not started";
			}
			else if (jobs[foundIndex].status == NVM_JOB_STATUS_RUNNING)
			{
				jobState = "Running";
			}
			else if (jobs[foundIndex].status == NVM_JOB_STATUS_COMPLETE)
			{
				jobState = "Completed";
			}
			framework::Attribute a(jobState, false);
			pInstance->setAttribute(JOBSTATE_KEY, a, attributes);
		}

		//PercentComplete from the job struct
		if (containsAttribute(PERCENTCOMPLETE_KEY, attributes))
		{
			NVM_UINT16 percentComplete;
			percentComplete = jobs[foundIndex].percent_complete;
			framework::Attribute a(percentComplete, false);
			pInstance->setAttribute(PERCENTCOMPLETE_KEY, a, attributes);
		}

		// PercentComplete from the job struct
		if (containsAttribute(DELETEONCOMPLETION_KEY, attributes))
		{
			bool deleteOnCompletion = false;
			framework::Attribute a(deleteOnCompletion, false);
			pInstance->setAttribute(DELETEONCOMPLETION_KEY, a, attributes);
		}

		// TimeBeforeRemoval - set to beginning of computer time - Midnight of January 1st, 1970
		if (containsAttribute(TIMEBEFOREREMOVAL_KEY, attributes))
		{
			framework::Attribute timeAttr((unsigned long long) 0,
							wbem::framework::DATETIME_SUBTYPE_INTERVAL, false);
			pInstance->setAttribute(TIMEBEFOREREMOVAL_KEY, timeAttr, attributes);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pInstance;
		throw;
	}
	return pInstance;
}






