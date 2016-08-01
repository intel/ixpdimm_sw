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
 * Rule that checks that the user is not configuring only part of a socket
 */

#include "RulePartialSocketConfigured.h"

#include <LogEnterExit.h>
#include <uid/uid.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

core::memory_allocator::RulePartialSocketConfigured::RulePartialSocketConfigured(
		const std::vector<struct device_discovery> manageableDevices,
		core::NvmLibrary &nvmLib) :
		m_manageableDimms(manageableDevices), m_nvmLib(nvmLib)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::RulePartialSocketConfigured::~RulePartialSocketConfigured()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

std::list<NVM_UINT16> core::memory_allocator::RulePartialSocketConfigured::getRequestedSockets(std::vector<Dimm> dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::list<NVM_UINT16> socketList;
	for (std::vector<Dimm>::const_iterator iter = dimms.begin(); iter != dimms.end(); iter++)
	{
		socketList.push_back((*iter).socket);
	}

	socketList.unique();

	return socketList;
}

std::set<std::string> core::memory_allocator::RulePartialSocketConfigured::getSetOfAllDimmsOnSocket(
		NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::set<std::string> dimmSet;
	for (std::vector<struct device_discovery>::const_iterator iter = m_manageableDimms.begin();
			iter != m_manageableDimms.end(); iter++)
	{
		if ((*iter).socket_id == socketId)
		{
			NVM_UID uidStr;
			uid_copy((*iter).uid, uidStr);
			dimmSet.insert(uidStr);
		}
	}

	return dimmSet;
}

std::set<std::string> core::memory_allocator::RulePartialSocketConfigured::getSetOfRequestedDimmsOnSocket(
		const std::vector<Dimm> &requestedDimms, NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::set<std::string> dimmSet;
	for (std::vector<Dimm>::const_iterator iter = requestedDimms.begin();
			iter != requestedDimms.end(); iter++)
	{
		if ((*iter).socket == socketId)
		{
			dimmSet.insert((*iter).uid);
		}
	}

	return dimmSet;
}

bool core::memory_allocator::RulePartialSocketConfigured::deviceIsNew(NVM_UID uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool deviceIsNew = false;

	struct device_status status = m_nvmLib.getDeviceStatus(uid);
	if (status.is_new)
	{
		deviceIsNew = true;
	}

	return deviceIsNew;
}

std::set<std::string> core::memory_allocator::RulePartialSocketConfigured::getSetOfNewDimmsOnSocket(
		NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::set<std::string> newDimms;
	for (std::vector<struct device_discovery>::iterator iter = m_manageableDimms.begin();
			iter != m_manageableDimms.end(); iter ++)
	{
		if ((*iter).socket_id == socketId && deviceIsNew(iter->uid))
		{
			NVM_UID uidStr;
			uid_copy(iter->uid, uidStr);
			newDimms.insert(uidStr);
		}
	}
	return newDimms;
}

void core::memory_allocator::RulePartialSocketConfigured::validateRequestForSocket(
		const std::vector<Dimm> &requestDimms, NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::set<std::string> setOfAllDimmsOnSocket = getSetOfAllDimmsOnSocket(socketId);
	std::set<std::string> setOfRequestedDimmsOnSocket =
			getSetOfRequestedDimmsOnSocket(requestDimms, socketId);

	if (setOfAllDimmsOnSocket != setOfRequestedDimmsOnSocket)
	{
		std::set<std::string> setOfNewDimmsOnSocket = getSetOfNewDimmsOnSocket(socketId);
		if (setOfNewDimmsOnSocket != setOfRequestedDimmsOnSocket)
		{
			throw core::NvmExceptionBadRequestDoesntContainRequiredDimms();
		}
	}
}

// The user can configure either the entire socket or all of the new dimms on the socket
// Anything else is a bad request
void core::memory_allocator::RulePartialSocketConfigured::verify(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::list<NVM_UINT16> socketList = getRequestedSockets(request.dimms);
	for (std::list<NVM_UINT16>::iterator iter = socketList.begin();
			iter != socketList.end(); iter++)
	{
		validateRequestForSocket(request.dimms, *iter);
	}
}
