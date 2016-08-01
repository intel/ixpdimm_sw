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
 * This file contains the helper functions for interfacing with the NVM Wbem Library.
 */


#include "WbemToCli_utilities.h"
#include <iomanip>
#include <libinvm-cli/CliFrameworkTypes.h>
#include <libinvm-cli/Parser.h>
#include "CommandParts.h"
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cli/ObjectListResult.h>
#include <libinvm-cim/ExceptionBadAttribute.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <libinvm-cim/ExceptionNotSupported.h>
#include <physical_asset/NVDIMMFactory.h>
#include <nvm_management.h>
#include <string.h>
#include <common_types.h>
#include <string/s_str.h>
#include <string/x_str.h>
#include <LogEnterExit.h>
#include <pmem_config/NamespaceViewFactory.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include "FieldSupportFeature.h"
#include <math.h>
#include <utility.h>
#include <exception/NvmExceptionLibError.h>
#include <exception/NvmExceptionInvalidPoolConfig.h>
#include <exception/NvmExceptionBadTarget.h>
#include <exception/NvmExceptionNotManageable.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <core/StringList.h>
#include <cli/features/core/framework/CliHelper.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

cli::framework::PropertyListResult* cli::nvmcli::NvmInstanceToPropertyListResult(
		const wbem::framework::Instance& instance,
		const wbem::framework::attribute_names_t &attributes,
		const std::string name)
{
	framework::PropertyListResult* pResult = new framework::PropertyListResult();
	pResult->setName(name);

	// if the attributes were not specified by the command, then just iterate over
	// what is returned by wbem
	if (attributes.empty())
	{
		wbem::framework::attributes_t::const_iterator attrIter = instance.attributesBegin();
		for (; attrIter != instance.attributesEnd(); attrIter++)
		{
			pResult->insert((*attrIter).first, AttributeToString((*attrIter).second));
		}
	}

	// if the attributes were specified by the command, then keep them in order by iterating
	// over the attribute list
	else
	{
		for (wbem::framework::attribute_names_t::const_iterator iter = attributes.begin();
				iter != attributes.end(); iter++)
		{
			// get the attribute from the instance
			wbem::framework::Attribute attr;
			std::string key = (*iter);
			if (instance.getAttributeI(key, attr) == wbem::framework::SUCCESS)
			{
				pResult->insert(key, AttributeToString(attr));
			}
		}
	}
	return pResult;
}

/*
 * Sets the list of matchedInstances, but returns whether or not all of the filters were valid
 * (matched some existing criteria)
 */
int cli::nvmcli::filterInstances( const wbem::framework::instances_t &instances,
					const std::string &name,
					const cli::nvmcli::filters_t &filters,
					wbem::framework::instances_t &matchedInstances,
					bool checkMatches)
{
	if (filters.empty())
	{
		matchedInstances = instances;
	}
	else
	{
		wbem::framework::attributes_t foundMatch;

		// set up the filter check map - use this to return if we've matched every filter condition
		for (std::vector<struct instanceFilter>::const_iterator filterIter = filters.begin();
						filterIter != filters.end(); filterIter++)
		{
			wbem::framework::Attribute filterAttribute;
			for (std::vector<std::string>::const_iterator filterValueIter = filterIter->attributeValues.begin();
					filterValueIter != filterIter->attributeValues.end(); filterValueIter++)
			{
				std::string foundEntry = filterIter->attributeName + *filterValueIter;
				foundMatch.insert(std::pair<std::string,
						wbem::framework::Attribute>(foundEntry,
						wbem::framework::Attribute(false, false)));
			}
		}

		// find instances that match all filters
		wbem::framework::instances_t::const_iterator instanceIter = instances.begin();
		for ( ;	instanceIter != instances.end(); instanceIter++)
		{
			bool matched = true;
			for (std::vector<struct instanceFilter>::const_iterator filterIter = filters.begin();
							filterIter != filters.end(); filterIter++)
			{
				wbem::framework::Attribute filterAttribute;
				if (instanceIter->getAttribute(filterIter->attributeName, filterAttribute) ==
						wbem::framework::SUCCESS)
				{
					bool matchValue = false;
					// must match one of the specified values
					for (std::vector<std::string>::const_iterator filterValueIter =
							filterIter->attributeValues.begin();
							filterValueIter != filterIter->attributeValues.end(); filterValueIter++)
					{
						std::string foundEntry = filterIter->attributeName + *filterValueIter;
						if (framework::stringsIEqual(AttributeToString(filterAttribute), (*filterValueIter)))
						{
							foundMatch.erase(foundEntry);
							foundMatch.insert(std::pair<std::string,
									wbem::framework::Attribute>(foundEntry,
											wbem::framework::Attribute(true, false)));
							matchValue = true;
							break; // matched - move on to next filter
						}
					}

					if (!matchValue)
					{
						matched = false; // instance does not match any of the specified values
						break; // instance does not match all filters - move on to next instance
					}
				}
			}

			// matches all filters so add it to the list
			if (matched)
			{
				matchedInstances.push_back((*instanceIter));
			}
		}

		if (checkMatches)
		{
			// was every filter valid, that is, every filter matched some valid result
			wbem::framework::attributes_t::iterator foundIter = foundMatch.begin();
			wbem::framework::Attribute attribute;
			for ( ;	foundIter != foundMatch.end(); foundIter++)
			{
				attribute = (*foundIter).second;
				if (!attribute.boolValue())
				{
					std::string attr = (*foundIter).first;
					std::string keyStr = "";
					std::string valueStr = "";
					int dimmKeySize = wbem::DIMMUID_KEY.size();
					int socketKeySize = wbem::SOCKETID_KEY.size();
					int poolKeySize = wbem::POOLID_KEY.size();
					int instanceIdKeySize = wbem::INSTANCEID_KEY.size();
					int namespaceIdKeySize = wbem::NAMESPACEID_KEY.size();

					if (attr.compare(0, dimmKeySize, wbem::DIMMUID_KEY) == 0)
					{
						keyStr = TARGET_DIMM.name;

						// attempt to convert uids to ID's for property output
						std::string dimmIdStr = attr.substr(dimmKeySize, attr.size()-dimmKeySize);
						try
						{
							valueStr = wbem::physical_asset::NVDIMMFactory::uidToDimmIdStr(dimmIdStr);
						}
						catch (wbem::framework::Exception &)
						{
							valueStr = dimmIdStr;
						}
					}
					else if (attr.compare(0, socketKeySize, wbem::SOCKETID_KEY) == 0)
					{
						keyStr = TARGET_SOCKET.name;
						valueStr = attr.substr(socketKeySize, attr.size()-socketKeySize);
					}
					else if (attr.compare(0, poolKeySize, wbem::POOLID_KEY) == 0)
					{
						keyStr = TARGET_POOL.name;
						valueStr = attr.substr(poolKeySize, attr.size()-poolKeySize);
					}
					else if ((attr.compare(0, instanceIdKeySize, wbem::INSTANCEID_KEY) == 0) &&
							(name.compare(wbem::JOBTABLENAME) == 0))
					{
						keyStr = TARGET_JOB.name;
						valueStr = attr.substr(instanceIdKeySize, attr.size()-instanceIdKeySize);
					}
					else if (attr.compare(0, namespaceIdKeySize, wbem::NAMESPACEID_KEY) == 0)
					{
						keyStr = TARGET_NAMESPACE.name;
						valueStr = attr.substr(namespaceIdKeySize, attr.size()-namespaceIdKeySize);
					}
					else
					{
						// otherwise, must be a filter, and we don't throw on filters, just allow the caller to
						// show "no results"
						break;
					}
					throw wbem::exception::NvmExceptionBadTarget(keyStr.c_str(), valueStr.c_str());
				}
			}
		}
	}

	return NVM_SUCCESS;
}

cli::framework::ObjectListResult * cli::nvmcli::NvmInstanceToObjectListResult(
		const wbem::framework::instances_t &instances,
		const std::string &name,
		const std::string &valueProperty,
		const wbem::framework::attribute_names_t &attributes,
		const cli::nvmcli::filters_t &filters)
{
	framework::ObjectListResult *pResult = new framework::ObjectListResult();
	pResult->setRoot(name);

	if (!instances.empty())
	{
		wbem::framework::instances_t matchedInstances;

		filterInstances(instances, name, filters, matchedInstances);

		// add all matched instances to the results
		for (wbem::framework::instances_t::const_iterator instanceIter = matchedInstances.begin();
				instanceIter != matchedInstances.end(); instanceIter++)
		{
			framework::PropertyListResult *pPropertyList =
					NvmInstanceToPropertyListResult(*instanceIter, attributes);

			wbem::framework::Attribute headerAttribute;
			instanceIter->getAttribute(valueProperty, headerAttribute);
			pResult->insert(AttributeToString(headerAttribute), *pPropertyList);
			delete pPropertyList;
		}
	}
	return pResult;
}

wbem::framework::attribute_names_t cli::nvmcli::GetAttributeNames(
		const cli::framework::StringMap& options)
{
	wbem::framework::attribute_names_t defaultNames;
	return GetAttributeNames(options, defaultNames);
}

wbem::framework::attribute_names_t cli::nvmcli::GetAttributeNames(
		const cli::framework::StringMap& options,
		const wbem::framework::attribute_names_t defaultNames)
{
	wbem::framework::attribute_names_t allNames;
	return GetAttributeNames(options, defaultNames, allNames);
}

wbem::framework::attribute_names_t cli::nvmcli::GetAttributeNames(
		const cli::framework::StringMap& options,
		const wbem::framework::attribute_names_t defaultNames,
		const wbem::framework::attribute_names_t allNames)
{
	wbem::framework::attribute_names_t result;
	// -all overrides -display, however parser should prevent from having both
	if (options.find(framework::OPTION_ALL.name) != options.end())
	{
		result = allNames;
	}
	else if (options.find(framework::OPTION_DISPLAY.name) != options.end())
	{
		std::string displayValue = options.at(framework::OPTION_DISPLAY.name);
		result = framework::CliHelper::splitCommaSeperatedString(displayValue);
	}
	else // if user doesn't specify all or display, then use the default attributes
	{
		result = defaultNames;
	}
	return result;
}

void cli::nvmcli::generateDimmFilter(
		const cli::framework::ParsedCommand& parsedCommand,
		wbem::framework::attribute_names_t &attributes,
		cli::nvmcli::filters_t &filters,
		std::string dimmAttributeKey)
{
	// filter on one or more dimm UIDs or dimm handles
	std::vector<std::string> dimmTargets =
			cli::framework::Parser::getTargetValues(parsedCommand,
					cli::nvmcli::TARGET_DIMM.name);
	if (!dimmTargets.empty())
	{
		// uids or handles can be mixed
		struct instanceFilter uidFilter;
		uidFilter.attributeName = dimmAttributeKey;

		for (std::vector<std::string>::const_iterator dimmTargetIter = dimmTargets.begin();
				dimmTargetIter != dimmTargets.end(); dimmTargetIter++)
		{
			// assume uid if not a number
			std::string target = (*dimmTargetIter);
			if (!isStringValidNumber(target))
			{
				uidFilter.attributeValues.push_back(target);
			}
			else
			{
				// convert the handle to a uid
				NVM_UINT32 handle = strtoul(target.c_str(), NULL, 0);
				std::string uidStr;
				try
				{
					if (handleToUid(handle, uidStr))
					{
						uidFilter.attributeValues.push_back(uidStr);
					}
					else // couldn't find it ... so just add the target value.
					{
						uidFilter.attributeValues.push_back(target);
					}
				}
				// handle the case where we throw an exception looking up the UID
				catch (wbem::framework::Exception &)
				{
					uidFilter.attributeValues.push_back(target);
				}
			}
		}

		if (!uidFilter.attributeValues.empty())
		{
			filters.push_back(uidFilter);
			// make sure we have the uid filter attribute
			if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(dimmAttributeKey, attributes))
			{
				attributes.insert(attributes.begin(), dimmAttributeKey);
			}
		}
	}
}

void cli::nvmcli::generateSocketFilter(
		const cli::framework::ParsedCommand& parsedCommand,
		wbem::framework::attribute_names_t &attributes,
		cli::nvmcli::filters_t &filters)
{
	// filter on one or more socket IDs
	std::vector<std::string> socketTargets =
			cli::framework::Parser::getTargetValues(parsedCommand,
					cli::nvmcli::TARGET_SOCKET.name);
	if (!socketTargets.empty())
	{
		struct instanceFilter socketFilter;
		socketFilter.attributeName = wbem::SOCKETID_KEY;

		for (std::vector<std::string>::const_iterator socketTargetIter = socketTargets.begin();
				socketTargetIter != socketTargets.end(); socketTargetIter++)
		{
			std::string target = (*socketTargetIter);
			socketFilter.attributeValues.push_back(target);
		}

		if (!socketFilter.attributeValues.empty())
		{
			filters.push_back(socketFilter);
			// make sure we have the socket ID filter attribute
			if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(wbem::SOCKETID_KEY, attributes))
			{
				attributes.insert(attributes.begin(), wbem::SOCKETID_KEY);
			}
		}
	}
}

/*
 * For commands that support an optional -dimm target,
 * retrieve the dimm UID(s) of the specified target
 * or all manageable dimm UIDs if not specified.
 */
cli::framework::ErrorResult *cli::nvmcli::getDimms(
		const framework::ParsedCommand &parsedCommand,
		std::vector<std::string> &dimms)
{
	framework::ErrorResult *pResult = NULL;
	std::vector<std::string> dimmTargets =
			cli::framework::Parser::getTargetValues(parsedCommand,
					cli::nvmcli::TARGET_DIMM.name);
	try
	{
		// a target value was specified
		if (!dimmTargets.empty())
		{
			// iterate through all the targets and convert any handles to uids
			for (std::vector<std::string>::const_iterator dimmIter = dimmTargets.begin();
					dimmIter != dimmTargets.end(); dimmIter++)
			{
				// assume handle if number, otherwise uid
				std::string target = (*dimmIter);
				std::string uid;
				if (isStringValidNumber(target))
				{
					NVM_UINT32 handle = strtoul(target.c_str(), NULL, 0);
					if (!handleToUid(handle, uid))
					{
						throw wbem::exception::NvmExceptionBadTarget(TARGET_DIMM.name.c_str(), target.c_str());
					}
				}
				else
				{
					uid = target;
				}
				// make sure it exists and is manageable
				int rc;
				if ((rc = wbem::physical_asset::NVDIMMFactory::existsAndIsManageable(uid))
							!= NVM_SUCCESS)
				{
					if (NVM_ERR_BADDEVICE == rc)
					{
						throw wbem::exception::NvmExceptionBadTarget(TARGET_DIMM.name.c_str(), target.c_str());
					}
					else if (NVM_ERR_NOTMANAGEABLE == rc)
					{
						// not manageable
						throw wbem::exception::NvmExceptionNotManageable(target.c_str());
					}
				}
				else
				{
					dimms.push_back(uid);
				}
			}
		}
		// a dimm target value was not specified so get all manageable devices
		else
		{
			dimms = wbem::physical_asset::NVDIMMFactory::getManageableDeviceUids();
		}
	}
	catch (wbem::framework::Exception &e)
	{
		pResult = NvmExceptionToResult(e);
	}
	return pResult;
}


/*
 * For commands that support an optional -socket target,
 * retrieve the manageable dimm UID(s) of the specified target
 * or all manageable dimm UIDs if not specified.
 */
cli::framework::ErrorResult *cli::nvmcli::getDimmsFromSockets(
		const framework::ParsedCommand &parsedCommand,
		std::vector<std::string> &dimms)
{
	framework::ErrorResult *pResult = NULL;
	std::vector<std::string> socketTargets =
			cli::framework::Parser::getTargetValues(parsedCommand,
					cli::nvmcli::TARGET_SOCKET.name);
	wbem::framework::instances_t *pInstances = NULL;
	try
	{
		// a target value was specified
		if (!socketTargets.empty())
		{
			// filter NVDIMMs
			wbem::framework::attribute_names_t socketAttributes;
			filters_t socketFilters;
			generateSocketFilter(parsedCommand, socketAttributes, socketFilters);
			wbem::framework::attribute_names_t attributes;
			wbem::physical_asset::NVDIMMFactory dimmFactory;
			pInstances = dimmFactory.getInstances(attributes);
			if (pInstances)
			{
				wbem::framework::instances_t matches;
				filterInstances(*pInstances, "", socketFilters, matches);

				if (matches.size() == 0) // no matches
				{
					pResult = new framework::SyntaxErrorBadValueResult(
							framework::TOKENTYPE_TARGET, TARGET_SOCKET.name,
							parsedCommand.targets.at(TARGET_SOCKET.name));
				}
				else
				{
					// add their uids to the dimm list
					for (wbem::framework::instances_t::const_iterator iter = matches.begin();
							iter != matches.end(); iter++)
					{
						wbem::framework::Attribute manageabilityAttr;
						if (iter->getAttribute(wbem::MANAGEABILITYSTATE_KEY, manageabilityAttr)
								== wbem::framework::SUCCESS)
						{
							if (manageabilityAttr.uintValue() == MANAGEMENT_VALIDCONFIG)
							{
								wbem::framework::Attribute uidAttr;
								if (iter->getAttribute(wbem::TAG_KEY, uidAttr) ==
										wbem::framework::SUCCESS)
								{
									dimms.push_back(uidAttr.stringValue());
								}
								else
								{
									COMMON_LOG_ERROR_F("Couldn't get attribute '%s' from NVDIMM instance",
											wbem::TAG_KEY.c_str());
									pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
											TRS(nvmcli::UNKNOWN_ERROR_STR));
									break;
								}
							}
						}
						else
						{
							COMMON_LOG_ERROR_F("Couldn't get attribute '%s' from NVDIMM instance",
									wbem::MANAGEABILITYSTATE_KEY.c_str());
							pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
									TRS(nvmcli::UNKNOWN_ERROR_STR));
							break;
						}
					}
				}
			}
		}

		else // no socket target so get all manageable DIMMs
		{
			dimms = wbem::physical_asset::NVDIMMFactory::getManageableDeviceUids();
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

	// clean up
	if (pInstances)
	{
		delete pInstances;
	}
	return pResult;
}

void cli::nvmcli::RenameAttributeKey(
		wbem::framework::instances_t &instances,
		wbem::framework::attribute_names_t &attributes,
		std::string from, std::string to)
{
	RenameAttributeKey(instances, from, to);
	RenameAttributeKey(attributes, from, to);
}

void cli::nvmcli::RenameAttributeKey(
		wbem::framework::attribute_names_t &attributes,
		std::string fromStr, std::string toStr)
{
	wbem::framework::attribute_names_t::iterator found =
			std::find(attributes.begin(), attributes.end(),fromStr);
	if (found != attributes.end())
	{
		*found = toStr;
	}
}

void cli::nvmcli::RenameAttributeKey(wbem::framework::instances_t &instances,
		std::string from, std::string to)
{
	for (size_t i = 0; i < instances.size(); i++)
	{
		wbem::framework::Attribute a;
		if (wbem::framework::SUCCESS == instances[i].getAttribute(from, a))
		{
			instances[i].setAttribute(to, a);
		}
	}
}

void cli::nvmcli::FilterAttributeNames(
		wbem::framework::attribute_names_t& attributes, std::string filter)
{
	if (!filter.empty())
	{
		// because this for loop removes from the collection, starting from the end and working
		// backwards won't change an attributes index until after it's been considered
		for(int i = attributes.size() - 1; i >= 0; i --)
		{
			if (!framework::stringsIEqual(attributes[i], filter))
			{
				attributes.erase(attributes.begin() + i);
			}
		}
	}
}

/*
 * Try to cast the exception to more specific exceptions. Return an appropriate Result
 */
cli::framework::ErrorResult *cli::nvmcli::NvmExceptionToResult(wbem::framework::Exception &e, std::string prefix)
{
	// try ExceptionNotSupported
	wbem::framework::ExceptionNotSupported *pNotSupportedException =
			dynamic_cast<wbem::framework::ExceptionNotSupported *>(&e);
	if (pNotSupportedException != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_NOTSUPPORTED, NOTSUPPORTED_ERROR_STR, prefix);
	}

	// try ExceptionNoMemory
	wbem::framework::ExceptionNoMemory *pNoMemory =
			dynamic_cast<wbem::framework::ExceptionNoMemory *>(&e);
	if (pNoMemory != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_OUTOFMEMORY, NOMEMORY_ERROR_STR, prefix);
	}

	// try ExceptionBadAttribute
	wbem::framework::ExceptionBadAttribute *pBadAttribute =
			dynamic_cast<wbem::framework::ExceptionBadAttribute *>(&e);
	if (pBadAttribute != NULL)
	{
		return new framework::SyntaxErrorBadValueResult(
				cli::framework::TOKENTYPE_OPTION, "-display", pBadAttribute->getBadAttribute());
	}

	// try NvmExceptionInvalidPoolConfig
	wbem::exception::NvmExceptionInvalidPoolConfig *pBadPoolConfig =
			dynamic_cast<wbem::exception::NvmExceptionInvalidPoolConfig *>(&e);
	if (pBadPoolConfig != NULL)
	{
		// return the wbem message
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN, e.what(), prefix);
	}

	// try NvmExceptionBadTarget
	wbem::exception::NvmExceptionBadTarget *pBadTarget =
			dynamic_cast<wbem::exception::NvmExceptionBadTarget *>(&e);
	if (pBadTarget != NULL)
	{
		// If the bad target was a DIMM, it's an invalid DIMM ID, not a syntax error
		if (pBadTarget->getTarget() == nvmcli::TARGET_DIMM.name)
		{
			return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
					getInvalidDimmIdErrorString(pBadTarget->getBadTargetValue()),
					prefix);
		}
		else
		{
			// return the wbem message
			return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_SYNTAX, e.what(), prefix);
		}
	}

	// try NvmExceptionNotManageable
	wbem::exception::NvmExceptionNotManageable *pNotManageable =
			dynamic_cast<wbem::exception::NvmExceptionNotManageable *>(&e);
	if (pNotManageable != NULL)
	{
		// return the wbem message
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN, e.what(), prefix);
	}

	// try NvmExceptionLibError
	wbem::exception::NvmExceptionLibError * pLibError =
			dynamic_cast<wbem::exception::NvmExceptionLibError *>(&e);
	if (pLibError != NULL)
	{
		int libRc = pLibError->getLibError();
		if (libRc == NVM_ERR_NOMEMORY)
		{
			return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_OUTOFMEMORY, NOMEMORY_ERROR_STR, prefix);
		}
		if (libRc == NVM_ERR_NOTSUPPORTED)
		{
			return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_NOTSUPPORTED, NOTSUPPORTED_ERROR_STR, prefix);
		}
		if (libRc == NVM_ERR_CONFIGNOTSUPPORTED || libRc == NVM_ERR_SKUVIOLATION)
		{
			return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_NOTSUPPORTED, e.what(), prefix);
		}
		if (libRc == NVM_ERR_BADSECURITYSTATE)
		{
			char errbuff[NVM_ERROR_LEN];
			s_snprintf(errbuff, NVM_ERROR_LEN, TRS(BADSECURITY_ERROR_STR));
			return new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN, errbuff);
		}
		// return the library message
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN, e.what(), prefix);
	}

	// ExceptionInvalidWqlQuery is not used in the CLI
	// ExceptionBadParameter returns unknown error
	return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN, UNKNOWN_ERROR_STR, prefix);
}

cli::framework::ErrorResult *cli::nvmcli::CoreExceptionToResult(std::exception &e, std::string prefix)
{
	// Try Library Exception
	core::LibraryException *pLibError =
			dynamic_cast<core::LibraryException *>(&e);
	if (pLibError != NULL)
	{
		int libRc = pLibError->getErrorCode();
		if (libRc == NVM_ERR_NOMEMORY)
		{
			return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_OUTOFMEMORY, TRS(NOMEMORY_ERROR_STR), prefix);
		}
		if (libRc == NVM_ERR_NOTSUPPORTED)
		{
			return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_NOTSUPPORTED, TRS(NOTSUPPORTED_ERROR_STR), prefix);
		}
		if (libRc == NVM_ERR_CONFIGNOTSUPPORTED || libRc == NVM_ERR_SKUVIOLATION)
		{
			return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_NOTSUPPORTED, TR(e.what()), prefix);
		}
		if (libRc == NVM_ERR_BADSECURITYSTATE)
		{
			return new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN, TRS(BADSECURITY_ERROR_STR));
		}
		// return the library message
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN, TR(e.what()), prefix);
	}

	// try NvmExceptionBadRequest subclasses
	core::NvmExceptionProvisionCapacityNotSupported *pBadRequestNotSupported =
			dynamic_cast<core::NvmExceptionProvisionCapacityNotSupported *>(&e);
	if (pBadRequestNotSupported != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_NOTSUPPORTED,
				TRS(NOTSUPPORTED_ERROR_STR), prefix);
	}

	core::NvmExceptionBadRequestMemorySize *pBadRequestMemorySize =
			dynamic_cast<core::NvmExceptionBadRequestMemorySize *>(&e);
	if (pBadRequestMemorySize != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_MEMORY_SIZE_STR), prefix);
	}

	core::NvmExceptionBadRequestSize *pBadRequestAppDirectSize =
			dynamic_cast<core::NvmExceptionBadRequestSize *>(&e);
	if (pBadRequestAppDirectSize != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_APP_DIRECT_SIZE_STR), prefix);
	}

	core::NvmExceptionDimmHasConfigGoal *pBadRequestExistingGoal =
			dynamic_cast<core::NvmExceptionDimmHasConfigGoal *>(&e);
	if (pBadRequestExistingGoal != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_GOAL_ALREADY_EXISTS_STR), prefix);
	}

	core::NvmExceptionNamespacesExist *pBadRequestNamespacesExist =
			dynamic_cast<core::NvmExceptionNamespacesExist *>(&e);
	if (pBadRequestNamespacesExist != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_MUST_DELETE_NAMESPACES_STR), prefix);
	}

	core::NvmExceptionBadRequestDoesntContainRequiredDimms *pBrokenConfig =
			dynamic_cast<core::NvmExceptionBadRequestDoesntContainRequiredDimms *>(&e);
	if (pBrokenConfig != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_GOAL_BREAKS_CONFIG_STR), prefix);
	}

	core::NvmExceptionOverAddressDecoderLimit *pTooManyInterleaves =
			dynamic_cast<core::NvmExceptionOverAddressDecoderLimit *>(&e);
	if (pTooManyInterleaves != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_EXCEEDS_SYSTEM_RESOURCES_STR), prefix);
	}

	core::NvmExceptionAppDirectSettingsNotSupported *pSettingsNotSupported =
			dynamic_cast<core::NvmExceptionAppDirectSettingsNotSupported *>(&e);
	if (pSettingsNotSupported != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_APP_DIRECT_SETTINGS_NOT_RECOMMENDED_STR), prefix);
	}

	core::NvmExceptionRequestNotSupported *pRequestNotSupported =
			dynamic_cast<core::NvmExceptionRequestNotSupported *>(&e);
	if (pRequestNotSupported != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_NOT_SUPPORTED_STR), prefix);
	}

	core::NvmExceptionRequestedDimmLocked *pDimmLocked =
			dynamic_cast<core::NvmExceptionRequestedDimmLocked *>(&e);
	if (pDimmLocked != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_DIMM_SECURITY_STATE), prefix);
	}

	core::NvmExceptionBadRequestReserveDimm *pBadReserveDimmRequest =
			dynamic_cast<core::NvmExceptionBadRequestReserveDimm *>(&e);
	if (pBadReserveDimmRequest!= NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_RESERVE_DIMM_STR), prefix);
	}

	// Generic bad request - all other bad request cases
	core::NvmExceptionBadRequest *pBadRequest =
			dynamic_cast<core::NvmExceptionBadRequest *>(&e);
	if (pBadRequest != NULL)
	{
		return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(BAD_REQUEST_STR), prefix);
	}

	// ExceptionInvalidWqlQuery is not used in the CLI
	// ExceptionBadParameter returns unknown error
	return new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN, UNKNOWN_ERROR_STR, prefix);
}


/*
 * Helper function to set an error code on a result
 */
void cli::nvmcli::SetResultErrorCodeFromException(cli::framework::ResultBase &result,
		wbem::framework::Exception &e, std::string prefix)
{
	// keep any existing errors
	if (result.getErrorCode() == framework::ErrorResult::ERRORCODE_SUCCESS)
	{
		cli::framework::ErrorResult *pError = NvmExceptionToResult(e);
		result.setErrorCode(pError->getErrorCode());
		delete pError;
	}
}

NVM_UINT64 cli::nvmcli::stringToUInt64(const std::string& value)
{
	NVM_UINT64 result;
	std::istringstream ss(value);
	if (value.size() > 1 && (value.substr(0, 2) == "0x" || value.substr(0, 2) == "0X"))
	{
		ss >> std::hex >> result;
	}
	else
	{
		ss >> result;
	}

	return result;
}

/*
 * try parsing the string str to an real32.  If it succeeds return true and put the real32 value into
 * p_value. Else return false;
 */
bool cli::nvmcli::stringToReal32(const std::string& str, NVM_REAL32 *p_value)
{
	bool result = false;
	std::istringstream stream(str);

	stream >> *p_value; // try to read the number
	stream >> std::ws; // eat any whitespace

	if (!stream.fail() && stream.eof())
	{
		result = true;
	}
	return result;
}

/*
 * try parsing the string str to an int.  If it succeeds return true and put the int value into
 * p_value. Else return false;
 */
bool cli::nvmcli::stringToInt(const std::string& str, int* p_value)
{
	bool result = false;
	std::istringstream stream(str);

	stream >> *p_value; // try to read the number
	stream >> std::ws; // eat any whitespace

	if (!stream.fail() && stream.eof())
	{
		result = true;
	}
	return result;
}

bool cli::nvmcli::isStringHex(const std::string &value)
{
	bool result = false;
	if (value.substr(0, 2) == "0x" || value.substr(0, 2) == "0X")
	{
		result = true;
		for (size_t i = 2; i < value.size() && result; i++)
		{
			result &= std::isxdigit(value[i]) != 0;
		}
	}

	return result;
}

bool cli::nvmcli::isStringValidNumber(const std::string &value)
{
	bool result = true;
	if (value.substr(0, 2) == "0x" || value.substr(0, 2) == "0X")
	{
		for (size_t i = 2; i < value.size() && result; i++)
		{
			result &= std::isxdigit(value[i]) != 0;
		}
	}
	else
	{
		for (size_t i = 0; i < value.size() && result; i++)
		{
			result &= std::isdigit(value[i]) != 0;
		}
	}
	return result;
}

void cli::nvmcli::findBestCapacityFormat(NVM_UINT64 capacityInBytes, char *capacity_format)
{
	if (capacityInBytes/BYTES_PER_GB != 0)
	{
		s_strcpy(capacity_format, PREFERENCE_SIZE_GIB.c_str(), CONFIG_VALUE_LEN);
	}
	else if  (capacityInBytes/BYTES_PER_MB != 0)
	{
		s_strcpy(capacity_format, PREFERENCE_SIZE_MIB.c_str(), CONFIG_VALUE_LEN);
	}
	else
	{
		s_strcpy(capacity_format, PREFERENCE_SIZE_B.c_str(), CONFIG_VALUE_LEN);
	}
}

// helper function to round number to decimal_places
NVM_REAL32 round(NVM_REAL32 number, int decimal_places)
{
	return floor(number * pow(10, decimal_places) + 0.5) / pow(10, decimal_places);
}

std::string cli::nvmcli::calculateAdvertisedCapacity(NVM_UINT64 capacityInBytes,
		const NVM_UINT64 blockCount, const NVM_UINT64 blockSize)
{
	std::stringstream capacityInRequestedFormatStr;
	NVM_REAL32 capacityInGB = 0;

	if (blockSize == 1)
	{ // appdirect namespace
		capacityInGB = ((NVM_REAL32)
				((NVM_INT64)(blockCount - (IDEMA_CONVERSION_CONSTANT1 * 512)))/(IDEMA_CONVERSION_CONSTANT2 * 512)) +
						IDEMA_CONVERSION_CONSTANT3;
	}
	else if (blockSizeIsPI(blockSize))
	{ // storage namespace with blocksize 5**
		capacityInGB = ((NVM_REAL32)
				((NVM_INT64)(blockCount - IDEMA_CONVERSION_CONSTANT1))/IDEMA_CONVERSION_CONSTANT2) +
						IDEMA_CONVERSION_CONSTANT3;
	}
	else if (blockSizeIs4KVariant(blockSize))
	{ // storage namespace with 4k variants of blocksize
		capacityInGB = ((NVM_REAL32)
				((NVM_INT64)(blockCount - (IDEMA_CONVERSION_CONSTANT1/8)))/(IDEMA_CONVERSION_CONSTANT2/8)) +
						IDEMA_CONVERSION_CONSTANT3;
	}
	else
	{ // pool
		capacityInGB = ((NVM_REAL32)
				((NVM_INT64)(capacityInBytes - (IDEMA_CONVERSION_CONSTANT1 * 512)))/(IDEMA_CONVERSION_CONSTANT2 * 512)) +
						IDEMA_CONVERSION_CONSTANT3;
	}

	if (capacityInGB >= 0.1)
	{ // more than one zero after the decimal point
		capacityInRequestedFormatStr << round(capacityInGB, 1) << " " << CAPACITY_UNITS_GB;
	}
	else if (capacityInGB >= 0)
	{ // display as bytes instead
		capacityInRequestedFormatStr << capacityInBytes << " " << PREFERENCE_SIZE_B;
	}

	return capacityInRequestedFormatStr.str();
}

/*
   helper function to calculate BlockCount/LBACount using formulae from IDEMA LBA1-03
   For logical block size of 512 bytes:
	LBA counts = (97,696,368) + (1,953,504 * (Advertised Capacity in GBytes – 50))
   For logical block size of 4096 bytes:
	LBA counts = (12,212,046) + (244,188 * (Advertised Capacity in GBytes – 50))
	(The formula is scaled by dividing the first two constants by eight.)
   Also, the lower three digits of the LBA count should be divisible by 8 with a remainder of
   0 (rounded up if necessary), to provide a even number of aligned sectors.
*/
NVM_UINT64 cli::nvmcli::calculateBlockCountForNamespace(const NVM_REAL32 capacityInGB,
		const NVM_UINT64 blockSize)
{
	NVM_UINT64 blockCount = 0;

	// block sizes of 512, 520, 528 bytes
	if (blockSizeIsPI(blockSize))
	{
		blockCount = round_up(IDEMA_CONVERSION_CONSTANT1 +
			(IDEMA_CONVERSION_CONSTANT2 * (capacityInGB - IDEMA_CONVERSION_CONSTANT3)), 8);
	}
	// Scale down the constants by 8 for namespace with block size of 4K variants (4096, 4160, etc)
	else if (blockSizeIs4KVariant(blockSize))
	{
		blockCount = round_up(IDEMA_CONVERSION_CONSTANT1/8 +
			((IDEMA_CONVERSION_CONSTANT2/8) * (capacityInGB - IDEMA_CONVERSION_CONSTANT3)), 8);
	}
	// Scale up the constants by 512 for namespace with a 1 byte block size
	else
	{ // appdirect namespace
		blockCount = round_up((IDEMA_CONVERSION_CONSTANT1 * 512) +
			((IDEMA_CONVERSION_CONSTANT2 * 512) * (capacityInGB - IDEMA_CONVERSION_CONSTANT3)), 8);
	}

	return blockCount;
}

// Note: desired capacity units requested via CLI command is preferred over capacity units
// from configuration setting SQL_KEY_CLI_SIZE
std::string cli::nvmcli::convertCapacityFormat(NVM_UINT64 capacityInBytes,
	const std::string capacityUnits, const NVM_UINT64 blockCount, const NVM_UINT64 blockSize)
{
	char capacity_format[CONFIG_VALUE_LEN] = "";
	std::stringstream capacityInRequestedFormatStr;
	NVM_REAL32 capacityInRequestedFormat = 1;

	try
	{
		if (framework::stringsIEqual(capacityUnits, PREFERENCE_SIZE_GIB))
		{
			s_strcpy(capacity_format, PREFERENCE_SIZE_GIB.c_str(), CONFIG_VALUE_LEN);
		}
		else
		{
			int rc = get_config_value(SQL_KEY_CLI_SIZE, capacity_format);
			if (rc != NVM_SUCCESS)
			{
				COMMON_LOG_DEBUG_F("Failed to retrieve key %s. ", SQL_KEY_CLI_SIZE);
				s_strcpy(capacity_format, PREFERENCE_SIZE_GIB.c_str(), CONFIG_VALUE_LEN);
			}
		}

		// find the best format for each capacity
		if (framework::stringsIEqual(capacity_format, PREFERENCE_SIZE_AUTO))
		{
			findBestCapacityFormat(capacityInBytes, capacity_format);
		}

		if (framework::stringsIEqual(capacity_format, PREFERENCE_SIZE_B))
		{
			capacityInRequestedFormatStr << capacityInBytes << " " << PREFERENCE_SIZE_B;
		}
		else if (framework::stringsIEqual(capacity_format, PREFERENCE_SIZE_GIB))
		{
			capacityInRequestedFormat = round((NVM_REAL32)capacityInBytes / BYTES_PER_GB, 1);
			capacityInRequestedFormatStr << capacityInRequestedFormat << " " << PREFERENCE_SIZE_GIB;
		}
		else if (framework::stringsIEqual(capacity_format, PREFERENCE_SIZE_MIB))
		{
			capacityInRequestedFormat = round((NVM_REAL32)capacityInBytes / BYTES_PER_MB, 1);
			capacityInRequestedFormatStr << capacityInRequestedFormat << " " << PREFERENCE_SIZE_MIB;
		}
		else
		{
			COMMON_LOG_ERROR_F("Invalid capacity format %s. ", capacity_format);
			throw wbem::framework::Exception();
		}
	}
	catch (wbem::framework::Exception &)
	{
		capacityInRequestedFormat = round((NVM_REAL32)capacityInBytes / BYTES_PER_MB, 1);
		capacityInRequestedFormatStr << capacityInRequestedFormat << " " << PREFERENCE_SIZE_MIB;
	}

	// return capacity in bytes if the value in requested format cannot be rounded off
	if ((capacityInBytes !=0) &&
			(capacityInRequestedFormat == 0))
	{
		capacityInRequestedFormatStr.str("");
		capacityInRequestedFormatStr.clear();
		capacityInRequestedFormatStr << capacityInBytes << " " << PREFERENCE_SIZE_B;
	}

	return capacityInRequestedFormatStr.str();
}

void cli::nvmcli::convertCapacityAttributeToGB(wbem::framework::Instance &wbemInstance,
		const std::string attributeName)
{
	NVM_UINT64 capacity = 0;
	NVM_UINT64 blockSize = 0;
	NVM_UINT64 blockCount = 0;
	wbem::framework::Attribute attr;

	if (wbemInstance.getAttribute(attributeName, attr) == wbem::framework::SUCCESS)
	{
		capacity = attr.uint64Value();
	}
	// get blocksize and blockcount values - used to calculate advertised capacity
	if (wbemInstance.getAttribute(wbem::BLOCKSIZE_KEY, attr) ==
			wbem::framework::SUCCESS)
	{
		blockSize = attr.uint64Value();
	}
	if (wbemInstance.getAttribute(wbem::NUMBEROFBLOCKS_KEY, attr) ==
			wbem::framework::SUCCESS)
	{
		blockCount = attr.uint64Value();
	}

	std::string advertisedCapacity =
		calculateAdvertisedCapacity(capacity, blockCount, blockSize);
	if (advertisedCapacity == "")
	{ // if capacity cannot be represented as GB
		advertisedCapacity = convertCapacityFormat(capacity, PREFERENCE_SIZE_B);
	}

	wbem::framework::Attribute attrStr(advertisedCapacity, false);
	wbemInstance.setAttribute(attributeName, attrStr);
}

void cli::nvmcli::convertCapacityAttribute(wbem::framework::Instance &wbemInstance,
		const std::string attributeName, const std::string capacityUnits)
{
	if (framework::stringsIEqual(capacityUnits, CAPACITY_UNITS_GB))
	{
		convertCapacityAttributeToGB(wbemInstance, attributeName);
	}
	else
	{
		wbem::framework::Attribute attr;
		if (wbemInstance.getAttribute(attributeName, attr) == wbem::framework::SUCCESS)
		{
			wbem::framework::Attribute attrStr(convertCapacityFormat(attr.uint64Value(),
					capacityUnits), false);
			wbemInstance.setAttribute(attributeName, attrStr);
		}
	}
}

void cli::nvmcli::RemoveAttributeName(wbem::framework::attribute_names_t &attributes,
		std::string nameToRemove)
{
	wbem::framework::attribute_names_t::iterator found =
			std::find(attributes.begin(), attributes.end(), nameToRemove);
	if (found != attributes.end())
	{
		attributes.erase(found);
	}

}

void ::cli::nvmcli::generateFilterForAttributeWithTargetValues(
	const cli::framework::ParsedCommand &parsedCommand, const std::string &target,
	const std::string &attributeName, cli::nvmcli::filters_t &filters)
{
	std::vector<std::string> targetValues =
			cli::framework::Parser::getTargetValues(parsedCommand, target);
	if (!targetValues.empty())
	{
		struct instanceFilter newFilter;
		newFilter.attributeName = attributeName;

		for (std::vector<std::string>::const_iterator targetIter = targetValues.begin();
		     targetIter != targetValues.end(); targetIter++)
		{
			std::string target = (*targetIter);
			newFilter.attributeValues.push_back(target);
		}

		if (!newFilter.attributeValues.empty())
		{
			filters.push_back(newFilter);
		}
	}
}

std::string cli::nvmcli::AttributeToString(const wbem::framework::Attribute &attr)
{
	std::string result;
	if (attr.getType() == wbem::framework::BOOLEAN_T)
	{
		result = attr.boolValue() ? "1" : "0";
	}
	else
	{
		result = attr.asStr();
	}
	return result;
}

std::string cli::nvmcli::AttributeToHexString(const wbem::framework::Attribute &attr)
{
	std::stringstream result;
	switch (attr.getType())
	{
		case wbem::framework::UINT8_T:
			result << "0x" << std::hex << std::setw(2) << std::setfill('0');
			result << attr.uintValue();
			break;
		case wbem::framework::UINT16_T:
			result << "0x" << std::hex << std::setw(4) << std::setfill('0');
			result << attr.uintValue();
			break;
		case wbem::framework::UINT32_T:
			result << "0x" << std::hex << std::setw(8) << std::setfill('0');
			result << attr.uintValue();
			break;
		case wbem::framework::UINT64_T:
			result << "0x" << std::hex << std::setw(16) << std::setfill('0');
			result << attr.uint64Value();
			break;
		case wbem::framework::SINT8_T:
			result << "0x" << std::hex << std::setw(2) << std::setfill('0');
			result << attr.intValue();
			break;
		case wbem::framework::SINT16_T:
			result << "0x" << std::hex << std::setw(4) << std::setfill('0');
			result << attr.intValue();
			break;
		case wbem::framework::SINT32_T:
			result << "0x" << std::hex << std::setw(8) << std::setfill('0');
			result << attr.intValue();
			break;
		case wbem::framework::SINT64_T:
			result << "0x" << std::hex << std::setw(16) << std::setfill('0');
			result << attr.sint64Value();
			break;
		case wbem::framework::UINT8_LIST_T:
			for (unsigned int i = 0; i < attr.uint8ListValue().size(); i++)
			{
				if (i > 0)
				{
					result << ", ";
				}
				result << "0x" << std::hex << std::setw(2) << std::setfill('0');
				result << (unsigned int)attr.uint8ListValue()[i];
			}
			break;
		case wbem::framework::UINT16_LIST_T:
			for (unsigned int i = 0; i < attr.uint16ListValue().size(); i++)
			{
				if (i > 0)
				{
					result << ", ";
				}
				result << "0x" << std::hex << std::setw(4) << std::setfill('0');
				result << attr.uint16ListValue()[i];
			}
			break;
		case wbem::framework::UINT32_LIST_T:
			for (unsigned int i = 0; i < attr.uint32ListValue().size(); i++)
			{
				if (i > 0)
				{
					result << ", ";
				}
				result << "0x" << std::hex << std::setw(8) << std::setfill('0');
				result << attr.uint32ListValue()[i];
			}
			break;
		case wbem::framework::UINT64_LIST_T:
			for (unsigned int i = 0; i < attr.uint64ListValue().size(); i++)
			{
				if (i > 0)
				{
					result << ", ";
				}
				result << "0x" << std::hex << std::setw(16) << std::setfill('0');
				result << attr.uint64ListValue()[i];
			}
			break;
		default:
			result << AttributeToString(attr);
			break;
	}
	return result.str();
}

std::string cli::nvmcli::getInvalidDimmIdErrorString(const std::string& invalidDimmId)
{
	std::string errorString = framework::ResultBase::stringFromArgList(
		TR(INVALID_DIMMID_ERROR_STR.c_str()), invalidDimmId.c_str());
	return errorString;
}

std::string cli::nvmcli::uint64ToString(const unsigned long long &value)
{
	std::stringstream result;
	result << value;
	return result.str();
}

std::string cli::nvmcli::uint64ToHexString(const unsigned long long &value)
{
	std::stringstream result;
	result << std::hex << value;
	return result.str();
}

bool cli::nvmcli::handleToUid(const NVM_UINT32 &handle, std::string &dimmUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool validHandle = false;

	wbem::physical_asset::devices_t devices = wbem::physical_asset::NVDIMMFactory::getAllDevices();

	wbem::physical_asset::devices_t::const_iterator iter = devices.begin();
	// find the matching handle
	for (; iter != devices.end(); iter++)
	{
		if ((*iter).device_handle.handle == handle)
		{
			validHandle = true;
			NVM_UID uidStr;
			uid_copy((*iter).uid, uidStr);
			dimmUid = uidStr;
			break;
		}
	}

	return validHandle;
}
