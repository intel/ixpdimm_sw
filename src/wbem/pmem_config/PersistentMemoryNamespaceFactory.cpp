/*
 * Copyright (c) 2015, Intel Corporation
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
 * This file contains the provider for the PeristentMemoryNamespace instances.
 */

#include <LogEnterExit.h>
#include <nvm_management.h>
#include <intel_cim_framework/Attribute.h>
#include <server/BaseServerFactory.h>
#include <guid/guid.h>
#include <intel_cim_framework/ExceptionBadParameter.h>
#include "PersistentMemoryNamespaceFactory.h"
#include "NamespaceViewFactory.h"
#include <memory/RawMemoryFactory.h>
#include <pmem_config/PersistentMemoryPoolFactory.h>
#include <string.h>
#include <intel_cim_framework/ExceptionNoMemory.h>
#include <intel_cim_framework/ExceptionNotSupported.h>
#include <exception/NvmExceptionLibError.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <NvmStrings.h>
#include "PersistentMemoryServiceFactory.h"

wbem::pmem_config::PersistentMemoryNamespaceFactory::PersistentMemoryNamespaceFactory()
throw(wbem::framework::Exception)
{
	m_modifyNamespaceEnabled = nvm_modify_namespace_enabled;
}

wbem::pmem_config::PersistentMemoryNamespaceFactory::~PersistentMemoryNamespaceFactory()
{
}

void wbem::pmem_config::PersistentMemoryNamespaceFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(DEVICEID_KEY);

	// add non-key attributes
	attributes.push_back(HEALTHSTATE_KEY);
	attributes.push_back(NAME_KEY);
	attributes.push_back(ENABLEDSTATE_KEY);
	attributes.push_back(NUMBEROFBLOCKS_KEY);
	attributes.push_back(BLOCKSIZE_KEY);
	attributes.push_back(VOLATILE_KEY);
	attributes.push_back(OPERATIONALSTATUS_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance *wbem::pmem_config::PersistentMemoryNamespaceFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);

		path.checkKey(SYSTEMCREATIONCLASSNAME_KEY,
				wbem::server::BASESERVER_CREATIONCLASSNAME);
		path.checkKey(SYSTEMNAME_KEY, server::getHostName());
		path.checkKey(CREATIONCLASSNAME_KEY, PMNS_CREATIONCLASSNAME);

		std::string nsGuidStr = path.getKeyValue(DEVICEID_KEY).stringValue();
		if (nsGuidStr.length() != NVM_GUIDSTR_LEN - 1)
		{
			COMMON_LOG_ERROR_F("PersistentMemoryNamespace DeviceID is not a valid namespace guid %s",
					nsGuidStr.c_str());
			throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
		}

		struct namespace_details ns = NamespaceViewFactory::getNamespaceDetails(nsGuidStr);

		// Health State = enum + string
		if (containsAttribute(HEALTHSTATE_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT16)ns.health,
					NamespaceViewFactory::namespaceHealthToStr(ns.health), false);
			pInstance->setAttribute(HEALTHSTATE_KEY, a, attributes);
		}

		// Name = Friendly Name
		if (containsAttribute(NAME_KEY, attributes))
		{
			framework::Attribute a(ns.discovery.friendly_name, false);
			pInstance->setAttribute(NAME_KEY, a, attributes);
		}

		// Enabled State = enum + string
		if (containsAttribute(ENABLEDSTATE_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT16)ns.enabled,
					NamespaceViewFactory::namespaceEnableStateToStr(ns.enabled), false);
			pInstance->setAttribute(ENABLEDSTATE_KEY, a, attributes);
		}

		// NumberOfBlocks
		if (containsAttribute(NUMBEROFBLOCKS_KEY, attributes))
		{
			framework::Attribute a(ns.block_count, false);
			pInstance->setAttribute(NUMBEROFBLOCKS_KEY, a, attributes);
		}

		// BlockSize
		if (containsAttribute(BLOCKSIZE_KEY, attributes))
		{
			framework::Attribute a((NVM_UINT64)ns.block_size, false);
			pInstance->setAttribute(BLOCKSIZE_KEY, a, attributes);
		}


		// Volatile = false
		if (containsAttribute(VOLATILE_KEY, attributes))
		{
			framework::Attribute a(false, false);
			pInstance->setAttribute(VOLATILE_KEY, a, attributes);
		}

		// OperationalStatus
		if (containsAttribute(OPERATIONALSTATUS_KEY, attributes))
		{
			wbem::framework::UINT16_LIST status;

			pInstance->setAttribute(OPERATIONALSTATUS_KEY,
					getOperationalStatusAttr(ns), attributes);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
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
wbem::framework::instance_names_t *wbem::pmem_config::PersistentMemoryNamespaceFactory::getInstanceNames()
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::vector<std::string> nsList = NamespaceViewFactory::getNamespaceGuidList();
		for (std::vector<std::string>::const_iterator iter = nsList.begin();
				iter != nsList.end(); iter++)
		{
			std::string nsGuid = *iter;
			wbem::framework::ObjectPath path;
			createPathFromGuid(nsGuid, path);

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

wbem::framework::Attribute wbem::pmem_config::PersistentMemoryNamespaceFactory::
	getOperationalStatusAttr(const struct namespace_details &ns)
{
	wbem::framework::UINT16_LIST status;
	switch (ns.enabled)
	{
		case NAMESPACE_ENABLE_STATE_ENABLED:
			status.push_back(PM_NAMESPACE_OPSTATUS_INSERVICE);
			break;
		case NAMESPACE_ENABLE_STATE_DISABLED:
			status.push_back(PM_NAMESPACE_OPSTATUS_STOPPED);
			break;
		case NAMESPACE_ENABLE_STATE_UNKNOWN:
		default:
			status.push_back(PM_NAMESPACE_OPSTATUS_UNKNOWN);
			break;
	}

	framework::Attribute opStatusAttr(status, false);
	return opStatusAttr;
}

void wbem::pmem_config::PersistentMemoryNamespaceFactory::createPathFromGuid(const std::string &nsGuid,
		framework::ObjectPath &path)
{
	NVM_GUID guid;

	str_to_guid(nsGuid.c_str(), guid);
	createPathFromGuid(guid, path);

}

void wbem::pmem_config::PersistentMemoryNamespaceFactory::createPathFromGuid(const NVM_GUID guid,
		framework::ObjectPath &path)
{
	NVM_GUID_STR nsGuid;
	guid_to_str(guid, nsGuid);

	wbem::framework::attributes_t keys;

	// SystemCreationClassName = Intel_BaseServer
	keys[wbem::SYSTEMCREATIONCLASSNAME_KEY] =
					framework::Attribute(wbem::server::BASESERVER_CREATIONCLASSNAME, true);

	// SystemName = Host Name
	const std::string hostName = wbem::server::getHostName();
	keys[wbem::SYSTEMNAME_KEY] = framework::Attribute(hostName, true);

	// CreationClassName = Intel_PersistentMemoryNamespace
	keys[wbem::CREATIONCLASSNAME_KEY] =
					framework::Attribute(wbem::pmem_config::PMNS_CREATIONCLASSNAME, true);

	// DeviceID = Namespace GUID
	keys[wbem::DEVICEID_KEY] = framework::Attribute(nsGuid, true);

	path.setObjectPath(hostName, NVM_NAMESPACE, PMNS_CREATIONCLASSNAME, keys);
}

/*
 * Determine if the PersistentMemoryNamespaceFactory has a complex association.
 */
bool wbem::pmem_config::PersistentMemoryNamespaceFactory::isAssociated(
		const std::string &associationClass,
		framework::Instance* pAntInstance,
		framework::Instance* pDepInstance)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	// Association: ElementAllocatedFromPool
	if (associationClass == wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTALLOCATEDFROMPOOL)
	{
		// Antecedent: PersistentMemoryPool
		// Dependent: PersistentMemoryNamespace
		if ((pDepInstance->getClass() == wbem::pmem_config::PMNS_CREATIONCLASSNAME)
			&& (pAntInstance->getClass() == wbem::pmem_config::PERSISTENTMEMORYPOOL_CREATIONCLASSNAME))
		{
			// get pool GUID from pool class
			framework::Attribute poolIdAttr;
			if (pAntInstance->getAttribute(INSTANCEID_KEY, poolIdAttr) == framework::SUCCESS)
			{
				// get the namespace details from the instance
				struct namespace_details nsDetails;
				if (namespaceDetailsFromInstance(pDepInstance, &nsDetails) == NVM_SUCCESS)
				{
					// check if the pool GUIDs match
					NVM_GUID poolId;
					str_to_guid(poolIdAttr.stringValue().c_str(), poolId);

					result = guid_cmp(poolId, nsDetails.pool_guid);
				}
				else
				{
					COMMON_LOG_ERROR("Couldn't get the namespace details from the instance.");
				}
			}
			else
			{
				COMMON_LOG_ERROR(
					"Couldn't get InstanceID attribute from the PersistentMemoryPool instance.");
			}
		}
		else // unrecognized instance classes
		{
			COMMON_LOG_ERROR("Incorrect antecedent and dependent class instances.");
		}
	}
	else
	{
		result = true;
	}

	return result;
}


int wbem::pmem_config::PersistentMemoryNamespaceFactory::namespaceDetailsFromInstance(
		wbem::framework::Instance *pNsInstance, struct namespace_details *pNsDetails)
{
	int rc = NVM_SUCCESS;

	// get the namespace GUID
	framework::Attribute nsGuidAttr;
	if (pNsInstance->getAttribute(DEVICEID_KEY, nsGuidAttr) == framework::SUCCESS)
	{
		// get the namespace details using the namespace GUID
		NVM_GUID nsGuid;
		str_to_guid(nsGuidAttr.stringValue().c_str(), nsGuid);
		rc = nvm_get_namespace_details(nsGuid, pNsDetails);
	}
	return rc;
}

wbem::framework::UINT32 wbem::pmem_config::PersistentMemoryNamespaceFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = framework::MOF_ERR_SUCCESS;

	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %d", method.c_str(), (int)(inParms.size()));

	try
	{
		if (PM_NAMESPACE_REQUESTSTATECHANGE == method)
		{
			std::string namespaceGuidStr;
			httpRc = PersistentMemoryServiceFactory::getNamespaceFromPath(object, namespaceGuidStr);
			if (httpRc == framework::MOF_ERR_SUCCESS)
			{
				NVM_UINT16 stateValue = inParms[PM_NAMESPACE_STATE].uintValue();
				enum namespace_enable_state enabled =
						wbem::pmem_config::PersistentMemoryServiceFactory::namespaceEnabledToEnum(stateValue);
				if (!isModifyNamespaceEnabledSupported(enabled))
				{
					throw exception::NvmExceptionLibError(NVM_ERR_NOTSUPPORTED);
				}
				modifyNamespace(namespaceGuidStr, stateValue);
				wbemRc = framework::MOF_ERR_SUCCESS;
			}
		}
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
		}
	}
	catch (framework::ExceptionBadParameter &)
	{
		httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		wbemRc = framework::MOF_ERR_INVALIDPARAMETER;
	}
	catch (exception::NvmExceptionLibError &e)
	{
		switch (e.getLibError())
		{
			case NVM_ERR_NOTSUPPORTED:
				wbemRc = PM_NAMESPACE_NOTSUPPORTED;
				break;
			case NVM_ERR_NOMEMORY:
				wbemRc = PM_NAMESPACE_FAILED;
				break;
			case NVM_ERR_INVALIDPARAMETER:
				wbemRc = framework::MOF_ERR_INVALIDPARAMETER;
				break;
			case NVM_ERR_INVALIDPERMISSIONS:
				wbemRc = PM_NAMESPACE_NOTSUPPORTED;
				break;
			case NVM_ERR_DEVICEBUSY:
				wbemRc = framework::MOF_ERR_INUSE;
				break;
			case NVM_ERR_NOSIMULATOR:
				wbemRc = PM_NAMESPACE_NOTSUPPORTED;
				break;
			case NVM_ERR_BADNAMESPACE:
				wbemRc = framework::MOF_ERR_INVALIDPARAMETER;
				break;
			case NVM_ERR_BADSIZE:
				wbemRc = PM_NAMESPACE_FAILED;
				break;
			default:
				wbemRc = PM_NAMESPACE_UNKNOWN;
				break;
		}
	}
	catch (framework::ExceptionNoMemory &)
	{
		wbemRc = PM_NAMESPACE_FAILED;
	}
	catch (framework::ExceptionNotSupported &)
	{
		wbemRc = PM_NAMESPACE_NOTSUPPORTED;
	}
	catch (framework::Exception &)
	{
		wbemRc = PM_NAMESPACE_UNKNOWN;
	}
	return httpRc;
}

bool wbem::pmem_config::PersistentMemoryNamespaceFactory::isModifyNamespaceEnabledSupported(
		const enum namespace_enable_state enabled)
{
	bool isSupported = true;

	struct nvm_capabilities capabilities;
	memset(&capabilities, 0, sizeof (capabilities));
	int rc = nvm_get_nvm_capabilities(&capabilities);

	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve driver capabilities");
		throw exception::NvmExceptionLibError(rc);
	}

	if (enabled  == NAMESPACE_ENABLE_STATE_DISABLED &&
		!capabilities.nvm_features.disable_namespace)
	{
		isSupported = false;
	}
	else if (enabled  == NAMESPACE_ENABLE_STATE_ENABLED &&
			!capabilities.nvm_features.enable_namespace)
	{
		isSupported = false;
	}

	return isSupported;
}

void wbem::pmem_config::PersistentMemoryNamespaceFactory::modifyNamespace(
		const std::string namespaceGuidStr, const NVM_UINT16 stateValue)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// check for valid enabled state
	if ((stateValue != PM_NAMESPACE_ENABLE_STATE_ENABLED) &&
			(stateValue != PM_NAMESPACE_ENABLE_STATE_DISABLED))
	{
		COMMON_LOG_ERROR_F("Invalid %s: %u.", PM_NAMESPACE_STATE.c_str(), stateValue);
		throw framework::ExceptionBadParameter(PM_NAMESPACE_STATE.c_str());
	}

	// check for valid namespace guid
	// note - .length() doesn't include NULL terminator
	if (namespaceGuidStr.empty() || namespaceGuidStr.length() != NVM_GUIDSTR_LEN - 1)
	{
		COMMON_LOG_ERROR("Invalid namespace guid");
		throw framework::ExceptionBadParameter(wbem::DEVICEID_KEY.c_str());
	}
	NVM_GUID namespaceGuid;
	str_to_guid(namespaceGuidStr.c_str(), namespaceGuid);

	enum namespace_enable_state enabled = PersistentMemoryServiceFactory::namespaceEnabledToEnum(stateValue);
	int rc = m_modifyNamespaceEnabled(namespaceGuid, enabled);
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}
