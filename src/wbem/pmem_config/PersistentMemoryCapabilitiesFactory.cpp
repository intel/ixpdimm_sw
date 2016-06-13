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
 * This file contains the provider for the PeristentMemoryCapabilities instances.
 */

#include <LogEnterExit.h>
#include <nvm_management.h>
#include <libinvm-cim/Attribute.h>
#include <server/BaseServerFactory.h>
#include <uid/uid.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <sstream>
#include "PersistentMemoryCapabilitiesFactory.h"
#include <mem_config/PoolViewFactory.h>
#include <string.h>

#include <exception/NvmExceptionLibError.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <NvmStrings.h>
#include <core/Helper.h>

wbem::pmem_config::PersistentMemoryCapabilitiesFactory::PersistentMemoryCapabilitiesFactory()
throw(wbem::framework::Exception)
{
}

wbem::pmem_config::PersistentMemoryCapabilitiesFactory::~PersistentMemoryCapabilitiesFactory()
{
}

void wbem::pmem_config::PersistentMemoryCapabilitiesFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(MAXNAMESPACES_KEY);
	attributes.push_back(SECURITYFEATURES_KEY);
	attributes.push_back(ACCESSGRANULARITY_KEY);
	attributes.push_back(MEMORYARCHITECTURE_KEY);
	attributes.push_back(REPLICATION_KEY);
	attributes.push_back(MEMORYPAGEALLOCATIONCAPABLE_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance *wbem::pmem_config::PersistentMemoryCapabilitiesFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	struct pool *pPool = NULL;
	try
	{
		checkAttributes(attributes);

		pPool = getPool(path);
		struct nvm_capabilities capabilities = getNvmCapabilities();

		// ElementName = "Pool Capabilities for: " + pool UUID
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			NVM_UID poolUidStr;
			uid_copy(pPool->pool_uid, poolUidStr);
			std::string elementNameStr = PMCAP_ELEMENTNAME + poolUidStr;
			framework::Attribute a(elementNameStr, false);
			pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
		}

		// MaxNamespaces = Max namespaces for this pool
		if (containsAttribute(MAXNAMESPACES_KEY, attributes))
		{
			framework::UINT64 maxNs = getMaxNamespacesPerPool(pPool, capabilities.sw_capabilities.min_namespace_size);
			framework::Attribute a(maxNs, false);
			pInstance->setAttribute(MAXNAMESPACES_KEY, a, attributes);
		}

		// SecurityFeatures = Supported security features of the pool
		if (containsAttribute(SECURITYFEATURES_KEY, attributes))
		{
			framework::UINT16_LIST secFeaturesList = getPoolSecurityFeatures(pPool);
			framework::Attribute a(secFeaturesList, false);
			pInstance->setAttribute(SECURITYFEATURES_KEY, a, attributes);
		}

		// AccessGranularity = Block/byte
		if (containsAttribute(ACCESSGRANULARITY_KEY, attributes))
		{
			framework::UINT16_LIST accessList;
			accessList.push_back(PMCAP_ACCESSGRANULARITY_BYTE); // all app direct is byte accessible
			if (pPool->type == POOL_TYPE_PERSISTENT)
			{
				accessList.push_back(PMCAP_ACCESSGRANULARITY_BLOCK); // non mirrored = block capable
			}
			framework::Attribute a(accessList, false);
			pInstance->setAttribute(ACCESSGRANULARITY_KEY, a, attributes);
		}

		// MemoryArchitecture = NUMA
		if (containsAttribute(MEMORYARCHITECTURE_KEY, attributes))
		{
			framework::UINT16_LIST memArchList;
			memArchList.push_back(PMCAP_MEMORYARCHITECTURE_NUMA);
			framework::Attribute a(memArchList, false);
			pInstance->setAttribute(MEMORYARCHITECTURE_KEY, a, attributes);
		}

		// Replication = Mirrored locally or nothing
		if (containsAttribute(REPLICATION_KEY, attributes))
		{
			framework::UINT16_LIST replicationList;
			if (pPool->type == POOL_TYPE_PERSISTENT_MIRROR)
			{
				replicationList.push_back(PMCAP_REPLICATION_LOCAL);
			}
			framework::Attribute a(replicationList, false);
			pInstance->setAttribute(REPLICATION_KEY, a, attributes);
		}

		if (containsAttribute(MEMORYPAGEALLOCATIONCAPABLE_KEY, attributes))
		{
			framework::BOOLEAN capable = capabilities.sw_capabilities.namespace_memory_page_allocation_capable;
			framework::Attribute a(capable, false);
			pInstance->setAttribute(MEMORYPAGEALLOCATIONCAPABLE_KEY, a, attributes);
		}
		delete pPool;
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		if (pInstance)
		{
			delete pInstance;
			pInstance = NULL;
		}
		if (pPool)
		{
			delete pPool;
			pPool = NULL;
		}
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for each instance of the class.
 * One per persistent memory pool in the system.
 */
wbem::framework::instance_names_t *wbem::pmem_config::PersistentMemoryCapabilitiesFactory::getInstanceNames()
	throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// get PM pools
		std::vector<struct pool> pools = wbem::mem_config::PoolViewFactory::getPoolList(true);
		for (std::vector<struct pool>::const_iterator iter = pools.begin();
				iter != pools.end(); iter++)
		{
			framework::attributes_t keys;

			// Instance ID = Pool UID
			NVM_UID poolUidStr;
			uid_copy((*iter).pool_uid, poolUidStr);
			keys[INSTANCEID_KEY] = framework::Attribute(poolUidStr, true);

			framework::ObjectPath path(wbem::server::getHostName(),
					NVM_NAMESPACE, PMCAP_CREATIONCLASSNAME, keys);
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
 * Execute an extrinsic method
 */
wbem::framework::UINT32 wbem::pmem_config::PersistentMemoryCapabilitiesFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = framework::MOF_ERR_SUCCESS;
	struct pool *pPool = NULL;

	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %d", method.c_str(), (int)(inParms.size()));

	try
	{
		if (method == PMCAP_GETBLOCKSIZES)
		{
			// get the supported block sizes for this pool
			wbem::framework::UINT64_LIST blockSizes;
			pPool = getPool(object);
			// if pool is block capable, then retrieve supported block sizes, else empty
			if (pPool->type == POOL_TYPE_PERSISTENT)
			{
				// get system supported block sizes
				getSupportedBlockSizes(blockSizes);
			}
			// return in outParams
			outParms[PMCAP_BLOCKSIZES_PARAMNAME] = framework::Attribute(blockSizes, false);
		}
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
		}
	}
	catch (framework::ExceptionBadParameter &)
	{
		wbemRc = PMCAP_ERR_INVALID_PARAMETER;
	}
	catch(exception::NvmExceptionLibError &e)
	{
		wbemRc = getReturnCodeFromLibException(e);
	}
	catch(framework::Exception &)
	{
		wbemRc = PMCAP_ERR_UNKNOWN;
	}
	if (pPool)
	{
		delete pPool;
	}

	COMMON_LOG_EXIT_RETURN("httpRc: %u, wbemRc: %u", httpRc, wbemRc);
	return httpRc;
}

wbem::framework::UINT32
wbem::pmem_config::PersistentMemoryCapabilitiesFactory::getReturnCodeFromLibException(
		exception::NvmExceptionLibError e)
{
	wbem::framework::UINT32 rc = framework::MOF_ERR_SUCCESS;

	switch(e.getLibError())
	{
		case NVM_ERR_UNKNOWN:
			rc = PMCAP_ERR_UNKNOWN;
			break;
		case NVM_ERR_NOTSUPPORTED:
			rc = PMCAP_ERR_NOT_SUPPORTED;
			break;
		case NVM_ERR_INVALIDPARAMETER:
			rc = PMCAP_ERR_INVALID_PARAMETER;
			break;
		case NVM_ERR_NOMEMORY:
			rc = PMCAP_ERR_INSUFFICIENT_RESOURCES;
			break;
		default:
			rc = PMCAP_ERR_FAILED;
			break;
	}

	return rc;
}
/*!
 * Determine the security features of the dimms in the pool
 */
wbem::framework::UINT16_LIST wbem::pmem_config::PersistentMemoryCapabilitiesFactory::getPoolSecurityFeatures(struct pool *pPool)
	throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::UINT16_LIST secFeaturesList;

	bool encryptionCapable = false;
	bool cryptoCapable = false;
	// iterate over all the dimms and check security capabilities
	for (NVM_UINT16 i = 0; i < pPool->dimm_count; i++)
	{
		struct device_discovery devDiscovery;
		int rc = nvm_get_device_discovery(pPool->dimms[i], &devDiscovery);
		if (rc != NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}

		if (devDiscovery.security_capabilities.passphrase_capable)
		{
			encryptionCapable = true;
		}
		if (devDiscovery.security_capabilities.erase_crypto_capable)
		{
			cryptoCapable = true;
		}
	}
	// if any of the dimms in the pool support encryption, add the encryption feature
	if (encryptionCapable)
	{
		secFeaturesList.push_back(PMCAP_SECURITYFEATURES_ENCRYPTION);
	}
	// if any of the dimms in the pool support cryto erase, add the crypto feature
	if (cryptoCapable)
	{
		secFeaturesList.push_back(PMCAP_SECURITYFEATURES_CRYPTO);
	}
	return secFeaturesList;
}

wbem::framework::UINT64 wbem::pmem_config::PersistentMemoryCapabilitiesFactory::getMaxNamespacesPerPool(struct pool *pPool,
		NVM_UINT64 minNamespaceSize)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_UINT64 maxAppDirectNS = 0;
	NVM_UINT64 maxBlockNS = 0;

	NVM_UID poolUidStr;
	uid_copy(pPool->pool_uid, poolUidStr);

	// A pool can have as many App Direct Namespaces as its interleave sets as long as the size is greater
	// than minimum namespace size
	for (int i = 0; i < pPool->ilset_count; i++)
	{
		if (pPool->ilsets[i].size >= minNamespaceSize)
		{
			maxAppDirectNS++;
		}
	}

	// A pool can have as many storage namespaces as its dimms as long as the size is greater
	// than minimum namespace size
	for (int j = 0; j < pPool->dimm_count; j++)
	{
		if (pPool->storage_capacities[j] >= minNamespaceSize)
		{
			maxBlockNS++;
		}
	}

	return (maxAppDirectNS + maxBlockNS);
}

/*
 * Helper function to retrieve system capabilities
 */
struct nvm_capabilities wbem::pmem_config::PersistentMemoryCapabilitiesFactory::getNvmCapabilities()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct nvm_capabilities capabilities;
	int rc = nvm_get_nvm_capabilities(&capabilities);
	if (rc < 0)
	{
		throw exception::NvmExceptionLibError(rc);
	}

	return capabilities;
}

/*
 * Retrieve pool struct given an object path
 */
struct pool *wbem::pmem_config::PersistentMemoryCapabilitiesFactory::getPool(
	wbem::framework::ObjectPath &object)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string poolUidStr = object.getKeyValue(INSTANCEID_KEY).stringValue();
	if (!core::Helper::isValidPoolUid(poolUidStr))
	{
		COMMON_LOG_ERROR_F("PersistentMemoryCapabilitiesFactory InstanceID is not a valid pool uid %s",
				poolUidStr.c_str());
		throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
	}
	return mem_config::PoolViewFactory::getPool(poolUidStr);
}

void wbem::pmem_config::PersistentMemoryCapabilitiesFactory::getSupportedBlockSizes(
		framework::UINT64_LIST &list)
{
	// get system supported block sizes
	struct nvm_capabilities capabilities = getNvmCapabilities();
	for (NVM_UINT32 i = 0; i < capabilities.sw_capabilities.block_size_count; i++)
	{
		list.push_back((NVM_UINT64)capabilities.sw_capabilities.block_sizes[i]);
	}
}

NVM_BOOL wbem::pmem_config::PersistentMemoryCapabilitiesFactory::getMemoryPageAllocationCapability()
{
	struct nvm_capabilities capabilities = getNvmCapabilities();
	return capabilities.sw_capabilities.namespace_memory_page_allocation_capable;
}
