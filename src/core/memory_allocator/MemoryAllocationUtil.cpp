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

#include <uid/uid.h>
#include <core/exceptions/LibraryException.h>
#include <core/device/DeviceService.h>
#include <LogEnterExit.h>

core::memory_allocator::MemoryAllocationUtil::MemoryAllocationUtil(core::NvmLibrary &nvmLib) :
		m_nvmLib(nvmLib)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::MemoryAllocationUtil::~MemoryAllocationUtil()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

NVM_UINT16 core::memory_allocator::MemoryAllocationUtil::getNextAvailableInterleaveSetId(
		const core::memory_allocator::MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 maxId = 0;
	getLastInterleaveSetIdFromCurrentConfig(maxId);
	getLastInterleaveSetIdFromConfigGoals(maxId);
	getLastInterleaveSetIdFromLayout(layout, maxId);

	return maxId + 1;
}

void core::memory_allocator::MemoryAllocationUtil::getLastInterleaveSetIdFromCurrentConfig(NVM_UINT16 &maxId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct pool> pools = m_nvmLib.getPools();
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

void core::memory_allocator::MemoryAllocationUtil::getLastInterleaveSetIdFromConfigGoals(NVM_UINT16 &maxId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	core::device::DeviceService service(m_nvmLib);
	std::vector<std::string> uids = service.getManageableUids();
	for (NVM_UINT32 i = 0; i < uids.size(); i++)
	{
		NVM_UINT32 maxDimmInterleaveIndex = getDimmInterleaveInfoMaxSetIndex(uids[i]);
		if (maxDimmInterleaveIndex > maxId)
		{
			maxId = maxDimmInterleaveIndex;
		}
	}
}

NVM_UINT16 core::memory_allocator::MemoryAllocationUtil::getDimmInterleaveInfoMaxSetIndex(const std::string &dimmUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_UINT16 maxInterleaveIndex = 0;
	try
	{
		struct config_goal goal = m_nvmLib.getConfigGoal(dimmUid);

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
	catch (core::LibraryException &e)
	{
		// no goal is OK - but anything else is bad news
		if (e.getErrorCode() != NVM_ERR_NOTFOUND)
		{
			throw;
		}
	}

	return maxInterleaveIndex;
}

void core::memory_allocator::MemoryAllocationUtil::getLastInterleaveSetIdFromLayout(
		const core::memory_allocator::MemoryAllocationLayout &layout, NVM_UINT16 &maxId)
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

core::memory_allocator::Dimm core::memory_allocator::MemoryAllocationUtil::deviceDiscoveryToDimm(
		const struct device_discovery& deviceDiscovery)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	core::memory_allocator::Dimm dimm;

	NVM_UID uidStr;
	uid_copy(deviceDiscovery.uid, uidStr);
	dimm.uid = uidStr;

	dimm.capacity = deviceDiscovery.capacity;
	dimm.socket = deviceDiscovery.socket_id;
	dimm.memoryController = deviceDiscovery.memory_controller_id;
	dimm.channel = deviceDiscovery.device_handle.parts.mem_channel_id;

	return dimm;
}
