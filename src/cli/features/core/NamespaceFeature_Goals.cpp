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
 * This file contains the implementation of the NVMCLI commands to create, delete,
 * and show configuration goals.
 */

#include "NamespaceFeature.h"
#include <LogEnterExit.h>
#include <string/s_str.h>
#include <uid/uid.h>
#include <core/memory_allocator/MemoryAllocator.h>
#include <core/memory_allocator/ReserveDimmSelector.h>
#include <mem_config/MemoryConfigurationFactory.h>
#include <pmem_config/PersistentMemoryCapabilitiesFactory.h>
#include <memory/SystemProcessorFactory.h>
#include <libinvm-cim/ExceptionBadAttribute.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include "CommandParts.h"
#include <libinvm-cli/SimpleListResult.h>
#include <libinvm-cli/CliFrameworkTypes.h>
#include <exception/NvmExceptionLibError.h>

/*
 * Helper function to convert a MemoryConfiguration config goal instance into a CLI config goal
 * display instance
 */
bool cli::nvmcli::NamespaceFeature::convertConfigGoalInstance(
		const wbem::framework::Instance *pWbemInstance,
		wbem::framework::Instance *pCliInstance,
		const wbem::framework::attribute_names_t &displayAttributes)
{
	bool rc = true;

	try
	{
		wbem::framework::Attribute parentAttr;
		if (pWbemInstance->getAttribute(wbem::PARENT_KEY,
				parentAttr) != wbem::framework::SUCCESS)
		{
			// invalid wbem instance, throw exception and
			// bypass the rest of method instead of nested if/else
			throw false;
		}
		// add the UID for filtering
		pCliInstance->setAttribute(wbem::DIMMUID_KEY,
				parentAttr);

		// Add dimm ID
		wbem::framework::Attribute dimmIdAttr =
				wbem::physical_asset::NVDIMMFactory::uidToDimmIdAttribute(parentAttr.stringValue());
		pCliInstance->setAttribute(wbem::DIMMID_KEY,
					dimmIdAttr,
					displayAttributes);

		// SocketID
		wbem::framework::Attribute socketIdAttr;
		if (pWbemInstance->getAttribute(wbem::SOCKETID_KEY,
				socketIdAttr) != wbem::framework::SUCCESS)
		{
			throw false;
		}
		pCliInstance->setAttribute(wbem::SOCKETID_KEY, socketIdAttr, displayAttributes);

		// MemorySize
		wbem::framework::Attribute memorySizeAttr;
		if (pWbemInstance->getAttribute(wbem::MEMORYSIZE_KEY,
				memorySizeAttr) != wbem::framework::SUCCESS)
		{
			throw false;
		}
		pCliInstance->setAttribute(wbem::MEMORYSIZE_KEY,
				wbem::framework::Attribute(memorySizeAttr.uint64Value(), false),
				displayAttributes);
		cli::nvmcli::convertCapacityAttribute(*pCliInstance, wbem::MEMORYSIZE_KEY);

		// AppDirect1Size, AppDirect1Settings, AppDirect2Size, AppDirect2Settings
		// all come from the interleave format, size and redundancy vectors
		// gather all the vectors first

		// Interleave format vector
		wbem::framework::Attribute intFormatsAttr;
		if (pWbemInstance->getAttribute(wbem::INTERLEAVEFORMATS_KEY,
				intFormatsAttr) != wbem::framework::SUCCESS)
		{
			throw false;
		}
		wbem::framework::UINT32_LIST intFormatList = intFormatsAttr.uint32ListValue();

		// Interleave Size vector
		wbem::framework::Attribute intSizesAttr;
		if (pWbemInstance->getAttribute(wbem::INTERLEAVESIZES_KEY,
				intSizesAttr) != wbem::framework::SUCCESS)
		{
			throw false;
		}
		wbem::framework::UINT64_LIST intSizeList = intSizesAttr.uint64ListValue();

		// Package Redundancy vector
		wbem::framework::Attribute mirrorsAttr;
		if (pWbemInstance->getAttribute(wbem::PACKAGEREDUNDANCY_KEY,
				mirrorsAttr) != wbem::framework::SUCCESS)
		{
			throw false;
		}
		wbem::framework::UINT16_LIST mirrorsList = mirrorsAttr.uint16ListValue();

		// Interleave Index vector
		wbem::framework::Attribute intIndexesAttr;
		if (pWbemInstance->getAttribute(wbem::INTERLEAVEINDEXES_KEY,
				intIndexesAttr) != wbem::framework::SUCCESS)
		{
			throw false;
		}
		wbem::framework::UINT16_LIST intIndexList = intIndexesAttr.uint16ListValue();

		NVM_UINT16 appDirect1Index = 0;
		NVM_UINT64 appDirect1SizeB = 0;
		bool appDirect1Mirrored = false;
		NVM_UINT32 appDirect1Settings = 0;
		NVM_UINT16 appDirect2Index = 0;
		NVM_UINT64 appDirect2SizeB = 0;
		bool appDirect2Mirrored = false;
		NVM_UINT32 appDirect2Settings = 0;
		if (intFormatList.size() >= 1u &&
				intSizeList.size() >= 1u &&
				mirrorsList.size() >= 1u &&
				intIndexList.size() >= 1u)
		{
			appDirect1Index = intIndexList[0];
			appDirect1Settings = intFormatList[0];
			appDirect1SizeB = intSizeList[0];
			appDirect1Mirrored = (mirrorsList[0] == 1);
		}
		if (intFormatList.size() >= 2u &&
					intSizeList.size() >= 2u &&
					mirrorsList.size() >= 2u &&
					intIndexList.size() >= 2u)
		{
			appDirect2Index = intIndexList[1];
			appDirect2Settings = intFormatList[1];
			appDirect2SizeB = intSizeList[1];
			appDirect2Mirrored = (mirrorsList[1] == 1);
		}

		pCliInstance->setAttribute(wbem::APPDIRECT1SIZE_KEY,
				wbem::framework::Attribute(appDirect1SizeB, false),
				displayAttributes);
		cli::nvmcli::convertCapacityAttribute(*pCliInstance, wbem::APPDIRECT1SIZE_KEY);

		// add mirror string
		if (appDirect1Mirrored)
		{
			wbem::framework::Attribute appDirectAttr;
			if (pCliInstance->getAttribute(wbem::APPDIRECT1SIZE_KEY,
				appDirectAttr) == wbem::framework::SUCCESS)
			{
				std::stringstream appDirectSizeStr;
				appDirectSizeStr << appDirectAttr.stringValue()<< cli::nvmcli::MIRRORED_CAPACITY;
				pCliInstance->setAttribute(wbem::APPDIRECT1SIZE_KEY,
						wbem::framework::Attribute(appDirectSizeStr.str(), false), displayAttributes);
			}
		}

		// Interleave1Index
		pCliInstance->setAttribute(wbem::APPDIRECT1INDEX_KEY,
				wbem::framework::Attribute(appDirect1Index, false),
				displayAttributes);

		// AppDirect1Settings
		// convert NVM_UINT32 to struct
		pCliInstance->setAttribute(wbem::APPDIRECT1SETTINGS_KEY,
				wbem::framework::Attribute(wbem::mem_config::InterleaveSet::getInterleaveFormatStringFromInt(appDirect1Settings), false),
				displayAttributes);

		pCliInstance->setAttribute(wbem::APPDIRECT2SIZE_KEY,
				wbem::framework::Attribute(appDirect2SizeB, false), displayAttributes);
		cli::nvmcli::convertCapacityAttribute(*pCliInstance, wbem::APPDIRECT2SIZE_KEY);

		// add mirror string
		if (appDirect2Mirrored)
		{
			wbem::framework::Attribute appDirect2Attr;
			if (pCliInstance->getAttribute(wbem::APPDIRECT2SIZE_KEY,
				appDirect2Attr) == wbem::framework::SUCCESS)
			{
				std::stringstream appDirect2SizeStr;
				appDirect2SizeStr << appDirect2Attr.stringValue()<< cli::nvmcli::MIRRORED_CAPACITY;
				pCliInstance->setAttribute(wbem::APPDIRECT2SIZE_KEY,
					wbem::framework::Attribute(appDirect2SizeStr.str(), false), displayAttributes);
			}
		}

		// AppDirect2Index
		pCliInstance->setAttribute(wbem::APPDIRECT2INDEX_KEY,
				wbem::framework::Attribute(appDirect2Index, false),
				displayAttributes);

		// AppDirect2Settings
		pCliInstance->setAttribute(wbem::APPDIRECT2SETTINGS_KEY,
				wbem::framework::Attribute(wbem::mem_config::InterleaveSet::getInterleaveFormatStringFromInt(appDirect2Settings), false),
				displayAttributes);

		// StorageCapacity
		wbem::framework::Attribute storageCapacityAttr;
		if (pWbemInstance->getAttribute(wbem::STORAGECAPACITY_KEY,
				storageCapacityAttr) != wbem::framework::SUCCESS)
		{
			throw false;
		}
		pCliInstance->setAttribute(wbem::STORAGECAPACITY_KEY,
				wbem::framework::Attribute(storageCapacityAttr.uint64Value(), false),
				displayAttributes);
		cli::nvmcli::convertCapacityAttribute(*pCliInstance, wbem::STORAGECAPACITY_KEY);

		// Status - already set to a translated string by the view class
		wbem::framework::Attribute statusAttr;
		if (pWbemInstance->getAttribute(wbem::STATUS_KEY,
				statusAttr) != wbem::framework::SUCCESS)
		{
			throw false;
		}
		pCliInstance->setAttribute(wbem::STATUS_KEY, statusAttr,
				displayAttributes);

		// action required
		struct event_filter filter;
		memset(&filter, 0, sizeof (filter));
		filter.filter_mask = NVM_FILTER_ON_UID | NVM_FILTER_ON_AR | NVM_FILTER_ON_TYPE;
		// uid to str
		NVM_UID dimmUid;
		uid_copy(parentAttr.stringValue().c_str(), dimmUid);
		memmove(filter.uid, dimmUid, NVM_MAX_UID_LEN);
		filter.action_required = true;
		filter.type = EVENT_TYPE_CONFIG;

		// only query db if attribute is requested
		if (std::find(displayAttributes.begin(), displayAttributes.end(),
				wbem::ACTIONREQUIRED_KEY) != displayAttributes.end())
		{
			int eventCount = 0;
			eventCount = nvm_get_event_count(&filter);
			if (eventCount < 0)
			{
				COMMON_LOG_ERROR_F("Failed to retrieve the action required events "
						"for the config goal on dimm %s, error %d",
						parentAttr.stringValue().c_str(), eventCount);
				throw wbem::exception::NvmExceptionLibError(eventCount);
			}

			pCliInstance->setAttribute(wbem::ACTIONREQUIRED_KEY,
					wbem::framework::Attribute(eventCount > 0 ? true : false, false),
					displayAttributes);
		}

		// action required event list
		int eventCount = 0;
		eventCount = nvm_get_event_count(&filter);
		if (eventCount < 0)
		{
			COMMON_LOG_ERROR_F("Failed to retrieve the action required events "
					"for the config goal on dimm %s, error %d",
					parentAttr.stringValue().c_str(), eventCount);
			throw wbem::exception::NvmExceptionLibError(eventCount);
		}
		wbem::framework::STR_LIST arEventList;
		if (eventCount > 0)
		{
			// get the events
			struct event events[eventCount];
			eventCount = nvm_get_events(&filter, events, eventCount);
			if (eventCount < 0)
			{
				COMMON_LOG_ERROR_F("Failed to retrieve the action required events "
						"for the config goal on dimm %s, error %d",
						parentAttr.stringValue().c_str(), eventCount);
				throw wbem::exception::NvmExceptionLibError(eventCount);
			}

			for (int i = 0; i < eventCount; i++)
			{
				std::stringstream eventMsg;
				eventMsg << "Event " << events[i].event_id;
				char msg[NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)];
				s_snprintf(msg, (NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN)),
						events[i].message,
						events[i].args[0],
						events[i].args[1],
						events[i].args[2]);
				eventMsg << " - " << msg;
				arEventList.push_back(eventMsg.str());
			}
		}
		else
		{
			arEventList.push_back(wbem::NA);
		}
		pCliInstance->setAttribute(wbem::ACTIONREQUIREDEVENTS_KEY,
				wbem::framework::Attribute(arEventList, false),
				displayAttributes);
	}
	catch (bool)
	{
		rc = false;
	}
	return rc;
}

void cli::nvmcli::NamespaceFeature::validateRequestedDisplayOptions(
	std::map<std::string, std::string> options,
	wbem::framework::attribute_names_t allAttributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::framework::attribute_names_t displayAttributes = GetAttributeNames(options);

	// compare requested display attributes to valid display attributes
	for (wbem::framework::attribute_names_t::const_iterator displayIter = displayAttributes.begin();
			displayIter != displayAttributes.end(); displayIter++)
	{
		bool found = false;
		for (wbem::framework::attribute_names_t::const_iterator allIter = allAttributes.begin();
			allIter != allAttributes.end(); allIter++)
		{
			if (framework::stringsIEqual(displayIter->c_str(), allIter->c_str()))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			throw wbem::framework::ExceptionBadAttribute(displayIter->c_str());
		}
	}
}

void cli::nvmcli::NamespaceFeature::populateCreateConfigGoalPromptDefaultAttributes(
		wbem::framework::attribute_names_t& defaultAttributes)
{
	defaultAttributes.push_back(wbem::SOCKETID_KEY);
	defaultAttributes.push_back(wbem::DIMMID_KEY);
	defaultAttributes.push_back(wbem::MEMORYSIZE_KEY);
	defaultAttributes.push_back(wbem::APPDIRECT1SIZE_KEY);
	defaultAttributes.push_back(wbem::APPDIRECT2SIZE_KEY);
	defaultAttributes.push_back(wbem::STORAGECAPACITY_KEY);
}

void cli::nvmcli::NamespaceFeature::populateCurrentConfigGoalDefaultAttributes(
		wbem::framework::attribute_names_t& defaultAttributes)
{
	defaultAttributes.push_back(wbem::SOCKETID_KEY);
	defaultAttributes.push_back(wbem::DIMMID_KEY);
	defaultAttributes.push_back(wbem::MEMORYSIZE_KEY);
	defaultAttributes.push_back(wbem::APPDIRECT1SIZE_KEY);
	defaultAttributes.push_back(wbem::APPDIRECT2SIZE_KEY);
	defaultAttributes.push_back(wbem::STORAGECAPACITY_KEY);
	defaultAttributes.push_back(wbem::ACTIONREQUIRED_KEY);

}

void cli::nvmcli::NamespaceFeature::populateAllAttributes(
		wbem::framework::attribute_names_t& allAttributes)
{
	allAttributes.push_back(wbem::SOCKETID_KEY);
	allAttributes.push_back(wbem::DIMMID_KEY);
	allAttributes.push_back(wbem::MEMORYSIZE_KEY);
	allAttributes.push_back(wbem::APPDIRECT1SIZE_KEY);
	allAttributes.push_back(wbem::APPDIRECT1INDEX_KEY);
	allAttributes.push_back(wbem::APPDIRECT1SETTINGS_KEY);
	allAttributes.push_back(wbem::APPDIRECT2SIZE_KEY);
	allAttributes.push_back(wbem::APPDIRECT2INDEX_KEY);
	allAttributes.push_back(wbem::APPDIRECT2SETTINGS_KEY);
	allAttributes.push_back(wbem::STORAGECAPACITY_KEY);
	allAttributes.push_back(wbem::STATUS_KEY);
	allAttributes.push_back(wbem::ACTIONREQUIRED_KEY);
	allAttributes.push_back(wbem::ACTIONREQUIREDEVENTS_KEY);
}

void cli::nvmcli::NamespaceFeature::generateCliDisplayInstances(
		wbem::framework::instances_t *pWbemInstances,
		wbem::framework::instances_t& displayInstances,
		wbem::framework::attribute_names_t& displayAttributes)
{
	for (wbem::framework::instances_t::const_iterator iter = pWbemInstances->begin();
			iter != pWbemInstances->end(); iter++)
	{
		wbem::framework::Instance displayInstance;
		if (convertConfigGoalInstance(&(*iter), &displayInstance, displayAttributes))
		{
			displayInstances.push_back(displayInstance);
		}
	}

}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::showConfigGoal(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	try
	{
		wbem::framework::attribute_names_t allAttributes;
		populateAllAttributes(allAttributes);

		validateRequestedDisplayOptions(parsedCommand.options, allAttributes);

		// get all attributes of the goal instances of only unapplied goals
		wbem::mem_config::MemoryConfigurationFactory isetProvider;
		wbem::framework::attribute_names_t attributes;
		wbem::framework::instances_t *pWbemInstances = isetProvider.getGoalInstances(attributes, true);
		if (!pWbemInstances)
		{
			if (pResult)
			{
				delete pResult;
			}
			pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
					TRS(nvmcli::UNKNOWN_ERROR_STR));
		}
		else
		{
			// set up display attributes based on what the user passed in
			wbem::framework::attribute_names_t defaultAttributes;
			populateCurrentConfigGoalDefaultAttributes(defaultAttributes);
			wbem::framework::attribute_names_t displayAttributes =
					GetAttributeNames(parsedCommand.options, defaultAttributes, allAttributes);

			// generate the display filters
			cli::nvmcli::filters_t filters;
			generateDimmFilter(parsedCommand, attributes, filters);
			generateSocketFilter(parsedCommand, attributes, filters);

			// include DimmID in our display for when specific display attributes are requested
			if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(
					wbem::DIMMID_KEY, displayAttributes))
			{
				displayAttributes.insert(displayAttributes.begin(), wbem::DIMMID_KEY);
			}

			pResult = showConfigGoalForInstances(filters, displayAttributes, pWbemInstances);

			// set layout to table unless the -all option is present
			if (parsedCommand.options.find(framework::OPTION_ALL.name)
					== parsedCommand.options.end())
			{
				pResult->setOutputType(framework::ResultBase::OUTPUT_TEXTTABLE);
			}
			delete pWbemInstances;
		}
	}
	catch (wbem::framework::Exception &e)
	{
		if (pResult)
		{
			delete pResult;
		}
		pResult = NvmExceptionToResult(e);
	}
	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::showConfigGoalForInstances(
		const cli::nvmcli::filters_t &filters,
		wbem::framework::attribute_names_t &displayAttributes,
		wbem::framework::instances_t *pWbemInstances)
{
	framework::ResultBase *pResult = NULL;

	wbem::framework::instances_t displayInstances;
	generateCliDisplayInstances(pWbemInstances, displayInstances, displayAttributes);

	pResult = NvmInstanceToObjectListResult(displayInstances, wbem::CONFIGGOALTABLENAME,
			wbem::DIMMID_KEY, displayAttributes, filters);

	return pResult;
}

/*
 * Check to see if the user is violating the rule for deleting goal instances.
 * Basically, the rule is that the user may not delete part of a configured socket.
 * If the user asks to delete specific dimms then they must delete the goals of all
 * configured dimms on a socket.
 */
bool cli::nvmcli::NamespaceFeature::validRequest(const wbem::framework::instances_t &allInstances,
						const wbem::framework::instances_t &requestedInstances)
{
	bool result = true;
	wbem::framework::instances_t::const_iterator requestIter = requestedInstances.begin();

	for (; requestIter != requestedInstances.end() && result; requestIter++)
	{
		wbem::framework::Attribute requestIdAttr;
		if (requestIter->getAttribute(wbem::SOCKETID_KEY, requestIdAttr)
					== wbem::framework::SUCCESS)
		{
			int requestedSocket = requestIdAttr.uintValue();
			wbem::framework::instances_t::const_iterator iter = allInstances.begin();
			for(; iter != allInstances.end(); iter++)
			{
				wbem::framework::Attribute idAttr;
				if (iter->getAttribute(wbem::SOCKETID_KEY, idAttr)
						== wbem::framework::SUCCESS)
				{
					int socket = idAttr.uintValue();
					if (socket == requestedSocket)
					{
						wbem::framework::instances_t::const_iterator found;
						found = std::find(requestedInstances.begin(),
									requestedInstances.end(), *iter);
						if (found == requestedInstances.end())
						{
							result = false;
							break;
						}
					}
				}
				else
				{
					throw wbem::framework::ExceptionBadAttribute(
							wbem::SOCKETID_KEY.c_str());
				}
			}
		}
		else
		{
			throw wbem::framework::ExceptionBadAttribute(
							wbem::SOCKETID_KEY.c_str());
		}
	}
	return result;
}

/*
 * Delete a configuration goal from a list of sockets or list of dimms
 */
cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::deleteConfigGoal(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string basePrefix = TRS(DELETECONFIGGOAL_FROMDIMM_MSG);

	wbem::framework::instances_t *pInstances = NULL;
	wbem::framework::instances_t matchedInstances;
	try
	{
		// Get the instances
		wbem::mem_config::MemoryConfigurationFactory provider;
		wbem::framework::attribute_names_t attributes;
		// get applied and unapplied goals
		pInstances = provider.getGoalInstances(attributes, false);
		if (!pInstances)
		{
			throw wbem::framework::Exception("MemoryConfigurationFactory.getInstances returned NULL");
		}

		if (pInstances->size() == 0)
		{
			pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				"The requested " NVM_DIMM_NAME "(s) are not configured.", basePrefix);
		}
		else
		{
			// rename the Parent to UID to work with filterInstances
			RenameAttributeKey(*pInstances, wbem::PARENT_KEY, wbem::DIMMUID_KEY);

			cli::nvmcli::filters_t filters;

			generateDimmFilter(parsedCommand, attributes, filters);
			generateSocketFilter(parsedCommand, attributes, filters);
			filterInstances(*pInstances, wbem::CONFIGGOALTABLENAME, filters, matchedInstances);

			if (!validRequest(*pInstances, matchedInstances))
			{
				pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
					"The configuration goals from all the " NVM_DIMM_NAME "s on the "
					"socket must be deleted together.", basePrefix);
			}
			else if (matchedInstances.size() == 0)
			{
				// verify the user entered correct Dimm IDs
				std::vector<std::string> dimmList;
				pResult = getDimms(parsedCommand, dimmList);
				if (pResult == NULL)
				{
					// verify the user entered correct sockets
					pResult = getDimmsFromSockets(parsedCommand, dimmList);
					// requested dimms/sockets are valid but don't have goals
					if (pResult == NULL)
					{
						pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
							"The requested " NVM_DIMM_NAME "(s) are not configured.", basePrefix);
					}
				}
			}
		}
	}
	catch (wbem::framework::Exception &e)
	{
		if (pResult)
		{
			delete pResult;
			pResult = NULL;
		}
		pResult = NvmExceptionToResult(e);
	}

	if (pResult == NULL)
	{
		framework::SimpleListResult *pListResults = new framework::SimpleListResult();

		wbem::framework::instances_t::iterator iter;
		for (iter = matchedInstances.begin();
				iter != matchedInstances.end(); iter++)
		{
			wbem::framework::Attribute idAttr;
			iter->getAttribute(wbem::INSTANCEID_KEY, idAttr);
			std::string instanceId = idAttr.stringValue();
			std::string dimmId = instanceId.substr(0, instanceId.length() - 1);
			std::string prefix = cli::framework::ResultBase::stringFromArgList(
						(basePrefix + " %s").c_str(),
						wbem::physical_asset::NVDIMMFactory::uidToDimmIdStr(dimmId).c_str()) + ": ";

			wbem::framework::ObjectPath path = (*iter).getObjectPath();

			try
			{
				wbem::mem_config::MemoryConfigurationServiceFactory provider;
				provider.deleteInstance(path);
				pListResults->insert(prefix + TRS(cli::framework::SUCCESS_MSG));
			}
			catch (wbem::framework::Exception &e)
			{
				framework::ErrorResult *pResult = NvmExceptionToResult(e);
				pListResults->insert(prefix + pResult->outputText());
				if (!pListResults->getErrorCode()) // keep existing errors
				{
					pListResults->setErrorCode(pResult->getErrorCode());
				}
				delete pResult;
			}
		}
		pResult = pListResults;
	}

	if (pInstances)
	{
		delete pInstances;
	}
	return pResult;
}

std::string cli::nvmcli::NamespaceFeature::getPromptStringForLayout(
		const core::memory_allocator::MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream promptStr;
	wbem::framework::instances_t *pGoalInstances = NULL;
	cli::framework::ResultBase *pDisplayGoal = NULL;

	try
	{
		wbem::framework::attribute_names_t attributes;
		wbem::mem_config::MemoryConfigurationFactory configFactory;
		pGoalInstances = configFactory.getInstancesFromLayout(layout, attributes);

		filters_t noFilters;
		wbem::framework::attribute_names_t defaultAttributes;
		populateCreateConfigGoalPromptDefaultAttributes(defaultAttributes);
		pDisplayGoal = showConfigGoalForInstances(noFilters, defaultAttributes, pGoalInstances);
		pDisplayGoal->setOutputType(framework::ResultBase::OUTPUT_TEXTTABLE);

		promptStr << CREATE_GOAL_CONFIRMATION_PREFIX << std::endl << std::endl;
		promptStr << pDisplayGoal->output() << std::endl;

		int warningsAdded = 0;
		for (std::vector<enum core::memory_allocator::LayoutWarningCode>::const_iterator warningIter =
			layout.warnings.begin(); warningIter != layout.warnings.end(); warningIter++)
		{
			std::string warningStr = getStringForLayoutWarning(*warningIter);
			if (!warningStr.empty())
			{
				warningsAdded++;
				promptStr << warningStr << std::endl;
			}
		}

		if (warningsAdded > 0)
		{
			promptStr << std::endl;
		}

		promptStr << CREATE_GOAL_CONFIRMATION_SUFFIX;

		delete pDisplayGoal;
		delete pGoalInstances;
	}
	catch (wbem::framework::Exception &)
	{
		if  (pDisplayGoal)
		{
			delete pDisplayGoal;
		}
		if (pGoalInstances)
		{
			delete pGoalInstances;
		}
		throw;
	}

	return promptStr.str();
}

std::string cli::nvmcli::NamespaceFeature::getStringForLayoutWarning(
		enum core::memory_allocator::LayoutWarningCode warningCode)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string warningStr;

	switch (warningCode)
	{
	case core::memory_allocator::LAYOUT_WARNING_APP_DIRECT_NOT_SUPPORTED_BY_DRIVER:
		warningStr = CREATE_GOAL_APP_DIRECT_NOT_SUPPORTED_BY_DRIVER_WARNING;
		break;
	case core::memory_allocator::LAYOUT_WARNING_STORAGE_NOT_SUPPORTED_BY_DRIVER:
		warningStr = CREATE_GOAL_STORAGE_ONLY_NOT_SUPPORTED_BY_DRIVER_WARNING;
		break;
	case core::memory_allocator::LAYOUT_WARNING_APP_DIRECT_SETTINGS_NOT_RECOMMENDED:
		warningStr = CREATE_GOAL_APP_DIRECT_SETTINGS_NOT_RECOMMENDED_BY_BIOS_WARNING;
		break;
	case core::memory_allocator::LAYOUT_WARNING_NONOPTIMAL_POPULATION:
		warningStr = CREATE_GOAL_NON_OPTIMAL_DIMM_POPULATION_WARNING;
		break;
	case core::memory_allocator::LAYOUT_WARNING_REQUESTED_MEMORY_MODE_NOT_USABLE:
		warningStr = CREATE_GOAL_REQUESTED_MEMORY_MODE_NOT_USABLE_WARNING;
		break;
	default:
		COMMON_LOG_ERROR_F("Unrecognized layout warning code: %d", warningCode);
		warningStr = "";
	}

	return warningStr;
}

/*
 * Dump the current config into a file.
 */
cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::dumpConfig(
		const framework::ParsedCommand &parsedCommand)
{
	framework::ResultBase *pResult = NULL;

	// Fetch the destination file path
	std::string path = cli::framework::Parser::getOptionValue(parsedCommand, framework::OPTION_DESTINATION_R.name, NULL);

	try
	{
		wbem::mem_config::MemoryConfigurationServiceFactory provider;
		provider.exportSystemConfigToPath(path);
	}
	catch (wbem::framework::Exception &e)
	{
		pResult = dumpConfigNvmExceptionToResult(e);
	}

	if (!pResult) // no errors
	{
		framework::SimpleListResult *pSimpleList = new framework::SimpleListResult();
		pSimpleList->insert(TR("Successfully dumped system configuration to file: ") + path);
		pResult = pSimpleList;
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::dumpConfigNvmExceptionToResult(
		wbem::framework::Exception& e)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string prefix = TRS(DUMPSYSTEMCONFIG_FROMDIMM_MSG);
	wbem::exception::NvmExceptionLibError * pLibError =
			dynamic_cast<wbem::exception::NvmExceptionLibError *>(&e);
	if (pLibError)
	{
		switch (pLibError->getLibError())
		{
			case NVM_ERR_NOTFOUND:
				return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
					"The requested " NVM_DIMM_NAME "(s) are not configured.",
					prefix);
				break;

			default:
				NVM_ERROR_DESCRIPTION errStr;
				nvm_get_error((return_code)pLibError->getLibError(), errStr, NVM_ERROR_LEN);
				return new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN, errStr, prefix);
				break;
		}
	}

	return NvmExceptionToResult(e);
}

/*
 * Load the config goal from a file
 */
cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::loadGoal(const framework::ParsedCommand &parsedCommand)
{
	framework::ResultBase *pResult = NULL;

	std::string path = framework::Parser::getOptionValue(parsedCommand, framework::OPTION_SOURCE_R.name, NULL);

	// Validate socket and DIMM targets
	bool dimmTargetExists = false;
	bool socketTargetExists = false;
	std::string dimmStr = framework::Parser::getTargetValue(parsedCommand, TARGET_DIMM.name, &dimmTargetExists);
	std::string socketStr = framework::Parser::getTargetValue(parsedCommand, TARGET_SOCKET.name, &socketTargetExists);

	// don't know what to do if -socket and -dimm are both provided
	if (socketTargetExists && dimmTargetExists)
	{
		pResult = new framework::SyntaxErrorResult(framework::ResultBase::stringFromArgList(
				TR("'%s' and '%s' targets cannot be used together."), TARGET_DIMM.name.c_str(),
				TARGET_SOCKET.name.c_str()));
	}
	else
	{
		try
		{
			bool forceOption = parsedCommand.options.find(framework::OPTION_FORCE.name)
					!= parsedCommand.options.end();

			// if user didn't specify the force option, prompt them to continue
			std::string prompt = framework::ResultBase::stringFromArgList(
					LOAD_GOAL_PROMPT.c_str(), path.c_str());
			if (!forceOption && !promptUserYesOrNo(prompt))
			{
				pResult = new framework::SimpleResult(LOAD_CONFIG_GOAL_MSG + cli::framework::UNCHANGED_MSG);
			}
			else
			{
				std::vector<std::string> dimms; // list of DIMM UIDs to be constructed from target list
				if (socketTargetExists) // -socket target provided
				{
					pResult = getDimmsFromSockets(parsedCommand, dimms);
				}
				else // no -socket target, so use -dimm
				{
					pResult = getDimms(parsedCommand, dimms);
				}

				if (!pResult) // no error
				{
					wbem::mem_config::MemoryConfigurationServiceFactory provider;
					provider.importDimmConfigsFromPath(path, dimms);
				}
			}
		}
		catch (wbem::framework::Exception &e)
		{
			if (pResult)
			{
				delete pResult;
			}
			pResult = NvmExceptionToResult(e);
		}
	}

	// Success - show the config goal that was loaded
	if (!pResult)
	{
		pResult = showConfigGoal(parsedCommand);
	}

	return pResult;
}
