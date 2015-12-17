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
 * Rule that checks that the user is not configuring only part of a socket
 */

#include "RulePartialSocketConfigured.h"
#include <exception/NvmExceptionLibError.h>
#include <exception/NvmExceptionBadRequest.h>
#include <LogEnterExit.h>
#include <guid/guid.h>
#include <nvm_management.h>
#include <lib_interface/NvmApi.h>
#include <mem_config/MemoryConfigurationServiceFactory.h>

wbem::logic::RulePartialSocketConfigured::RulePartialSocketConfigured(
		const std::vector<struct device_discovery> manageableDevices) :
		m_manageableDimms(manageableDevices)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

wbem::logic::RulePartialSocketConfigured::~RulePartialSocketConfigured()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

std::list<NVM_UINT16> wbem::logic::RulePartialSocketConfigured::getRequestedSockets(std::vector<Dimm> dimms)
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

std::set<std::string> wbem::logic::RulePartialSocketConfigured::getSetOfAllDimmsOnSocket(
		NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::set<std::string> dimmSet;
	for (std::vector<struct device_discovery>::const_iterator iter = m_manageableDimms.begin();
			iter != m_manageableDimms.end(); iter++)
	{
		if ((*iter).socket_id == socketId)
		{
			NVM_GUID_STR guidStr;
			guid_to_str((*iter).guid, guidStr);
			dimmSet.insert(guidStr);
		}
	}

	return dimmSet;
}

std::set<std::string> wbem::logic::RulePartialSocketConfigured::getSetOfRequestedDimmsOnSocket(
		const std::vector<Dimm> &requestedDimms, NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::set<std::string> dimmSet;
	for (std::vector<Dimm>::const_iterator iter = requestedDimms.begin();
			iter != requestedDimms.end(); iter++)
	{
		if ((*iter).socket == socketId)
		{
			dimmSet.insert((*iter).guid);
		}
	}

	return dimmSet;
}

bool wbem::logic::RulePartialSocketConfigured::deviceIsNew(NVM_GUID guid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool deviceIsNew = false;
	lib_interface::NvmApi *pApi = lib_interface::NvmApi::getApi();

	struct device_status status;
	memset(&status, 0, sizeof (struct device_status));
	int rc = pApi->getDeviceStatus(guid, &status);
	if (rc != NVM_SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to retrieve device status rc = ", rc);
		throw exception::NvmExceptionLibError(rc);
	}

	if (status.is_new)
	{
		deviceIsNew = true;
	}

	return deviceIsNew;
}

std::set<std::string> wbem::logic::RulePartialSocketConfigured::getSetOfNewDimmsOnSocket(
		NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::set<std::string> newDimms;
	for (std::vector<struct device_discovery>::iterator iter = m_manageableDimms.begin();
			iter != m_manageableDimms.end(); iter ++)
	{
		if ((*iter).socket_id == socketId && deviceIsNew(iter->guid))
		{
			NVM_GUID_STR guidStr;
			guid_to_str(iter->guid, guidStr);
			newDimms.insert(guidStr);
		}
	}
	return newDimms;
}

void wbem::logic::RulePartialSocketConfigured::validateRequestForSocket(
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
			throw exception::NvmExceptionBadRequestDoesntContainRequiredDimms();
		}
	}
}

// The user can configure either the entire socket or all of the new dimms on the socket
// Anything else is a bad request
void wbem::logic::RulePartialSocketConfigured::verify(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::list<NVM_UINT16> socketList = getRequestedSockets(request.dimms);
	for (std::list<NVM_UINT16>::iterator iter = socketList.begin();
			iter != socketList.end(); iter++)
	{
		validateRequestForSocket(request.dimms, *iter);
	}
}
