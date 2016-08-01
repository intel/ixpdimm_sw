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

#include "RuleDimmListInvalid.h"

#include <LogEnterExit.h>
#include <nvm_types.h>
#include <uid/uid.h>
#include <list>
#include <core/exceptions/NvmExceptionBadRequest.h>

core::memory_allocator::RuleDimmListInvalid::RuleDimmListInvalid(
		const std::vector<struct device_discovery> manageableDimms) :
		m_manageableDimms(manageableDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::RuleDimmListInvalid::~RuleDimmListInvalid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::RuleDimmListInvalid::checkifDimmsInRequestAreUnique(
		const std::vector<Dimm> &dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::list<std::string> dimmList;
	std::vector<Dimm>::const_iterator requestIter = dimms.begin();
	for (; requestIter != dimms.end(); requestIter++)
	{
		dimmList.push_back(requestIter->uid);
	}

	std::list<std::string> uniqueDimmList(dimmList.begin(), dimmList.end());
	uniqueDimmList.unique();

	if (uniqueDimmList.size() != dimmList.size())
	{
		throw core::NvmExceptionBadDimmList();
	}
}

void core::memory_allocator::RuleDimmListInvalid::checkIfSocketIdsMatch(
		std::vector<Dimm>::const_iterator requestIter,
		std::vector<struct device_discovery>::const_iterator manageableDimmIter)
{
	if (requestIter->socket != manageableDimmIter->socket_id)
	{
		throw core::NvmExceptionBadDimmList();
	}
}

void core::memory_allocator::RuleDimmListInvalid::checkIfMemControllersMatch(
		std::vector<Dimm>::const_iterator requestIter,
		std::vector<struct device_discovery>::const_iterator manageableDimmIter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (requestIter->memoryController
			!= manageableDimmIter->memory_controller_id)
	{
		throw core::NvmExceptionBadDimmList();
	}
}

void core::memory_allocator::RuleDimmListInvalid::checkIfDimmCapacitiesMatch(
		std::vector<Dimm>::const_iterator requestIter,
		std::vector<struct device_discovery>::const_iterator manageableDimmIter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (requestIter->capacity != manageableDimmIter->capacity)
	{
		throw core::NvmExceptionBadDimmList();
	}
}

void core::memory_allocator::RuleDimmListInvalid::checkIfMemChannelsMatch(
		std::vector<Dimm>::const_iterator requestIter,
		std::vector<struct device_discovery>::const_iterator manageableDimmIter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (requestIter->channel != manageableDimmIter->device_handle.parts.mem_channel_id)
	{
		throw core::NvmExceptionBadDimmList();
	}
}

void core::memory_allocator::RuleDimmListInvalid::checkIfDimmListIsValid(
		const std::vector<Dimm> &dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm>::const_iterator requestIter = dimms.begin();
	for (; requestIter != dimms.end(); requestIter++)
	{
		bool uidExists = false;

		NVM_UID requesteddimmUid;
		uid_copy(requestIter->uid.c_str(), requesteddimmUid);

		for (std::vector<struct device_discovery>::const_iterator manageableDimmIter =
				m_manageableDimms.begin(); manageableDimmIter != m_manageableDimms.end();
				manageableDimmIter++)
		{
			if (uid_cmp(requesteddimmUid, manageableDimmIter->uid))
			{
				uidExists = true;
				checkIfSocketIdsMatch(requestIter, manageableDimmIter);
				checkIfMemControllersMatch(requestIter, manageableDimmIter);
				checkIfDimmCapacitiesMatch(requestIter, manageableDimmIter);
			}
		}
		if (!uidExists)
		{
			throw core::NvmExceptionInvalidDimm();
		}
	}
}

void core::memory_allocator::RuleDimmListInvalid::verify(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkifDimmsInRequestAreUnique(request.dimms);
	checkIfDimmListIsValid(request.dimms);
}
