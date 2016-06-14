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
 * This file contains the provider for the MemoryAllocationSettings instances.
 */

#include <stdlib.h>
#include <algorithm>
#include <string.h>
#include <uid/uid.h>
#include <string>
#include <sstream>
#include <string/s_str.h>
#include <nvm_management.h>
#include <LogEnterExit.h>
#include <libinvm-cim/Types.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include <memory/SystemProcessorFactory.h>
#include <memory/VolatileMemoryFactory.h>
#include <memory/PersistentMemoryFactory.h>
#include <mem_config/MemoryResourcesFactory.h>
#include <mem_config/PoolViewFactory.h>
#include <mem_config/InterleaveSet.h>
#include "MemoryAllocationSettingsFactory.h"
#include <server/BaseServerFactory.h>
#include <libinvm-cim/ExceptionNotSupported.h>
#include <exception/NvmExceptionLibError.h>
#include <lib_interface/NvmApi.h>
#include <NvmStrings.h>

wbem::mem_config::MemoryAllocationSettingsFactory::MemoryAllocationSettingsFactory()
{
}

wbem::mem_config::MemoryAllocationSettingsFactory::~MemoryAllocationSettingsFactory()
{
}

void wbem::mem_config::MemoryAllocationSettingsFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(PARENT_KEY);
	attributes.push_back(ALLOCATIONUNITS_KEY);
	attributes.push_back(RESERVATION_KEY);
	attributes.push_back(POOLID_KEY);
	attributes.push_back(RESOURCETYPE_KEY);
	attributes.push_back(CHANNELINTERLEAVESIZE_KEY);
	attributes.push_back(CHANNELCOUNT_KEY);
	attributes.push_back(CONTROLLERINTERLEAVESIZE_KEY);
	attributes.push_back(REPLICATION_KEY);
}

/*
 * Retrieve a specific current config instance given an object path
 */
wbem::framework::Instance* wbem::mem_config::MemoryAllocationSettingsFactory::getInstance(
		framework::ObjectPath &path,
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);
		std::string instanceIdStr = path.getKeyValue(INSTANCEID_KEY).stringValue();

		// First make sure the name is in the correct format
		validateNameFormat(instanceIdStr);

		// Make sure the socket is valid
		NVM_UINT16 socketId = validateAndReturnSocketId(instanceIdStr);

		// ElementName - host name + " NVM allocation setting"
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			// Get the real host name
			std::string elementName = wbem::server::getHostName() +
									MEMORYALLOCATIONSETTINGS_ELEMENTNAME;
			framework::Attribute a(elementName, false);
			pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
		}

		// Parent - "CPU" + socketId
		if (containsAttribute(PARENT_KEY, attributes))
		{
			std::string parent = memory::SystemProcessorFactory::getDeviceId(socketId);
			framework::Attribute a(parent, false);
			pInstance->setAttribute(PARENT_KEY, a, attributes);
		}

		// AllocationUnits - "bytes"
		if (containsAttribute(ALLOCATIONUNITS_KEY, attributes))
		{
			framework::Attribute a(MEMORYALLOCATIONSETTINGS_ALLOCATIONUNITS, false);
			pInstance->setAttribute(ALLOCATIONUNITS_KEY, a, attributes);
		}

		// PoolID - "NVMPool1"
		if (containsAttribute(POOLID_KEY, attributes))
		{
			std::string parent = MEMORYRESOURCES_POOLID;
			framework::Attribute a(parent, false);
			pInstance->setAttribute(POOLID_KEY, a, attributes);
		}

		// ResourceType - One of resourcetype_memory or resourcetype_nonvolatile
		if (containsAttribute(RESOURCETYPE_KEY, attributes))
		{
			NVM_UINT16 resourceType = MEMORYALLOCATIONSETTINGS_RESOURCETYPE_UNKNOWN;
			if (isMemory(instanceIdStr))
			{
				resourceType = MEMORYALLOCATIONSETTINGS_RESOURCETYPE_MEMORY;
			}
			else
			{
				resourceType = MEMORYALLOCATIONSETTINGS_RESOURCETYPE_NONVOLATILE;
			}
			framework::Attribute a(resourceType, false);
			pInstance->setAttribute(RESOURCETYPE_KEY, a, attributes);
		}

		if (isGoal(instanceIdStr))
		{
			finishGoalInstance(pInstance, instanceIdStr, attributes);
		}
		else
		{
			finishCurrentConfigInstance(pInstance, instanceIdStr, attributes);
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


/*
 * Retrieve a specific current config instance given an object path
 */
void wbem::mem_config::MemoryAllocationSettingsFactory::finishGoalInstance(
		framework::Instance *pInstance, const std::string instanceIdStr,
		const framework::attribute_names_t attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	physical_asset::devices_t devices = physical_asset::NVDIMMFactory::getManageableDevices();

	if (isMemory(instanceIdStr))
	{
		NVM_UINT64 reservation = getMemoryReservationFromGoals(devices, instanceIdStr);
		finishMemoryOrStorageInstance(pInstance, reservation, attributes);
	}
	else // app direct
	{
		InterleaveSet ilset = getInterleaveSetFromGoals(devices, instanceIdStr);
		finishAppDirectInstance(pInstance, ilset, attributes);
	}
}

/*
 * Retrieve a specific current config instance given an object path
 */
void wbem::mem_config::MemoryAllocationSettingsFactory::finishCurrentConfigInstance(
		framework::Instance *pInstance, std::string instanceIdStr,
		framework::attribute_names_t attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// There can be more than one App Direct region on a socket and the name
	// alone is not sufficient to identify a specific region. So we
	// will adopt the convention that App Direct regions are ordered by
	// interleave_set index and numbered accordingly.

	std::vector<struct pool> pools = wbem::mem_config::PoolViewFactory::getPoolList();

	if (isMemory(instanceIdStr))
	{
		NVM_UINT64 reservation = getMemoryReservationFromPools(pools, instanceIdStr);
		finishMemoryOrStorageInstance(pInstance, reservation, attributes);
	}
	else if (isAppDirectMemory(instanceIdStr))
	{
		InterleaveSet ilset = getInterleaveSetFromPools(pools, instanceIdStr);
		finishAppDirectInstance(pInstance, ilset, attributes);
	}
	else // unmapped
	{
		NVM_UINT64 reservation = getUnmappedReservationFromPools(pools, instanceIdStr);
		finishMemoryOrStorageInstance(pInstance, reservation, attributes);
	}
}

/*
 * Finish a app direct instance given a reservation
 */
void wbem::mem_config::MemoryAllocationSettingsFactory::finishAppDirectInstance(
		framework::Instance *pInstance, InterleaveSet &ilset,
		framework::attribute_names_t attributes)
{
	// Reservation - requested capacity in bytes
	if (containsAttribute(RESERVATION_KEY, attributes))
	{
		NVM_UINT64 reservation = ilset.getSize();
		framework::Attribute a(reservation, false);
		pInstance->setAttribute(RESERVATION_KEY, a, attributes);
	}

	// ChannelInterleaveSize - only applicable for App Direct, otherwise 0
	if (containsAttribute(CHANNELINTERLEAVESIZE_KEY, attributes))
	{
		NVM_UINT16 channelInterleaveSize = ilset.getChannelInterleaveSize();
		framework::Attribute a(channelInterleaveSize, false);
		pInstance->setAttribute(CHANNELINTERLEAVESIZE_KEY, a, attributes);
	}

	// ChannelCount - only applicable for App Direct, otherwise 0
	if (containsAttribute(CHANNELCOUNT_KEY, attributes))
	{
		NVM_UINT16 channelCount = ilset.getChannelCount();
		framework::Attribute a(channelCount, false);
		pInstance->setAttribute(CHANNELCOUNT_KEY, a, attributes);
	}

	// ControllerInterleaveSize - only applicable for App Direct, otherwise 0
	if (containsAttribute(CONTROLLERINTERLEAVESIZE_KEY, attributes))
	{
		NVM_UINT16 controllerInterleaveSize = ilset.getControllerInterleaveSize();
		framework::Attribute a(controllerInterleaveSize, false);
		pInstance->setAttribute(CONTROLLERINTERLEAVESIZE_KEY, a, attributes);
	}

	// Replication - "Local replication" for mirrored interleave set, otherswise "Not Replicated"
	if (containsAttribute(REPLICATION_KEY, attributes))
	{
		NVM_UINT16 replication = ilset.getReplication();
		framework::Attribute a(replication, false);
		pInstance->setAttribute(REPLICATION_KEY, a, attributes);
	}
}

/*
 * Finish a memory mode or storage mode instance given a reservation
 */
void wbem::mem_config::MemoryAllocationSettingsFactory::finishMemoryOrStorageInstance(
		framework::Instance *pInstance, NVM_UINT64 reservation,
		framework::attribute_names_t attributes)
{
	// Reservation - requested capacity in bytes
	if (containsAttribute(RESERVATION_KEY, attributes))
	{
		framework::Attribute a(reservation, false);
		pInstance->setAttribute(RESERVATION_KEY, a, attributes);
	}

	// ChannelInterleaveSize - only applicable for App Direct, otherwise 0
	if (containsAttribute(CHANNELINTERLEAVESIZE_KEY, attributes))
	{
		framework::Attribute a(0, false);
		pInstance->setAttribute(CHANNELINTERLEAVESIZE_KEY, a, attributes);
	}

	// ChannelCount - only applicable for App Direct, otherwise 0
	if (containsAttribute(CHANNELCOUNT_KEY, attributes))
	{
		framework::Attribute a(0, false);
		pInstance->setAttribute(CHANNELCOUNT_KEY, a, attributes);
	}

	// ControllerInterleaveSize - only applicable for App Direct, otherwise 0
	if (containsAttribute(CONTROLLERINTERLEAVESIZE_KEY, attributes))
	{
		framework::Attribute a(0, false);
		pInstance->setAttribute(CONTROLLERINTERLEAVESIZE_KEY, a, attributes);
	}

	// Replication - "Local replication" for mirrored interleave set, otherswise "Not Replicated"
	if (containsAttribute(REPLICATION_KEY, attributes))
	{
		framework::Attribute a(MEMORYALLOCATIONSETTINGS_REPLICATION_NONE, false);
		pInstance->setAttribute(REPLICATION_KEY, a, attributes);
	}
}

NVM_UINT64 wbem::mem_config::MemoryAllocationSettingsFactory::getMemoryReservationFromGoals
		(const physical_asset::devices_t &devices, std::string instanceIdStr)
{
	NVM_UINT64 reservation = 0;

	NVM_UINT16 socketId = getSocketId(instanceIdStr);

	for (size_t i = 0; i < devices.size(); i++)
	{
		if (devices[i].socket_id == socketId)
		{
			struct config_goal goal;
			memset(&goal, 0, sizeof(struct config_goal));
			int rc = nvm_get_config_goal(devices[i].uid, &goal);

			if (rc == NVM_SUCCESS)
			{
				reservation += goal.memory_size * BYTES_PER_GB;
			}
			else if (rc != NVM_ERR_NOTFOUND)
			{
				COMMON_LOG_ERROR("Could not retrieve config_goal");
				throw exception::NvmExceptionLibError(rc);
			}
		}
	}
	return reservation;
}

void wbem::mem_config::MemoryAllocationSettingsFactory::addIlsetsFromGoal
	(std::vector<InterleaveSet> &ilsets, const struct config_goal *pGoal)
{
	std::vector<InterleaveSet>::iterator iter;
	if (pGoal->app_direct_count > 0)
	{
		addIlset(ilsets, pGoal, 1);
	}
	if (pGoal->app_direct_count > 1)
	{
		addIlset(ilsets, pGoal, 2);
	}
}

void wbem::mem_config::MemoryAllocationSettingsFactory::addIlset
	(std::vector<InterleaveSet> &ilsets, const struct config_goal *pGoal, const int setNum)
{
	InterleaveSet ilset(pGoal, setNum);
	std::vector<InterleaveSet>::iterator iter;
	if ((iter = std::find(ilsets.begin(), ilsets.end(), ilset)) != ilsets.end())
	{
		NVM_UINT64 size = iter->getSize();
		size += ilset.getSize();
		iter->setSize(size);
	}
	else
	{
		ilsets.push_back(ilset);
	}
}

wbem::mem_config::InterleaveSet
	wbem::mem_config::MemoryAllocationSettingsFactory::getInterleaveSetFromGoals
		(const physical_asset::devices_t &devices, std::string instanceIdStr)
{
	std::vector<InterleaveSet> ilsets;

	NVM_UINT16 socketId = getSocketId(instanceIdStr);
	size_t interleaveSetIndex = getInterleaveSetIndex(instanceIdStr);

	for (size_t i = 0; i < devices.size(); i++)
	{
		if (devices[i].socket_id == socketId && devices[i].manageability == MANAGEMENT_VALIDCONFIG)
		{
			int rc = NVM_SUCCESS;
			struct config_goal goal;
			memset(&goal, 0, sizeof(struct config_goal));
			rc = nvm_get_config_goal(devices[i].uid, &goal);
			if (rc  == NVM_SUCCESS)
			{
				addIlsetsFromGoal(ilsets, &goal);
			}
			else if (rc != NVM_ERR_NOTFOUND)
			{
				COMMON_LOG_ERROR("Could not retrieve config goal");
				throw wbem::exception::NvmExceptionLibError(rc);
			}
		}
	}

	std::sort(ilsets.begin(), ilsets.end());

	if (interleaveSetIndex > ilsets.size() - 1)
	{
		std::stringstream index;
		index << "index out of range: interleaveSetIndex = ";
		index << interleaveSetIndex;
		COMMON_LOG_ERROR(index.str().c_str());
		throw framework::ExceptionBadParameter(instanceIdStr.c_str());
	}

	return ilsets[interleaveSetIndex];
}

NVM_UINT64 wbem::mem_config::MemoryAllocationSettingsFactory::getUnmappedReservationFromPools
			(const std::vector<struct pool> &pools, const std::string instanceIdStr)
{
	NVM_UID uid;

	NVM_UINT16 socketId = getSocketId(instanceIdStr);
	NVM_UINT32 memoryControllerId = getMemoryControllerId(instanceIdStr);
	NVM_UINT32 channelId = getChannelId(instanceIdStr);

	getUidFromHandleInfo(socketId, memoryControllerId, channelId, uid);

	return getStorageCapacityForDimm(pools, uid);
}

void wbem::mem_config::MemoryAllocationSettingsFactory::getUidFromHandleInfo
			(const NVM_UINT16 socketId, const NVM_UINT32 memoryControllerId, const NVM_UINT32 channelId, NVM_UID uid)
{
	physical_asset::devices_t devices = physical_asset::NVDIMMFactory::getManageableDevices();

	for (size_t i = 0; i < devices.size(); i++)
	{
		if (devices[i].socket_id == socketId &&
			devices[i].device_handle.parts.memory_controller_id == memoryControllerId &&
			devices[i].device_handle.parts.mem_channel_id == channelId)
		{
			memmove(uid, devices[i].uid, NVM_MAX_UID_LEN);
			break;
		}
	}
}

NVM_UINT64 wbem::mem_config::MemoryAllocationSettingsFactory::getStorageCapacityForDimm
			(const std::vector<struct pool> &pools, const NVM_UID uid)
{
	NVM_UINT64 storageCapacity = 0;

	for (size_t i = 0; i < pools.size(); i++)
	{
		if (pools[i].type == POOL_TYPE_PERSISTENT)
		{
			int index = getIndexOfDimmInPoolOrReturnNotFound(&(pools[i]), uid);
			if ((index != framework::NOTFOUND) &&
					(index < NVM_MAX_DEVICES_PER_POOL))
			{
				storageCapacity = pools[i].storage_capacities[index];
			}
		}
	}
	return storageCapacity;
}

int wbem::mem_config::MemoryAllocationSettingsFactory::getIndexOfDimmInPoolOrReturnNotFound
			(const struct pool *pPool, const NVM_UID uid)
{
	int index = framework::NOTFOUND;

	for (size_t i = 0; i < pPool->dimm_count; i++)
	{
		if (memcmp(uid, pPool->dimms[i], NVM_MAX_UID_LEN) == 0)
		{
			index = i;
			break;
		}
	}
	return index;
}

NVM_UINT64 wbem::mem_config::MemoryAllocationSettingsFactory::getMemoryCapacityForSocket
	(const struct pool *pPool, const NVM_UINT16 socketId)
{
	NVM_UINT64 memoryCapacity = 0;

	for (size_t i = 0; i < pPool->dimm_count; i++)
	{
		int rc = NVM_SUCCESS;
		struct device_details details;
		memset(&details, 0, sizeof(struct device_details));

		if ((rc = nvm_get_device_details(pPool->dimms[i], &details)) == NVM_SUCCESS)
		{
			if (details.discovery.socket_id == socketId)
			{
				memoryCapacity += details.capacities.memory_capacity;
			}
		}
		else
		{
			COMMON_LOG_ERROR("Could not retrieve device details");
			throw wbem::exception::NvmExceptionLibError(rc);
		}
	}
	return memoryCapacity;
}

NVM_UINT64 wbem::mem_config::MemoryAllocationSettingsFactory::getMemoryReservationFromPools
	(const std::vector<struct pool> &pools, std::string instanceIdStr)
{
	NVM_UINT64 memoryReservation = 0;

	NVM_UINT16 socketId = getSocketId(instanceIdStr);

	for (size_t i = 0; i < pools.size(); i++)
	{
		if (pools[i].type == POOL_TYPE_VOLATILE)
		{
			memoryReservation = getMemoryCapacityForSocket(&(pools[i]), socketId);
		}
	}
	return memoryReservation;
}

NVM_UINT16 wbem::mem_config::MemoryAllocationSettingsFactory::validateAndReturnSocketId(const std::string instanceIdStr)
{
	int rc = NVM_SUCCESS;
	bool found = false;

	NVM_UINT16 socketId = getSocketId(instanceIdStr);

	int socketCount = 0;
	if ((socketCount = nvm_get_socket_count()) < 0)
	{
		COMMON_LOG_ERROR("Could not retrieve socket count");
		throw wbem::exception::NvmExceptionLibError(socketCount);
	}

	struct socket sockets[socketCount];
	memset(&sockets, 0, sizeof(sockets));

	if ((rc = nvm_get_sockets(sockets, socketCount)) != socketCount)
	{
		COMMON_LOG_ERROR("Could not retrieve sockets");
		throw wbem::exception::NvmExceptionLibError(rc);
	}

	for (int i = 0; i < socketCount; i++)
	{
		if (sockets[i].id == socketId)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		COMMON_LOG_ERROR("Socket id is not on the system");
		throw wbem::framework::ExceptionBadParameter(instanceIdStr.c_str());
	}

	return socketId;
}

void wbem::mem_config::MemoryAllocationSettingsFactory::validateNameFormat(const std::string instanceIdStr)
{
	// The correct format is AA.B.CCCC.D where A and C are digits, B is one of (P,U,V)
	// and D is either C or G
	if (instanceIdStr.length() != MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN)
	{
		COMMON_LOG_ERROR_F("Bad MemoryAllocationSettings instanceId format: Incorrect length - %s", instanceIdStr.c_str());
		throw wbem::framework::ExceptionBadParameter(instanceIdStr.c_str());
	}

	if (instanceIdStr[MEMORYALLOCATIONSETTINGS_PERIOD1_POSITION] != '.' ||
			instanceIdStr[MEMORYALLOCATIONSETTINGS_PERIOD2_POSITION] != '.' ||
			instanceIdStr[MEMORYALLOCATIONSETTINGS_PERIOD3_POSITION] != '.')
	{
		COMMON_LOG_ERROR_F("Bad MemoryAllocationSettings instanceId format: Decimals out of position - %s", instanceIdStr.c_str());
		throw wbem::framework::ExceptionBadParameter(instanceIdStr.c_str());
	}

	if (instanceIdStr[MEMORYALLOCATIONSETTINGS_MEMORYTYPE_POSITION] != MEMORY_MODE_PREFIX &&
			instanceIdStr[MEMORYALLOCATIONSETTINGS_MEMORYTYPE_POSITION] != APP_DIRECT_PREFIX &&
			instanceIdStr[MEMORYALLOCATIONSETTINGS_MEMORYTYPE_POSITION] != STORAGE_PREFIX)
	{
		COMMON_LOG_ERROR_F("Bad MemoryAllocationSettings instanceId format: Memory type incorrect - %s", instanceIdStr.c_str());
		throw wbem::framework::ExceptionBadParameter(instanceIdStr.c_str());
	}

	if (instanceIdStr[MEMORYALLOCATIONSETTINGS_CONFIGTYPE_POSITION] != GOAL_CONFIG &&
			instanceIdStr[MEMORYALLOCATIONSETTINGS_CONFIGTYPE_POSITION] != CURRENT_CONFIG)
	{
		COMMON_LOG_ERROR_F("Bad MemoryAllocationSettings instanceId format: Bad config type - %s", instanceIdStr.c_str());
		throw wbem::framework::ExceptionBadParameter(instanceIdStr.c_str());
	}

	if (!isdigit(instanceIdStr[MEMORYALLOCATIONSETTINGS_SOCKETNUMBERFIRSTDIGIT_POSITION]) ||
		!isdigit(instanceIdStr[MEMORYALLOCATIONSETTINGS_SOCKETNUMBERSECONDDIGIT_POSITION]) ||
		!isdigit(instanceIdStr[MEMORYALLOCATIONSETTINGS_REGIONIDFIRSTDIGIT_POSITION]) ||
		!isdigit(instanceIdStr[MEMORYALLOCATIONSETTINGS_REGIONIDSECONDDIGIT_POSITION]) ||
		!isdigit(instanceIdStr[MEMORYALLOCATIONSETTINGS_REGIONIDTHIRDDIGIT_POSITION]) ||
		!isdigit(instanceIdStr[MEMORYALLOCATIONSETTINGS_REGIONIDFOURTHDIGIT_POSITION]))
	{
		COMMON_LOG_ERROR_F("Bad MemoryAllocationSettings instanceId format: Digit expected - %s", instanceIdStr.c_str());
		throw wbem::framework::ExceptionBadParameter(instanceIdStr.c_str());
	}
}

/*
 * This section of code implements isAssociated
 */
/*
 *  * Determine if the VolatileMemory has a complex association.
 *   */
bool wbem::mem_config::MemoryAllocationSettingsFactory::isAssociated(
                const std::string &associationClass,
                framework::Instance* pAntInstance,
                framework::Instance* pDepInstance)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::framework::Attribute attr;

    bool result = true;

    if (associationClass == wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTSETTINGDATA)
    {
    	try
    	{
    		if (pDepInstance->getClass() == MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME &&
    			pAntInstance->getClass() == MEMORYRESOURCES_CREATIONCLASSNAME)
    		{
    			result = (isGoalInstance(pDepInstance));
    		}
    		else if (pDepInstance->getClass() == MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME &&
    				 pAntInstance->getClass() == memory::VOLATILEMEMORY_CREATIONCLASSNAME)
    		{
    			result = (isMemoryModeCurrentConfigInstance(pDepInstance));
    		}
    		else if (pDepInstance->getClass() == MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME &&
    				 pAntInstance->getClass() == memory::PERSISTENTMEMORY_CREATIONCLASSNAME)
    		{
    			result = (isSettingAssociatedWithMemoryInstance(pDepInstance, pAntInstance));
    		}
    		else
    		{
    			COMMON_LOG_WARN("Incorrect antecedent and dependent class instances");
    		}
    	}
    	catch (wbem::framework::Exception &)
    	{
			COMMON_LOG_WARN_F("Cannot calculate if instances are an association "
					"based on association class: %s", associationClass.c_str());
    	}
    }
    else
	{
		COMMON_LOG_WARN_F("This class has no associations of type: %s", associationClass.c_str());
	}
	return result;
}

bool
	wbem::mem_config::MemoryAllocationSettingsFactory::isSettingAssociatedWithMemoryInstance
		(const framework::Instance* pSettingInstance, const framework::Instance* pMemoryInstance)
{
	bool associated = false;

	if (isUnmappedInstance(pSettingInstance))
	{
		associated = isUnmappedSettingAssociatedWithMemoryInstance(pSettingInstance, pMemoryInstance);
	}
	else if (isAppDirectInstance(pSettingInstance))
	{
		associated = isAppDirectSettingAssociatedWithMemoryInstance(pSettingInstance, pMemoryInstance);
	}
	return associated;
}

bool
	wbem::mem_config::MemoryAllocationSettingsFactory::isAppDirectSettingAssociatedWithMemoryInstance
		(const framework::Instance* pSettingInstance, const framework::Instance* pMemoryInstance)
{
	bool associated = false;
	NVM_UID ilsetUid;

	getIlsetUidFromSettingInstance(pSettingInstance, ilsetUid);

	NVM_UID appDirectMemoryUid;
	getAppDirectMemoryUid(pMemoryInstance, appDirectMemoryUid);

	if (memcmp(appDirectMemoryUid, ilsetUid, NVM_MAX_UID_LEN) == 0)
	{
		associated = true;
	}
	return associated;
}

void
	wbem::mem_config::MemoryAllocationSettingsFactory::getIlsetUidFromSettingInstance
		(const framework::Instance* pSettingInstance, NVM_UID ilsetUid)
{
	std::vector<struct pool> pools = wbem::mem_config::PoolViewFactory::getPoolList(false);

	wbem::framework::Attribute attr;
	pSettingInstance->getAttribute(wbem::INSTANCEID_KEY, attr);
	std::string instanceIdStr = attr.stringValue();

	NVM_UINT16 socketId = getSocketId(instanceIdStr);

	InterleaveSet ilset = getInterleaveSetFromPools(pools, instanceIdStr);

	std::string ilsetUidStr = memory::PersistentMemoryFactory::getInterleaveSetUuid(
			ilset.getSetIndex(), socketId);

	uid_copy(ilsetUidStr.c_str(), ilsetUid);
}

bool
	wbem::mem_config::MemoryAllocationSettingsFactory::isUnmappedSettingAssociatedWithMemoryInstance
		(const framework::Instance* pSettingInstance, const framework::Instance* pMemoryInstance)
{
	bool associated = false;
	NVM_UID uid;

	getAppDirectMemoryUid(pMemoryInstance, uid);

	// The uid contained in the deviceId of the AppDirectMemory instance
	// may be an interleave set uid and if so, we'll just return false
	if (isADeviceUid(uid))
	{
		if (deviceUidMatchesSetting(pSettingInstance, uid))
		{
			associated = true;
		}
	}
	return associated;
}

bool wbem::mem_config::MemoryAllocationSettingsFactory::isADeviceUid(const NVM_UID uid)
{
	bool isADeviceUid = false;

	struct device_discovery device;
	memset(&device, 0, sizeof(struct device_discovery));
	int rc = nvm_get_device_discovery(uid, &device);

	if (rc == NVM_SUCCESS)
	{
		isADeviceUid = true;
	}
	return isADeviceUid;
}

void
	wbem::mem_config::MemoryAllocationSettingsFactory::getAppDirectMemoryUid
		(const framework::Instance* pMemoryInstance, NVM_UID uid)
{
	wbem::framework::Attribute attr;

	pMemoryInstance->getAttribute(wbem::DEVICEID_KEY, attr);
	uid_copy(attr.stringValue().c_str(), uid);
}

bool
	wbem::mem_config::MemoryAllocationSettingsFactory::deviceUidMatchesSetting
		(const framework::Instance* pSettingInstance, const NVM_UID uid)
{
	bool matches = false;

	physical_asset::devices_t devices = physical_asset::NVDIMMFactory::getManageableDevices();

	NVM_NFIT_DEVICE_HANDLE handle = getHandleForDimmUid(devices, uid);

	NVM_UINT32 socketId = getSocketIdFromSettingInstance(pSettingInstance);
	NVM_UINT32 memoryControllerId = getMemoryControllerIdFromSettingInstance(pSettingInstance);
	NVM_UINT32 channelId = getChannelIdFromSettingInstance(pSettingInstance);

	if (handle.parts.socket_id == socketId &&
		handle.parts.memory_controller_id == memoryControllerId &&
		handle.parts.mem_channel_id == channelId)
	{
		matches = true;
	}
	return matches;
}

bool
	wbem::mem_config::MemoryAllocationSettingsFactory::isAppDirectInstance
		(const framework::Instance* pSettingInstance)
{
	bool isAppDirect = false;

	wbem::framework::Attribute attr;
	pSettingInstance->getAttribute(wbem::INSTANCEID_KEY, attr);
	std::string instanceIdStr = attr.stringValue();
	if (getRegionType(instanceIdStr) == APP_DIRECT_PREFIX)
	{
		isAppDirect = true;
	}
	return isAppDirect;
}

bool
	wbem::mem_config::MemoryAllocationSettingsFactory::isUnmappedInstance
		(const framework::Instance* pSettingInstance)
{
	bool isUnmapped = false;

	wbem::framework::Attribute attr;
	pSettingInstance->getAttribute(wbem::INSTANCEID_KEY, attr);
	std::string instanceIdStr = attr.stringValue();
	if (getRegionType(instanceIdStr) == STORAGE_PREFIX)
	{
		isUnmapped = true;
	}
	return isUnmapped;
}

bool wbem::mem_config::MemoryAllocationSettingsFactory::isMemoryModeCurrentConfigInstance
	(const framework::Instance* pInstance)
{
	bool isMemory = false;
	wbem::framework::Attribute attr;

	pInstance->getAttribute(wbem::INSTANCEID_KEY, attr);

	char instanceType = getInstanceType(attr.stringValue());
	char regionType = getRegionType(attr.stringValue());

	if (instanceType == CURRENT_CONFIG &&
		regionType == MEMORY_MODE_PREFIX)
	{
		isMemory = true;
	}
	return isMemory;
}

bool wbem::mem_config::MemoryAllocationSettingsFactory::isMemoryModeGoalConfigInstance
	(const framework::Instance* pInstance)
{
	bool isMemory = false;
	wbem::framework::Attribute attr;

	pInstance->getAttribute(wbem::INSTANCEID_KEY, attr);

	char instanceType = getInstanceType(attr.stringValue());
	char regionType = getRegionType(attr.stringValue());

	if (instanceType == GOAL_CONFIG &&
		regionType == MEMORY_MODE_PREFIX)
	{
		isMemory = true;
	}
	return isMemory;
}

bool wbem::mem_config::MemoryAllocationSettingsFactory::isAppDirectGoalConfigInstance
	(const framework::Instance* pInstance)
{
	bool isAppDirect = false;
	wbem::framework::Attribute attr;

	pInstance->getAttribute(wbem::INSTANCEID_KEY, attr);

	char instanceType = getInstanceType(attr.stringValue());
	char regionType = getRegionType(attr.stringValue());

	if (instanceType == GOAL_CONFIG &&
		regionType == APP_DIRECT_PREFIX)
	{
		isAppDirect = true;
	}
	return isAppDirect;
}

/*
 * This section of code implements getInstanceNames
 */
/*
 * Return the object paths for the MemoryAllocationSettings Class.
 */
wbem::framework::instance_names_t* wbem::mem_config::MemoryAllocationSettingsFactory::getInstanceNames()
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();

	try
	{
		std::string hostName = wbem::server::getHostName();

		physical_asset::devices_t devices = physical_asset::NVDIMMFactory::getManageableDevices();

		StringListType names = getCurrentConfigNames(devices);

		StringListType goalNames = getGoalNames(devices);

		names.insert(names.end(), goalNames.begin(), goalNames.end());

		StringListType::iterator iter = names.begin();
		for (; iter != names.end(); iter++)
		{
			framework::attributes_t keys;
			keys[INSTANCEID_KEY] = framework::Attribute(*iter, true);
			framework::ObjectPath path(hostName, NVM_NAMESPACE,
					MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME, keys);
			pNames->push_back(path);
		}
	}
	catch (framework::Exception &)
	{
		if (pNames != NULL)
		{
			delete pNames;
		}
		throw;
	}

	return pNames;
}

wbem::mem_config::StringListType wbem::mem_config::MemoryAllocationSettingsFactory::getCurrentConfigNames
	(const physical_asset::devices_t &devices)
{
	StringListType names;
	std::vector<struct pool> pools = wbem::mem_config::PoolViewFactory::getPoolList(false);
	std::vector<struct pool>::const_iterator poolIter = pools.begin();
	for (; poolIter != pools.end(); poolIter++)
	{
		int appDirectRegionId = 0;
		for (int i = 0; i < poolIter->dimm_count; i++)
		{
			NVM_NFIT_DEVICE_HANDLE handle = getHandleForDimmUid(devices, poolIter->dimms[i]);
			// for each Memory Mode region, generate an instance name
			if (poolIter->memory_capacities[i] > 0)
			{
				char memoryInstanceName[MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1];
				s_snprintf(memoryInstanceName, MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1,
						"%02u.%c.0000.%c", handle.parts.socket_id, MEMORY_MODE_PREFIX, CURRENT_CONFIG);
				names.push_back(memoryInstanceName);
			}

			// for each unmapped instance, generate an instance name
			if (poolIter->storage_capacities[i] > 0)
			{
				char unmappedName[MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1];
				s_snprintf(unmappedName, MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1,
						"%02u.%c.%02u%02u.%c", handle.parts.socket_id, STORAGE_PREFIX,
						handle.parts.memory_controller_id,
						handle.parts.mem_channel_id,
						CURRENT_CONFIG);
				names.push_back(unmappedName);
			}
		}

		// for each interleave set, generate an instance name
		for (size_t i = 0; i < poolIter->ilset_count; i++)
		{
			char appDirectName[MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1];
			s_snprintf(appDirectName, MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1,
					"%02u.%c.%04u.%c", poolIter->socket_id, APP_DIRECT_PREFIX,
					 appDirectRegionId, CURRENT_CONFIG);
			appDirectRegionId++;
			names.push_back(appDirectName);
		}
	}
	names.sort();
	names.unique();
	return names;
}

wbem::mem_config::StringListType wbem::mem_config::MemoryAllocationSettingsFactory::getGoalNames
	(const physical_asset::devices_t &devices)
{
	StringListType allNames, namesForSocket;
	NVM_UINT16 socketId = devices[0].socket_id;

	namesForSocket = getGoalNamesForSocket(devices, socketId);
	allNames.insert(allNames.end(), namesForSocket.begin(), namesForSocket.end());

	physical_asset::devices_t::const_iterator devIter = devices.begin();
	for (;devIter != devices.end(); devIter++)
	{
		if ((*devIter).socket_id != socketId)
		{
			namesForSocket.clear();
			socketId = (*devIter).socket_id;
			namesForSocket =  getGoalNamesForSocket(devices, socketId);
			allNames.insert(allNames.end(), namesForSocket.begin(), namesForSocket.end());
		}
	}
	return allNames;
}

wbem::mem_config::StringListType wbem::mem_config::MemoryAllocationSettingsFactory::getGoalNamesForSocket
	(const physical_asset::devices_t &devices, NVM_UINT16 socketId)
{
	StringListType names;
	std::map<NVM_UINT16, int> appDirectIds;
	int appDirectRegionId = 0;
	physical_asset::devices_t::const_iterator devIter = devices.begin();
	for (;devIter != devices.end(); devIter++)
	{
		if ((*devIter).socket_id == socketId)
		{
			struct config_goal goal;
			memset(&goal, 0, sizeof(struct config_goal));
			int rc = nvm_get_config_goal((*devIter).uid, &goal);
			if (rc == NVM_SUCCESS)
			{
				if (goal.memory_size > 0)
				{
					char memoryName[MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1];
					s_snprintf(memoryName, MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1,
							"%02u.%c.0000.%c", (*devIter).socket_id, MEMORY_MODE_PREFIX, GOAL_CONFIG);
					names.push_back(memoryName);
				}

				if (goal.app_direct_count > 1)
				{
					char appDirectName[MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1];
					if (appDirectIds.find(goal.app_direct_2_set_id) == appDirectIds.end())
					{
						appDirectIds[goal.app_direct_2_set_id] = appDirectRegionId;
						appDirectRegionId++;
					}
					s_snprintf(appDirectName, MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1,
							"%02u.%c.%04u.%c", (*devIter).socket_id, APP_DIRECT_PREFIX,
							appDirectIds[goal.app_direct_2_set_id], GOAL_CONFIG);
					names.push_back(appDirectName);
				}

				if (goal.app_direct_count > 0)
				{
					char appDirectName[MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1];
					if (appDirectIds.find(goal.app_direct_1_set_id) == appDirectIds.end())
					{
						appDirectIds[goal.app_direct_1_set_id] = appDirectRegionId;
						appDirectRegionId++;
					}
					s_snprintf(appDirectName, MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN + 1,
							"%02u.%c.%04u.%c", (*devIter).socket_id, APP_DIRECT_PREFIX,
							appDirectIds[goal.app_direct_1_set_id], GOAL_CONFIG);
					names.push_back(appDirectName);
				}
			}
			else if (rc != NVM_ERR_NOTFOUND)
			{
				COMMON_LOG_ERROR("Could not retrieve config goal");
				throw exception::NvmExceptionLibError(rc);
			}
		}
	}

	names.sort();
	names.unique();
	return names;
}

NVM_NFIT_DEVICE_HANDLE
	wbem::mem_config::MemoryAllocationSettingsFactory::getHandleForDimmUid
			(const wbem::physical_asset::devices_t &devices, const NVM_UID uid)
{
	NVM_NFIT_DEVICE_HANDLE handle;
	bool found = false;

	for (size_t i = 0; i < devices.size(); i++)
	{
		if (uid_cmp(uid, devices[i].uid))
		{
			handle = devices[i].device_handle;
			found = true;
			break;
		}
	}

	if (!found)
	{
		NVM_UID uidStr;
		uid_copy(uid, uidStr);
		COMMON_LOG_ERROR_F("Device uid %s could not be found.", uidStr);
		throw framework::ExceptionBadParameter(uidStr);
	}
	return handle;
}

wbem::mem_config::InterleaveSet
	wbem::mem_config::MemoryAllocationSettingsFactory::getInterleaveSetFromPools
		(const std::vector<struct pool> &pools, std::string instanceIdStr)
{
	std::vector<InterleaveSet> ilsets;

	NVM_UINT16 socketId = getSocketId(instanceIdStr);
	size_t interleaveSetIndex = getInterleaveSetIndex(instanceIdStr);

	for (size_t i = 0; i < pools.size(); i++)
	{

		if (pools[i].socket_id == socketId)
		{
			for (int j = 0; j < pools[i].ilset_count; j++)
			{
				ilsets.push_back(InterleaveSet(&(pools[i].ilsets[j])));
			}
		}
	}
	std::sort(ilsets.begin(), ilsets.end());

	if (interleaveSetIndex > ilsets.size() - 1)
	{
		std::stringstream index;
		index << "index out of range: interleaveSetIndex = ";
		index << interleaveSetIndex;
		COMMON_LOG_ERROR(index.str().c_str());
		throw framework::ExceptionBadParameter(instanceIdStr.c_str());
	}

	return ilsets[interleaveSetIndex];
}

/*
 * This section of code implements utilities function
 */

bool wbem::mem_config::MemoryAllocationSettingsFactory::isAppDirectMemory(const std::string instanceIdStr)
{
	bool isAppDirect = false;
	if (getRegionType(instanceIdStr) == APP_DIRECT_PREFIX)
	{
		isAppDirect = true;
	}
	return isAppDirect;
}

bool wbem::mem_config::MemoryAllocationSettingsFactory::isMemory(const std::string instanceIdStr)
{
	bool isMemory = false;
	if (getRegionType(instanceIdStr) == MEMORY_MODE_PREFIX)
	{
		isMemory = true;
	}
	return isMemory;
}

bool
	wbem::mem_config::MemoryAllocationSettingsFactory::isGoalInstance
		(const framework::Instance* pInstance)
{
	wbem::framework::Attribute attr;
	pInstance->getAttribute(wbem::INSTANCEID_KEY, attr);
	return isGoal(attr.stringValue());
}

bool
wbem::mem_config::MemoryAllocationSettingsFactory::isGoal
		(const std::string instanceId)
{
	bool isGoal = false;
	if (getInstanceType(instanceId) == GOAL_CONFIG)
	{
		isGoal = true;
	}
	return isGoal;
}

std::string
	wbem::mem_config::MemoryAllocationSettingsFactory::getInstanceIdStrFromSettingInstance
			(const framework::Instance* pSettingInstance)
{
	wbem::framework::Attribute attr;
	pSettingInstance->getAttribute(wbem::INSTANCEID_KEY, attr);
	return attr.stringValue();
}

NVM_UINT32
	wbem::mem_config::MemoryAllocationSettingsFactory::getSocketIdFromSettingInstance
		(const framework::Instance* pSettingInstance)
{
	std::string instanceIdStr = getInstanceIdStrFromSettingInstance(pSettingInstance);

	return getSocketId(instanceIdStr);
}

NVM_UINT32
	wbem::mem_config::MemoryAllocationSettingsFactory::getSocketId(const std::string instanceIdStr)
{

	std::string socketIdStr = instanceIdStr.substr(MEMORYALLOCATIONSETTINGS_SOCKETNUMBERFIRSTDIGIT_POSITION,2);

	return atoi(socketIdStr.c_str());
}

NVM_UINT32
	wbem::mem_config::MemoryAllocationSettingsFactory::getMemoryControllerIdFromSettingInstance
		(const framework::Instance* pSettingInstance)
{
	wbem::framework::Attribute attr;
	pSettingInstance->getAttribute(wbem::INSTANCEID_KEY, attr);
	std::string instanceIdStr = attr.stringValue();

	return getMemoryControllerId(instanceIdStr);
}

NVM_UINT32
	wbem::mem_config::MemoryAllocationSettingsFactory::getMemoryControllerId(const std::string instanceIdStr)
{
	return atoi(instanceIdStr.substr(MEMORYALLOCATIONSETTINGS_MEMORYCONTROLLERIDFIRSTDIGIT_POSITION,
										MEMORYALLOCATIONSETTINGS_MEMORYCONTROLLERID_LENGTH).c_str());
}

NVM_UINT32
	wbem::mem_config::MemoryAllocationSettingsFactory::getChannelIdFromSettingInstance
		(const framework::Instance* pSettingInstance)
{
	wbem::framework::Attribute attr;
	pSettingInstance->getAttribute(wbem::INSTANCEID_KEY, attr);
	std::string instanceIdStr = attr.stringValue();

	return getChannelId(instanceIdStr);
}

NVM_UINT32
	wbem::mem_config::MemoryAllocationSettingsFactory::getChannelId(const std::string instanceIdStr)
{
	return atoi(instanceIdStr.substr(MEMORYALLOCATIONSETTINGS_CHANNELIDFIRSTDIGIT_POSITION,
				MEMORYALLOCATIONSETTINGS_CHANNELID_LENGTH).c_str());
}

NVM_UINT32
	wbem::mem_config::MemoryAllocationSettingsFactory::getInterleaveSetIndex(const std::string instanceIdStr)
{
	return atoi(instanceIdStr.substr(MEMORYALLOCATIONSETTINGS_REGIONIDFIRSTDIGIT_POSITION,
					MEMORYALLOCATIONSETTINGS_REGIONID_LENGTH).c_str());
}

char wbem::mem_config::MemoryAllocationSettingsFactory::getRegionType(const std::string instanceIdStr)
{
	return instanceIdStr[MEMORYALLOCATIONSETTINGS_MEMORYTYPE_POSITION];
}

char wbem::mem_config::MemoryAllocationSettingsFactory::getInstanceType(const std::string instanceIdStr)
{
	return instanceIdStr[MEMORYALLOCATIONSETTINGS_CONFIGTYPE_POSITION];
}
