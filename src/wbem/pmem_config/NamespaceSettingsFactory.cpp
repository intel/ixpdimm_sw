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
 * This file contains the provider for the NamespaceSettings instances which
 * represent the current settings for a namespace as well as the transient
 * settings used when creating a new namespace.
 */

#include <LogEnterExit.h>
#include <libinvm-cim/Attribute.h>
#include <server/BaseServerFactory.h>
#include <uid/uid.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <mem_config/InterleaveSet.h>
#include "NamespaceSettingsFactory.h"
#include "NamespaceViewFactory.h"
#include <sstream>
#include <NvmStrings.h>
#include <core/Helper.h>

wbem::pmem_config::NamespaceSettingsFactory::NamespaceSettingsFactory()
throw(wbem::framework::Exception)
{
}

wbem::pmem_config::NamespaceSettingsFactory::~NamespaceSettingsFactory()
{
}

void wbem::pmem_config::NamespaceSettingsFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(ALLOCATIONUNITS_KEY);
	attributes.push_back(CHANGEABLETYPE_KEY);
	attributes.push_back(RESERVATION_KEY);
	attributes.push_back(POOLID_KEY);
	attributes.push_back(RESOURCETYPE_KEY);
	attributes.push_back(OPTIMIZE_KEY);
	attributes.push_back(ENCRYPTIONENABLED_KEY);
	attributes.push_back(ERASECAPABLE_KEY);
	attributes.push_back(CHANNELINTERLEAVESIZE_KEY);
	attributes.push_back(CONTROLLERINTERLEAVESIZE_KEY);
	attributes.push_back(MEMORYPAGEALLOCATION_KEY);

	// NOTE: No need to populate InitialState - only for create
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance *wbem::pmem_config::NamespaceSettingsFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		std::string nsUidStr = path.getKeyValue(INSTANCEID_KEY).stringValue();
		if (!core::Helper::isValidNamespaceUid(nsUidStr))
		{
			COMMON_LOG_ERROR_F("NamespaceSettings InstanceID is not a valid namespace uid %s",
					nsUidStr.c_str());
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}

		struct namespace_details ns = NamespaceViewFactory::getNamespaceDetails(nsUidStr);

		// ElementName = Friendly Name
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			framework::Attribute a(
					std::string(NSSETTINGS_ELEMENTNAME_PREFIX + ns.discovery.friendly_name), false);
			pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
		}

		// AllocationUnits = block size as a string
		if (containsAttribute(ALLOCATIONUNITS_KEY, attributes))
		{
			std::stringstream allocationUnits;
			allocationUnits << NSSETTINGS_ALLOCATIONUNITS_BYTES;
			allocationUnits << "*";
			allocationUnits << ns.block_size;
			framework::Attribute a(allocationUnits.str(), false);
			pInstance->setAttribute(ALLOCATIONUNITS_KEY, a, attributes);
		}

		// Reservation = block count
		if (containsAttribute(RESERVATION_KEY, attributes))
		{
			NVM_UINT64 nsBytes = ns.block_count * ns.block_size;
			framework::Attribute a(nsBytes, false);
			pInstance->setAttribute(RESERVATION_KEY, a, attributes);
		}

		// PoolID = Pool UID
		if (containsAttribute(POOLID_KEY, attributes))
		{
			NVM_UID poolUidStr;
			uid_copy(ns.pool_uid, poolUidStr);
			framework::Attribute a(poolUidStr, false);
			pInstance->setAttribute(POOLID_KEY, a, attributes);
		}

		// ResourceType = type
		if (containsAttribute(RESOURCETYPE_KEY, attributes))
		{
			framework::Attribute a(
					namespaceResourceTypeToValue(ns.type),
					namespaceResourceTypeToStr(ns.type), false);
			pInstance->setAttribute(RESOURCETYPE_KEY, a, attributes);
		}

		// Optimize = Btt
		if (containsAttribute(OPTIMIZE_KEY, attributes))
		{
			framework::Attribute a(
					NamespaceViewFactory::namespaceOptimizeToValue(ns.btt),
					NamespaceViewFactory::namespaceOptimizeToStr(ns.btt), false);
			pInstance->setAttribute(OPTIMIZE_KEY, a, attributes);
		}

		// ChangeableType = 0 - "Fixed � Not Changeable"
		if (containsAttribute(CHANGEABLETYPE_KEY, attributes))
		{
			framework::Attribute a(NSSETTINGS_CHANGEABLETYPE_NOTCHANGEABLETRANSIENT, false);
			pInstance->setAttribute(CHANGEABLETYPE_KEY, a, attributes);
		}
	
		// SecurityFeatures
                if (containsAttribute(ENCRYPTIONENABLED_KEY, attributes))
                {
                        pInstance->setAttribute(ENCRYPTIONENABLED_KEY,
                                framework::Attribute((NVM_UINT16)ns.security_features.encryption, false));
                }

                if (containsAttribute(ERASECAPABLE_KEY, attributes))
                {
                        pInstance->setAttribute(ERASECAPABLE_KEY,
                                framework::Attribute((NVM_UINT16)ns.security_features.erase_capable, false));
                }

		// ChannelInterleaveSize
		if (containsAttribute(CHANNELINTERLEAVESIZE_KEY, attributes))
		{
			NVM_UINT16 channelSize =
					(NVM_UINT16)mem_config::InterleaveSet::getExponentFromInterleaveSize(
								ns.interleave_format.channel);

			framework::Attribute a(channelSize, false);
			pInstance->setAttribute(CHANNELINTERLEAVESIZE_KEY, a, attributes);
		}

		// ControllerInterleaveSize
		if (containsAttribute(CONTROLLERINTERLEAVESIZE_KEY, attributes))
		{
			NVM_UINT16 channelSize =
					(NVM_UINT16)mem_config::InterleaveSet::getExponentFromInterleaveSize(
								ns.interleave_format.imc);

			framework::Attribute a(channelSize, false);
			pInstance->setAttribute(CONTROLLERINTERLEAVESIZE_KEY, a, attributes);
		}

		if (containsAttribute(MEMORYPAGEALLOCATION_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT16)ns.memory_page_allocation,
				NamespaceViewFactory::namespaceMemoryPageAllocationToStr(ns.memory_page_allocation), false);
			pInstance->setAttribute(MEMORYPAGEALLOCATION_KEY, a, attributes);
		}

		// NOTE: No need to populate Parent, or InitialState - only for create
	}
	catch (framework::Exception &)
	{
		if (pInstance)
		{
			delete pInstance;
			pInstance = NULL;
		}
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for the Intel_PoolView class.
 */
wbem::framework::instance_names_t *wbem::pmem_config::NamespaceSettingsFactory::getInstanceNames()
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::vector<std::string> nsList = NamespaceViewFactory::getNamespaceUidList();
		for (std::vector<std::string>::const_iterator iter = nsList.begin();
				iter != nsList.end(); iter++)
		{
			framework::attributes_t keys;

			// InstanceID = Namespace UID
			keys[INSTANCEID_KEY] = framework::Attribute(*iter, true);

			framework::ObjectPath path(server::getHostName(), NVM_NAMESPACE,
					NSSETTINGS_CREATIONCLASSNAME, keys);
			pNames->push_back(path);
		}
	}
	catch (framework::Exception &)
	{
		delete pNames;
		throw;
	}
	return pNames;
}

/*
 * Helper function to convert namespace type to resource type string
 */
std::string wbem::pmem_config::NamespaceSettingsFactory::namespaceResourceTypeToStr(
		const enum namespace_type &type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string typeStr;
	switch (type)
	{
		case NAMESPACE_TYPE_STORAGE:
			typeStr = "Storage Volume";
			break;
		case NAMESPACE_TYPE_APP_DIRECT:
			typeStr = "Non-Volatile Memory";
			break;
		default:
			typeStr = "Unknown";
			break;
	}
	return typeStr;
}

/*
 * Helper function to convert namespace type to resource type value
 */
NVM_UINT16 wbem::pmem_config::NamespaceSettingsFactory::namespaceResourceTypeToValue(
		const enum namespace_type &type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 typeVal;
	switch (type)
	{
		case NAMESPACE_TYPE_STORAGE:
			typeVal = NS_RESOURCETYPE_BLOCK_ADDRESSABLE;
			break;
		case NAMESPACE_TYPE_APP_DIRECT:
			typeVal = NS_RESOURCETYPE_BYTE_ADDRESSABLE;
			break;
		default:
			typeVal = 0;
			break;
	}
	return typeVal;
}

