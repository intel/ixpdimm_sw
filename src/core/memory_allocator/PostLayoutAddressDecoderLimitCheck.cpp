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
 * Rule that checks that the address decoder limit is not exceeded
 */

#include "PostLayoutAddressDecoderLimitCheck.h"

#include <LogEnterExit.h>
#include <uid/uid.h>
#include <map>
#include <iostream>
#include <core/exceptions/NvmExceptionBadRequest.h>

core::memory_allocator::PostLayoutAddressDecoderLimitCheck::PostLayoutAddressDecoderLimitCheck(
		const std::vector<struct device_discovery> &devices,
		const std::vector<struct pool> &pools,
		const NVM_UINT16 numSystemSockets) :
		m_devices(devices),
		m_pools(pools),
		m_numSystemSockets(numSystemSockets)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::PostLayoutAddressDecoderLimitCheck::~PostLayoutAddressDecoderLimitCheck()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

NVM_UINT16 core::memory_allocator::PostLayoutAddressDecoderLimitCheck::getSocketIdForDimm(NVM_UID &uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 socketId = 0;
	for (std::vector<struct device_discovery>::const_iterator iter = m_devices.begin();
			iter != m_devices.end(); iter++)
	{
		if (uid_cmp(iter->uid, uid))
		{
			socketId = iter->socket_id;
			break;
		}
	}

	return socketId;
}

std::list<NVM_UINT16> core::memory_allocator::PostLayoutAddressDecoderLimitCheck::getListOfSocketsInLayout(
		const struct MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::list<NVM_UINT16> socketList;
	for (std::map<std::string, struct config_goal>::const_iterator iter = layout.goals.begin();
			iter != layout.goals.end(); iter++)
	{
		NVM_UID uid;
		uid_copy((*iter).first.c_str(), uid);
		NVM_UINT16 socketId = getSocketIdForDimm(uid);
		socketList.push_back(socketId);
	}

	socketList.sort();
	socketList.unique();
	return socketList;
}

std::vector<struct config_goal> core::memory_allocator::PostLayoutAddressDecoderLimitCheck::getConfigGoalsForSocket(
		const struct MemoryAllocationLayout &layout, const NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct config_goal> configGoalList;
	for (std::map<std::string, struct config_goal>::const_iterator iter = layout.goals.begin();
			iter != layout.goals.end(); iter++)
	{
		NVM_UID uid;
		uid_copy((*iter).first.c_str(), uid);
		NVM_UINT16 dimmSocketId = getSocketIdForDimm(uid);

		if (dimmSocketId == socketId)
		{
			configGoalList.push_back((*iter).second);
		}
	}

	return configGoalList;
}

NVM_UINT16 core::memory_allocator::PostLayoutAddressDecoderLimitCheck::getNumberOfIlsetsOnSocket(
		const struct MemoryAllocationLayout &layout, const NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 numGoalInterleaveSets = getNumberOfConfigGoalInterleaveSetsOnSocket(layout, socketId);
	NVM_UINT16 numPoolInterleaveSets = getNumberOfUnchangedPoolInterleaveSetsOnSocket(layout, socketId);

	return numGoalInterleaveSets + numPoolInterleaveSets;
}

void core::memory_allocator::PostLayoutAddressDecoderLimitCheck::verify(
		const struct MemoryAllocationRequest& request,
		const struct MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_numSystemSockets >= criticalNumberOfSockets)
	{
		// Only need to check the sockets we are configuring
		std::list<NVM_UINT16> listOfSocketsInLayout = getListOfSocketsInLayout(layout);
		for (std::list<NVM_UINT16>::iterator iter = listOfSocketsInLayout.begin();
				iter != listOfSocketsInLayout.end(); iter++)
		{
			NVM_UINT16 numberOfIlsetsOnSocket = getNumberOfIlsetsOnSocket(layout, *iter);

			if (numberOfIlsetsOnSocket > criticalNumberOfIlsetsOnSocket)
			{
				throw core::NvmExceptionOverAddressDecoderLimit();
			}
		}
	}
}

NVM_UINT16 core::memory_allocator::PostLayoutAddressDecoderLimitCheck::getNumberOfConfigGoalInterleaveSetsOnSocket(
		const struct MemoryAllocationLayout& layout, const NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct config_goal> configGoalsForSocket = getConfigGoalsForSocket(layout, socketId);
	std::list<NVM_UINT16> interleaveSetIndexList;
	for (std::vector<struct config_goal>::iterator iter = configGoalsForSocket.begin();
			iter != configGoalsForSocket.end(); iter++)
	{
		if (iter->app_direct_count > 0)
		{
			interleaveSetIndexList.push_back(iter->app_direct_1_set_id);
		}

		if (iter->app_direct_count > 1)
		{
			interleaveSetIndexList.push_back(iter->app_direct_2_set_id);
		}
	}

	interleaveSetIndexList.sort();
	interleaveSetIndexList.unique();
	return interleaveSetIndexList.size();
}

NVM_UINT16 core::memory_allocator::PostLayoutAddressDecoderLimitCheck::getNumberOfUnchangedPoolInterleaveSetsOnSocket(
		const struct MemoryAllocationLayout& layout, const NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 numPoolInterleaveSets = 0;

	for (std::vector<struct pool>::const_iterator poolIter = m_pools.begin();
			poolIter != m_pools.end(); poolIter++)
	{
		if (poolIter->socket_id == socketId)
		{
			numPoolInterleaveSets += getNumberOfUnchangedInterleaveSetsInPool(layout, *poolIter);
		}
	}

	return numPoolInterleaveSets;
}

NVM_UINT16 core::memory_allocator::PostLayoutAddressDecoderLimitCheck::getNumberOfUnchangedInterleaveSetsInPool(
		const struct MemoryAllocationLayout& layout, const struct pool& pool)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 numUnchangedInterleaveSets = 0;

	for (NVM_UINT16 i = 0; i < pool.ilset_count; i++)
	{
		if (!isInterleaveSetOverwrittenByLayout(layout, pool.ilsets[i]))
		{
			numUnchangedInterleaveSets++;
		}
	}

	return numUnchangedInterleaveSets;
}

bool core::memory_allocator::PostLayoutAddressDecoderLimitCheck::isInterleaveSetOverwrittenByLayout(
		const struct MemoryAllocationLayout& layout, const struct interleave_set& interleave)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isOverwritten = false;

	for (NVM_UINT16 i = 0; i < interleave.dimm_count; i++)
	{
		NVM_UID uidStr;
		uid_copy(interleave.dimms[i], uidStr);

		// Any DIMM in the new layout will have all its interleave sets overwritten
		if (layout.goals.find(uidStr) != layout.goals.end())
		{
			isOverwritten = true;
			break;
		}
	}

	return isOverwritten;
}
