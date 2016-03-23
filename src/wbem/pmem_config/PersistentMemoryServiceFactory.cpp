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
 * This file contains the provider for the PersistentMemoryService instances.
 */

#include "PersistentMemoryServiceFactory.h"
#include "PersistentMemoryNamespaceFactory.h"
#include <LogEnterExit.h>
#include <intel_cim_framework/ExceptionNoMemory.h>
#include <intel_cim_framework/ExceptionBadParameter.h>
#include <guid/guid.h>
#include <intel_cim_framework/ExceptionNotSupported.h>
#include <intel_cim_framework/ObjectPathBuilder.h>
#include "pmem_config/PersistentMemoryPoolFactory.h"
#include "pmem_config/NamespaceViewFactory.h"
#include <string/s_str.h>
#include <exception/NvmExceptionLibError.h>
#include <lib_interface/NvmApi.h>

wbem::pmem_config::PersistentMemoryServiceFactory::PersistentMemoryServiceFactory()
throw (framework::Exception)
{
	m_deleteNamespace = nvm_delete_namespace;
	m_createNamespace = nvm_create_namespace;
	m_modifyNamespaceName = nvm_modify_namespace_name;
	m_modifyNamespaceBlockCount = nvm_modify_namespace_block_count;
}

wbem::pmem_config::PersistentMemoryServiceFactory::~PersistentMemoryServiceFactory()
{

}


void wbem::pmem_config::PersistentMemoryServiceFactory::populateAttributeList(std::vector<std::string> &attributes)
throw (framework::Exception)
{
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(NAME_KEY);
}

wbem::framework::Instance *wbem::pmem_config::PersistentMemoryServiceFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// Inspect key attributes from object path
	// SystemCreationClassName -Intel_ BaseServer
	if (path.getKeyValue(SYSTEMCREATIONCLASSNAME_KEY).stringValue()
			!= PM_SERVICE_SYSTEMCREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(SYSTEMCREATIONCLASSNAME_KEY.c_str());
	}
	// SystemName - host name
	if (path.getKeyValue(SYSTEMNAME_KEY).stringValue() != wbem::server::getHostName())
	{
		throw framework::ExceptionBadParameter(SYSTEMNAME_KEY.c_str());
	}
	// CreationClassName - Intel_PersistentMemoryService
	if(path.getKeyValue(CREATIONCLASSNAME_KEY).stringValue() !=
			PM_SERVICE_CLASSNAME)
	{
		throw framework::ExceptionBadParameter(CREATIONCLASSNAME_KEY.c_str());
	}
	// Name
	if (path.getKeyValue(NAME_KEY).stringValue() != PM_SERVICE_NAME)
	{
		throw framework::ExceptionBadParameter(NAME_KEY.c_str());
	}

	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);

		// All attributes are key attributes added from the path
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

wbem::framework::instance_names_t *wbem::pmem_config::PersistentMemoryServiceFactory::getInstanceNames()
throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string hostName = lib_interface::NvmApi::getApi()->getHostName();

	wbem::framework::instance_names_t *pNames = new wbem::framework::instance_names_t ();
	framework::attributes_t keys;
	keys[SYSTEMCREATIONCLASSNAME_KEY] =
			framework::Attribute(server::BASESERVER_CREATIONCLASSNAME, true);
	keys[SYSTEMNAME_KEY] = framework::Attribute(hostName, true);
	keys[CREATIONCLASSNAME_KEY] = framework::Attribute(PM_SERVICE_CLASSNAME, true);
	keys[NAME_KEY] = framework::Attribute(PM_SERVICE_NAME, true);

	pNames->push_back(framework::ObjectPath(hostName, NVM_NAMESPACE,
			PM_SERVICE_CLASSNAME, keys));

	return pNames;
}

void wbem::pmem_config::PersistentMemoryServiceFactory::validatePool(std::string poolRef)
	throw (framework::Exception)
{
	// Turn the ref into an object path
	framework::ObjectPathBuilder pathBuilder(poolRef);
	framework::ObjectPath poolPath;
	if (pathBuilder.Build(&poolPath)) // successfully built the path
	{
		// get the instance
		wbem::pmem_config::PersistentMemoryPoolFactory poolFactory;
		framework::attribute_names_t attributes; // all attributes
		framework::Instance *pInstance = poolFactory.getInstance(poolPath, attributes);
		if (!pInstance) // not found
		{
			COMMON_LOG_ERROR_F("'%s' is not a PersistentMemoryPoolFactory instance", poolRef.c_str());
			throw framework::ExceptionBadParameter(PM_SERVICE_RESOURCE_POOL.c_str());
		}
	}
	else
	{
		COMMON_LOG_ERROR_F("parameter '%s' was not a valid object path: %s", PM_SERVICE_RESOURCE_POOL.c_str(),
			poolRef.c_str());
		throw framework::ExceptionBadParameter(PM_SERVICE_RESOURCE_POOL.c_str());
	}
}

/*
 * Utility fn to convert namespace type int to enum value
 */
enum namespace_type wbem::pmem_config::PersistentMemoryServiceFactory::namespaceTypeToEnum(const NVM_UINT32 type)
{
	enum namespace_type enum_val;
	switch (type)
	{
		case wbem::pmem_config::PM_SERVICE_BLOCK_TYPE:
			enum_val = NAMESPACE_TYPE_BLOCK;
			break;
		case wbem::pmem_config::PM_SERVICE_PM_TYPE:
			enum_val = NAMESPACE_TYPE_PMEM;
			break;
		default:
			enum_val = NAMESPACE_TYPE_UNKNOWN;
			break;
	};
	return enum_val;
}

/*
 * Utility fn to convert namespace enabled int to enum value
 */
enum namespace_enable_state wbem::pmem_config::PersistentMemoryServiceFactory::namespaceEnabledToEnum(
		const unsigned int enabled)
{
	enum namespace_enable_state enum_val;
	switch (enabled)
	{
		case wbem::pmem_config::PM_SERVICE_NAMESPACE_ENABLE_STATE_ENABLED:
			enum_val = NAMESPACE_ENABLE_STATE_ENABLED;
			break;
		case wbem::pmem_config::PM_SERVICE_NAMESPACE_ENABLE_STATE_DISABLED:
			enum_val = NAMESPACE_ENABLE_STATE_DISABLED;
			break;
		default:
			enum_val = NAMESPACE_ENABLE_STATE_UNKNOWN;
			break;
	}
	return enum_val;
}

/*
 * Utility fn to convert encryption type int to enum value
 */
enum encryption_status wbem::pmem_config::PersistentMemoryServiceFactory::encryptionTypeToEnum(const NVM_UINT16 encryption)
{
	enum encryption_status enum_val;
	switch (encryption)
	{
		case wbem::pmem_config::PM_SERVICE_SECURITY_ENCRYPTION_ON:
			enum_val = NVM_ENCRYPTION_ON;
			break;
		case wbem::pmem_config::PM_SERVICE_SECURITY_ENCRYPTION_OFF:
		default:
			enum_val = NVM_ENCRYPTION_OFF;
			break;
	};
	return enum_val;
}

enum encryption_status wbem::pmem_config::PersistentMemoryServiceFactory::integerToEncryptionEnum(const NVM_UINT16 encryption)
{
	return encryption ? NVM_ENCRYPTION_ON : NVM_ENCRYPTION_OFF;
}

/*
 * Helper function to convert block size string to block size int
 */
NVM_UINT64 blocksizestr_to_int(std::string blockSizeStr)
{
	NVM_UINT16 size;
	std::string s = "bytes*";
	std::string::size_type i = blockSizeStr.find(s);
	if (i != std::string::npos)
		blockSizeStr.erase(i, s.length());
	size = (NVM_UINT64)atoi(blockSizeStr.c_str());
	return size;
}

wbem::framework::UINT32 wbem::pmem_config::PersistentMemoryServiceFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = framework::MOF_ERR_SUCCESS;

	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %d",
			method.c_str(), (int)(inParms.size()));

	framework::Instance *pGoalInstance = NULL;
	try
	{
		if (PM_SERVICE_RETURNTOPOOL == method)
		{
			// convert namespace param to appropriate object path
			std::string namespaceValue = inParms[PM_SERVICE_NAMESPACE].stringValue();
			framework::ObjectPathBuilder pathBuilder(namespaceValue);
			framework::ObjectPath path;
			pathBuilder.Build(&path);

			std::string namespaceGuid;
			httpRc = getNamespaceFromPath(path, namespaceGuid);

			if (httpRc == framework::MOF_ERR_SUCCESS)
			{
				deleteNamespace(namespaceGuid);
				wbemRc = framework::MOF_ERR_SUCCESS;
			}
		}
		else if (PM_SERVICE_ALLOCATEFROMPOOL == method)
		{
			allocateFromPool(inParms, outParms, httpRc);
		}
		else if (PM_SERVICE_MODIFYNAMESPACE == method)
		{
			modifyNamespace(inParms, outParms, httpRc);
		}
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
		}
	}
	catch (framework::ExceptionBadParameter &)
	{
		wbemRc = PM_SERVICE_ERR_INVALIDPARAMETER;
	}
	catch (exception::NvmExceptionLibError &e)
	{
		wbemRc = getReturnCodeFromLibException(e);
	}
	catch (framework::ExceptionNoMemory &)
	{
		wbemRc = PM_SERVICE_ERR_FAILED;
	}
	catch (framework::ExceptionNotSupported &)
	{
		wbemRc = PM_SERVICE_ERR_FAILED;
	}
	catch (framework::Exception &)
	{
		wbemRc = PM_SERVICE_ERR_UNKNOWN;
	}

	if (pGoalInstance)
	{
		delete pGoalInstance;
	}

	return httpRc;
}

wbem::framework::UINT32
wbem::pmem_config::PersistentMemoryServiceFactory::getReturnCodeFromLibException(
		const exception::NvmExceptionLibError &e)
{
	wbem::framework::UINT32 rc = framework::MOF_ERR_SUCCESS;

	switch(e.getLibError())
	{
		case NVM_ERR_UNKNOWN:
			rc = PM_SERVICE_ERR_UNKNOWN;
			break;
		case NVM_ERR_BADSIZE:
		case NVM_ERR_INVALIDPARAMETER:
			rc = PM_SERVICE_ERR_INVALIDPARAMETER;
			break;
		case NVM_ERR_DEVICEBUSY:
			rc = PM_SERVICE_ERR_IN_USE;
			break;
		case NVM_ERR_BADNAMESPACETYPE:
			rc = PM_SERVICE_ERR_INCONSISTENT_PARAMETERS;
			break;
		case NVM_ERR_NOMEMORY:
			rc = PM_SERVICE_ERR_INSUFFICIENT_RESOURCES;
			break;
		default:
			rc = PM_SERVICE_ERR_FAILED;
			break;
	}

	return rc;
}

void wbem::pmem_config::PersistentMemoryServiceFactory::allocateFromPool(
		framework::attributes_t &inParms,
		framework::attributes_t &outParms,
		NVM_UINT32 &httpRc)
{
	/*
	 * uint32 AllocateFromPool(
		CIM_ResourcePool REF Pool, // Intel_PersistentMemoryPool
		string Goal, // embedded instance of CIM_PersistentMemoryNamespaceSettingData
		CIM_PersistentMemoryNamespace REF Namespace);
	 */

	httpRc = wbem::framework::MOF_ERR_SUCCESS;

	// Validate Pool attribute
	std::string poolRef = inParms[wbem::pmem_config::PM_SERVICE_RESOURCE_POOL].stringValue();
	wbem::pmem_config::PersistentMemoryServiceFactory::validatePool(poolRef);

	// get pool id from pool ref
	framework::ObjectPathBuilder builder(poolRef);
	framework::ObjectPath poolPath;
	builder.Build(&poolPath);
	wbem::framework::Attribute poolIdAttribute = poolPath.getKeyValue(wbem::INSTANCEID_KEY);
	std::string poolGuidStr = poolIdAttribute.stringValue();
	if (poolGuidStr.empty())
	{
		COMMON_LOG_ERROR_F("Invalid pool guid in object path: %s", poolRef.c_str());
		throw framework::ExceptionBadParameter(PM_SERVICE_GOAL.c_str());
	}

	std::string goalString = inParms[wbem::pmem_config::PM_SERVICE_GOAL].stringValue();

	if (goalString.empty())
	{
		COMMON_LOG_ERROR_F("%s is required.", wbem::pmem_config::PM_SERVICE_GOAL.c_str());
		httpRc = wbem::framework::CIM_ERR_INVALID_PARAMETER;
		throw framework::ExceptionBadParameter(PM_SERVICE_GOAL.c_str());
	}
	else
	{
		wbem::framework::Instance *pGoalInstance = new wbem::framework::Instance(goalString);
		try
		{
			// get InitialState from goal
			wbem::framework::Attribute initialStateAttribute;
			pGoalInstance->getAttribute(wbem::INITIALSTATE_KEY, initialStateAttribute);
			NVM_UINT16 initialState = initialStateAttribute.uintValue();

			// get friendly name from goal
			wbem::framework::Attribute elementAttribute;
			pGoalInstance->getAttribute(wbem::ELEMENTNAME_KEY, elementAttribute);
			std::string friendlyNameStr = elementAttribute.stringValue();

			// get block size from goal
			wbem::framework::Attribute allocationAttribute;
			pGoalInstance->getAttribute(wbem::ALLOCATIONUNITS_KEY, allocationAttribute);
			std::string blockSizeStr = allocationAttribute.stringValue();

			if (blockSizeStr.empty())
			{
				COMMON_LOG_ERROR_F("Invalid block size in object path: %s", blockSizeStr.c_str());
				throw framework::ExceptionBadParameter(PM_SERVICE_GOAL.c_str());
			}

			// get block count from goal
			wbem::framework::Attribute reservationAttribute;
			pGoalInstance->getAttribute(wbem::RESERVATION_KEY, reservationAttribute);
			NVM_UINT64 blockCount = reservationAttribute.uint64Value();

			if (blockCount == 0)
			{
				COMMON_LOG_ERROR_F("Invalid block count in object path: %llu", blockCount);
				throw framework::ExceptionBadParameter(PM_SERVICE_GOAL.c_str());
			}

			// get namespace type from goal
			wbem::framework::Attribute namespaceTypeAttribute;
			pGoalInstance->getAttribute(wbem::RESOURCETYPE_KEY, namespaceTypeAttribute);
			NVM_UINT16 type = namespaceTypeAttribute.uintValue();

			if ((type != wbem::pmem_config::PM_SERVICE_PM_TYPE) &&
				(type != wbem::pmem_config::PM_SERVICE_BLOCK_TYPE))
			{
				COMMON_LOG_ERROR_F("Invalid namespace type in object path: %d", type);
				throw framework::ExceptionBadParameter(PM_SERVICE_GOAL.c_str());
			}

			// get optimize type from goal
			wbem::framework::Attribute optimizeAttribute;
			pGoalInstance->getAttribute(wbem::OPTIMIZE_KEY, optimizeAttribute);
			NVM_UINT16 optimize = optimizeAttribute.uintValue();
			if (optimize != wbem::pmem_config::PM_SERVICE_OPTIMIZE_SMALLEST_SIZE)
			{
				optimize = wbem::pmem_config::PM_SERVICE_OPTIMIZE_COPYONWRITE;
			}

			// get security features from goal and then harvest encryption and erase capable fields
			NVM_UINT16 encryption = 0;
			NVM_UINT16 eraseCapable = 0;
			wbem::framework::Attribute securityFeaturesAttribute;
			pGoalInstance->getAttribute(wbem::SECURITYFEATURES_KEY, securityFeaturesAttribute);
			wbem::framework::UINT16_LIST securityValueList = securityFeaturesAttribute.uint16ListValue();
			for (int i = 0; i < (int)securityValueList.size(); i++)
			{
				if (securityValueList[i] == wbem::pmem_config::NS_SECURITY_ENCRYPTION_ON)
				{
					encryption = 1;
				}
				else
				{
					encryption = 0;
				}

				if (securityValueList[i] == wbem::pmem_config::NS_SECURITY_ERASE)
				{
					eraseCapable = 1;
				}
			}

			std::string namespaceGuidStr;

			// interleave sizes
			framework::Attribute channelSizeAttribute;
			framework::Attribute controllerSizeAttribute;
			if (pGoalInstance->getAttribute(CHANNELINTERLEAVESIZE_KEY, channelSizeAttribute) == framework::SUCCESS &&
			pGoalInstance->getAttribute(CONTROLLERINTERLEAVESIZE_KEY, controllerSizeAttribute) == framework::SUCCESS)
			{
				mem_config::MemoryAllocationSettingsInterleaveSizeExponent channelSize =
						(mem_config::MemoryAllocationSettingsInterleaveSizeExponent)channelSizeAttribute.uintValue();
				mem_config::MemoryAllocationSettingsInterleaveSizeExponent controllerSize =
						(mem_config::MemoryAllocationSettingsInterleaveSizeExponent)controllerSizeAttribute.uintValue();

				wbem::pmem_config::PersistentMemoryServiceFactory::createNamespace(namespaceGuidStr,
					poolGuidStr, initialState,
					friendlyNameStr, blocksizestr_to_int(blockSizeStr), blockCount, type, optimize,
					encryption, eraseCapable,
					channelSize, controllerSize, false);
			}
			else
			{
				wbem::pmem_config::PersistentMemoryServiceFactory::createNamespace(namespaceGuidStr,
					poolGuidStr, initialState,
					friendlyNameStr, blocksizestr_to_int(blockSizeStr), blockCount, type, optimize,
					encryption, eraseCapable,
					wbem::mem_config::MEMORYALLOCATIONSETTINGS_EXPONENT_UNKNOWN,
					wbem::mem_config::MEMORYALLOCATIONSETTINGS_EXPONENT_UNKNOWN, false);
			}

			if (pGoalInstance)
			{
				delete pGoalInstance;
			}

			framework::Attribute outputNSAttribute;
			generateNamespaceRefAttribute(namespaceGuidStr, outputNSAttribute);
			outParms[wbem::pmem_config::PM_SERVICE_NAMESPACE] = outputNSAttribute;
		}
		catch (framework::Exception &) // clean up and re-throw
		{
			if (pGoalInstance)
			{
				delete pGoalInstance;
			}
			throw;
		}
	}
}

void wbem::pmem_config::PersistentMemoryServiceFactory::modifyNamespace(
		framework::attributes_t &inParms,
		framework::attributes_t &outParms,
		NVM_UINT32 &httpRc)
{
	/*
	* uint32 ModifyNamespace(
	*	CIM_PersistentMemoryNamespace REF Namespace, // Ref to PM NS to be changed
	*	string Goal  // embedded instance of CIM_PersistentMemoryNamespaceSettingData
	*	);
	*/
	httpRc = wbem::framework::MOF_ERR_SUCCESS;

	std::string namespaceValue = inParms[PM_SERVICE_NAMESPACE].stringValue();
	std::string goalString = inParms[PM_SERVICE_GOAL].stringValue();
	if (goalString.empty())
	{
		COMMON_LOG_ERROR_F("%s is required.", PM_SERVICE_GOAL.c_str());
		httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		throw framework::ExceptionBadParameter(PM_SERVICE_NAMESPACE.c_str());
	}
	else if (namespaceValue.empty())
	{
		COMMON_LOG_ERROR_F("%s is required.", PM_SERVICE_NAMESPACE.c_str());
		httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		throw framework::ExceptionBadParameter(PM_SERVICE_NAMESPACE.c_str());
	}
	else
	{
		framework::ObjectPathBuilder pathBuilder(namespaceValue);
		framework::ObjectPath namespacePath;
		if (!pathBuilder.Build(&namespacePath))
		{
			COMMON_LOG_ERROR_F("parameter '%s' was not a valid object path: %s", PM_SERVICE_NAMESPACE.c_str(),
					namespaceValue.c_str());
			httpRc = framework::CIM_ERR_INVALID_PARAMETER;
			throw framework::ExceptionBadParameter(PM_SERVICE_NAMESPACE.c_str());
		}
		else
		{
			std::string namespaceGuidStr;
			httpRc = getNamespaceFromPath(namespacePath, namespaceGuidStr);
			if (httpRc == framework::MOF_ERR_SUCCESS)
			{
				wbem::framework::Instance goalInstance(goalString);

				// get pool guid from goal
				wbem::framework::Attribute guidAttribute;
				goalInstance.getAttribute(wbem::POOLID_KEY, guidAttribute);
				std::string poolGuidStr = guidAttribute.stringValue();
				if (poolGuidStr.empty() || poolGuidStr.length() != NVM_GUIDSTR_LEN - 1)
				{
					COMMON_LOG_ERROR_F("Invalid pool guid in object path: %s", goalString.c_str());
					httpRc = framework::CIM_ERR_INVALID_PARAMETER;
					throw framework::ExceptionBadParameter(PM_SERVICE_GOAL.c_str());
				}

				wbem::framework::Attribute elementAttribute;
				goalInstance.getAttribute(wbem::ELEMENTNAME_KEY, elementAttribute);
				std::string friendlyNameStr = elementAttribute.stringValue();

				wbem::framework::Attribute reservationAttribute;
				goalInstance.getAttribute(wbem::RESERVATION_KEY, reservationAttribute);
				NVM_UINT64 reservation = reservationAttribute.uint64Value();

				performAtomicModification(namespaceGuidStr, reservation, friendlyNameStr);
			}
		}
	}
}

wbem::framework::UINT32 wbem::pmem_config::PersistentMemoryServiceFactory::getNamespaceFromPath(
		const wbem::framework::ObjectPath &path, std::string &namespaceGuid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;

	try
	{
		// validate path
		if (path.getKeyValue(CREATIONCLASSNAME_KEY).stringValue()
				!= PMNS_CREATIONCLASSNAME
			|| path.getKeyValue(SYSTEMNAME_KEY).stringValue()
				!=  server::getHostName()
			|| path.getKeyValue(SYSTEMCREATIONCLASSNAME_KEY).stringValue()
				!=  server::BASESERVER_CREATIONCLASSNAME)
		{
			httpRc = framework::CIM_ERR_INVALID_PARAMETER;
		}
		else
		{
			namespaceGuid = path.getKeyValue(wbem::DEVICEID_KEY).stringValue();
			httpRc = framework::MOF_ERR_SUCCESS;
		}
	}
	catch (wbem::framework::ExceptionBadParameter &)
	{
		httpRc = framework::CIM_ERR_INVALID_PARAMETER;
	}

	return httpRc;
}

void wbem::pmem_config::PersistentMemoryServiceFactory::deleteNamespace(const std::string &namespaceGuid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// note - .length() doesn't include NULL terminator
	if (namespaceGuid.empty() || namespaceGuid.length() != NVM_GUIDSTR_LEN - 1)
	{
		throw framework::ExceptionBadParameter(PM_SERVICE_NAMESPACE.c_str());
	}

	NVM_GUID guid;
	str_to_guid(namespaceGuid.c_str(), guid);

	int rc = m_deleteNamespace(guid);
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void wbem::pmem_config::PersistentMemoryServiceFactory::createNamespace(std::string &namespaceGuidStr,
		const std::string poolGuidStr, const NVM_UINT16 stateValue,
		const std::string friendlyNameStr, const NVM_UINT64 blockSize,
		const NVM_UINT64 blockCount, const NVM_UINT16 type,
		const NVM_UINT16 optimize,
		const NVM_UINT16 encryption, const NVM_UINT16 eraseCapable,
		const mem_config::MemoryAllocationSettingsInterleaveSizeExponent channelSize,
		const mem_config::MemoryAllocationSettingsInterleaveSizeExponent controllerSize,
		const bool byOne)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// note - .length() doesn't include NULL terminator
	if (poolGuidStr.empty() || poolGuidStr.length() != NVM_GUIDSTR_LEN - 1)
	{
		throw framework::ExceptionBadParameter(PM_SERVICE_RESOURCE_POOL.c_str());
	}

	NVM_GUID poolGuid;
	str_to_guid(poolGuidStr.c_str(), poolGuid);
	NVM_GUID namespaceGuid;
	NVM_GUID_STR namespace_guid_str;

	// populate the struct
	struct namespace_create_settings namespace_settings;
	memset(&namespace_settings, 0, sizeof (namespace_settings));

	bool btt;
	if (optimize == PM_SERVICE_OPTIMIZE_COPYONWRITE ||
			optimize == PM_SERVICE_OPTIMIZE_SMALLEST_SIZE)
	{
		btt = true;
	}
	else if (optimize == PM_SERVICE_OPTIMIZE_NONE ||
			optimize == PM_SERVICE_OPTIMIZE_BEST_PERFORMANCE )
	{
		btt = false;
	}
	else
	{
		throw framework::ExceptionBadParameter("Optimize");
	}

	namespace_settings.btt = btt ? 1 : 0;
	namespace_settings.block_count = blockCount;
	namespace_settings.block_size = blockSize;
	namespace_settings.enabled = namespaceEnabledToEnum(stateValue);
	namespace_settings.type = namespaceTypeToEnum(type);
	namespace_settings.security_features.erase_capable = eraseCapable ? 1 : 0;
	namespace_settings.security_features.encryption = encryptionTypeToEnum(encryption);

	s_strncpy(namespace_settings.friendly_name, NVM_NAMESPACE_NAME_LEN,
			friendlyNameStr.c_str(), NVM_NAMESPACE_NAME_LEN);

	struct interleave_format *p_format = NULL;
	struct interleave_format format;
	memset(&format, 0, sizeof (format));
	if (channelSize != mem_config::MEMORYALLOCATIONSETTINGS_EXPONENT_UNKNOWN &&
			controllerSize != mem_config::MEMORYALLOCATIONSETTINGS_EXPONENT_UNKNOWN)
	{
		format.channel = mem_config::InterleaveSet::getInterleaveSizeFromExponent(channelSize);
		format.imc = mem_config::InterleaveSet::getInterleaveSizeFromExponent(controllerSize);
		format.ways = byOne ? INTERLEAVE_WAYS_1 : INTERLEAVE_WAYS_0; // 0 indicates that byOne wasn't specified
		p_format = &format;
	}

	int rc = m_createNamespace(&namespaceGuid, poolGuid, &namespace_settings, p_format, 0);
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
	guid_to_str(namespaceGuid, namespace_guid_str);
	namespaceGuidStr = std::string(namespace_guid_str);
}

bool wbem::pmem_config::PersistentMemoryServiceFactory::isModifyNamespaceNameSupported()
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

	if (!capabilities.nvm_features.rename_namespace)
	{
		isSupported = false;
	}

	return isSupported;
}

bool wbem::pmem_config::PersistentMemoryServiceFactory::isModifyNamespaceBlockCountSupported(
		const namespace_details &details, const NVM_UINT64 blockCount)
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

	if (details.block_count < blockCount && !capabilities.nvm_features.grow_namespace)
	{
		isSupported = false;
	}

	if (details.block_count > blockCount && !capabilities.nvm_features.shrink_namespace)
	{
		isSupported = false;;
	}

	return isSupported;
}

void wbem::pmem_config::PersistentMemoryServiceFactory::performAtomicModification(
		std::string namespaceGuidStr, NVM_UINT64 reservation, std::string friendlyNameStr)
{
	NVM_GUID namespaceGuid;
	str_to_guid(namespaceGuidStr.c_str(), namespaceGuid);
	struct namespace_details details;
	memset(&details, 0, sizeof (details));
	int rc = nvm_get_namespace_details(namespaceGuid, &details);

	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Failed to retrieve namespace details");
		throw exception::NvmExceptionLibError(rc);
	}

	NVM_UINT64 blockCount = getBlockCountFromReservation(details, reservation);

	if (std::string(details.discovery.friendly_name) != friendlyNameStr &&
		!isModifyNamespaceNameSupported())
	{
		throw exception::NvmExceptionLibError(NVM_ERR_NOTSUPPORTED);
	}

	if (details.block_count != blockCount &&
		!isModifyNamespaceBlockCountSupported(details, blockCount))
	{
		throw exception::NvmExceptionLibError(NVM_ERR_NOTSUPPORTED);
	}

	try
	{
		if (blockCount != details.block_count)
		{
			modifyNamespaceBlockCount(namespaceGuidStr, blockCount);
		}

		if (s_strncmp(friendlyNameStr.c_str(), details.discovery.friendly_name, NVM_NAMESPACE_NAME_LEN) != 0)
		{
			modifyNamespaceName(namespaceGuidStr, friendlyNameStr);
		}
	}
	catch (framework::Exception &)
	{
		if (s_strncmp(friendlyNameStr.c_str(), details.discovery.friendly_name, NVM_NAMESPACE_NAME_LEN) != 0)
		{
			modifyNamespaceName(namespaceGuidStr, details.discovery.friendly_name);
		}

		if (blockCount != details.block_count)
		{
			modifyNamespaceBlockCount(namespaceGuidStr, details.block_count);
		}
		throw;
	}
}

void wbem::pmem_config::PersistentMemoryServiceFactory::modifyNamespaceName(
		const std::string namespaceGuidStr, const std::string friendlyNameStr)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// note - .length() doesn't include NULL terminator
	if (namespaceGuidStr.empty() || namespaceGuidStr.length() != NVM_GUIDSTR_LEN - 1)
	{
		throw framework::ExceptionBadParameter(PM_SERVICE_NAMESPACE.c_str());
	}

	NVM_GUID namespaceGuid;
	str_to_guid(namespaceGuidStr.c_str(), namespaceGuid);

	int rc = m_modifyNamespaceName(namespaceGuid, friendlyNameStr.c_str());
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void wbem::pmem_config::PersistentMemoryServiceFactory::modifyNamespaceBlockCount(
		const std::string namespaceGuidStr, const NVM_UINT64 blockCount)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// note - .length() doesn't include NULL terminator
	if (namespaceGuidStr.empty() || namespaceGuidStr.length() != NVM_GUIDSTR_LEN - 1)
	{
		throw framework::ExceptionBadParameter(PM_SERVICE_NAMESPACE.c_str());
	}

	NVM_GUID namespaceGuid;
	str_to_guid(namespaceGuidStr.c_str(), namespaceGuid);

	int rc = m_modifyNamespaceBlockCount(namespaceGuid, blockCount, 1);
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

NVM_UINT64 wbem::pmem_config::PersistentMemoryServiceFactory::getBlockCountFromReservation(
		const struct namespace_details &details, NVM_UINT64 reservation)
{
	return reservation / details.block_size;
}

void wbem::pmem_config::PersistentMemoryServiceFactory::getNamespaceDetails(
		const std::string namespaceGuidStr, struct namespace_details &details)
{
	NVM_GUID namespaceGuid;
	str_to_guid(namespaceGuidStr.c_str(), namespaceGuid);
	memset(&details, 0, sizeof (struct namespace_details));
	int rc = nvm_get_namespace_details(namespaceGuid, &details);

	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Could not retrieve namespace details");
		throw exception::NvmExceptionLibError(rc);
	}
}

void wbem::pmem_config::PersistentMemoryServiceFactory::createNamespace(
		const createNamespaceParams &settings, std::string &namespaceGuid)
{
	createNamespace(namespaceGuid,
		settings.poolId, settings.enabled,
		settings.friendlyName, settings.blockSize, settings.blockCount, settings.type,
		settings.optimize, settings.encryption, settings.eraseCapable,
		settings.interleaveChannelSize, settings.interleaveControllerSize, settings.byOne);
}

NVM_UINT64 wbem::pmem_config::PersistentMemoryServiceFactory::getAdjustedCreateNamespaceBlockCount(
		std::string poolGuidStr,  const NVM_UINT16 type, const NVM_UINT32 blockSize,
		const NVM_UINT64 blockCount, const NVM_UINT16 eraseCapable,
		const NVM_UINT16 encryption, const NVM_UINT16 enableState)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc = NVM_SUCCESS;

	struct namespace_create_settings settings;
	memset(&settings, 0, sizeof (settings));

	settings.type = namespaceTypeToEnum(type);
	settings.block_size = blockSize;
	settings.block_count = blockCount;
	settings.security_features.erase_capable = eraseCapable;
	settings.security_features.encryption = integerToEncryptionEnum(encryption);
	settings.enabled = (enum namespace_enable_state) enableState;

	NVM_GUID poolGuid;
	str_to_guid(poolGuidStr.c_str(), poolGuid);

	if ((rc = nvm_adjust_create_namespace_block_count(poolGuid, &settings, NULL)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Could not adjust namespace block count");
		throw exception::NvmExceptionLibError(rc);
	}
	return settings.block_count;
}

NVM_UINT64 wbem::pmem_config::PersistentMemoryServiceFactory::getAdjustedModifyNamespaceBlockCount(
		std::string namespaceGuidStr, const NVM_UINT64 blockCount)
{
	int rc = NVM_SUCCESS;
	NVM_GUID namespaceGuid;
	NVM_UINT64 newBlockCount = blockCount;
	str_to_guid(namespaceGuidStr.c_str(), namespaceGuid);

	if ((rc = nvm_adjust_modify_namespace_block_count(namespaceGuid, &newBlockCount)) != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR("Could not adjust namespace block count");
		throw exception::NvmExceptionLibError(rc);
	}
	return newBlockCount;
}

void wbem::pmem_config::PersistentMemoryServiceFactory::generateNamespaceRefAttribute(
		std::string namespaceGuidStr, wbem::framework::Attribute& value)
{
	// generate an object path for the given namespace GUID
	wbem::framework::ObjectPath path;
	wbem::pmem_config::PersistentMemoryNamespaceFactory::createPathFromGuid(namespaceGuidStr, path);
	value = framework::Attribute(path.asString(true), true);
	// marks the attribute as a REF
	value.setIsAssociationClassInstance(true);
}
