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
 * modify, and show namespaces.
 */

#include "NamespaceFeature.h"
#include <LogEnterExit.h>
#include <string/s_str.h>
#include "CommandParts.h"
#include <libinvm-cli/SimpleListResult.h>
#include <pmem_config/NamespaceViewFactory.h>
#include <pmem_config/PersistentMemoryServiceFactory.h>
#include <pmem_config/PersistentMemoryCapabilitiesFactory.h>
#include <pmem_config/PersistentMemoryPoolFactory.h>
#include <pmem_config/PersistentMemoryNamespaceFactory.h>
#include <libinvm-cim/ExceptionNotSupported.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <utility.h>
#include <exception/NvmExceptionLibError.h>
#include <exception/NvmExceptionUndoModifyFailed.h>

/*
 * Create a filter for the NamespaceID
 */
void cli::nvmcli::NamespaceFeature::generateNamespaceFilter(
		const cli::framework::ParsedCommand &parsedCommand,
		wbem::framework::attribute_names_t &attributes,
		cli::nvmcli::filters_t &filters)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::vector<std::string> nsTargets =
			cli::framework::Parser::getTargetValues(parsedCommand,
					cli::nvmcli::TARGET_NAMESPACE.name);
	if (!nsTargets.empty())
	{
		struct instanceFilter nsFilter;
		nsFilter.attributeName = wbem::NAMESPACEID_KEY;

		for (std::vector<std::string>::const_iterator iter = nsTargets.begin();
			 iter != nsTargets.end(); iter++)
		{
			std::string target = (*iter);
			nsFilter.attributeValues.push_back(target);
		}

		if (!nsFilter.attributeValues.empty())
		{
			filters.push_back(nsFilter);
			// make sure we have the Namespace ID filter attribute
			if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(wbem::NAMESPACEID_KEY, attributes))
			{
				attributes.insert(attributes.begin(), wbem::NAMESPACEID_KEY);
			}
		}
	}
}

/*
 * Filter namespaces based on namespace type
 */
cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::generateNamespaceTypeFilter(
		const cli::framework::ParsedCommand &parsedCommand,
		wbem::framework::attribute_names_t &attributes,
		cli::nvmcli::filters_t &filters)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	cli::framework::ResultBase *pResult = NULL;
	// check for the type property
	bool exists = false;
	std::string type = cli::framework::Parser::getPropertyValue(parsedCommand,
			wbem::TYPE_KEY, &exists);
	if (exists)
	{
		// make sure the type is valid
		if (!cli::framework::stringsIEqual(type, wbem::pmem_config::NS_TYPE_STR_UNKNOWN) &&
			!cli::framework::stringsIEqual(type, wbem::pmem_config::NS_TYPE_STR_STORAGE) &&
			!cli::framework::stringsIEqual(type, wbem::pmem_config::NS_TYPE_STR_APPDIRECT))
		{
			pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
					wbem::TYPE_KEY, type);
		}
		else
		{
			struct instanceFilter typeFilter;
			typeFilter.attributeName = wbem::TYPE_KEY;
			typeFilter.attributeValues.push_back(type);
			filters.push_back(typeFilter);

			// make sure we have type being requested from wbem
			if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(
					wbem::TYPE_KEY, attributes))
			{
				attributes.insert(attributes.begin(), wbem::TYPE_KEY);
			}
		}
	}
	return pResult;
}

/*
 * Filter namespaces based on namespace type
 */
cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::generateNamespaceHealthFilter(
		const cli::framework::ParsedCommand &parsedCommand,
		wbem::framework::attribute_names_t &attributes,
		cli::nvmcli::filters_t &filters)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	cli::framework::ResultBase *pResult = NULL;
	// check for the health state property
	bool exists = false;
	std::string health = cli::framework::Parser::getPropertyValue(parsedCommand,
			wbem::HEALTHSTATE_KEY, &exists);
	if (exists)
	{
		// make sure the type is valid
		if (!cli::framework::stringsIEqual(health, wbem::pmem_config::NS_HEALTH_STR_UNKNOWN) &&
			!cli::framework::stringsIEqual(health, wbem::pmem_config::NS_HEALTH_STR_NORMAL) &&
			!cli::framework::stringsIEqual(health, wbem::pmem_config::NS_HEALTH_STR_WARN) &&
			!cli::framework::stringsIEqual(health, wbem::pmem_config::NS_HEALTH_STR_ERR) &&
			!cli::framework::stringsIEqual(health, wbem::pmem_config::NS_HEALTH_STR_BROKENMIRROR))
		{
			pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
					wbem::HEALTHSTATE_KEY, health);
		}
		else
		{
			struct instanceFilter healthFilter;
			healthFilter.attributeName = wbem::HEALTHSTATE_KEY;
			healthFilter.attributeValues.push_back(health);
			filters.push_back(healthFilter);

			// make sure we have health being requested from wbem
			if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(
					wbem::HEALTHSTATE_KEY, attributes))
			{
				attributes.insert(attributes.begin(), wbem::HEALTHSTATE_KEY);
			}
		}
	}
	return pResult;
}

cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::showNamespaces(
		cli::framework::ParsedCommand const &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ResultBase *pResult = NULL;
	wbem::framework::instances_t *pInstances = NULL;
	try
	{
		wbem::framework::attribute_names_t attributes;
		populateNamespaceAttributes(attributes, parsedCommand);

		// create the display filters
		cli::nvmcli::filters_t filters;
		generateNamespaceFilter(parsedCommand, attributes, filters);
		generatePoolFilter(parsedCommand, attributes, filters);
		pResult = generateNamespaceTypeFilter(parsedCommand, attributes, filters);
		if (pResult == NULL)
		{
			pResult = generateNamespaceHealthFilter(parsedCommand, attributes, filters);
			if (pResult == NULL)
			{
				// get the instances from wbem
				pInstances = m_pNsViewFactoryProvider->getInstances(attributes);
				if (pInstances == NULL)
				{
					COMMON_LOG_ERROR("NamespaceViewFactory getInstances returned a NULL instances pointer");
					pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
						TRS(nvmcli::UNKNOWN_ERROR_STR));
				}
				else
				{
					// get the desired units of capacity
					std::string capacityUnits;
					pResult = getCapacityUnits(parsedCommand, &capacityUnits);
					if (pResult == NULL)
					{
						for (size_t i = 0; i < pInstances->size(); i++)
						{
							wbem::framework::Instance &instance = (*pInstances)[i];
							convertCapacityAndAddIsMirroredText(instance, capacityUnits);
							generateBlockSizeAttributeValue(instance);
							convertEnabledStateAttributes(instance);
							convertActionRequiredEventsToNAIfEmpty(instance);
						}

						// add/remove cli display attributes
						RenameAttributeKey(*pInstances, attributes, wbem::NUMBEROFBLOCKS_KEY, wbem::BLOCKCOUNT_KEY);
						RenameAttributeKey(*pInstances, attributes, wbem::ENABLEDSTATE_KEY, wbem::ENABLED_KEY);
						RemoveAttributeName(attributes, wbem::REPLICATION_KEY);

						pResult = NvmInstanceToObjectListResult(*pInstances, "Namespace",
								wbem::NAMESPACEID_KEY, attributes, filters);

						// Set layout to table unless the -all or -display option is present
						if (!framework::parsedCommandContains(parsedCommand, framework::OPTION_DISPLAY) &&
							!framework::parsedCommandContains(parsedCommand, framework::OPTION_ALL))
						{
							pResult->setOutputType(framework::ResultBase::OUTPUT_TEXTTABLE);
						}
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

	if (pInstances)
	{
		delete pInstances;
	}

	return pResult;
}

void cli::nvmcli::NamespaceFeature::generateBlockSizeAttributeValue(wbem::framework::Instance &instance)
{
	wbem::framework::Attribute bsAttr;
	NVM_UINT64 blockSizeInt = 0;

	if (instance.getAttribute(wbem::BLOCKSIZE_KEY, bsAttr) ==
		wbem::framework::SUCCESS)
	{
		blockSizeInt = bsAttr.uint64Value();
		NVM_UINT64 alignedBlockSize = get_real_block_size(blockSizeInt);
		if (blockSizeInt != alignedBlockSize)
		{ // report the selected block size and the actual size used
			std::stringstream bsStr;
			bsStr << bsAttr.asStr();
			bsStr << " (" << alignedBlockSize << " B aligned)";
			instance.setAttribute(wbem::BLOCKSIZE_KEY,
				wbem::framework::Attribute(bsStr.str(), false));
		}
	}
}

void cli::nvmcli::NamespaceFeature::convertCapacityAndAddIsMirroredText(
	wbem::framework::Instance &instance, const std::string capacityUnits)
{
	cli::nvmcli::convertCapacityAttribute(instance,
		wbem::CAPACITY_KEY, capacityUnits);

	wbem::framework::Attribute capAttribute;
	if (instance.getAttribute(wbem::CAPACITY_KEY, capAttribute) ==
		wbem::framework::SUCCESS)
	{
		wbem::framework::Attribute isMirroredAttribute;
		if(instance.getAttribute(wbem::REPLICATION_KEY, isMirroredAttribute) ==
			   wbem::framework::SUCCESS &&
			isMirroredAttribute.boolValue())
		{
			std::stringstream capStr;
			capStr << capAttribute.asStr();
			capStr << cli::nvmcli::MIRRORED_CAPACITY;
			instance.setAttribute(wbem::CAPACITY_KEY,
					wbem::framework::Attribute(capStr.str(), false));
		}
	}
}

/*
 * delete namespaces
 */
cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::deleteNamespaces(
		cli::framework::ParsedCommand const &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::vector<std::string> namespaceList;
	pResult = m_pWbemToCli->getNamespaces(parsedCommand, namespaceList);

	if (pResult == NULL)
	{
		framework::SimpleListResult *pList = new framework::SimpleListResult();
		pResult = pList;
		const std::string prefix = TR("Delete namespace ");

		bool forceOption = parsedCommand.options.find(framework::OPTION_FORCE.name)
				!= parsedCommand.options.end();

		for (size_t i = 0; i < namespaceList.size(); i++)
		{
			std::string deleteMsg = prefix + namespaceList[i] + ": ";
			try
			{
				// if user didn't specify the force option, prompt them to continue
				std::string prompt = framework::ResultBase::stringFromArgList(
						DELETE_NS_PROMPT.c_str(), namespaceList[i].c_str());
				if (!forceOption && !promptUserYesOrNo(prompt))
				{
					pList->insert(deleteMsg + TRS(cli::framework::UNCHANGED_MSG));
				}
				else
				{
					m_deleteNamespace(namespaceList[i]);
					pList->insert(deleteMsg + cli::framework::SUCCESS_MSG);
				}
			}
			catch(wbem::framework::Exception &e)
			{
				cli::framework::ErrorResult *eResult = NvmExceptionToResult(e);
				pList->insert(deleteMsg +  eResult->outputText());
				pList->setErrorCode(eResult->getErrorCode());
				delete(eResult);
			}
		}
	}

	return pResult;
}

/*
 * Wrapper around wbem deleteNamespace function
 */
void cli::nvmcli::NamespaceFeature::wbemDeleteNamespace(const std::string &namespaceUid)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::pmem_config::PersistentMemoryServiceFactory provider;
	provider.deleteNamespace(namespaceUid);
}

/*
 * Create namespace
 */
cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::createNamespace(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	// Get the Pool UID
	pResult = m_pWbemToCli->checkPoolUid(parsedCommand, m_poolUid);

	m_forceOption = parsedCommand.options.find(framework::OPTION_FORCE.name)
					!= parsedCommand.options.end();

	// Type
	if (!pResult)
	{
		pResult = parseCreateNsType(parsedCommand);
	}

	// Block Size
	if (!pResult)
	{
		pResult = parseCreateNsBlockSize(parsedCommand);
	}

	// Advertised Capacity in GB
	if (!pResult)
	{
		pResult = parseCreateNsCapacity(parsedCommand);
	}

	// Block Count
	if (!pResult)
	{
		pResult = parseCreateNsBlockCount(parsedCommand);
	}

	if (!pResult)
	{
		pResult = parseNsFriendlyName(parsedCommand);
	}

	if (!pResult)
	{
		pResult = parseCreateNsEnabled(parsedCommand);
	}

	if (!pResult)
	{
		pResult = parseCreateNsOptimize(parsedCommand);
	}

	if (!pResult)
	{
		pResult = parseCreateNsEncryption(parsedCommand);
	}

	if (!pResult)
	{
		pResult = parseCreateNsEraseCapable(parsedCommand);
	}

	if (!pResult)
	{
		pResult = parseInterleaveSizes(parsedCommand);
	}

	if (!pResult)
	{
		pResult = parseCreateNsMemoryPageAllocation(parsedCommand);
	}

	if (!pResult)
	{
		try
		{
			std::string namespaceUid;

			NVM_UINT64 adjustedBlockCount = m_pPmServiceProvider->getAdjustedCreateNamespaceBlockCount(
					m_poolUid, m_nsType, m_blockSize, m_blockCount,
					m_eraseCapable, m_encryption, m_enableState);

			if (!adjustNamespaceBlockCount(adjustedBlockCount) || !confirmNamespaceBlockSizeUsage())
			{
				throw wbem::exception::NvmExceptionLibError(NVM_ERR_BADALIGNMENT);
			}

			// So far so good. Now try to create the namespace ...
			wbem::pmem_config::PersistentMemoryServiceFactory::createNamespaceParams parms;
			parms.type = m_nsType;
			parms.blockCount = m_blockCount;
			parms.blockSize = m_blockSize;
			parms.enabled = m_enableState;
			parms.encryption = m_encryption;
			parms.eraseCapable = m_eraseCapable;
			parms.friendlyName = m_friendlyName;
			parms.optimize = m_optimize;
			parms.poolId = m_poolUid;
			parms.interleaveChannelSize = m_channelSize;
			parms.interleaveControllerSize = m_controllerSize;
			parms.byOne = m_byOne;
			parms.memoryPageAllocation = m_memoryPageAllocation;
			m_pPmServiceProvider->createNamespace(parms, namespaceUid);

			// display output from showNamespace
			framework::ParsedCommand showNamespaceCommand;
			showNamespaceCommand.targets[TARGET_NAMESPACE.name] = namespaceUid;
			showNamespaceCommand.options[framework::OPTION_ALL.name] = "";
			std::string capacityUnits;
			pResult = getCapacityUnits(parsedCommand, &capacityUnits);
			if (pResult == NULL)
			{
				showNamespaceCommand.options[framework::OPTION_UNITS.name] = capacityUnits;
				pResult = showNamespaces(showNamespaceCommand);
			}
		}
		catch(wbem::framework::Exception &e)
		{
			if (pResult)
			{
				delete pResult;
			}
			pResult = nsNvmExceptionToResult(e);
		}
	}

	return pResult;
}

void cli::nvmcli::NamespaceFeature::wbemGetSupportedBlockSizes(
		std::vector<COMMON_UINT64> &sizes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::pmem_config::PersistentMemoryCapabilitiesFactory provider;
	provider.getSupportedBlockSizes(sizes);
}

void cli::nvmcli::NamespaceFeature::wbemGetSupportedSizeRange(const std::string &poolUid,
		COMMON_UINT64 &largestPossibleAdNs,
		COMMON_UINT64 &smallestPossibleAdNs,
		COMMON_UINT64 &adIncrement,
		COMMON_UINT64 &largestPossibleStorageNs,
		COMMON_UINT64 &smallestPossibleStorageNs,
		COMMON_UINT64 &storageIncrement)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::pmem_config::PersistentMemoryPoolFactory provider;

	provider.getSupportedSizeRange(poolUid, largestPossibleAdNs, smallestPossibleAdNs, adIncrement,
			largestPossibleStorageNs, smallestPossibleStorageNs, storageIncrement);
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseCreateNsType(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;
	m_nsTypeStr = framework::Parser::getPropertyValue(parsedCommand, CREATE_NS_PROP_TYPE);

	if (framework::stringsIEqual(m_nsTypeStr, CREATE_NS_PROP_TYPE_APPDIRECT))
	{
		m_nsType = wbem::pmem_config::PM_SERVICE_APP_DIRECT_TYPE;
	}
	else if (framework::stringsIEqual(m_nsTypeStr, CREATE_NS_PROP_TYPE_STORAGE))
	{
		m_nsType = wbem::pmem_config::PM_SERVICE_STORAGE_TYPE;
	}
	else
	{
		pResult = new framework::SyntaxErrorBadValueResult(
				framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_TYPE, m_nsTypeStr);
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseCreateNsBlockSize(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string value = framework::Parser::getPropertyValue(parsedCommand,
			CREATE_NS_PROP_BLOCKSIZE, &m_blockSizeExists);
	m_blockSize = 0;

	if (m_nsType == wbem::pmem_config::PM_SERVICE_APP_DIRECT_TYPE)
	{
		m_blockSize = 1u;
		if (m_blockSizeExists)
		{
			if(!isStringValidNumber(value))
			{
				pResult = new framework::SyntaxErrorBadValueResult(
						framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_BLOCKSIZE, value);
			}
			else
			{
				// the library will determine if this is a valid size
				m_blockSize = stringToUInt64(value);
			}
		}
	}
	else if (m_nsType == wbem::pmem_config::PM_SERVICE_STORAGE_TYPE)
	{
		if (m_blockSizeExists)
		{
			if(!isStringValidNumber(value))
			{
				pResult = new framework::SyntaxErrorBadValueResult(
						framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_BLOCKSIZE, value);
			}
			else
			{
				// the library will determine if this is a valid size
				m_blockSize = stringToUInt64(value);
			}
		}
		else
		{
			try
			{
				// get system supported values
				std::vector<COMMON_UINT64> supportedSizes;
				m_pCapProvider->getSupportedBlockSizes(supportedSizes);

				if (supportedSizes.size() > 0u)
				{
					m_blockSize = supportedSizes[0];
					for(size_t i = 1; i < supportedSizes.size(); i++)
					{
						m_blockSize = supportedSizes[i] < m_blockSize ? supportedSizes[i] : m_blockSize;
					}
				}
				else
				{
					COMMON_LOG_ERROR("GetSupportedBlockSizes returned 0 block sizes");
					pResult = new framework::ErrorResult(
							framework::ErrorResult::ERRORCODE_NOTSUPPORTED, NOTSUPPORTED_ERROR_STR, "");
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
	}

	// blocksize of 0 is not supported, fail before further parsing of capacity property
	if (m_blockSize == 0)
	{
		std::string errorString = framework::ResultBase::stringFromArgList(
				TR(BLOCKSIZE_NOT_SUPPORTED_STR.c_str()), m_blockSize);

		pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				errorString);
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseCreateNsCapacity(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string value = framework::Parser::getPropertyValue(parsedCommand,
			CREATE_NS_PROP_CAPACITY, &m_capacityExists);
	m_capacityGB = 0;

	if (m_capacityExists)
	{
		if ((!stringToReal32(value, &m_capacityGB)) ||
			(m_capacityGB == 0))
		{
			pResult = new framework::SyntaxErrorBadValueResult(
				framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_CAPACITY, value);
		}
		else if (m_nsType == wbem::pmem_config::PM_SERVICE_APP_DIRECT_TYPE)
		{
			if (m_blockSizeExists)
			{
				COMMON_LOG_ERROR(
						"Capacity cannot be used in conjunction with BlockSize and BlockCount properties.");
				std::string errorString = framework::ResultBase::stringFromArgList(
						TR(CANT_USE_TOGETHER_ERROR_STR.c_str()),
						CREATE_NS_PROP_CAPACITY.c_str(),
						CREATE_NS_PROP_BLOCKSIZE.c_str());
				pResult = new framework::SyntaxErrorResult(errorString);
			}
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseCreateNsBlockCount(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string value = framework::Parser::getPropertyValue(parsedCommand,
			CREATE_NS_PROP_BLOCKCOUNT, &m_blockCountExists);
	m_blockCount = 0;

	if (m_blockCountExists)
	{
		if (m_capacityExists)
		{
			COMMON_LOG_ERROR(
					"Capacity and BlockCount properties are exclusive and cannot be used together.");
			std::string errorString = framework::ResultBase::stringFromArgList(
					TR(CANT_USE_TOGETHER_ERROR_STR.c_str()),
					CREATE_NS_PROP_CAPACITY.c_str(),
					CREATE_NS_PROP_BLOCKCOUNT.c_str());
			pResult = new framework::SyntaxErrorResult(errorString);
		}
		else if (!isStringValidNumber(value))
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_BLOCKCOUNT, value);
		}
		else
		{
			// the library will determine if this is a valid count
			m_blockCount = stringToUInt64(value);
		}
	}
	else
	{
		try
		{
			if (!m_capacityExists)
			{
				COMMON_UINT64 minAdNamespaceSize = 0;
				COMMON_UINT64 maxAdNamespaceSize = 0;
				COMMON_UINT64 adNamespaceDivisor = 0;
				COMMON_UINT64 minStorageNamespaceSize = 0;
				COMMON_UINT64 maxStorageNamespaceSize = 0;
				COMMON_UINT64 storageNamespaceDivisor = 0;
				m_pPmPoolProvider->getSupportedSizeRange(m_poolUid,
						maxAdNamespaceSize, minAdNamespaceSize, adNamespaceDivisor,
						maxStorageNamespaceSize, minStorageNamespaceSize, storageNamespaceDivisor);

				if (m_nsType == wbem::pmem_config::PM_SERVICE_APP_DIRECT_TYPE)
				{
					m_blockCount = maxAdNamespaceSize; // App Direct NS has block size = 1

				}
				else if (m_nsType == wbem::pmem_config::PM_SERVICE_STORAGE_TYPE)
				{
					NVM_UINT32 realBlockSize = get_real_block_size(m_blockSize);
					m_blockCount = maxStorageNamespaceSize / realBlockSize;
				}
			}
			else
			{
				m_blockCount =
					calculateBlockCountForNamespace(m_capacityGB, m_blockSize);
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

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseNsFriendlyName(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string value = framework::Parser::getPropertyValue(parsedCommand,
			CREATE_NS_PROP_FRIENDLYNAME, &m_friendlyNameExists);

	if (m_friendlyNameExists)
	{
		if (value.size() > CREATE_NS_PROP_FRIENDLYNAME_MAX - 1) // -1 to account for null terminator
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_FRIENDLYNAME, value);
		}
		else
		{
			m_friendlyName = value;
		}
	}
	else
	{
		m_friendlyName = "";
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseCreateNsEnabled(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string value = framework::Parser::getPropertyValue(parsedCommand, CREATE_NS_PROP_ENABLED,
			&m_enabledStateExists);

	if (m_enabledStateExists)
	{
		if (value == "0" || framework::stringsIEqual(value, "false"))
		{
			m_enableState = wbem::pmem_config::PM_SERVICE_NAMESPACE_ENABLE_STATE_DISABLED;
		}
		else if (value == "1" || framework::stringsIEqual(value, "true"))
		{
			m_enableState = wbem::pmem_config::PM_SERVICE_NAMESPACE_ENABLE_STATE_ENABLED;
		}
		else
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_ENABLED, value);
		}
	}
	else
	{
		m_enableState = wbem::pmem_config::PM_SERVICE_NAMESPACE_ENABLE_STATE_ENABLED;
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseCreateNsOptimize(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string value = framework::Parser::getPropertyValue(parsedCommand, CREATE_NS_PROP_OPTIMIZE, &m_optimizeExists);

	if (m_optimizeExists)
	{
		if (framework::stringsIEqual(value, CREATE_NS_PROP_OPTIMIZE_COPYONWRITE))
		{
			m_optimize = wbem::pmem_config::PM_SERVICE_OPTIMIZE_COPYONWRITE;
		}
		else if (framework::stringsIEqual(value, wbem::NONE))
		{
			m_optimize = wbem::pmem_config::PM_SERVICE_OPTIMIZE_NONE;
		}
		else
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_ENABLED, value);
		}
	}
	else
	{
		if (m_nsType == wbem::pmem_config::PM_SERVICE_APP_DIRECT_TYPE)
		{
			m_optimize = wbem::pmem_config::PM_SERVICE_OPTIMIZE_NONE;
		}
		else if (m_nsType == wbem::pmem_config::PM_SERVICE_STORAGE_TYPE)
		{
			m_optimize = wbem::pmem_config::PM_SERVICE_OPTIMIZE_COPYONWRITE;
		}
	}

	return pResult;
}

bool cli::nvmcli::NamespaceFeature::optimizePropertyExists()
{
	return (m_optimizeExists)
			&& (m_optimize != wbem::pmem_config::PM_SERVICE_OPTIMIZE_NONE);
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseMemoryPageAllocationForAppDirectNS(
		const std::string& requestedMode)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	if (framework::stringsIEqual(requestedMode, CREATE_NS_PROP_MEMORYPAGEALLOCATION_DRAM) &&
			m_pCapProvider->getMemoryPageAllocationCapability())
	{
		m_memoryPageAllocation =
				wbem::pmem_config::PM_SERVICE_MEMORYPAGEALLOCATION_DRAM;
	}
	else if (framework::stringsIEqual(requestedMode, CREATE_NS_PROP_MEMORYPAGEALLOCATION_APPDIRECT) &&
			m_pCapProvider->getMemoryPageAllocationCapability())
	{
		m_memoryPageAllocation =
				wbem::pmem_config::PM_SERVICE_MEMORYPAGEALLOCATION_APP_DIRECT;
	}
	else if ((framework::stringsIEqual(requestedMode, CREATE_NS_PROP_MEMORYPAGEALLOCATION_DRAM) ||
		framework::stringsIEqual(requestedMode, CREATE_NS_PROP_MEMORYPAGEALLOCATION_APPDIRECT)) &&
		(!m_pCapProvider->getMemoryPageAllocationCapability()))
	{
		COMMON_LOG_ERROR("Driver does not support legacy memory page protocols.");
		pResult = new framework::ErrorResult(
				framework::ErrorResult::ERRORCODE_NOTSUPPORTED,
				NOTSUPPORTED_ERROR_STR, "");
	}
	else if (framework::stringsIEqual(requestedMode, wbem::NONE))
	{
		m_memoryPageAllocation =
				wbem::pmem_config::PM_SERVICE_MEMORYPAGEALLOCATION_NONE;
	}
	else
	{
		pResult = new framework::SyntaxErrorBadValueResult(
				framework::TOKENTYPE_PROPERTY,
				CREATE_NS_PROP_MEMORYPAGEALLOCATION, requestedMode);
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseCreateNsMemoryPageAllocation(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	m_memoryPageAllocation = wbem::pmem_config::PM_SERVICE_MEMORYPAGEALLOCATION_NONE;

	bool hasProp;
	std::string requestedMode =
			framework::Parser::getPropertyValue(parsedCommand, CREATE_NS_PROP_MEMORYPAGEALLOCATION, &hasProp);
	if (hasProp)
	{
		if (m_nsType == wbem::pmem_config::PM_SERVICE_APP_DIRECT_TYPE)
		{
			pResult = parseMemoryPageAllocationForAppDirectNS(requestedMode);
		}
		else
		{ // storage namespace
			if (!framework::stringsIEqual(requestedMode, wbem::NONE))
			{
				COMMON_LOG_ERROR("Memory page allocation is not supported for storage namespaces");
				pResult = new framework::ErrorResult(
						framework::ErrorResult::ERRORCODE_NOTSUPPORTED, NOTSUPPORTED_ERROR_STR, "");
			}
		}
	}
	else
	{ // default values
		m_memoryPageAllocation = wbem::pmem_config::PM_SERVICE_MEMORYPAGEALLOCATION_NONE;

		// default to AppDirect for AppDirect Namespaces if capable and if btt is not requested
		if (!optimizePropertyExists() &&
				(m_nsType == wbem::pmem_config::PM_SERVICE_APP_DIRECT_TYPE) &&
				(m_pCapProvider->getMemoryPageAllocationCapability()))
		{
			m_memoryPageAllocation = wbem::pmem_config::PM_SERVICE_MEMORYPAGEALLOCATION_APP_DIRECT;
		}
	}

	if (!pResult && optimizePropertyExists() &&
			m_memoryPageAllocation != wbem::pmem_config::PM_SERVICE_MEMORYPAGEALLOCATION_NONE)
	{
		COMMON_LOG_ERROR(
				"Namespace can be claimed by either btt or pfn configurations.");
		std::string errorString = framework::ResultBase::stringFromArgList(
				TR(CANT_USE_TOGETHER_ERROR_STR.c_str()),
				CREATE_NS_PROP_OPTIMIZE.c_str(),
				CREATE_NS_PROP_MEMORYPAGEALLOCATION.c_str());
		pResult = new framework::SyntaxErrorResult(errorString);
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseCreateNsEncryption(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	bool hasProp;
	std::string value = framework::Parser::getPropertyValue(parsedCommand, CREATE_NS_PROP_ENCRYPTION, &hasProp);
	if (hasProp)
	{
		if (value == "0" || framework::stringsIEqual(value, "false") ||
			framework::stringsIEqual(value, "no"))
		{
			m_encryption = wbem::pmem_config::PM_SERVICE_SECURITY_ENCRYPTION_OFF;
		}
		else if (value == "1" || framework::stringsIEqual(value, "true") ||
			framework::stringsIEqual(value, "yes"))
		{
			m_encryption = wbem::pmem_config::PM_SERVICE_SECURITY_ENCRYPTION_ON;
		}
		else if (framework::stringsIEqual(value, "ignore"))
		{
			m_encryption = wbem::pmem_config::PM_SERVICE_SECURITY_ENCRYPTION_IGNORE;
		}
		else
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_ENCRYPTION, value);
		}
	}
	else
	{
		m_encryption = wbem::pmem_config::PM_SERVICE_SECURITY_ENCRYPTION_IGNORE;
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseCreateNsEraseCapable(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	bool hasProp;
	std::string value = framework::Parser::getPropertyValue(parsedCommand, CREATE_NS_PROP_ERASECAPABLE, &hasProp);
	if (hasProp)
	{
		if (value == "0" || framework::stringsIEqual(value, "false") ||
			framework::stringsIEqual(value, "no"))
		{
			m_eraseCapable = wbem::pmem_config::PM_SERVICE_SECURITY_ERASE_CAPABLE_FALSE;
		}
		else if (value == "1" || framework::stringsIEqual(value, "true") ||
			framework::stringsIEqual(value, "yes"))
		{
			m_eraseCapable = wbem::pmem_config::PM_SERVICE_SECURITY_ERASE_CAPABLE_TRUE;
		}
		else if (framework::stringsIEqual(value, "ignore"))
		{
			m_eraseCapable = wbem::pmem_config::PM_SERVICE_SECURITY_ERASE_CAPABLE_IGNORE;
		}
		else
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_ERASECAPABLE, value);
		}
	}
	else
	{
		m_eraseCapable = wbem::pmem_config::PM_SERVICE_SECURITY_ERASE_CAPABLE_IGNORE;
	}

	return pResult;
}

cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::parseInterleaveSizes(
		const cli::framework::ParsedCommand &parsedCommand)
{
	cli::framework::ResultBase * pResult = NULL;

	bool exists;
	const std::string &value = cli::framework::Parser::getPropertyValue(parsedCommand,
		APPDIRECTSETTINGS_PROPERTYNAME, &exists);

	MemoryProperty interleaveSizes(parsedCommand, "", APPDIRECTSETTINGS_PROPERTYNAME);
	if (exists)
	{
		const bool isNotValidNsAppDirectSetting =
			interleaveSizes.getIsMirrored() || !interleaveSizes.getIsSettingsValid();
		if (isNotValidNsAppDirectSetting)
		{
			pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
				APPDIRECTSETTINGS_PROPERTYNAME, value);
		}
		else if (m_nsType != wbem::pmem_config::PM_SERVICE_APP_DIRECT_TYPE)
		{
			pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(INVALID_NS_APP_DIRECT_SETTINGS));
		}
		else
		{
			m_channelSize = wbem::mem_config::InterleaveSet::getExponentFromInterleaveSize(
				interleaveSizes.getFormatSizes().channel);
			m_controllerSize = wbem::mem_config::InterleaveSet::getExponentFromInterleaveSize(
				interleaveSizes.getFormatSizes().imc);
			m_byOne = interleaveSizes.getIsByOne();
		}
	}

	return pResult;
}


cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::modifyNamespace(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	cli::framework::ResultBase* pResult = NULL;

	if (parsedCommand.properties.size() == 0)
	{
		pResult = new framework::SyntaxErrorResult(TRS(NOMODIFIABLEPROPERTY_ERROR_STR));
	}

	std::vector<std::string>nsList;
	if (!pResult)
	{
		pResult = m_pWbemToCli->getNamespaces(parsedCommand, nsList);
	}
	if (!pResult)
	{
		pResult = parseModifyNsBlockCount(parsedCommand);
	}

	if (!pResult)
	{
		pResult = parseModifyNsCapacity(parsedCommand);
	}

	if (!pResult)
	{
		pResult = parseNsFriendlyName(parsedCommand);
	}

	if (!pResult)
	{
		pResult  = parseCreateNsEnabled(parsedCommand);
	}

	if (!pResult)
	{
		framework::SimpleListResult *pList = new framework::SimpleListResult();
		pResult = pList;
		m_prefix = "Modify namespace ";

		m_forceOption = parsedCommand.options.find(framework::OPTION_FORCE.name)
				!= parsedCommand.options.end();

		for (size_t i = 0; i < nsList.size(); i++)
		{
			try
			{
				// if user didn't specify the force option, prompt them to continue
				std::string prompt = framework::ResultBase::stringFromArgList(
						MODIFY_NS_PROMPT.c_str(), nsList[i].c_str());

				if (!m_forceOption && !promptUserYesOrNo(prompt))
				{
					pList->insert(m_prefix + nsList[i] + ": " + TRS(cli::framework::UNCHANGED_MSG));
				}
				else
				{
					struct namespace_details details;
					m_pPmServiceProvider->getNamespaceDetails(nsList[i], details);
					m_blockSize = details.block_size;

					if (isNamespaceModificationSupported(details))
					{
						if (isBlockCountAligned(nsList[i]))
						{
							atomicModifyNamespace(nsList[i], details);
						}
						else
						{
							pList->insert(m_prefix + nsList[i] + ": " + TRS(cli::framework::UNCHANGED_MSG));
							continue;
						}
					}
					else
					{
						COMMON_LOG_ERROR("Requested namespace modification is not supported");
						throw wbem::framework::ExceptionNotSupported(__FILE__, (char *)__func__);
					}
					pList->insert(m_prefix + nsList[i] + ": " + cli::framework::SUCCESS_MSG);
				}
			}
			catch (wbem::framework::Exception &e)
			{
				framework::ErrorResult *pError = nsNvmExceptionToResult(e);
				pList->insert(m_prefix + nsList[i] + ": " + pError->outputText());
				if (!pList->getErrorCode()) // keep errors
				{
					pList->setErrorCode(pError->getErrorCode());
				}
				delete(pError);
			}
		}
	}

	return pResult;
}

/*
 * Return true if the namespace block count has the correct alignment
 */
bool cli::nvmcli::NamespaceFeature::isBlockCountAligned(std::string namespaceUidStr)
{
	bool isAligned = true;

	if (m_blockCountExists || m_capacityExists)
	{
		NVM_UINT64 adjustedBlockCount = m_pPmServiceProvider->getAdjustedModifyNamespaceBlockCount(
				namespaceUidStr, m_blockCount);

		isAligned = adjustNamespaceBlockCount(adjustedBlockCount);
	}

	return isAligned;
}

/*
 * Parsing helpers for modify namespace
 */
cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseModifyNsBlockCount(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	cli::framework::ResultBase* pResult = NULL;
	std::string value = framework::Parser::getPropertyValue(parsedCommand,
			CREATE_NS_PROP_BLOCKCOUNT, &m_blockCountExists);

	if (m_blockCountExists)
	{
		if(!isStringValidNumber(value))
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_BLOCKSIZE, value);
		}
		else
		{
			// the library will determine if this is a valid
			m_blockCount = stringToUInt64(value);
			if (m_blockCount <= 0)
			{
				pResult = new framework::SyntaxErrorBadValueResult(
						framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_BLOCKSIZE, value);
			}
		}
	}

	return pResult;
}

cli::framework::ResultBase* cli::nvmcli::NamespaceFeature::parseModifyNsCapacity(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;

	std::string value = framework::Parser::getPropertyValue(parsedCommand,
			CREATE_NS_PROP_CAPACITY, &m_capacityExists);
	m_capacityGB = 0;

	if (m_capacityExists)
	{
		if (m_blockCountExists)
		{
			COMMON_LOG_ERROR(
					"Capacity and BlockCount are exclusive and cannot be used together.");
			std::string errorString = framework::ResultBase::stringFromArgList(
					TR(CANT_USE_TOGETHER_ERROR_STR.c_str()),
					CREATE_NS_PROP_CAPACITY.c_str(),
					CREATE_NS_PROP_BLOCKCOUNT.c_str());
			pResult = new framework::SyntaxErrorResult(errorString);
		}
		else if ((!stringToReal32(value, &m_capacityGB)) ||
				(m_capacityGB == 0))
		{
			pResult = new framework::SyntaxErrorBadValueResult(
					framework::TOKENTYPE_PROPERTY, CREATE_NS_PROP_CAPACITY, value);
		}
	}

	return pResult;
}

cli::framework::ErrorResult* cli::nvmcli::NamespaceFeature::nsNvmExceptionToResult(
		wbem::framework::Exception& e, std::string prefix)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::exception::NvmExceptionLibError * pLibError =
			dynamic_cast<wbem::exception::NvmExceptionLibError *>(&e);

	framework::ErrorResult *pResult = NULL;
	if (pLibError)
	{
		switch (pLibError->getLibError())
		{
		case NVM_ERR_BADPOOL:
		{
			// Use the underlying API error message string
			pResult = new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN,
					TR(pLibError->what()));
			break;
		}
		case NVM_ERR_BADBLOCKSIZE:
		{
			std::string errorString = framework::ResultBase::stringFromArgList(
					TR(BLOCKSIZE_NOT_SUPPORTED_STR.c_str()), m_blockSize);

			pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
					errorString);
			break;
		}
		case NVM_ERR_BADSIZE:
		{
			char errbuff[NVM_ERROR_LEN];
			s_snprintf(errbuff, NVM_ERROR_LEN,
					TR("The block count '%llu' is not valid."),
					m_blockCount);
			pResult = new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN,
					errbuff);
			break;
		}
		case NVM_ERR_BADNAMESPACETYPE:
		{
			char errbuff[NVM_ERROR_LEN];
			s_snprintf(errbuff, NVM_ERROR_LEN,
					TR("The namespace type '%s' is not valid for the given pool."),
					m_nsTypeStr.c_str());
			pResult = new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN,
					errbuff);
			break;
		}
		case NVM_ERR_BADNAMESPACESETTINGS:
		{
			pResult = new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN,
					TRS(INVALID_NS_APP_DIRECT_SETTINGS));
			break;
		}
		} // end switch
	}

	if (!pResult)
	{
		pResult = NvmExceptionToResult(e, prefix);
	}
	return pResult;
}

bool cli::nvmcli::NamespaceFeature::namespaceCapacityModificationIsSupported(
		const namespace_details &details)
{
	m_blockCount = calculateBlockCountForNamespace(m_capacityGB, m_blockSize);
	return m_pPmServiceProvider->isModifyNamespaceBlockCountSupported(details, m_blockCount);
}

bool cli::nvmcli::NamespaceFeature::isNamespaceModificationSupported(const namespace_details &details)
{
	bool isSupported = true;

	if (m_friendlyNameExists && !m_pPmServiceProvider->isModifyNamespaceNameSupported())
	{
		isSupported = false;
	}

	if (m_blockCountExists && !m_pPmServiceProvider->isModifyNamespaceBlockCountSupported(details, m_blockCount))
	{
		isSupported = false;
	}

	if (m_capacityExists)
	{
		isSupported = namespaceCapacityModificationIsSupported(details);
	}

	enum namespace_enable_state enabled =
			wbem::pmem_config::PersistentMemoryServiceFactory::namespaceEnabledToEnum(m_enableState);
	if (m_enabledStateExists && !m_pPmNamespaceProvider->isModifyNamespaceEnabledSupported(enabled))
	{
		isSupported = false;
	}

	return isSupported;
}

bool cli::nvmcli::NamespaceFeature::adjustNamespaceBlockCount(NVM_UINT64 adjustedBlockCount)
{
	bool result = true;

	NVM_UINT32 realBlockSize = get_real_block_size(m_blockSize);

	NVM_UINT64 requestedCapacity = realBlockSize * m_blockCount;

	if (adjustedBlockCount != m_blockCount)
	{
		NVM_UINT64 adjustedCapacity = realBlockSize * adjustedBlockCount;

		// if user didn't specify the force option, prompt them to continue
		std::string prompt = framework::ResultBase::stringFromArgList(NS_ALIGNMENT_PROMPT.c_str(),
			requestedCapacity, adjustedCapacity);

		if ((m_blockCountExists || m_capacityExists)  && !m_forceOption && !promptUserYesOrNo(prompt))
		{
			result = false;
		}
		else
		{
			m_blockCount = adjustedBlockCount;
		}
	}

	return result;
}

bool cli::nvmcli::NamespaceFeature::confirmNamespaceBlockSizeUsage()
{
	bool result = true;

	// actual block size used is rounded up to the nearest multiple of cache line size
	NVM_UINT64 blockSizeInt = get_real_block_size(m_blockSize);
	if (blockSizeInt != m_blockSize)
	{
		std::string prompt = framework::ResultBase::stringFromArgList(
			CREATE_NS_SMALL_BLOCK_SIZE_PROMPT.c_str(),
			m_blockSize * m_blockCount , blockSizeInt * m_blockCount);

		if (!m_forceOption && !promptUserYesOrNo(prompt))
		{
			result = false;
		}
	}

	return result;
}

void cli::nvmcli::NamespaceFeature::atomicModifyNamespace(
		const std::string namespaceUidStr,
		const struct namespace_details &details)
{
	try
	{
		modifyNamespace(namespaceUidStr);
	}
	catch (wbem::framework::ExceptionBadParameter &)
	{
		COMMON_LOG_ERROR("Namespace uid is not valid.");
		throw;
	}
	catch (wbem::exception::NvmExceptionLibError &e)
	{
		undoModifyNamespace(namespaceUidStr, details, e.getLibError());
		throw;
	}
}

void cli::nvmcli::NamespaceFeature::modifyNamespace(
		const std::string namespaceUidStr)
{
	if (m_friendlyNameExists)
	{
		m_pPmServiceProvider->modifyNamespaceName(namespaceUidStr, m_friendlyName);
	}

	if (m_blockCountExists || m_capacityExists)
	{
		m_pPmServiceProvider->modifyNamespaceBlockCount(namespaceUidStr, m_blockCount);
	}

	if (m_enabledStateExists)
	{
		m_pPmNamespaceProvider->modifyNamespace(namespaceUidStr, m_enableState);
	}
}

void cli::nvmcli::NamespaceFeature::undoModifyNamespace(
		const std::string namespaceUidStr, const struct namespace_details &previousDetails, const int modifyError)
{
	struct namespace_details newDetails;

	try
	{
		m_pPmServiceProvider->getNamespaceDetails(namespaceUidStr, newDetails);

		if (s_strncmp(newDetails.discovery.friendly_name,
						previousDetails.discovery.friendly_name, NVM_NAMESPACE_NAME_LEN) != 0)
		{
			m_pPmServiceProvider->modifyNamespaceName(namespaceUidStr, previousDetails.discovery.friendly_name);
		}

		if (newDetails.block_count != previousDetails.block_count)
		{
			m_pPmServiceProvider->modifyNamespaceBlockCount(namespaceUidStr, previousDetails.block_count);
		}

		if (newDetails.enabled != previousDetails.enabled)
		{
			 m_pPmNamespaceProvider->modifyNamespace(namespaceUidStr, previousDetails.enabled);
		}
	}
	catch (wbem::framework::Exception &)
	{
		COMMON_LOG_ERROR("Failed to retrieve namespace details");
		throw wbem::exception::NvmExceptionUndoModifyFailed(modifyError);
	}
}

void cli::nvmcli::NamespaceFeature::convertEnabledStateAttributes(wbem::framework::Instance &wbemInstance)
{
	wbem::framework::Attribute enabledAattr;
	if (wbemInstance.getAttribute(wbem::ENABLEDSTATE_KEY, enabledAattr) ==
		wbem::framework::SUCCESS)
	{
		int enabled;
		switch (enabledAattr.intValue())
		{
			case NAMESPACE_ENABLE_STATE_ENABLED:
				enabled = 1;
				break;
			case NAMESPACE_ENABLE_STATE_DISABLED:
			default:
				enabled = 0;
				break;
		}

		wbemInstance.setAttribute(wbem::ENABLEDSTATE_KEY,
			wbem::framework::Attribute(enabled, false));
	}
}

void cli::nvmcli::NamespaceFeature::populateNamespaceAttributes(
	wbem::framework::attribute_names_t &attributes,
	cli::framework::ParsedCommand const &parsedCommand)
{
	// define default display attributes
	wbem::framework::attribute_names_t defaultAttributes;
	defaultAttributes.push_back(wbem::NAMESPACEID_KEY);
	defaultAttributes.push_back(wbem::TYPE_KEY);
	defaultAttributes.push_back(wbem::CAPACITY_KEY);
	defaultAttributes.push_back(wbem::HEALTHSTATE_KEY);
	defaultAttributes.push_back(wbem::ACTIONREQUIRED_KEY);

	// define all attributes
	wbem::framework::attribute_names_t allAttributes(defaultAttributes);
	allAttributes.push_back(wbem::ACTIONREQUIREDEVENTS_KEY);
	allAttributes.push_back(wbem::NAME_KEY);
	allAttributes.push_back(wbem::POOLID_KEY);
	allAttributes.push_back(wbem::BLOCKSIZE_KEY);
	allAttributes.push_back(wbem::BLOCKCOUNT_KEY);
	allAttributes.push_back(wbem::ENABLED_KEY);
	allAttributes.push_back(wbem::OPTIMIZE_KEY);
	allAttributes.push_back(wbem::ERASECAPABLE_KEY);
	allAttributes.push_back(wbem::ENCRYPTIONENABLED_KEY);
	allAttributes.push_back(wbem::APP_DIRECT_SETTINGS_KEY);
	allAttributes.push_back(wbem::MEMORYPAGEALLOCATION_KEY);

	// get the desired attributes
	wbem::framework::attribute_names_t desiredAttributes =
		GetAttributeNames(parsedCommand.options, defaultAttributes, allAttributes);
	attributes = desiredAttributes;

	// make sure we have the NamespaceID in our display
	// this would cover the case the user asks for specific display attributes, but they
	// don't include the ID
	if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(wbem::NAMESPACEID_KEY,
			attributes))
	{
		attributes.insert(attributes.begin(), wbem::NAMESPACEID_KEY);
	}

	// make sure we have the Replication (mirrored) information. It will be filtered out later.
	if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(wbem::REPLICATION_KEY,
		attributes))
	{
		attributes.push_back(wbem::REPLICATION_KEY);
	}

	// wbem attribute names are different than CLI
	RenameAttributeKey(attributes, wbem::BLOCKCOUNT_KEY, wbem::NUMBEROFBLOCKS_KEY);
	RenameAttributeKey(attributes, wbem::ENABLED_KEY, wbem::ENABLEDSTATE_KEY);
}

void cli::nvmcli::NamespaceFeature::convertActionRequiredEventsToNAIfEmpty(wbem::framework::Instance &wbemInstance)
{
	wbem::framework::Attribute arEventAttr;
	if (wbemInstance.getAttribute(wbem::ACTIONREQUIREDEVENTS_KEY, arEventAttr) == wbem::framework::SUCCESS)
	{
		wbem::framework::STR_LIST arEventList = arEventAttr.strListValue();
		if (!arEventList.size())
		{
			arEventList.push_back(wbem::NA);
			wbemInstance.setAttribute(wbem::ACTIONREQUIREDEVENTS_KEY,
					wbem::framework::Attribute(arEventList, false));
		}
	}
}
