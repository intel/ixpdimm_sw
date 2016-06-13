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
 * This file contains the provider for the DiagnosticLog instance
 * which represents the log of diagnostic results.
 */


#include <string.h>
#include <uid/uid.h>
#include <string/s_str.h>
#include "DiagnosticLogFactory.h"
#include <LogEnterExit.h>
#include <server/BaseServerFactory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>

wbem::support::DiagnosticLogFactory::DiagnosticLogFactory()
throw (wbem::framework::Exception)
{
	m_PurgeDiagnosticLog = nvm_purge_events;
}

wbem::support::DiagnosticLogFactory::~DiagnosticLogFactory()
{ }


void wbem::support::DiagnosticLogFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(CURRENTNUMBEROFRECORDS_KEY);

}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::DiagnosticLogFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		// get the host server name
		std::string hostName = wbem::server::getHostName();

		// make sure the instance ID passed in matches this host
		framework::Attribute instanceID = path.getKeyValue(INSTANCEID_KEY);
		if (instanceID.stringValue() == std::string(DIAGNOSTICLOG_NAME + hostName))
		{
			// ElementName - "NVDIMM Diagnostic Log"
			if (containsAttribute(ELEMENTNAME_KEY, attributes))
			{
				framework::Attribute elementNameAttr(DIAGNOSTICLOG_NAME, false);
				pInstance->setAttribute(ELEMENTNAME_KEY, elementNameAttr, attributes);
			}

			// CurrentNumberOfRecords - One per test type per dimm
			if (containsAttribute(CURRENTNUMBEROFRECORDS_KEY, attributes))
			{
				diagnosticResults_t results;
				int count = gatherDiagnosticResults(&results);
				framework::Attribute recordCntAttr((NVM_UINT64)count, false);
				pInstance->setAttribute(CURRENTNUMBEROFRECORDS_KEY, recordCntAttr, attributes);
			}
		}
		else
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pInstance;
		throw;
	}

	return pInstance;
}

/*
 * Return the object path for the single instance for this server
 */
wbem::framework::instance_names_t* wbem::support::DiagnosticLogFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// get the host server name
		std::string hostName = wbem::server::getHostName();
		framework::attributes_t keys;

		// Instance ID = "NVDIMM Diagnostic Log" + host name
		keys[INSTANCEID_KEY] =
				framework::Attribute(DIAGNOSTICLOG_NAME + hostName, true);

		// create the object path
		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				DIAGNOSTICLOG_CREATIONCLASSNAME, keys);
		pNames->push_back(path);
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	return pNames;
}

wbem::framework::UINT32 wbem::support::DiagnosticLogFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = framework::MOF_ERR_SUCCESS;

	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %d", method.c_str(), (int)(inParms.size()));

	try
	{
		if (method == DIAGNOSTICLOG_CLEARLOG)
		{
			clearDiagnosticLog();
		}
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
		}
	}
	catch(exception::NvmExceptionLibError &)
	{
		wbemRc = DIAGNOSTICLOG_ERR_FAILED;
	}
	catch(framework::Exception &)
	{
		wbemRc = DIAGNOSTICLOG_ERR_FAILED;
	}

	COMMON_LOG_EXIT_RETURN("httpRc: %u, wbemRc: %u", httpRc, wbemRc);
	return httpRc;
}

void wbem::support::DiagnosticLogFactory::clearDiagnosticLog()
	throw (framework::Exception)
{
	int rc;
	struct event_filter filter;
	memset(&filter, 0, sizeof (struct event_filter));
	filter.filter_mask = NVM_FILTER_ON_TYPE;
	filter.type = EVENT_TYPE_DIAG;
	if ((rc = m_PurgeDiagnosticLog(&filter)) < NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}


std::string wbem::support::DiagnosticLogFactory::buildDiagnosticResultMessage(
		struct event *p_event)
{
	std::string msg_str;
	char msg[NVM_EVENT_MSG_LEN];
	s_snprintf(msg, (NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)),
			p_event->message, p_event->args[0], p_event->args[1], p_event->args[2]);
	msg_str = msg;
	return msg_str;
}

/*!
 * Helper method to convert library events into a vector of diagnostic result structures.
 * Used by the DiagnosticCompletionRecordFactory class as well.
 */
int wbem::support::DiagnosticLogFactory::gatherDiagnosticResults(diagnosticResults_t *pResults)
throw (framework::Exception)
{
	int count = 0;

	// get all diagnostic events
	struct event_filter filter;
	memset(&filter, 0, sizeof (struct event_filter));
	filter.filter_mask = NVM_FILTER_ON_TYPE;
	filter.type = EVENT_TYPE_DIAG;
	int total_count = nvm_get_event_count(&filter);
	if (total_count < 0)
	{
		throw exception::NvmExceptionLibError(total_count);
	}
	if (total_count > 0)
	{
		struct event events[total_count];
		total_count = nvm_get_events(&filter, events, total_count);
		if (total_count < 0)
		{
			throw exception::NvmExceptionLibError(total_count);
		}
		for (int i = 0; i < total_count; i++)
		{
			bool matched = false;
			// only add a new structure if the test is unique to the type and uid
			// else another message and update the result
			for (diagnosticResults_t::iterator iter = pResults->begin();
					iter != pResults->end(); iter++)
			{
				if ((*iter).type == events[i].type)
				{
					// matched existing result - update data but do not count
					if (uid_cmp((*iter).device_uid, events[i].uid))
					{
						matched = true;
						// update id (if necessary)
						if (events[i].event_id < (*iter).id)
						{
							(*iter).id = events[i].event_id;
						}
						// update overall result (if necessary)
						if (events[i].diag_result > (*iter).result)
						{
							(*iter).result = events[i].diag_result;
						}
						// update time (if necessary)
						if (events[i].time < (*iter).time)
						{
							(*iter).time = events[i].time;
						}
						// add message
						(*iter).messages.push_back(buildDiagnosticResultMessage(&events[i]));
						break;
					}
				}
			}
			// add new result
			if (!matched)
			{
				count++;
				// copy the event information into the diagnostic result struct
				struct diagnosticResult diag;
				diag.id = events[i].event_id;
				diag.time = events[i].time;
				diag.type = events[i].type;
				memmove(diag.device_uid, events[i].uid, NVM_MAX_UID_LEN);
				diag.result = events[i].diag_result;
				diag.messages.push_back(buildDiagnosticResultMessage(&events[i]));
				pResults->push_back(diag);
			}
		} // end for
	} // end if events

	return count;
}
