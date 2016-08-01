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
 * This file contains the provider for the MemoryConfiguration instances. This class is no longer
 * part of the actual CIM Schema, however, a lot depends on it so it still exists in the code.
 */

#include <stdlib.h>
#include <string>
#include <nvm_management.h>
#include <utility.h>
#include <LogEnterExit.h>
#include <libinvm-cim/Types.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ExceptionBadAttribute.h>
#include <libinvm-cim/ExceptionNoMemory.h>
#include <physical_asset/NVDIMMFactory.h>
#include "MemoryConfigurationFactory.h"

#include <exception/NvmExceptionLibError.h>
#include <lib_interface/NvmApi.h>
#include <NvmStrings.h>
#include <core/device/DeviceHelper.h>
#include "MemoryConfigurationServiceFactory.h"

wbem::mem_config::MemoryConfigurationFactory::MemoryConfigurationFactory()
{
}

wbem::mem_config::MemoryConfigurationFactory::~MemoryConfigurationFactory()
{
}

void wbem::mem_config::MemoryConfigurationFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(PARENT_KEY);
	attributes.push_back(MEMORYSIZE_KEY);
	attributes.push_back(INTERLEAVEINDEXES_KEY);
	attributes.push_back(INTERLEAVEFORMATS_KEY);
	attributes.push_back(INTERLEAVESIZES_KEY);
	attributes.push_back(STORAGECAPACITY_KEY);
	attributes.push_back(PACKAGEREDUNDANCY_KEY);
	attributes.push_back(SOCKETID_KEY);
	attributes.push_back(STATUS_KEY);
}

/*
 * Return the index where the uid is found in the pool's dimms array. Return -1 if the uid
 * is not in the array.
 */
int wbem::mem_config::MemoryConfigurationFactory::getDimmIndexInPoolOrReturnNotFound
		(const NVM_UID uid, const struct pool *pool)
{
	int result = framework::NOTFOUND;

	for (int i = 0; i < pool->dimm_count; i++)
	{
		if (uid_cmp(uid, pool->dimms[i]))
		{
			result = i;
			break;
		}
	}
	return result;
}

/*
 * Return true or false depending on whether or not the dimm belongs to the interleave set
 */
bool wbem::mem_config::MemoryConfigurationFactory::dimmIsInIlset(const NVM_UID uid,
					const struct interleave_set &ilset)
{
	bool result = false;

	for (int i = 0; i < ilset.dimm_count; i++)
	{
		if (uid_cmp(uid, ilset.dimms[i]))
		{
			result = true;
			break;
		}
	}
	return result;
}

/*
 * Get the interleave set info for the given dimm. To do this we search through all the PM pools
 * (mirrored and unmirrored) to see if 1) the dimm is in the pool and 2) the dimm is in one of the
 * interleave sets in the pool.
 */
void wbem::mem_config::MemoryConfigurationFactory::getCurrentIlsetInfo(const NVM_UID deviceUid,
			const std::vector<struct pool> &pools, std::vector<struct InterleaveSetInfo> &ilsetInfos)
{
	for (size_t i = 0; i < pools.size(); i++)
	{
		if (pools[i].type != POOL_TYPE_VOLATILE &&
				getDimmIndexInPoolOrReturnNotFound(deviceUid, &(pools[i])) != framework::NOTFOUND)
		{
			for (int j = 0; j < pools[i].ilset_count; j++)
			{
				if (dimmIsInIlset(deviceUid, (pools[i].ilsets[j])))
				{
					struct InterleaveSetInfo ilsetInfo;
					// If this dimm belongs to more that two interleave sets
					// then that is an illegal device configuration
					if (ilsetInfos.size() == NVM_MAX_INTERLEAVE_SETS_PER_DIMM)
					{
						COMMON_LOG_ERROR("Config has more than two interleave sets on a dimm");
						throw exception::NvmExceptionLibError(NVM_ERR_BADDEVICECONFIG);
					}

					ilsetInfo.size = pools[i].ilsets[j].size / pools[i].ilsets[j].dimm_count;
					ilsetInfo.mirrored = pools[i].ilsets[j].mirrored;
					interleave_struct_to_format(&(pools[i].ilsets[j].settings), &(ilsetInfo.settings));
					ilsetInfo.setIndex =pools[i].ilsets[j].set_index;
					ilsetInfos.push_back(ilsetInfo);
				}
			} 
		}
	}
}

/*
 * find the memory capacity of this dimm
 */
NVM_UINT64 wbem::mem_config::MemoryConfigurationFactory::getDimmMemoryCapacityFromCurrentConfig
		(const NVM_UID uid, const std::vector<struct pool> &pools) throw (wbem::framework::Exception)
{
	int index = 0;
	NVM_UINT64 size = 0;

	for (size_t i = 0; i < pools.size(); i++)
	{
		if (pools[i].type == POOL_TYPE_VOLATILE && 
			(index = getDimmIndexInPoolOrReturnNotFound(uid, &(pools[i]))) != framework::NOTFOUND)
		{
			if (index < NVM_MAX_DEVICES_PER_POOL)
			{
				size = pools[i].memory_capacities[index];
				break;
			}
			else
			{
				throw exception::NvmExceptionLibError(NVM_ERR_BADDEVICECONFIG);
			}
		}
	}
	return size;
}

/*
 * find the storage capacity of this dimm
 */
NVM_UINT64 wbem::mem_config::MemoryConfigurationFactory::getDimmStorageCapacityFromCurrentConfig(
		const NVM_UID uid, const std::vector<struct pool> &pools) throw (wbem::framework::Exception)
{
	int index;
	NVM_UINT64 size = 0;
	for (size_t i = 0; i < pools.size(); i++)
	{
		if (pools[i].type == POOL_TYPE_PERSISTENT && 
			(index = getDimmIndexInPoolOrReturnNotFound(uid, &(pools[i]))) != framework::NOTFOUND)
		{
			if (index < NVM_MAX_DEVICES_PER_POOL)
			{
				size = pools[i].storage_capacities[index];
				break;
			}
			else
			{
				throw exception::NvmExceptionLibError(NVM_ERR_BADDEVICECONFIG);
			}
		}
	}
	return size;
}

/*
 * Populate a current config instance with memory and storage sizes and also interleave set information
 */
void wbem::mem_config::MemoryConfigurationFactory::populateCurrentConfigInstance(
		const framework::attribute_names_t &attributes,
		const std::string &uidStr,
		wbem::framework::Instance* pInstance,
		const struct device_discovery *p_discovery)
{
	framework::UINT64 memoryCapacity = 0;
	framework::UINT64 storageCapacity = 0;
	framework::UINT32_LIST interleaveFormats;
	framework::UINT64_LIST interleaveSizes;
	framework::UINT16_LIST redundancies;
	framework::UINT16_LIST setIndexes;

	NVM_UID uid;
	lib_interface::NvmApi::stringToNvmUid(uidStr, uid);

	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();

	std::vector<struct pool> pools;
	pApi->getPools(pools);

	if (pools.size() > 0)
	{
		memoryCapacity = getDimmMemoryCapacityFromCurrentConfig(uid, pools);
		storageCapacity = getDimmStorageCapacityFromCurrentConfig(uid, pools);

		std::vector <InterleaveSetInfo> infos;
		getCurrentIlsetInfo(uid, pools, infos);

		for (size_t i = 0; i < infos.size(); i++)
		{

			interleaveFormats.push_back(infos[i].settings);
			interleaveSizes.push_back(infos[i].size);
			redundancies.push_back(infos[i].mirrored);
			setIndexes.push_back(infos[i].setIndex);
		}
	}

	// MemorySize
	if (containsAttribute(MEMORYSIZE_KEY, attributes))
	{
		framework::Attribute a(memoryCapacity, false);
		pInstance->setAttribute(MEMORYSIZE_KEY, a, attributes);
	}

	// StorageCapacity - unmapped storage capacity in this pool
	if (containsAttribute(STORAGECAPACITY_KEY, attributes))
	{
		framework::Attribute a(storageCapacity, false);
		pInstance->setAttribute(STORAGECAPACITY_KEY, a, attributes);
	}

	// InterleaveFormats - list of uint16 bitmaps representing channel size, iMC size, way
	// of each interleave set in this pool.
	if (containsAttribute(INTERLEAVEFORMATS_KEY, attributes))
	{
		framework::Attribute a(interleaveFormats, false);
		pInstance->setAttribute(INTERLEAVEFORMATS_KEY, a, attributes);
	}
		
	// InterleaveSizes - corresponding sizes of each interleave set listed in InterleaveFormats
	if (containsAttribute(INTERLEAVESIZES_KEY, attributes))
	{
		framework::Attribute a(interleaveSizes, false);
		pInstance->setAttribute(INTERLEAVESIZES_KEY, a, attributes);
	}

	// PackageRedundancy - 1 for mirrored, 0 for other (including memory mode)
	if (containsAttribute(PACKAGEREDUNDANCY_KEY, attributes))
	{
		framework::Attribute a(redundancies, false);
		pInstance->setAttribute(PACKAGEREDUNDANCY_KEY, a, attributes);
	}

	// InterleaveSetIndex - unique index of interleave set
	if (containsAttribute(INTERLEAVEINDEXES_KEY, attributes))
	{
		framework::Attribute a(setIndexes, false);
		pInstance->setAttribute(INTERLEAVEINDEXES_KEY, a, attributes);
	}

	// Status - doesn't apply to current config
	if (containsAttribute(STATUS_KEY, attributes))
	{
		framework::Attribute a(wbem::NA, false);
		pInstance->setAttribute(STATUS_KEY, a, attributes);
	}
}

void wbem::mem_config::MemoryConfigurationFactory::configGoalToGoalInstance(
		const framework::attribute_names_t &attributes, const struct device_discovery *pDiscovery,
		const struct config_goal &goal, wbem::framework::Instance *pInstance)
{
	// MemorySize
	if (containsAttribute(MEMORYSIZE_KEY, attributes))
	{
		NVM_UINT64 memorySizeBytes = goal.memory_size * BYTES_PER_GB;
		framework::Attribute a(memorySizeBytes, false);
		pInstance->setAttribute(MEMORYSIZE_KEY, a, attributes);
	}

	framework::UINT32_LIST interleaveFormats;
	framework::UINT16_LIST interleaveIndexes;
	framework::UINT64_LIST interleaveSizes;
	framework::UINT16_LIST packageRedundancy;

	if (goal.app_direct_count > 0)
	{
		interleaveSizes.push_back(goal.app_direct_1_size * BYTES_PER_GB);
		packageRedundancy.push_back(goal.app_direct_1_settings.mirrored);

		NVM_UINT32 format1 = 0;
		interleave_struct_to_format(&goal.app_direct_1_settings.interleave, &format1);
		interleaveFormats.push_back(format1);
		interleaveIndexes.push_back(goal.app_direct_1_set_id);
	}

	if (goal.app_direct_count > 1)
	{
		interleaveSizes.push_back(goal.app_direct_2_size * BYTES_PER_GB);
		packageRedundancy.push_back(goal.app_direct_2_settings.mirrored);

		NVM_UINT32 format2 = 0;
		interleave_struct_to_format(&goal.app_direct_2_settings.interleave, &format2);
		interleaveFormats.push_back(format2);
		interleaveIndexes.push_back(goal.app_direct_2_set_id);
	}

	// Interleave set sizes
	if (containsAttribute(INTERLEAVESIZES_KEY, attributes))
	{
		framework::Attribute a(interleaveSizes, false);
		pInstance->setAttribute(INTERLEAVESIZES_KEY, a, attributes);
	}

	// Package redundancy
	if (containsAttribute(PACKAGEREDUNDANCY_KEY, attributes))
	{
		framework::Attribute a(packageRedundancy, false);
		pInstance->setAttribute(PACKAGEREDUNDANCY_KEY, a, attributes);
	}

	// Interleave formats
	if (containsAttribute(INTERLEAVEFORMATS_KEY, attributes))
	{
		framework::Attribute a(interleaveFormats, false);
		pInstance->setAttribute(INTERLEAVEFORMATS_KEY, a, attributes);
	}

	// Interleave indexes
	if (containsAttribute(INTERLEAVEINDEXES_KEY, attributes))
	{
		framework::Attribute indexes(interleaveIndexes, false);
		pInstance->setAttribute(INTERLEAVEINDEXES_KEY, indexes, attributes);
	}

	// Storage capacity
	if (containsAttribute(STORAGECAPACITY_KEY, attributes))
	{
		NVM_UINT64 storageCapacity = getDimmStorageCapacityFromGoal(pDiscovery, goal);

		framework::Attribute a(storageCapacity, false);
		pInstance->setAttribute(STORAGECAPACITY_KEY, a, attributes);
	}

	// Status
	if (containsAttribute(STATUS_KEY, attributes))
	{
		framework::Attribute a((NVM_UINT32)goal.status,
				configGoalStatusToString(goal.status), false);
		pInstance->setAttribute(STATUS_KEY, a, attributes);
	}
}

/*
 * Populate a goal config instance with memory and storage sizes and also interleave set information
 */
void wbem::mem_config::MemoryConfigurationFactory::populateGoalInstance(
			const framework::attribute_names_t &attributes,
			const std::string &uidStr,
			wbem::framework::Instance* pInstance,
			const struct device_discovery *pDiscovery)
{
	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();

	struct config_goal goal;
	pApi->getConfigGoalForDimm(uidStr, goal);

	// Goals that have already been applied are no longer goals
	if (goal.status == CONFIG_GOAL_STATUS_SUCCESS)
	{
		throw framework::ExceptionBadAttribute(INSTANCEID_KEY.c_str());
	}

	configGoalToGoalInstance(attributes, pDiscovery, goal, pInstance);
}

std::string wbem::mem_config::MemoryConfigurationFactory::configGoalStatusToString(
		const enum config_goal_status status)
{
	std::string statusString;

	// Note that success is ignored: we don't display applied goals.
	switch (status)
	{
	case CONFIG_GOAL_STATUS_NEW:
		statusString = TR("New");
		break;
	case CONFIG_GOAL_STATUS_ERR_BADREQUEST:
		statusString = TR("Failed - Bad request");
		break;
	case CONFIG_GOAL_STATUS_ERR_INSUFFICIENTRESOURCES:
		statusString = TR("Failed - Not enough resources");
		break;
	case CONFIG_GOAL_STATUS_ERR_FW:
		statusString = TR("Failed - Firmware error");
		break;
	case CONFIG_GOAL_STATUS_ERR_UNKNOWN:
		statusString = TR("Failed - Unknown");
		break;
	case CONFIG_GOAL_STATUS_UNKNOWN:
	default:
		statusString = TR("Unknown");
		break;
	}

	return statusString;
}

void wbem::mem_config::MemoryConfigurationFactory::populateInstanceDimmInfoFromDiscovery(
		framework::attribute_names_t &attributes,
		wbem::framework::Instance *pInstance,
		const struct device_discovery &discovery)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Parent - dimm UID
	if (containsAttribute(PARENT_KEY, attributes))
	{
		NVM_UID uidStr;
		uid_copy(discovery.uid, uidStr);

		framework::Attribute uidAttr(uidStr, false);
		pInstance->setAttribute(PARENT_KEY, uidAttr, attributes);
	}

	// SocketID of DIMM
	if (containsAttribute(SOCKETID_KEY, attributes))
	{
		framework::Attribute socketIdAttr(discovery.socket_id, false);
		pInstance->setAttribute(SOCKETID_KEY, socketIdAttr, attributes);
	}

}

wbem::framework::instances_t *wbem::mem_config::MemoryConfigurationFactory::getInstancesFromLayout(
		const core::memory_allocator::MemoryAllocationLayout &layout,
		framework::attribute_names_t &attributes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkAttributes(attributes);

	wbem::framework::instances_t *pInstances = new wbem::framework::instances_t();
	if (!pInstances)
	{
		throw wbem::framework::ExceptionNoMemory(__FILE__, __FUNCTION__,
				"Couldn't allocate new instances_t");
	}

	try
	{
		wbem::lib_interface::NvmApi *pApi = wbem::lib_interface::NvmApi::getApi();
		for (std::map<std::string, struct config_goal>::const_iterator goalIter = layout.goals.begin();
				goalIter != layout.goals.end(); goalIter++)
		{
			wbem::framework::Instance instance;

			std::string goaldimmUid = goalIter->first;

			struct device_discovery discovery;
			pApi->getDeviceDiscoveryForDimm(goaldimmUid, discovery);

			populateInstanceDimmInfoFromDiscovery(attributes, &instance, discovery);
			configGoalToGoalInstance(attributes, &discovery, goalIter->second, &instance);
			pInstances->push_back(instance);
		}
	}
	catch (wbem::framework::Exception &)
	{
		delete pInstances;
		throw;
	}

	return pInstances;
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::mem_config::MemoryConfigurationFactory::getInstance(
		framework::ObjectPath &path,
		framework::attribute_names_t &attributes)
				throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);
		lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();

		// parse InstanceID
		framework::Attribute idAttr = path.getKeyValue(INSTANCEID_KEY);
		std::string instanceId = idAttr.stringValue();

		if (!isValidInstanceId(instanceId))
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}

		std::string uidStr = instanceId.substr(0, instanceId.length() - 1);
		struct device_discovery deviceDiscovery;
		pApi->getDeviceDiscoveryForDimm(uidStr, deviceDiscovery);

		// ElementName - host name + " NVM allocation setting"
		if (containsAttribute(ELEMENTNAME_KEY, attributes))
		{
			// Get the real host name
			std::string elementName = pApi->getHostName() +
					MEMORYCONFIGURATION_ELEMENTNAME;
			framework::Attribute a(elementName, false);
			pInstance->setAttribute(ELEMENTNAME_KEY, a, attributes);
		}

		populateInstanceDimmInfoFromDiscovery(attributes, pInstance, deviceDiscovery);

		if (isGoalConfig(instanceId))
		{
			populateGoalInstance(attributes, uidStr, pInstance, &deviceDiscovery);
		}
		else
		{
			populateCurrentConfigInstance(attributes, uidStr, pInstance, &deviceDiscovery);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pInstance;
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for the MemoryConfiguration Class.
 */
wbem::framework::instance_names_t* wbem::mem_config::MemoryConfigurationFactory::getInstanceNames()
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();
		std::string hostName = pApi->getHostName();

		std::vector<struct pool> pools;
		pApi->getPools(pools);

		std::vector<std::string> uids = wbem::physical_asset::NVDIMMFactory::getManageableDeviceUids();

		for (unsigned int i = 0; i < uids.size(); i++)
		{
			framework::attributes_t keys;

			NVM_UID uid;
			uid_copy(uids[i].c_str(), uid);

			if (MemoryConfigurationServiceFactory::dimmHasGoal(uid))
			{
				std::string instanceIdStr(uids[i]);
				instanceIdStr += GOAL_CONFIG;
				keys[INSTANCEID_KEY] = framework::Attribute(instanceIdStr, true);

				framework::ObjectPath configPath(hostName, NVM_NAMESPACE,
							MEMORYCONFIGURATION_CREATIONCLASSNAME, keys);
				pNames->push_back(configPath);
			}

			if (dimmIsInAPool(uid, pools))
			{
				std::string instanceIdStr(uids[i]);
				instanceIdStr += CURRENT_CONFIG;
				keys[INSTANCEID_KEY] = framework::Attribute(instanceIdStr, true);

				framework::ObjectPath goalPath(hostName, NVM_NAMESPACE,
						MEMORYCONFIGURATION_CREATIONCLASSNAME, keys);
				pNames->push_back(goalPath);
			}
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}

	return pNames;
}

/*
 * Helper function to retrieve just goal instance names
 */
wbem::framework::instance_names_t* wbem::mem_config::MemoryConfigurationFactory::getGoalInstanceNames(
	const bool onlyUnappliedGoals)
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::instance_names_t *pNames = new framework::instance_names_t();

	try
	{
		std::string hostName = wbem::server::getHostName();

		// get dimm uids
		std::vector<std::string> uids = wbem::physical_asset::NVDIMMFactory::getManageableDeviceUids();

		for (unsigned int i = 0; i < uids.size(); i++)
		{
			framework::attributes_t keys;
			struct config_goal goal;

			NVM_UID uid;
			uid_copy(uids[i].c_str(), uid);
			int rc;
			if ((rc = nvm_get_config_goal(uid, &goal)) == NVM_SUCCESS)
			{
				if (goal.status != CONFIG_GOAL_STATUS_SUCCESS)
				{
					std::string instanceIdStr(uids[i]);
					instanceIdStr += GOAL_CONFIG;
					keys[INSTANCEID_KEY] = framework::Attribute(instanceIdStr, true);

					framework::ObjectPath configPath(hostName, NVM_NAMESPACE,
						MEMORYCONFIGURATION_CREATIONCLASSNAME, keys);
					pNames->push_back(configPath);
				}
				if ((!onlyUnappliedGoals) &&
					(goal.status == CONFIG_GOAL_STATUS_SUCCESS))
				{
					std::string instanceIdStr(uids[i]);
					instanceIdStr += CURRENT_CONFIG;
					keys[INSTANCEID_KEY] = framework::Attribute(instanceIdStr, true);

					framework::ObjectPath configPath(hostName, NVM_NAMESPACE,
						MEMORYCONFIGURATION_CREATIONCLASSNAME, keys);
					pNames->push_back(configPath);
				}
			}
			else if (rc != NVM_ERR_NOTFOUND)
			{
				throw exception::NvmExceptionLibError(rc);
			}
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}

	return pNames;
}

/*
 * Helper function to retrieve just goal instances
 */
wbem::framework::instances_t* wbem::mem_config::MemoryConfigurationFactory::getGoalInstances(
	wbem::framework::attribute_names_t &attributes,
	const bool onlyUnappliedGoals)
		throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pGoalPaths = NULL;
	framework::instances_t *pGoalList = NULL;
	try
	{
		checkAttributes(attributes);
		pGoalPaths = getGoalInstanceNames(onlyUnappliedGoals);
		if (pGoalPaths != NULL)
		{
			// create the return list
			pGoalList = new framework::instances_t();

			// loop through the names
			for (framework::instance_names_t::iterator iter = pGoalPaths->begin();
					iter != pGoalPaths->end(); iter++)
			{
				framework::Instance* pInst = NULL;
				pInst = getInstance(*iter, attributes);
				if (pInst != NULL)
				{
					pGoalList->push_back(*pInst);
					delete pInst;
				}
			}
		}
	}

	// on error, cleanup but don't handle
	catch (wbem::framework::Exception &)
	{
		if (pGoalPaths)
		{
			delete pGoalPaths;
		}
		if (pGoalList)
		{
			delete pGoalList;
		}
		throw;
	}

	// clean up
	if (pGoalPaths)
	{
		pGoalPaths->clear();
		delete pGoalPaths;
	}

	return pGoalList;
}


NVM_UINT64 wbem::mem_config::MemoryConfigurationFactory::getDimmStorageCapacityFromGoal
	(const struct device_discovery *pDiscovery, const struct config_goal &goal)
{
	NVM_UINT64 storageCapacity = USABLE_CAPACITY_BYTES(pDiscovery->capacity);
	storageCapacity -= (goal.memory_size * BYTES_PER_GB);
	if (goal.app_direct_count > 0)
	{
		if (goal.app_direct_1_settings.mirrored)
		{
			storageCapacity -= (goal.app_direct_1_size * BYTES_PER_GB * 2);
		}
		else
		{
			storageCapacity -= (goal.app_direct_1_size * BYTES_PER_GB);
		}
	}
	if (goal.app_direct_count > 1)
	{
		if (goal.app_direct_2_settings.mirrored)
		{
			storageCapacity -= (goal.app_direct_2_size * BYTES_PER_GB * 2);
		}
		else
		{
			storageCapacity -= (goal.app_direct_2_size * BYTES_PER_GB);
		}
	}
	return storageCapacity;
}

bool wbem::mem_config::MemoryConfigurationFactory::dimmIsInAPool(NVM_UID uid, std::vector<struct pool> pools)
{
	bool inAPool = false;
	for (size_t i = 0; i < pools.size(); i++)
	{
		if (getDimmIndexInPoolOrReturnNotFound(uid, &(pools[i])) != framework::NOTFOUND)
		{
			inAPool = true;
			break;
		}
	}
	return inAPool;
}

bool wbem::mem_config::MemoryConfigurationFactory::isValidInstanceId(std::string instanceId)
{
	// InstanceID = dimmUid + 'G' or 'C'.
	std::string uid = instanceId.substr(0, instanceId.length() - 1);
	return core::device::isUidValid(uid) &&
			 (isCurrentConfig(instanceId) || isGoalConfig(instanceId));
}

bool wbem::mem_config::MemoryConfigurationFactory::isGoalConfig(std::string instanceId)
{
	bool isGoal = false;
	if (instanceId[instanceId.length() -1] == GOAL_CONFIG)
	{
		isGoal = true;
	}
	return isGoal;
}

bool wbem::mem_config::MemoryConfigurationFactory::isCurrentConfig(std::string instanceId)
{
	bool isGoal = false;
	if (instanceId[instanceId.length() -1] == CURRENT_CONFIG)
	{
		isGoal = true;
	}
	return isGoal;
}
