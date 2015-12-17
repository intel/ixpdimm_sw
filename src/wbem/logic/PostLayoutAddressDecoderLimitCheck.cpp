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
 * Rule that checks that the address decoder limit is not exceeded
 */

#include "PostLayoutAddressDecoderLimitCheck.h"
#include <exception/NvmExceptionBadRequest.h>
#include <exception/NvmExceptionNotManageable.h>
#include <LogEnterExit.h>
#include <guid/guid.h>
#include <map>
#include <iostream>

wbem::logic::PostLayoutAddressDecoderLimitCheck::PostLayoutAddressDecoderLimitCheck(
		const std::vector<struct device_discovery> &devices) :
		m_devices(devices)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

wbem::logic::PostLayoutAddressDecoderLimitCheck::~PostLayoutAddressDecoderLimitCheck()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

NVM_UINT16 wbem::logic::PostLayoutAddressDecoderLimitCheck::getSocketIdForDimm(NVM_GUID &guid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT16 socketId = 0;
	for (std::vector<struct device_discovery>::const_iterator iter = m_devices.begin();
			iter != m_devices.end(); iter++)
	{
		if (guid_cmp(iter->guid, guid))
		{
			socketId = iter->socket_id;
			break;
		}
	}

	return socketId;
}

std::list<NVM_UINT16> wbem::logic::PostLayoutAddressDecoderLimitCheck::getListOfSocketsInLayout(
		const struct MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::list<NVM_UINT16> socketList;
	for (std::map<std::string, struct config_goal>::const_iterator iter = layout.goals.begin();
			iter != layout.goals.end(); iter++)
	{
		NVM_GUID guid;
		str_to_guid((*iter).first.c_str(), guid);
		NVM_UINT16 socketId = getSocketIdForDimm(guid);
		socketList.push_back(socketId);
	}

	socketList.unique();
	return socketList;
}

std::vector<struct config_goal> wbem::logic::PostLayoutAddressDecoderLimitCheck::getConfigGoalsForSocket(
		const struct MemoryAllocationLayout &layout, const NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct config_goal> configGoalList;
	for (std::map<std::string, struct config_goal>::const_iterator iter = layout.goals.begin();
			iter != layout.goals.end(); iter++)
	{
		NVM_GUID guid;
		str_to_guid((*iter).first.c_str(), guid);
		NVM_UINT16 dimmSocketId = getSocketIdForDimm(guid);

		if (dimmSocketId == socketId)
		{
			configGoalList.push_back((*iter).second);
		}
	}

	return configGoalList;
}

NVM_UINT16 wbem::logic::PostLayoutAddressDecoderLimitCheck::getNumberOfIlsetsOnSocket(
		const struct MemoryAllocationLayout &layout, const NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<struct config_goal> configGoalsForSocket = getConfigGoalsForSocket(layout, socketId);
	std::list<NVM_UINT16> interleaveSetIndexList;
	for (std::vector<struct config_goal>::iterator iter = configGoalsForSocket.begin();
			iter != configGoalsForSocket.end(); iter++)
	{
		if (iter->persistent_count > 0)
		{
			interleaveSetIndexList.push_back(iter->persistent_1_set_id);
		}

		if (iter->persistent_count > 1)
		{
			interleaveSetIndexList.push_back(iter->persistent_2_set_id);
		}
	}

	interleaveSetIndexList.sort();
	interleaveSetIndexList.unique();
	return interleaveSetIndexList.size();
}

void wbem::logic::PostLayoutAddressDecoderLimitCheck::verify(
		const struct MemoryAllocationRequest& request,
		const struct MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::list<NVM_UINT16> listOfSocketsInLayout = getListOfSocketsInLayout(layout);

	if (listOfSocketsInLayout.size() >= criticalNumberOfSockets)
	{
		for (std::list<NVM_UINT16>::iterator iter = listOfSocketsInLayout.begin();
				iter != listOfSocketsInLayout.end(); iter++)
		{
			NVM_UINT16 numberOfIlsetsOnSocket = getNumberOfIlsetsOnSocket(layout, *iter);

			if (numberOfIlsetsOnSocket > criticalNumberOfIlsetsOnSocket)
			{
				throw wbem::exception::NvmExceptionOverAddressDecoderLimit();
			}
		}
	}
}
