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
 * This file contains the provider for the NVDIMMEventLog instance
 * which contain the NVM DIMM related events for the host system.
 */

#include <string.h>
#include <uid/uid.h>
#include <string/s_str.h>
#include "NVDIMMEventLogFactory.h"
#include <LogEnterExit.h>
#include <server/BaseServerFactory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <libinvm-cim/ExceptionInvalidWqlQuery.h>
#include <support/NVDIMMLogEntryFactory.h>
#include <sstream>
#include <cerrno>
#include <persistence/config_settings.h>
#include <persistence/lib_persistence.h>
#include <exception/NvmExceptionBadFilter.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>

wbem::support::NVDIMMEventLogFactory::NVDIMMEventLogFactory()
throw (wbem::framework::Exception)
{
	m_PurgeEventLog = nvm_purge_events;
}

wbem::support::NVDIMMEventLogFactory::~NVDIMMEventLogFactory()
{ }


void wbem::support::NVDIMMEventLogFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(CURRENTNUMBEROFRECORDS_KEY);
	attributes.push_back(MAXNUMBEROFRECORDS_KEY);
	attributes.push_back(OVERWRITEPOLICY_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::NVDIMMEventLogFactory::getInstance(
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
		if (instanceID.stringValue() == std::string(NVDIMMEVENTLOG_INSTANCEID + hostName))
		{
			// ElementName - "NVDIMM Event Log for: [host name]"
			if (containsAttribute(ELEMENTNAME_KEY, attributes))
			{
				std::string element_name = NVDIMMEVENTLOG_ELEMENTNAME + hostName;
				framework::Attribute elementNameAttr(element_name, false);
				pInstance->setAttribute(ELEMENTNAME_KEY, elementNameAttr, attributes);
			}

			// CurrentNumberOfRecords
			if (containsAttribute(CURRENTNUMBEROFRECORDS_KEY, attributes))
			{
				NVM_UINT64 count = (NVM_UINT64)nvm_get_event_count(NULL);
				if ((int)count < 0)
				{
					throw exception::NvmExceptionLibError(count);
				}
				else
				{
					framework::Attribute recordCntAttr(count, false);
					pInstance->setAttribute(CURRENTNUMBEROFRECORDS_KEY, recordCntAttr, attributes);
				}
			}
			// MaxNumberOfRecords
			if (containsAttribute(MAXNUMBEROFRECORDS_KEY, attributes))
			{
				int max_events;
				get_config_value_int(SQL_KEY_EVENT_LOG_MAX, &max_events);
				framework::Attribute recordMaxAttr((NVM_UINT64)max_events, false);
				pInstance->setAttribute(MAXNUMBEROFRECORDS_KEY, recordMaxAttr, attributes);

			}
			// OverwritePolicy = Wraps when full
			if (containsAttribute(OVERWRITEPOLICY_KEY, attributes))
			{
				framework::Attribute overWritePolicyAttr(
						NVDIMMEVENTLOG_OVERWRITEPOLICY_WRAPSWHENFULL, false);
				pInstance->setAttribute(OVERWRITEPOLICY_KEY, overWritePolicyAttr, attributes);
			}
		}
		else
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
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
wbem::framework::instance_names_t* wbem::support::NVDIMMEventLogFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// get the host server name
		std::string hostName = wbem::server::getHostName();
		framework::attributes_t keys;

		// Instance ID = "NVDIMM Event Log" + host name
		keys[INSTANCEID_KEY] =
				framework::Attribute(NVDIMMEVENTLOG_INSTANCEID + hostName, true);

		// create the object path
		framework::ObjectPath path(hostName, NVM_NAMESPACE,
				NVDIMMEVENTLOG_CREATIONCLASSNAME, keys);
		pNames->push_back(path);
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	return pNames;
}

wbem::framework::UINT32 wbem::support::NVDIMMEventLogFactory::executeMethod(
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
		if (method == NVDIMMEVENTLOG_CLEARLOG)
		{
			clearEventLog();
		}
		else if (method == NVDIMMEVENTLOG_FILTEREVENTS)
		{
			// Verify required parameters were supplied
			framework::attributes_t::const_iterator paramIter =
					inParms.find(NVDIMMEVENTLOG_FILTER);
			if (paramIter == inParms.end()) // filter parameter not supplied
			{
				COMMON_LOG_ERROR_F("method '%s' requires parameter '%s'",
						NVDIMMEVENTLOG_FILTEREVENTS.c_str(), NVDIMMEVENTLOG_FILTER.c_str());
				httpRc = wbem::framework::CIM_ERR_INVALID_PARAMETER;
			}
			else
			{
				// Output - a list of object paths
				framework::STR_LIST logEntryRefs =
						getFilteredEventRefsFromFilterString(paramIter->second.stringValue());

				// Save the output parameter
				outParms[NVDIMMEVENTLOG_LOGENTRIES] = framework::Attribute(logEntryRefs, false);
			}
		}
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
		}
	}
	catch(exception::NvmExceptionBadFilter &)
	{
		wbemRc = NVDIMMEVENTLOG_ERR_INVALID_PARAMETER;
	}
	catch (framework::ExceptionBadParameter &)
	{
		wbemRc = NVDIMMEVENTLOG_ERR_INVALID_PARAMETER;
	}
	catch(exception::NvmExceptionLibError &e)
	{
		if (e.getLibError() == NVM_ERR_NOMEMORY)
		{
			wbemRc = NVDIMMEVENTLOG_ERR_INSUFFICIENT_RESOURCES;
		}
		else
		{
			wbemRc = NVDIMMEVENTLOG_ERR_FAILED;
		}
	}
	catch(framework::Exception &)
	{
		wbemRc = NVDIMMEVENTLOG_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN("httpRc: %u, wbemRc: %u", httpRc, wbemRc);
	return httpRc;
}

void wbem::support::NVDIMMEventLogFactory::clearEventLog()
	throw (framework::Exception)
{
	int rc;
	struct event_filter filter;
	memset(&filter, 0, sizeof (struct event_filter));
	if ((rc = m_PurgeEventLog(&filter)) < NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void wbem::support::NVDIMMEventLogFactory::convertFactoryToEventFilter(
		EventLogFilter *pEventLogFilter, struct event_filter *pFilter)
{
	COMMON_LOG_ENTRY();
	if ((NULL == pFilter) || (NULL == pEventLogFilter))
	{
		COMMON_LOG_EXIT();
		return;
	}

	pFilter->filter_mask = 0;

	// convert EventLogFilter to strut event_filter
	if (pEventLogFilter->hasEventId())
	{
		pFilter->event_id = pEventLogFilter->getEventId();
		pFilter->filter_mask |= NVM_FILTER_ON_EVENT;
	}
	if (pEventLogFilter->hasSeverity())
	{
		pFilter->severity = (enum event_severity)pEventLogFilter->getSeverity();
		pFilter->filter_mask |= NVM_FILTER_ON_SEVERITY;
	}
	if (pEventLogFilter->hasType())
	{
		pFilter->type = (enum event_type)pEventLogFilter->getType();
		pFilter->filter_mask |= NVM_FILTER_ON_TYPE;
	}
	if (pEventLogFilter->hasCode())
	{
		pFilter->code = pEventLogFilter->getCode();
		pFilter->filter_mask |= NVM_FILTER_ON_CODE;
	}
	if (pEventLogFilter->hasUid())
	{
		uid_copy(pEventLogFilter->getUid().c_str(), pFilter->uid);
		pFilter->filter_mask |= NVM_FILTER_ON_UID;
	}
	if (pEventLogFilter->hasActionRequired())
	{
		pFilter->action_required = pEventLogFilter->getActionRequired();
		pFilter->filter_mask |= NVM_FILTER_ON_AR;
	}
	if (pEventLogFilter->hasBeforeTimestamp())
	{
		pFilter->before = pEventLogFilter->getBeforeTimestamp();
		pFilter->filter_mask |= NVM_FILTER_ON_BEFORE;
	}
	if (pEventLogFilter->hasAfterTimestamp())
	{
		pFilter->after = pEventLogFilter->getAfterTimestamp();
		pFilter->filter_mask |= NVM_FILTER_ON_AFTER ;
	}

	COMMON_LOG_EXIT();
}

/*
 * Returns number of events found, or throws error
 */
wbem::framework::instances_t * wbem::support::NVDIMMEventLogFactory::getFilteredEvents(EventLogFilter *pEventLogFilter)
	throw (framework::Exception)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	wbem::framework::instances_t *pFilteredInstances = new wbem::framework::instances_t();
	struct event *pEvents = NULL;
	try
	{
		// Construct the event_filter
		struct event_filter filter;
		memset(&filter, 0, sizeof (filter));
		struct event_filter *pFilter = NULL; // pointer to the final filter - could be NULL
		if (pEventLogFilter)
		{
			pFilter = &filter;
			convertFactoryToEventFilter(pEventLogFilter, pFilter);
		}

		if ((rc = nvm_get_event_count(pFilter)) < NVM_SUCCESS)
		{
			// couldn't get the events
			throw exception::NvmExceptionLibError(rc);
		}
		else if (rc > 0)
		{
			int count = rc;
			pEvents = (struct event *)malloc(count * (sizeof (struct event)));

			if (pEvents == NULL)
			{
				throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "couldn't allocate pEvents");
			}

			if ((rc = nvm_get_events(pFilter, pEvents, count)) != count)
			{
				throw exception::NvmExceptionLibError(rc);
			}

			wbem::framework::attribute_names_t attributes;
			wbem::support::NVDIMMLogEntryFactory::populateAttributes(attributes);

			// Get host name for object path attributes
			std::string hostName = server::getHostName();

			for (int i = 0; i < count; i++)
			{
				// convert each event to an Instance
				framework::attributes_t keys;
				std::stringstream instanceIdStr;
				instanceIdStr << pEvents[i].event_id;
				keys[INSTANCEID_KEY] = framework::Attribute(instanceIdStr.str(), true);
				framework::ObjectPath path(hostName, NVM_NAMESPACE,
						support::NVDIMMLOGENTRY_CREATIONCLASSNAME, keys);
				framework::Instance instance(path);
				wbem::support::NVDIMMLogEntryFactory::eventToInstance(&instance, &(pEvents[i]), attributes);

				pFilteredInstances->push_back(instance);
			}

			if (pEvents)
			{
				free(pEvents);
			}
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pFilteredInstances)
		{
			delete pFilteredInstances;
		}
		if (pEvents)
		{
			free(pEvents);
		}
		throw;
	}

	COMMON_LOG_EXIT();
	return pFilteredInstances;
}

/*
 * Filter events using a WQL-type conditional and return a list of object paths for resulting
 * events.
 * @param queryStr - conditional string
 * @return NVM_STR_LIST - list of object paths as strings
 * @throw Exception - if the query is invalid
 * @remark A well-formed query may return 0 results if it doesn't apply to the
 * 		LogEntry class or if it describes a filter we don't support.
 */
wbem::framework::STR_LIST wbem::support::NVDIMMEventLogFactory::getFilteredEventRefsFromFilterString(
		const std::string &queryStr)
	throw (wbem::framework::Exception)
{
	COMMON_LOG_ENTRY();

	framework::STR_LIST logEntryRefs;

	try
	{
		// Parse the query string into a filter
		framework::WqlConditional conditional(queryStr);
		EventLogFilter filter = getEventFilterFromConditional(conditional);

		// Get the filtered events
		framework::instances_t *pInstances = NULL;
		try
		{
			pInstances = getFilteredEvents(&filter);
			if (pInstances)
			{
				for (framework::instances_t::iterator instIter = pInstances->begin();
						instIter != pInstances->end(); instIter++)
				{
					framework::ObjectPath path = instIter->getObjectPath();
					logEntryRefs.push_back(path.asString());
				}

				delete pInstances;
			}
		}
		catch (framework::Exception &)
		{
			if (pInstances) // clean up
			{
				delete pInstances;
			}
			throw;
		}
	}
	catch (framework::ExceptionInvalidWqlQuery &)
	{
		// Indicates invalid query string
		throw framework::ExceptionBadParameter(NVDIMMEVENTLOG_FILTER.c_str());
	}

	COMMON_LOG_EXIT();
	return logEntryRefs;
}

wbem::support::EventLogFilter wbem::support::NVDIMMEventLogFactory::getEventFilterFromConditional(
		const framework::WqlConditional &conditional)
	throw (framework::Exception)
{
	COMMON_LOG_ENTRY();
	EventLogFilter filter;

	// Walk through the comparisons
	const std::vector<struct framework::WqlComparisonClause> &comparisons = conditional.getConditions();
	for (std::vector<struct framework::WqlComparisonClause>::const_iterator iter = comparisons.begin();
			iter != comparisons.end(); iter++)
	{
		// Event filters have limited abilities
		// Attribute names map to specific event filters
		const std::string &attrName = iter->attributeName;
		const framework::Attribute &attr = iter->value;

		// InstanceID (string) -> event sequence number
		if (s_strncmpi(attrName.c_str(), INSTANCEID_KEY.c_str(), attrName.size() + 1) == 0)
		{
			// make sure the attribute is a string
			if (attr.getType() != framework::STR_T)
			{
				COMMON_LOG_ERROR_F("expected value of '%s' in query to be string. Actual type: %d",
						INSTANCEID_KEY.c_str(), attr.getType());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}

			// try to convert the string to a number
			// invalid if the conversion failed, the string wasn't a number, or the number < 0
			errno = 0;
			char *pCh = NULL;
			NVM_UINT32 newValue = strtoul(attr.stringValue().c_str(), &pCh, 0);
			if ((errno != 0) || !pCh || (*pCh != '\0'))
			{
				COMMON_LOG_ERROR_F("invalid ID string '%s'", attr.stringValue().c_str());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}

			// Equality operator is the only one that applies for this property
			if (iter->op == framework::OP_EQ)
			{
				// treat different values like 'AND' - different values => filter would return 0 results
				if (filter.hasEventId() && (newValue != filter.getEventId()))
				{
					COMMON_LOG_ERROR_F("duplicate of '%s' in query",
							INSTANCEID_KEY.c_str());
					throw exception::NvmExceptionBadFilter(conditional.getString());
				}
				filter.setEventId(newValue);
			}
			else
			{
				COMMON_LOG_ERROR_F("invalid operator type: %d for attribute '%s'",
						iter->op, INSTANCEID_KEY.c_str());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}
		}
		// PerceivedSeverity (uint16) -> severity
		else if (s_strncmpi(attrName.c_str(), PERCEIVEDSEVERITY_KEY.c_str(), attrName.size() + 1) == 0)
		{
			// make sure the attribute is a numeric value
			if (!attr.isNumeric())
			{
				COMMON_LOG_ERROR_F("expected value of '%s' in query to be numeric. Actual type: %d",
						PERCEIVEDSEVERITY_KEY.c_str(), attr.getType());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}

			// We can only filter on severity >= value
			if (iter->op == framework::OP_GE)
			{
				NVM_UINT16 newValue = attr.uintValue();
				// treat different values like 'AND' - different values => filter would return 0 results
				if (!filter.hasSeverity() || (newValue > filter.getSeverity()))
				{
					filter.setSeverity(newValue);
				}
			}
			else
			{
				COMMON_LOG_ERROR_F("invalid operator type: %d for attribute '%s'",
						iter->op, PERCEIVEDSEVERITY_KEY.c_str());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}
		}
		// MessageID (string) -> type
		else if (s_strncmpi(attrName.c_str(), MESSAGEID_KEY.c_str(), attrName.size() + 1) == 0)
		{
			// make sure the attribute is a string
			if (attr.getType() != framework::STR_T)
			{
				COMMON_LOG_ERROR_F("expected value of '%s' in query to be string. Actual type: %d",
						INSTANCEID_KEY.c_str(), attr.getType());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}

			// Equality operator is the only one that applies for this property
			if (iter->op == framework::OP_EQ)
			{
				NVM_UINT32 newValue = NVDIMMLogEntryFactory::eventTypeStringToType(attr.stringValue());
				if (newValue == (NVM_UINT32)-1) // flag value - unrecognized string
				{
					COMMON_LOG_ERROR_F("invalid value associated with '%s' in query",
							MESSAGEID_KEY.c_str());
					throw exception::NvmExceptionBadFilter(conditional.getString());
				}

				// treat different values like 'AND' - different values => filter would return 0 results
				if (filter.hasType() && (newValue != filter.getType()))
				{
					COMMON_LOG_ERROR_F("duplicate of '%s' in query",
							MESSAGEID_KEY.c_str());
					throw exception::NvmExceptionBadFilter(conditional.getString());
				}
				filter.setType(newValue);
			}
			else
			{
				COMMON_LOG_ERROR_F("invalid operator type: %d for attribute '%s'",
						iter->op, MESSAGEID_KEY.c_str());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}
		}
		// ActionRequired (bool) -> action required
		else if (s_strncmpi(attrName.c_str(), ACTIONREQUIRED_KEY.c_str(), attrName.size() + 1) == 0)
		{
			// make sure the attribute is a boolean value
			if (attr.getType() != framework::BOOLEAN_T)
			{
				COMMON_LOG_ERROR_F("expected value of '%s' in query to be bool or numeric. Actual type: %d",
						ACTIONREQUIRED_KEY.c_str(), attr.getType());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}

			// Equality operator is the only one that applies for this property
			if (iter->op == framework::OP_EQ)
			{
				bool newValue = attr.boolValue();

				// treat different values like 'AND' - different values => filter would return 0 results
				if (filter.hasActionRequired() && (newValue != filter.getActionRequired()))
				{
					COMMON_LOG_ERROR_F("duplicate of '%s' in query",
							ACTIONREQUIRED_KEY.c_str());
					throw exception::NvmExceptionBadFilter(conditional.getString());
				}
				filter.setActionRequired(attr.boolValue());
			}
			else
			{
				COMMON_LOG_ERROR_F("invalid operator type: %d for attribute '%s'",
						iter->op, ACTIONREQUIRED_KEY.c_str());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}
		}
		// CreationTimeStamp (datetime) -> time before and time after
		else if (s_strncmpi(attrName.c_str(), CREATIONTIMESTAMP_KEY.c_str(), attrName.size() + 1) == 0)
		{
			// make sure the attribute is a datetime or numeric value
			if (attr.getType() != framework::DATETIME_T)
			{
				COMMON_LOG_ERROR_F("expected value of '%s' in query to be datetime. Actual type: %d",
						CREATIONTIMESTAMP_KEY.c_str(), attr.getType());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}

			time_t newValue = (time_t)attr.uint64Value();
			// Greater than - time after
			if (iter->op == framework::OP_GT)
			{
				// Valid AND statement - but we ignore whichever value is lesser
				if (!filter.hasAfterTimestamp() || (newValue > filter.getAfterTimestamp()))
				{
					filter.setAfterTimestamp(newValue);
				}
			}
			// Less than - time before
			else if (iter->op == framework::OP_LT)
			{
				// Valid AND statement - but we ignore whichever value is greater
				if (!filter.hasBeforeTimestamp() || (newValue < filter.getBeforeTimestamp()))
				{
					filter.setBeforeTimestamp(newValue);
				}
			}
			else
			{
				COMMON_LOG_ERROR_F("invalid operator type: %d for attribute '%s'",
						iter->op, CREATIONTIMESTAMP_KEY.c_str());
				throw exception::NvmExceptionBadFilter(conditional.getString());
			}
		}
		// Not a parameter we can filter on
		else
		{
			COMMON_LOG_ERROR_F("cannot filter events on attribute '%s'",
					attrName.c_str());
			throw exception::NvmExceptionBadFilter(conditional.getString());
		}
	}

	COMMON_LOG_EXIT();
	return filter;
}
