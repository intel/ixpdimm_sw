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

#include "MemoryAllocationUtil.h"
#include <physical_asset/NVDIMMFactory.h>
#include <uid/uid.h>
#include <exception/NvmExceptionLibError.h>
#include <LogEnterExit.h>

wbem::logic::MemoryAllocationUtil::MemoryAllocationUtil() : m_pApi(NULL)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	m_pApi = lib_interface::NvmApi::getApi();
}

wbem::logic::MemoryAllocationUtil::MemoryAllocationUtil(lib_interface::NvmApi* pApi) :
		m_pApi(pApi)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

wbem::logic::MemoryAllocationUtil::~MemoryAllocationUtil()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

NVM_UINT16 wbem::logic::MemoryAllocationUtil::getNextAvailableInterleaveSetId(
		const wbem::logic::MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 maxId = 0;
	getLastInterleaveSetIdFromCurrentConfig(maxId);
	getLastInterleaveSetIdFromConfigGoals(maxId);
	getLastInterleaveSetIdFromLayout(layout, maxId);

	return maxId + 1;
}

void wbem::logic::MemoryAllocationUtil::getLastInterleaveSetIdFromCurrentConfig(NVM_UINT16 &maxId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct pool> pools;
	m_pApi->getPools(pools);
	for (std::vector<struct pool>::const_iterator iter = pools.begin();
			iter != pools.end(); iter++)
	{
		const struct pool &currentPool = *iter;
		for (int i = 0; i < currentPool.ilset_count; i++)
		{
			NVM_UINT32 interleaveId = currentPool.ilsets[i].set_index;
			if (interleaveId > maxId)
			{
				maxId = interleaveId;
			}
		}
	}
}

void wbem::logic::MemoryAllocationUtil::getLastInterleaveSetIdFromConfigGoals(NVM_UINT16 &maxId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<std::string> guids = physical_asset::NVDIMMFactory::getManageableDeviceGuids();
	for (NVM_UINT32 i = 0; i < guids.size(); i++)
	{
		NVM_UINT32 maxDimmInterleaveIndex = getDimmInterleaveInfoMaxSetIndex(guids[i]);
		if (maxDimmInterleaveIndex > maxId)
		{
			maxId = maxDimmInterleaveIndex;
		}
	}
}

NVM_UINT16 wbem::logic::MemoryAllocationUtil::getDimmInterleaveInfoMaxSetIndex(const std::string &dimmGuid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_UINT16 maxInterleaveIndex = 0;
	try
	{
		struct config_goal goal;
		m_pApi->getConfigGoalForDimm(dimmGuid, goal);

		if (goal.app_direct_count > 0)
		{
			maxInterleaveIndex = goal.app_direct_1_set_id;
		}
		if (goal.app_direct_count > 1)
		{
			if (goal.app_direct_2_set_id > maxInterleaveIndex)
			{
				maxInterleaveIndex = goal.app_direct_2_set_id;
			}
		}
	}
	catch (exception::NvmExceptionLibError &e)
	{
		// no goal is OK - but anything else is bad news
		if (e.getLibError() != NVM_ERR_NOTFOUND)
		{
			throw;
		}
	}

	return maxInterleaveIndex;
}

void wbem::logic::MemoryAllocationUtil::getLastInterleaveSetIdFromLayout(
		const wbem::logic::MemoryAllocationLayout &layout, NVM_UINT16 &maxId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::map<std::string, struct config_goal>::const_iterator goalIter = layout.goals.begin();
			goalIter != layout.goals.end(); goalIter++)
	{
		if (goalIter->second.app_direct_count > 1)
		{
			if (goalIter->second.app_direct_2_set_id > maxId)
			{
				maxId = goalIter->second.app_direct_2_set_id;
			}
		}
		if (goalIter->second.app_direct_count > 0)
		{
			if (goalIter->second.app_direct_1_set_id > maxId)
			{
				maxId = goalIter->second.app_direct_1_set_id;
			}
		}
	}
}

wbem::logic::Dimm wbem::logic::MemoryAllocationUtil::deviceDiscoveryToDimm(
		const struct device_discovery& deviceDiscovery)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	wbem::logic::Dimm dimm;

	NVM_GUID_STR guidStr;
	uid_copy(deviceDiscovery.guid, guidStr);
	dimm.guid = guidStr;

	dimm.capacity = deviceDiscovery.capacity;
	dimm.socket = deviceDiscovery.socket_id;
	dimm.memoryController = deviceDiscovery.memory_controller_id;
	dimm.channel = deviceDiscovery.device_handle.parts.mem_channel_id;

	return dimm;
}
