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
 * This file contains the provider for the PersistentMemory instances
 * which represent individual persistent memory extents in the system.
 */

#include "PersistentMemoryFactory.h"
#include <mem_config/PoolViewFactory.h>
#include <pmem_config/PersistentMemoryPoolFactory.h>
#include <pmem_config/PersistentMemoryNamespaceFactory.h>
#include <mem_config/MemoryAllocationSettingsFactory.h>
#include <memory/SystemProcessorFactory.h>
#include <memory/RawMemoryFactory.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <libinvm-cim/ExceptionBadAttribute.h>
#include <exception/NvmExceptionLibError.h>
#include <LogEnterExit.h>
#include <uid/uid.h>
#include <sstream>
#include <string.h>
#include <lib_interface/NvmApi.h>
#include <core/device/DeviceHelper.h>

wbem::memory::PersistentMemoryFactory::PersistentMemoryFactory() :
		framework_interface::NvmInstanceFactory()
{
}

wbem::memory::PersistentMemoryFactory::~PersistentMemoryFactory()
{
}

void wbem::memory::PersistentMemoryFactory::populateAttributeList(
	framework::attribute_names_t &attributes) throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(DEVICEID_KEY);

	// add non-key attributes
	attributes.push_back(NUMBEROFBLOCKS_KEY);
	attributes.push_back(BLOCKSIZE_KEY);
	attributes.push_back(VOLATILE_KEY);
	attributes.push_back(PRIMORDIAL_KEY);
	attributes.push_back(HEALTHSTATE_KEY);
	attributes.push_back(OPERATIONALSTATUS_KEY);
	attributes.push_back(ACCESSGRANULARITY_KEY);
	attributes.push_back(PROCESSORAFFINITY_KEY);
	attributes.push_back(REPLICATION_KEY);
	attributes.push_back(ENABLEDSTATE_KEY);
}

wbem::framework::instance_names_t* wbem::memory::PersistentMemoryFactory::getInstanceNames()
		throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct pool> pools = mem_config::PoolViewFactory::getPoolList(true);
	framework::instance_names_t *pNames = NULL;
	try
	{
		pNames = new framework::instance_names_t;
		if (!pNames)
		{
			throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
					"couldn't allocate instance names");
		}

		for (size_t i = 0; i < pools.size(); i++)
		{
			struct pool &pool = pools[i];
			if (pool.type != POOL_TYPE_VOLATILE)
			{
				// All non-volatile pools can have interleave sets
				getInterleaveSetInstanceNames(*pNames, pool);
			}

			if (pool.type == POOL_TYPE_PERSISTENT)
			{
				// un-mirrored persistent pools can have storage-only regions
				getStorageRegionInstanceNames(*pNames, pool);
			}
		}
	}
	catch (framework::Exception &)
	{
		if (pNames)
		{
			delete pNames;
		}
		throw;
	}

	return pNames;
}


wbem::framework::ObjectPath wbem::memory::PersistentMemoryFactory::getInstanceName(const std::string& deviceId)
{
	std::string hostName = server::getHostName();

	framework::attributes_t keys;
	keys[CREATIONCLASSNAME_KEY] =
			framework::Attribute(PERSISTENTMEMORY_CREATIONCLASSNAME, true);
	keys[SYSTEMCREATIONCLASSNAME_KEY] =
			framework::Attribute(PERSISTENTMEMORY_SYSTEMCREATIONCLASSNAME, true);
	keys[SYSTEMNAME_KEY] = framework::Attribute(hostName, true);
	keys[DEVICEID_KEY] = framework::Attribute(deviceId, true);

	return framework::ObjectPath(hostName, NVM_NAMESPACE,
			PERSISTENTMEMORY_CREATIONCLASSNAME, keys);
}

void wbem::memory::PersistentMemoryFactory::getInterleaveSetInstanceNames(
		framework::instance_names_t& instanceNames, const struct pool &pool)
{
	for (size_t i = 0; i < pool.ilset_count; i++)
	{
		const struct interleave_set &interleave = pool.ilsets[i];
		std::string deviceId = getInterleaveSetUuid(interleave.set_index,
				interleave.socket_id);

		instanceNames.push_back(getInstanceName(deviceId));
	}
}

void wbem::memory::PersistentMemoryFactory::getStorageRegionInstanceNames(
		wbem::framework::instance_names_t& instanceNames, const struct pool &pool)
{
	for (size_t i = 0; i < pool.dimm_count; i++)
	{
		NVM_UID uidStr;
		uid_copy(pool.dimms[i], uidStr);
		std::string deviceId = getStorageRegionUuid(std::string(uidStr));

		instanceNames.push_back(getInstanceName(deviceId));
	}
}

wbem::framework::Instance* wbem::memory::PersistentMemoryFactory::getInstance(
		framework::ObjectPath& path, framework::attribute_names_t& attributes)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkAttributes(attributes);
	validatePath(path);

	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		// Find the appropriate App Direct region
		std::string deviceId = path.getKeyValue(DEVICEID_KEY).stringValue();

		std::vector<struct pool> pools = mem_config::PoolViewFactory::getPoolList(true);
		bool found = false;
		for (size_t i = 0; !found && (i < pools.size()); i++)
		{
			struct pool &pool = pools[i];
			if (pool.type != POOL_TYPE_VOLATILE)
			{
				// All non-volatile pools can have interleave sets
				struct interleave_set interleave;
				memset(&interleave, 0, sizeof (interleave));
				found = findInterleaveSetForUuid(deviceId, pool, interleave);
				if (found)
				{
					setInterleaveSetInstanceAttributes(*pInstance, attributes, interleave);
				}
				else if (pool.type == POOL_TYPE_PERSISTENT)
				{
					// Non-mirrored persistent pools may have Storage-only regions
					size_t dimmIndex = 0;
					found = findStorageDimmIndexForUuid(deviceId, pool, dimmIndex);
					if (found)
					{
						setStorageCapacityInstanceAttributes(*pInstance, attributes, pool, dimmIndex);
					}
				}
			}
		}

		if (!found)
		{
			COMMON_LOG_ERROR_F("no app direct or storage region found with UUID %s",
					deviceId.c_str());
			throw framework::ExceptionBadAttribute(DEVICEID_KEY.c_str());
		}
	}
	catch (framework::Exception &)
	{
		if (pInstance)
		{
			delete pInstance;
		}
		throw;
	}

	return pInstance;
}

bool wbem::memory::PersistentMemoryFactory::findInterleaveSetForUuid(const std::string& uuid,
		const struct pool& pool, struct interleave_set& interleave)
{
	bool found = false;

	for (size_t i = 0; i < pool.ilset_count; i++)
	{
		const struct interleave_set &tmpInterleave = pool.ilsets[i];

		std::string tmpUuid = getInterleaveSetUuid(tmpInterleave.set_index,
				tmpInterleave.socket_id);

		if (uuid == tmpUuid)
		{
			found = true;
			memmove(&interleave, &tmpInterleave, sizeof (struct interleave_set));
			break;
		}
	}

	return found;
}

bool wbem::memory::PersistentMemoryFactory::findStorageDimmIndexForUuid(const std::string& uuid,
		const struct pool& pool, size_t& index)
{
	bool found = false;

	for (size_t i = 0; i < pool.dimm_count; i++)
	{
		NVM_UID uidStr;
		uid_copy(pool.dimms[i], uidStr);
		std::string tmpUuid = getStorageRegionUuid(std::string(uidStr));

		if (uuid == tmpUuid)
		{
			found = true;
			index = i;
			break;
		}
	}

	return found;
}

bool wbem::memory::PersistentMemoryFactory::isAssociated(const std::string& associationClass,
		framework::Instance* pAntInstance, framework::Instance* pDepInstance)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = true;

	// BasedOn
	if (associationClass == framework_interface::ASSOCIATION_CLASS_BASEDON)
	{
		// Antecedent - RawMemory
		if ((pAntInstance->getClass() == memory::RAWMEMORY_CREATIONCLASSNAME) &&
				(pDepInstance->getClass() == memory::PERSISTENTMEMORY_CREATIONCLASSNAME))
		{
			framework::Attribute rawMemUidAttr;
			if(pAntInstance->getAttribute(DEVICEID_KEY, rawMemUidAttr) != framework::SUCCESS)
			{
				COMMON_LOG_ERROR("Couldn't get RawMemory DeviceID");
				throw framework::ExceptionBadAttribute(DEVICEID_KEY.c_str());
			}
			std::string dimmUid = rawMemUidAttr.stringValue();

			framework::Attribute persistentMemUuidAttr;
			if(pDepInstance->getAttribute(DEVICEID_KEY, persistentMemUuidAttr) != framework::SUCCESS)
			{
				COMMON_LOG_ERROR("Couldn't get PersistentMemory DeviceID");
				throw framework::ExceptionBadAttribute(DEVICEID_KEY.c_str());
			}
			std::string persistentMemUuid = persistentMemUuidAttr.stringValue();

			result = isPersistentMemoryUsingDimm(persistentMemUuid, dimmUid);
		}
		else if ((pAntInstance->getClass() == memory::PERSISTENTMEMORY_CREATIONCLASSNAME) &&
				(pDepInstance->getClass() == pmem_config::PMNS_CREATIONCLASSNAME))
		{
			result = isPersistentMemoryAssociatedToPersistentMemoryNamespace(
					*pAntInstance, *pDepInstance);
		}
		else // unrecognized instance classes
		{
			COMMON_LOG_ERROR_F("Incorrect antecedent (%s) and dependent (%s) class instances for association class %s.",
					pAntInstance->getClass().c_str(), pDepInstance->getClass().c_str(),  associationClass.c_str());
		}
	}
	else if (associationClass == framework_interface::ASSOCIATION_CLASS_CONCRETECOMPONENT)
	{
		// Antecedent - RawMemory
		if ((pAntInstance->getClass() == pmem_config::PERSISTENTMEMORYPOOL_CREATIONCLASSNAME) &&
				(pDepInstance->getClass() == memory::PERSISTENTMEMORY_CREATIONCLASSNAME))
		{
			struct pool *pool = (struct pool *) malloc(sizeof (struct pool));
			if (pool != NULL)
			{
				framework::Attribute instanceIdAttr;
				if (pAntInstance->getAttribute(INSTANCEID_KEY, instanceIdAttr) == framework::SUCCESS)
				{
					NVM_UID poolUid;
					uid_copy(instanceIdAttr.stringValue().c_str(), poolUid);
					if (nvm_get_pool(poolUid, pool) == NVM_SUCCESS)
					{
						result = poolMatchesPmObject(pool, pDepInstance);
					}
					else
					{
						COMMON_LOG_ERROR("Unable to retrieve pool struct");
					}
				}
				else
				{
					COMMON_LOG_ERROR("Unable to retrieve instanceId for PersistentMemoryPool.");
				}
			}
			else
			{
				COMMON_LOG_ERROR("Unable to allocate memory for pool struct.");
			}

			if (pool != NULL)
			{
				free(pool);
			}
		}
		else // unrecognized instance classes
		{
			COMMON_LOG_ERROR_F("Incorrect antecedent (%s) and dependent (%s) class instances for association class %s.",
					pAntInstance->getClass().c_str(), pDepInstance->getClass().c_str(),  associationClass.c_str());
		}
	}
	else // unknown association class
	{
		COMMON_LOG_ERROR_F("Cannot calculate if instances are an association "
				"based on association class: %s", associationClass.c_str());
	}

	return result;
}

bool wbem::memory::PersistentMemoryFactory::mirroringMatches(struct pool *pool, framework::Instance* pPMObject)
{
	bool matches = false;
	wbem::framework::Attribute replicationAttr;
	if (pPMObject->getAttribute(REPLICATION_KEY, replicationAttr) == framework::SUCCESS)
	{
		if (replicationAttr.intValue() == PERSISTENTMEMORY_REPLICATION_LOCAL &&
				pool->type == POOL_TYPE_PERSISTENT_MIRROR)
		{
			matches = true;
		}

		if (replicationAttr.intValue() == PERSISTENTMEMORY_REPLICATION_NONE &&
				pool->type == POOL_TYPE_PERSISTENT)
		{
			matches = true;
		}
	}
	else
	{
		COMMON_LOG_ERROR("Unable to retrieve Replication attribute from PersistentMemory instance.");
	}

	return matches;
}

bool wbem::memory::PersistentMemoryFactory::socketsMatch(struct pool *pool, framework::Instance* pPMObject)
{
	bool matches = false;
	wbem::framework::Attribute socketIdAttr;
	if (pPMObject->getAttribute(PROCESSORAFFINITY_KEY, socketIdAttr) == framework::SUCCESS)
	{
		// socketIdStr has the form "CPU 0001"
		std::string socketIdStr = socketIdAttr.stringValue();
		socketIdStr.erase(0,3);
		NVM_INT16 socketId;
		std::istringstream(socketIdStr) >> socketId;

		if (socketId == pool->socket_id)
		{
			matches = true;
		}
	}
	else
	{
		COMMON_LOG_ERROR("Unable to retrieve ProcessorAffinity attribute from PersistentMemory instance.");
	}
	return matches;
}

bool wbem::memory::PersistentMemoryFactory::pmTypesMatch(struct pool *pool, framework::Instance* pPMObject)
{
	bool matches = false;
	wbem::framework::Attribute deviceIdAttr;
	if (pPMObject->getAttribute(DEVICEID_KEY, deviceIdAttr) == framework::SUCCESS)
	{
		NVM_UID uid;
		uid_copy(deviceIdAttr.stringValue().c_str(), uid);
		if (mem_config::MemoryAllocationSettingsFactory::isADeviceUid(uid) && pool->type == POOL_TYPE_PERSISTENT)
		{
			matches = true;
		}
		else
		{
			if (mirroringMatches(pool, pPMObject))
			{
				matches = true;
			}
		}
	}
	else
	{
		COMMON_LOG_ERROR("Unable to retrieve DeviceId attribute from PersistentMemory instance.");
	}

	return matches;
}

bool wbem::memory::PersistentMemoryFactory::poolMatchesPmObject(struct pool *pool, framework::Instance* pPMObject)
{
	return socketsMatch(pool, pPMObject) && pmTypesMatch(pool, pPMObject);
}

std::string wbem::memory::PersistentMemoryFactory::getInterleaveSetUuid(const NVM_UINT32 setIndex,
		const NVM_UINT32 socketId)
{
	std::stringstream srcStream;
	srcStream << "/socket" << socketId << "/interleave" << setIndex;

	std::string srcStr = srcStream.str();
	NVM_UID uuid;
	guid_hash_str((NVM_UINT8 *) srcStr.c_str(), srcStr.size(), uuid);

	NVM_UID uuidStr;
	uid_copy(uuid, uuidStr);

	return std::string(uuidStr);
}

std::string wbem::memory::PersistentMemoryFactory::getStorageRegionUuid(const std::string& dimmUidStr)
{
	// There's only one storage region per DIMM, and the DIMM UID is already unique
	return dimmUidStr;
}

void wbem::memory::PersistentMemoryFactory::validatePath(const framework::ObjectPath& path)
		throw (framework::Exception)
{
	// SystemName == the system host name
	std::string hostName = server::getHostName();
	const framework::Attribute &systemName = path.getKeyValue(SYSTEMNAME_KEY);
	if (systemName.stringValue() != hostName)
	{
		COMMON_LOG_ERROR_F("invalid value for key '%s': %s",
				SYSTEMNAME_KEY.c_str(), systemName.stringValue().c_str());
		throw framework::ExceptionBadAttribute(SYSTEMNAME_KEY.c_str());
	}

	// SystemCreationClassName == BaseServer
	const framework::Attribute &systemCreationClassName =
			path.getKeyValue(SYSTEMCREATIONCLASSNAME_KEY);
	if (systemCreationClassName.stringValue() != PERSISTENTMEMORY_SYSTEMCREATIONCLASSNAME)
	{
		COMMON_LOG_ERROR_F("invalid value for key '%s': %s",
				SYSTEMCREATIONCLASSNAME_KEY.c_str(),
				systemCreationClassName.stringValue().c_str());
		throw framework::ExceptionBadAttribute(SYSTEMCREATIONCLASSNAME_KEY.c_str());
	}

	// CreationClassName == PersistentMemory
	const framework::Attribute &creationClassName =
			path.getKeyValue(CREATIONCLASSNAME_KEY);
	if (creationClassName.stringValue() != PERSISTENTMEMORY_CREATIONCLASSNAME)
	{
		COMMON_LOG_ERROR_F("invalid value for key '%s': %s",
				CREATIONCLASSNAME_KEY.c_str(),
				creationClassName.stringValue().c_str());
		throw framework::ExceptionBadAttribute(CREATIONCLASSNAME_KEY.c_str());
	}

	// DeviceID == UID for interleave or storage region
	const framework::Attribute &deviceId = path.getKeyValue(DEVICEID_KEY);
	if (!core::device::isUidValid(deviceId.stringValue()) &&
		!core::Helper::isValidPoolUid(deviceId.stringValue()))
	{
		COMMON_LOG_ERROR_F("invalid value for key '%s': %s",
				DEVICEID_KEY.c_str(),
				deviceId.stringValue().c_str());
		throw framework::ExceptionBadAttribute(DEVICEID_KEY.c_str());
	}
}

void wbem::memory::PersistentMemoryFactory::setInterleaveSetInstanceAttributes(
		framework::Instance &instance,
		const framework::attribute_names_t &attributes,
		const struct interleave_set& interleave)
	throw (framework::Exception)
{
	setGenericInstanceAttributes(instance, attributes, interleave.socket_id);

	// NumberOfBlocks - uint64
	if (containsAttribute(NUMBEROFBLOCKS_KEY, attributes))
	{
		framework::Attribute attr(getNumBlocks(interleave.size), false);
		instance.setAttribute(NUMBEROFBLOCKS_KEY, attr);
	}

	// HealthState - uint16 enum
	if (containsAttribute(HEALTHSTATE_KEY, attributes))
	{
		NVM_UINT16 healthState = getInterleaveSetHealthState(interleave);
		framework::Attribute attr(healthState, getHealthStateString(healthState), false);
		instance.setAttribute(HEALTHSTATE_KEY, attr);
	}

	// OperationalStatus - uint16 array
	if (containsAttribute(OPERATIONALSTATUS_KEY, attributes))
	{
		framework::UINT16_LIST opStatus;
		opStatus.push_back(getInterleaveSetOperationalStatus(interleave));
		framework::Attribute attr(opStatus, false);
		instance.setAttribute(OPERATIONALSTATUS_KEY, attr);
	}

	// AccessGranularity - uint16 - enum
	// Always byte-accessible for interleave set
	if (containsAttribute(ACCESSGRANULARITY_KEY, attributes))
	{
		NVM_UINT16 accessGranularity = PERSISTENTMEMORY_ACCESSGRANULARITY_BYTE;
		framework::Attribute attr(accessGranularity,
				getAccessGranularityString(accessGranularity), false);
		instance.setAttribute(ACCESSGRANULARITY_KEY, attr);
	}

	// Replication - uint16 - enum
	if (containsAttribute(REPLICATION_KEY, attributes))
	{
		NVM_UINT16 replication = PERSISTENTMEMORY_REPLICATION_NONE;
		if (interleave.mirrored)
		{
			replication = PERSISTENTMEMORY_REPLICATION_LOCAL;
		}

		framework::Attribute attr(replication, getReplicationString(replication), false);
		instance.setAttribute(REPLICATION_KEY, attr);
	}
}

void wbem::memory::PersistentMemoryFactory::setStorageCapacityInstanceAttributes(
		framework::Instance &instance,
		const framework::attribute_names_t &attributes,
		const struct pool& pool,
		const size_t& dimmIdx)
	throw (framework::Exception)
{
	setGenericInstanceAttributes(instance, attributes, pool.socket_id);

	// NumberOfBlocks - uint64
	if (containsAttribute(NUMBEROFBLOCKS_KEY, attributes))
	{
		framework::Attribute attr(getNumBlocks(pool.storage_capacities[dimmIdx]), false);
		instance.setAttribute(NUMBEROFBLOCKS_KEY, attr);
	}

	// HealthState - uint16 enum
	if (containsAttribute(HEALTHSTATE_KEY, attributes))
	{
		NVM_UINT16 healthState = getStorageRegionHealthState(pool.dimms[dimmIdx]);
		framework::Attribute attr(healthState, getHealthStateString(healthState), false);
		instance.setAttribute(HEALTHSTATE_KEY, attr);
	}

	// OperationalStatus - uint16 array
	if (containsAttribute(OPERATIONALSTATUS_KEY, attributes))
	{
		framework::UINT16_LIST opStatus;
		opStatus.push_back(getStorageRegionOperationalStatus(pool.dimms[dimmIdx]));
		framework::Attribute attr(opStatus, false);
		instance.setAttribute(OPERATIONALSTATUS_KEY, attr);
	}

	// AccessGranularity - uint16 - enum
	// Always block-accessible for Storage region
	if (containsAttribute(ACCESSGRANULARITY_KEY, attributes))
	{
		NVM_UINT16 accessGranularity = PERSISTENTMEMORY_ACCESSGRANULARITY_BLOCK;
		framework::Attribute attr(accessGranularity,
				getAccessGranularityString(accessGranularity), false);
		instance.setAttribute(ACCESSGRANULARITY_KEY, attr);
	}

	// Replication - uint16 - enum
	// Storage is never mirrored
	if (containsAttribute(REPLICATION_KEY, attributes))
	{
		NVM_UINT16 replication = PERSISTENTMEMORY_REPLICATION_NONE;
		framework::Attribute attr(replication, getReplicationString(replication), false);
		instance.setAttribute(REPLICATION_KEY, attr);
	}
}

void wbem::memory::PersistentMemoryFactory::setGenericInstanceAttributes(framework::Instance& instance,
		const framework::attribute_names_t& attributes,
		const NVM_UINT16 socketId) throw (framework::Exception)
{
	// Volatile - boolean
	if (containsAttribute(VOLATILE_KEY, attributes))
	{
		framework::Attribute attr(false, false);
		instance.setAttribute(VOLATILE_KEY, attr);
	}

	// Primordial - boolean
	if (containsAttribute(PRIMORDIAL_KEY, attributes))
	{
		framework::Attribute attr(true, false);
		instance.setAttribute(PRIMORDIAL_KEY, attr);
	}

	// ProcessorAffinity - string
	if (containsAttribute(PROCESSORAFFINITY_KEY, attributes))
	{
		std::string socketDevId = memory::SystemProcessorFactory::getDeviceId(socketId);
		framework::Attribute attr(socketDevId, false);
		instance.setAttribute(PROCESSORAFFINITY_KEY, attr);
	}

	// EnabledState - uint16
	if (containsAttribute(ENABLEDSTATE_KEY, attributes))
	{
		NVM_UINT16 enabledState = PERSISTENTMEMORY_ENABLEDSTATE_NA;
		framework::Attribute attr(enabledState, getEnabledStateString(enabledState), false);
		instance.setAttribute(ENABLEDSTATE_KEY, attr);
	}

	// BlockSize - uint64 - for PM, it's the alignment
	if (containsAttribute(BLOCKSIZE_KEY, attributes))
	{
		framework::Attribute attr(getAppDirectAlignment(), false);
		instance.setAttribute(BLOCKSIZE_KEY, attr);
	}
}

bool wbem::memory::PersistentMemoryFactory::isPersistentMemoryUsingDimm(const std::string& pmUuid,
		const std::string& dimmUid) throw (framework::Exception)
{
	bool result = false;


	// Storage region UUID is the same as the corresponding DIMM UID
	if (pmUuid == dimmUid)
	{
		result = true;
	}
	else // Might be an interleave set - see if it's using the DIMM
	{
		std::vector<struct pool> pools = mem_config::PoolViewFactory::getPoolList(true);
		for (size_t i = 0; i < pools.size(); i++)
		{
			struct pool &pool = pools[i];

			struct interleave_set interleave;
			memset(&interleave, 0, sizeof (interleave));
			bool found = findInterleaveSetForUuid(pmUuid, pool, interleave);
			if (found)
			{
				for (size_t j = 0; j < interleave.dimm_count; j++)
				{
					NVM_UID tmpUidStr;
					uid_copy(interleave.dimms[j], tmpUidStr);
					if (dimmUid == tmpUidStr)
					{
						result = true;
						break;
					}
				}

				break;
			}
		}
	}

	return result;
}

NVM_UINT64 wbem::memory::PersistentMemoryFactory::getAppDirectAlignment() throw (framework::Exception)
{
	struct nvm_capabilities systemCapabilities;
	memset(&systemCapabilities, 0, sizeof(systemCapabilities));
	int rc = m_pApi->getNvmCapabilities(&systemCapabilities);
	if (rc != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}

	// Alignment is provided as an exponent n - 2^n bytes
	NVM_UINT16 alignmentExponent = systemCapabilities.platform_capabilities.app_direct_mode.interleave_alignment_size;
	NVM_UINT64 alignment = (NVM_UINT64)1 << alignmentExponent;
	if (alignment == 0) // something went wrong
	{
		COMMON_LOG_ERROR("App Direct alignment from capabilities was 0");
		throw framework::Exception("App Direct alignment was 0");
	}

	return alignment;
}

NVM_UINT64 wbem::memory::PersistentMemoryFactory::getNumBlocks(const NVM_UINT64 capacity)
		throw (framework::Exception)
{
	NVM_UINT64 numBlocks = 0;
	NVM_UINT64 blockSize = getAppDirectAlignment();
	if (blockSize > 0)
	{
		numBlocks = capacity / blockSize;
	}

	return numBlocks;
}

NVM_UINT16 wbem::memory::PersistentMemoryFactory::getInterleaveSetHealthState(
		const struct interleave_set& interleave)
{
	NVM_UINT16 healthState = PERSISTENTMEMORY_HEALTHSTATE_UNKNOWN;
	switch (interleave.health)
	{
	case INTERLEAVE_HEALTH_NORMAL:
		healthState = PERSISTENTMEMORY_HEALTHSTATE_OK;
		break;
	case INTERLEAVE_HEALTH_DEGRADED:
		healthState = PERSISTENTMEMORY_HEALTHSTATE_DEGRADED;
		break;
	case INTERLEAVE_HEALTH_FAILED:
		healthState = PERSISTENTMEMORY_HEALTHSTATE_NONRECOVERABLE;
		break;
	case INTERLEAVE_HEALTH_UNKNOWN:
	default:
		break;
	}

	return healthState;
}

NVM_UINT16 wbem::memory::PersistentMemoryFactory::getStorageRegionHealthState(const NVM_UID dimmUid)
		throw (framework::Exception)
{
	NVM_UINT16 healthState = PERSISTENTMEMORY_HEALTHSTATE_UNKNOWN;

	// Storage region health is based on DIMM health
	struct device_status status;
	memset(&status, 0, sizeof (status));
	int rc = nvm_get_device_status(dimmUid, &status);
	if (rc == NVM_SUCCESS)
	{
		if (status.is_missing)
		{
			healthState = PERSISTENTMEMORY_HEALTHSTATE_NONRECOVERABLE;
		}
		else
		{
			switch (status.health)
			{
			case DEVICE_HEALTH_NORMAL:
				healthState = PERSISTENTMEMORY_HEALTHSTATE_OK;
				break;
			case DEVICE_HEALTH_NONCRITICAL:
			case DEVICE_HEALTH_CRITICAL:
				healthState = PERSISTENTMEMORY_HEALTHSTATE_DEGRADED;
				break;
			case DEVICE_HEALTH_FATAL:
				healthState = PERSISTENTMEMORY_HEALTHSTATE_NONRECOVERABLE;
				break;
			case DEVICE_HEALTH_UNKNOWN:
			default:
				break;
			}
		}
	}
	else
	{
		NVM_UID uidStr;
		uid_copy(dimmUid, uidStr);
		COMMON_LOG_ERROR_F("couldn't get status for DIMM %s", uidStr);

		if (rc == NVM_ERR_BADDEVICE) // missing
		{
			healthState = PERSISTENTMEMORY_HEALTHSTATE_NONRECOVERABLE;
		}
		// otherwise unknown
	}



	return healthState;
}

NVM_UINT16 wbem::memory::PersistentMemoryFactory::getInterleaveSetOperationalStatus(
		const struct interleave_set& interleave) throw (framework::Exception)
{
	NVM_UINT16 opStatus = PERSISTENTMEMORY_OPSTATUS_UNKNOWN;

	switch (interleave.health)
	{
	case INTERLEAVE_HEALTH_NORMAL:
		opStatus = PERSISTENTMEMORY_OPSTATUS_OK;
		break;
	case INTERLEAVE_HEALTH_DEGRADED:
	case INTERLEAVE_HEALTH_FAILED:
		opStatus = PERSISTENTMEMORY_OPSTATUS_SUPPORTINGENTITYERROR;
		break;
	case INTERLEAVE_HEALTH_UNKNOWN:
		opStatus = PERSISTENTMEMORY_OPSTATUS_UNKNOWN;
		break;
	default: // unexpected health - unknown
		break;
	}

	return opStatus;
}

NVM_UINT16 wbem::memory::PersistentMemoryFactory::getStorageRegionOperationalStatus(const NVM_UID dimmUid)
		throw (framework::Exception)
{
	NVM_UINT16 opStatus = PERSISTENTMEMORY_OPSTATUS_UNKNOWN;

	struct device_status status;
	memset(&status, 0, sizeof (status));
	int rc = nvm_get_device_status(dimmUid, &status);
	if (rc != NVM_SUCCESS)
	{
		NVM_UID uidStr;
		uid_copy(dimmUid, uidStr);
		COMMON_LOG_ERROR_F("couldn't get status for DIMM %s", uidStr);
	}

	switch (rc)
	{
	case NVM_ERR_BADDEVICE: // DIMM not found
		opStatus = PERSISTENTMEMORY_OPSTATUS_SUPPORTINGENTITYERROR;
		break;
	case NVM_ERR_DRIVERFAILED: // can't get the status
	case NVM_ERR_NOTSUPPORTED:
	case NVM_ERR_UNKNOWN:
		opStatus = PERSISTENTMEMORY_OPSTATUS_UNKNOWN;
		break;
	case NVM_SUCCESS: // got the DIMM
		{
			if (status.is_missing)
			{
				opStatus = PERSISTENTMEMORY_OPSTATUS_SUPPORTINGENTITYERROR;
			}
			else
			{
				switch (status.health)
				{
				case DEVICE_HEALTH_NORMAL:
					opStatus = PERSISTENTMEMORY_OPSTATUS_OK;
					break;
				case DEVICE_HEALTH_NONCRITICAL:
				case DEVICE_HEALTH_CRITICAL:
				case DEVICE_HEALTH_FATAL:
					opStatus = PERSISTENTMEMORY_OPSTATUS_SUPPORTINGENTITYERROR;
					break;
				case DEVICE_HEALTH_UNKNOWN:
					opStatus = PERSISTENTMEMORY_OPSTATUS_UNKNOWN;
					break;
				default: // unexpected health - unknown
					break;
				}
			}
		}
		break;
	default: // undefined error - unknown
		break;
	}

	return opStatus;
}

std::string wbem::memory::PersistentMemoryFactory::getHealthStateString(const NVM_UINT16 value)
{
	std::string str;

	switch (value)
	{
	case PERSISTENTMEMORY_HEALTHSTATE_OK:
		str = TR("Healthy");
		break;
	case PERSISTENTMEMORY_HEALTHSTATE_DEGRADED:
		str = TR("Degraded/Warning");
		break;
	case PERSISTENTMEMORY_HEALTHSTATE_NONRECOVERABLE:
		str = TR("Non-recoverable error");
		break;
	case PERSISTENTMEMORY_HEALTHSTATE_UNKNOWN:
	default:
		str = TR("Unknown");
		break;
	}

	return str;
}

std::string wbem::memory::PersistentMemoryFactory::getOperationalStatusString(const NVM_UINT16 value)
{
	std::string str;

	switch (value)
	{
	case PERSISTENTMEMORY_OPSTATUS_OK:
		str = TR("OK");
		break;
	case PERSISTENTMEMORY_OPSTATUS_LOSTCOMM:
		str = TR("Lost Communication");
		break;
	case PERSISTENTMEMORY_OPSTATUS_SUPPORTINGENTITYERROR:
		str = TR("Supporting Entity in Error");
		break;
	case PERSISTENTMEMORY_OPSTATUS_UNKNOWN:
	default:
		str = TR("Unknown");
		break;
	}

	return str;
}

std::string wbem::memory::PersistentMemoryFactory::getAccessGranularityString(const NVM_UINT16 value)
{
	std::string str;

	switch (value)
	{
	case PERSISTENTMEMORY_ACCESSGRANULARITY_BLOCK:
		str = TR("Block Addressable");
		break;
	case PERSISTENTMEMORY_ACCESSGRANULARITY_BYTE:
		str = TR("Byte Addressable");
		break;
	case PERSISTENTMEMORY_ACCESSGRANULARITY_UNKNOWN:
	default:
		str = TR("Unknown");
		break;
	}

	return str;
}

std::string wbem::memory::PersistentMemoryFactory::getReplicationString(const NVM_UINT16 value)
{
	std::string str;

	switch (value)
	{
	case PERSISTENTMEMORY_REPLICATION_NONE:
		str = TR("Not Replicated");
		break;
	case PERSISTENTMEMORY_REPLICATION_LOCAL:
		str = TR("Local Replication");
		break;
	case PERSISTENTMEMORY_REPLICATION_UNKNOWN:
	default:
		str = TR("Unknown");
		break;
	}

	return str;
}

std::string wbem::memory::PersistentMemoryFactory::getEnabledStateString(const NVM_UINT16 value)
{
	std::string str;

	switch (value)
	{
	case PERSISTENTMEMORY_ENABLEDSTATE_NA:
		str = TR("Not Applicable");
		break;
	default:
		str = TR("Unknown");
		break;
	}

	return str;
}

bool wbem::memory::PersistentMemoryFactory::isPersistentMemoryAssociatedToPersistentMemoryNamespace(
		framework::Instance& pmInstance, framework::Instance &pmnsInstance)
{
	bool result = false;

	// get device IDs (UIDs) from the PM and PMNS instances
	bool instancesAreGood = true;
	framework::Attribute pmDeviceIdAttribute;
	framework::Attribute pmnsDeviceIdAttribute;
	instancesAreGood &=
			pmInstance.getAttribute(DEVICEID_KEY, pmDeviceIdAttribute) == framework::SUCCESS;
	instancesAreGood &=
			pmnsInstance.getAttribute(DEVICEID_KEY, pmnsDeviceIdAttribute) == framework::SUCCESS;

	if (instancesAreGood)
	{
		std::string pmUidStr = pmDeviceIdAttribute.stringValue();
		std::string pmnsUidStr = pmnsDeviceIdAttribute.stringValue();

		NVM_UID nsUid;
		uid_copy(pmnsUidStr.c_str(), nsUid);
		struct namespace_details details;
		lib_interface::NvmApi::getApi()->getNamespaceDetails(nsUid, &details);

		std::string nsBasedOnUid = "";
		if (details.type == NAMESPACE_TYPE_STORAGE)
		{
			NVM_UID uidStr;
			uid_copy(details.creation_id.device_uid, uidStr);
			nsBasedOnUid = std::string(uidStr);
		}
		else if (details.type == NAMESPACE_TYPE_APP_DIRECT)
		{
			struct pool *pPool = new struct pool;
			if (pPool != NULL)
			{
				lib_interface::NvmApi::getApi()->getPool(details.pool_uid, pPool);
				nsBasedOnUid = getInterleaveSetUuid(details.creation_id.interleave_setid,
									pPool->socket_id);
				delete pPool;
			}
			else
			{
				throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
						"couldn't allocate pool");
			}
		}

		if (!nsBasedOnUid.empty())
		{
			result = nsBasedOnUid == pmUidStr;
		}
	}

	return result;
}
