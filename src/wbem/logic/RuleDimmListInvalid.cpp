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
 * Rule that checks a MemoryAllocationRequest to make sure all the DIMMs are
 * valid and belong in the socket
 */

#include <exception/NvmExceptionBadRequest.h>
#include <LogEnterExit.h>
#include <nvm_types.h>
#include <guid/guid.h>
#include "RuleDimmListInvalid.h"
#include <list>
#include "MemoryAllocator.h"

wbem::logic::RuleDimmListInvalid::RuleDimmListInvalid(
		const std::vector<struct device_discovery> manageableDimms) :
		m_manageableDimms(manageableDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

wbem::logic::RuleDimmListInvalid::~RuleDimmListInvalid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void wbem::logic::RuleDimmListInvalid::checkifDimmsInRequestAreUnique(
		const std::vector<Dimm> &dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::list<std::string> dimmList;
	std::vector<Dimm>::const_iterator requestIter = dimms.begin();
	for (; requestIter != dimms.end(); requestIter++)
	{
		dimmList.push_back(requestIter->guid);
	}

	std::list<std::string> uniqueDimmList(dimmList.begin(), dimmList.end());
	uniqueDimmList.unique();

	if (uniqueDimmList.size() != dimmList.size())
	{
		throw exception::NvmExceptionBadDimmList();
	}
}

void wbem::logic::RuleDimmListInvalid::checkIfSocketIdsMatch(
		std::vector<Dimm>::const_iterator requestIter,
		std::vector<struct device_discovery>::const_iterator manageableDimmIter)
{
	if (requestIter->socket != manageableDimmIter->socket_id)
	{
		throw exception::NvmExceptionBadDimmList();
	}
}

void wbem::logic::RuleDimmListInvalid::checkIfMemControllersMatch(
		std::vector<Dimm>::const_iterator requestIter,
		std::vector<struct device_discovery>::const_iterator manageableDimmIter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (requestIter->memoryController
			!= manageableDimmIter->memory_controller_id)
	{
		throw exception::NvmExceptionBadDimmList();
	}
}

void wbem::logic::RuleDimmListInvalid::checkIfDimmCapacitiesMatch(
		std::vector<Dimm>::const_iterator requestIter,
		std::vector<struct device_discovery>::const_iterator manageableDimmIter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (requestIter->capacity != manageableDimmIter->capacity)
	{
		throw exception::NvmExceptionBadDimmList();
	}
}

void wbem::logic::RuleDimmListInvalid::checkIfMemChannelsMatch(
		std::vector<Dimm>::const_iterator requestIter,
		std::vector<struct device_discovery>::const_iterator manageableDimmIter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (requestIter->channel != manageableDimmIter->device_handle.parts.mem_channel_id)
	{
		throw exception::NvmExceptionBadDimmList();
	}
}

void wbem::logic::RuleDimmListInvalid::checkIfDimmListIsValid(
		const std::vector<Dimm> &dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm>::const_iterator requestIter = dimms.begin();
	for (; requestIter != dimms.end(); requestIter++)
	{
		bool guidExists = false;

		NVM_GUID requestedDimmGuid;
		str_to_guid(requestIter->guid.c_str(), requestedDimmGuid);

		for (std::vector<struct device_discovery>::const_iterator manageableDimmIter =
				m_manageableDimms.begin(); manageableDimmIter != m_manageableDimms.end();
				manageableDimmIter++)
		{
			if (guid_cmp(requestedDimmGuid, manageableDimmIter->guid))
			{
				guidExists = true;
				checkIfSocketIdsMatch(requestIter, manageableDimmIter);
				checkIfMemControllersMatch(requestIter, manageableDimmIter);
				checkIfDimmCapacitiesMatch(requestIter, manageableDimmIter);
			}
		}
		if (!guidExists)
		{
			throw exception::NvmExceptionInvalidDimm();
		}
	}
}

void wbem::logic::RuleDimmListInvalid::verify(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkifDimmsInRequestAreUnique(request.dimms);
	checkIfDimmListIsValid(request.dimms);
}
