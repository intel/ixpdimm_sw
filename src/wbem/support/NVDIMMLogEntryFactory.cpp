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
 * This file contains the provider for the NVDIMMLogEntry instances
 * which represent an individual NVM DIMM event.
 */


#include <sstream>
#include <string.h>
#include <string/s_str.h>
#include "NVDIMMEventLogFactory.h"
#include "NVDIMMLogEntryFactory.h"
#include <physical_asset/NVDIMMFactory.h>
#include <LogEnterExit.h>
#include <server/BaseServerFactory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionNotSupported.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include <framework_interface/FrameworkExtensions.h>

wbem::support::NVDIMMLogEntryFactory::NVDIMMLogEntryFactory()
throw (wbem::framework::Exception)
{
	m_AcknowledgeEvent = nvm_acknowledge_event;
}

wbem::support::NVDIMMLogEntryFactory::~NVDIMMLogEntryFactory()
{ }


void wbem::support::NVDIMMLogEntryFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	populateAttributes(attributes);
}

std::string wbem::support::NVDIMMLogEntryFactory::eventTypeToString(int eventType)
{
	std::string eventTypeName;
	switch (eventType)
	{
		case EVENT_TYPE_ALL:
			eventTypeName = NVDIMMLOGENTRY_TYPE_ALL;
			break;
		case EVENT_TYPE_HEALTH:
			eventTypeName = NVDIMMLOGENTRY_TYPE_HEALTH;
			break;
		case EVENT_TYPE_MGMT:
			eventTypeName = NVDIMMLOGENTRY_TYPE_MGMT;
			break;
		case EVENT_TYPE_DIAG:
			eventTypeName = NVDIMMLOGENTRY_TYPE_DIAG;
			break;
		case EVENT_TYPE_DIAG_QUICK:
			eventTypeName = NVDIMMLOGENTRY_TYPE_QUICKDIAG;
			break;
		case EVENT_TYPE_DIAG_PLATFORM_CONFIG:
			eventTypeName = NVDIMMLOGENTRY_TYPE_PLATFORMCONFIGDIAG;
			break;
		case EVENT_TYPE_DIAG_PM_META:
			eventTypeName = NVDIMMLOGENTRY_TYPE_PMDIAG;
			break;
		case EVENT_TYPE_DIAG_SECURITY:
			eventTypeName = NVDIMMLOGENTRY_TYPE_SECURITYDIAG;
			break;
		case EVENT_TYPE_DIAG_FW_CONSISTENCY:
			eventTypeName = NVDIMMLOGENTRY_TYPE_FWCONSISTENCYDIAG;
			break;
		case EVENT_TYPE_CONFIG:
			eventTypeName = NVDIMMLOGENTRY_TYPE_CONFIG;
			break;
		default:
			eventTypeName = NVDIMMLOGENTRY_TYPE_UNKNOWN;
			break;
	}
	return eventTypeName;
}

int wbem::support::NVDIMMLogEntryFactory::eventTypeStringToType(const std::string &typeStr)
{
	int type = -1;

	if (s_strncmpi(NVDIMMLOGENTRY_TYPE_ALL.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_ALL;
	}
	else if (s_strncmpi(NVDIMMLOGENTRY_TYPE_HEALTH.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_HEALTH;
	}
	else if (s_strncmpi(NVDIMMLOGENTRY_TYPE_MGMT.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_MGMT;
	}
	else if (s_strncmpi(NVDIMMLOGENTRY_TYPE_DIAG.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_DIAG;
	}
	else if (s_strncmpi(NVDIMMLOGENTRY_TYPE_QUICKDIAG.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_DIAG_QUICK;
	}
	else if (s_strncmpi(NVDIMMLOGENTRY_TYPE_PLATFORMCONFIGDIAG.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_DIAG_PLATFORM_CONFIG;
	}
	else if (s_strncmpi(NVDIMMLOGENTRY_TYPE_PMDIAG.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_DIAG_PM_META;
	}
	else if (s_strncmpi(NVDIMMLOGENTRY_TYPE_SECURITYDIAG.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_DIAG_SECURITY;
	}
	else if (s_strncmpi(NVDIMMLOGENTRY_TYPE_FWCONSISTENCYDIAG.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_DIAG_FW_CONSISTENCY;
	}
	else if (s_strncmpi(NVDIMMLOGENTRY_TYPE_CONFIG.c_str(), typeStr.c_str(), typeStr.size() + 1) == 0)
	{
		type = EVENT_TYPE_CONFIG;
	}

	return type;
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::NVDIMMLogEntryFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);
		framework::Attribute instanceID = path.getKeyValue(INSTANCEID_KEY);
		int instanceId;
		std::istringstream(instanceID.stringValue().c_str()) >> instanceId;

		// check if this event exists
		struct event_filter filter;
		memset(&filter, 0, sizeof(struct event_filter));
		filter.filter_mask = NVM_FILTER_ON_EVENT;
		filter.event_id = instanceId;
		int count = nvm_get_event_count(&filter);
		if (count != 1)
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}
		else
		{
			struct event event_item;
			memset(&event_item, 0, sizeof(struct event));
			count = nvm_get_events(&filter, &event_item, 1);
			if (count == 1)
			{
				eventToInstance(pInstance, &event_item, attributes);
			}
			else
			{
				throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
			}
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pInstance != NULL)
		{
			delete pInstance;
		}
		throw;
	}
	return pInstance;
}

/*
 * Return the object path for the single instance for this server
 */
wbem::framework::instance_names_t* wbem::support::NVDIMMLogEntryFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// get the host server name
		std::string hostName = wbem::server::getHostName();
		framework::attributes_t keys;

		// get all events
		int count = nvm_get_event_count(NULL);
		if (count < 0)
		{
			throw exception::NvmExceptionLibError(count);
		}
		if (count > 0)
		{
			struct event events[count];
			count = nvm_get_events(NULL, events, count);
			for (int i = 0; i < count; i++)
			{
				std::stringstream instanceIdStr;
				instanceIdStr <<  events[i].event_id;
				keys[INSTANCEID_KEY] =
						framework::Attribute(instanceIdStr.str(), true);

				// create the object path
				framework::ObjectPath path(hostName, NVM_NAMESPACE,
						NVDIMMLOGENTRY_CREATIONCLASSNAME, keys);
				pNames->push_back(path);
			}
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pNames != NULL)
		{
			delete pNames;
		}
		throw;
	}
	return pNames;
}

/*
 * Helper method to get instance from a single struct event
 */
void wbem::support::NVDIMMLogEntryFactory::eventToInstance(framework::Instance *pInstance,
		struct event *pEvent,
		framework::attribute_names_t &attributes)
{
	if ((NULL == pEvent) || (NULL == pInstance))
	{
		return;
	}

	// PerceivedSeverity
	if (containsAttribute(PERCEIVEDSEVERITY_KEY, attributes))
	{
		std::string severityName;
		switch ((int)(pEvent->severity))
		{
		case EVENT_SEVERITY_INFO:
			severityName = TR("Information");
			break;
		case EVENT_SEVERITY_WARN:
			severityName = TR("Degraded/Warning");
			break;
		case EVENT_SEVERITY_CRITICAL:
			severityName = TR("Critical");
			break;
		case EVENT_SEVERITY_FATAL:
			severityName = TR("Fatal/NonRecoverable");
			break;
		default:
			severityName = TR("Unknown");
			break;
		}
		framework::Attribute severityNameAttr(
				(NVM_UINT16) pEvent->severity, severityName, false);
		pInstance->setAttribute(PERCEIVEDSEVERITY_KEY,
				severityNameAttr, attributes);
	}

	// LogInstanceID
	if (containsAttribute(LOGINSTANCEID_KEY, attributes))
	{
		// get the host server name
		std::string hostName = wbem::server::getHostName();
		std::string loginstanceid = wbem::support::NVDIMMEVENTLOG_INSTANCEID + hostName;
		framework::Attribute loginstanceidAttr(loginstanceid, false);
		pInstance->setAttribute(LOGINSTANCEID_KEY, loginstanceidAttr, attributes);
	}

	// CreationTimeStamp
	if (containsAttribute(CREATIONTIMESTAMP_KEY, attributes))
	{
		framework::Attribute timetAttr((NVM_UINT64)(pEvent->time),
				wbem::framework::DATETIME_SUBTYPE_DATETIME, false);
		pInstance->setAttribute(CREATIONTIMESTAMP_KEY, timetAttr, attributes);
	}

	// MessageID
	if (containsAttribute(MESSAGEID_KEY, attributes))
	{
		std::string eventTypeName;
		eventTypeName = eventTypeToString(pEvent->type);
		framework::Attribute messageidAttr(eventTypeName, false);
		pInstance->setAttribute(MESSAGEID_KEY, messageidAttr, attributes);
	}

	// Message
	if (containsAttribute(MESSAGE_KEY, attributes))
	{
		framework::Attribute msgsAttr(pEvent->message, false);
		pInstance->setAttribute(MESSAGE_KEY, msgsAttr, attributes);
	}

	// MessageArguments
	if (containsAttribute(MESSAGEARGS_KEY, attributes))
	{
		std::vector<std::string> messageArgs;
		for (int j=0; j< NVM_MAX_EVENT_ARGS; j++)
		{
			messageArgs.push_back(pEvent->args[j]);
		}
		framework::Attribute msgsAttr(messageArgs, false);
		pInstance->setAttribute(MESSAGEARGS_KEY, msgsAttr, attributes);
	}
	// ActionRequired
	if (containsAttribute(ACTIONREQUIRED_KEY, attributes))
	{
		framework::Attribute actionRequiredAttr((bool)pEvent->action_required, false);
		pInstance->setAttribute(ACTIONREQUIRED_KEY, actionRequiredAttr,
				attributes);
	}
}

/*
 * Modify an NVDIMMLogEntryFactory instance.
 */
wbem::framework::Instance* wbem::support::NVDIMMLogEntryFactory::modifyInstance(
		framework::ObjectPath &path, framework::attributes_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = NULL;

	framework::Attribute idAttr = path.getKeyValue(INSTANCEID_KEY);
	int instanceId;
	std::istringstream(idAttr.stringValue().c_str()) >> instanceId;

	// check if this event exists
	struct event_filter filter;
	memset(&filter, 0, sizeof(struct event_filter));
	filter.filter_mask = NVM_FILTER_ON_EVENT;
	filter.event_id = instanceId;
	int count = nvm_get_event_count(&filter);
	if (count == 0)
	{
		throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
	}
	else if (count < 0)
	{
		throw exception::NvmExceptionLibError(count);
	}
	else
	{
		framework::attribute_names_t attributeNames;
		pInstance = getInstance(path, attributeNames);

		framework::attribute_names_t modifyableAttributes;
		modifyableAttributes.push_back(ACTIONREQUIRED_KEY);
		checkAttributesAreModifiable(pInstance, attributes, modifyableAttributes);

		if (pInstance)
		{
			// verify ActionRequired - can only ack an event that has action required set to true
			wbem::framework::Attribute arAttribute;
			pInstance->getAttribute(ACTIONREQUIRED_KEY, arAttribute);
			if (arAttribute.boolValue() == 0)
			{
				throw framework::ExceptionNotSupported(__FILE__, (char*) __func__);
			}
			framework::Attribute arAttr(0, false);
			pInstance->setAttribute(ACTIONREQUIRED_KEY, arAttr);

			int rc;
			if ((rc = m_AcknowledgeEvent(instanceId)) < NVM_SUCCESS)
			{
				throw exception::NvmExceptionLibError(rc);
			}
		}
		else
		{
			throw framework::ExceptionBadParameter("pInstance");
		}
	}
	return pInstance;
}

void wbem::support::NVDIMMLogEntryFactory::populateAttributes(std::vector<std::string> &attributes)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(PERCEIVEDSEVERITY_KEY);
	attributes.push_back(LOGINSTANCEID_KEY);
	attributes.push_back(CREATIONTIMESTAMP_KEY);
	attributes.push_back(MESSAGEID_KEY);
	attributes.push_back(MESSAGE_KEY);
	attributes.push_back(MESSAGEARGS_KEY);
	attributes.push_back(ACTIONREQUIRED_KEY);
}
