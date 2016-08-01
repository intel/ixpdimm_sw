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
 * Lay out the reserve dimm in memory.
 */

#include "LayoutStepReserveDimm.h"

#include <utility.h>
#include <LogEnterExit.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

core::memory_allocator::LayoutStepReserveDimm::LayoutStepReserveDimm()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepReserveDimm::~LayoutStepReserveDimm()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepReserveDimm::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.reserveDimm)
	{
		if (request.dimms.size() == 0)
		{
			throw core::NvmExceptionBadRequestNoDimms();
		}
		else if (request.dimms.size() == 1)
		{
			setReserveDimmForStorage(request.dimms[0], layout);
		}
		else
		{
			std::map<NVM_UINT16, std::vector<Dimm> > socketDimmListMap;
			for (std::vector<struct Dimm>::const_iterator dimmIter = request.dimms.begin();
						dimmIter != request.dimms.end(); dimmIter++)
			{
				socketDimmListMap[dimmIter->socket].push_back(*dimmIter);
			}

			if (!loneDimmOnIMCIsSelectedAsReserve(socketDimmListMap, layout))
			{
				if (!unpartneredDimmIsSelectedAsReserve(socketDimmListMap, layout))
				{
					if (!oddlySizedDimmIsSelectedAsReserve(socketDimmListMap, layout))
					{
						setReserveDimmForStorage(request.dimms[0], layout);
					}
				}
			}
		}
	}
}

// lone dimm on an iMC while other iMC(s) are fully populated
bool core::memory_allocator::LayoutStepReserveDimm::loneDimmOnIMCIsSelectedAsReserve(
		const std::map<NVM_UINT16, std::vector<Dimm> > &socketDimmListMap,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool found = false;
	std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator socketIter = socketDimmListMap.begin();
	for (; socketIter != socketDimmListMap.end() && !found; socketIter++)
	{
		// Key, value = memoryControllerId, vector of dimms assoc' with the controllerId
		std::map<NVM_UINT16, std::vector<Dimm> > iMCDimmListMap;
		for (std::vector<struct Dimm>::const_iterator dimmIter = socketIter->second.begin();
				dimmIter != socketIter->second.end(); dimmIter++)
		{
			iMCDimmListMap[dimmIter->memoryController].push_back(*dimmIter);
		}

		std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator imcIter = iMCDimmListMap.begin();
		for(; imcIter != iMCDimmListMap.end(); imcIter++)
		{
			if (imcIter->second.size() == 1)
			{
				found = true;
				setReserveDimmForStorage(imcIter->second[0], layout);
				break;
			}
		}
	}

	return found;
}

// dimm without a partner on the other iMC
bool core::memory_allocator::LayoutStepReserveDimm::unpartneredDimmIsSelectedAsReserve(
		const std::map<NVM_UINT16, std::vector<Dimm> > &socketDimmListMap,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool found = false;
	std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator socketIter = socketDimmListMap.begin();
	for (; socketIter != socketDimmListMap.end() && !found; socketIter++)
	{
		// Key, value = memoryChannelId, vector of dimms assoc' with the channelId
		std::map<NVM_UINT16, std::vector<Dimm> > channelDimmListMap;
		for (std::vector<struct Dimm>::const_iterator dimmIter = socketIter->second.begin();
				dimmIter != socketIter->second.end(); dimmIter++)
		{
			NVM_UINT16 channelId = dimmIter->channel % core::memory_allocator::CHANNELS_PER_IMC;
			channelDimmListMap[channelId].push_back(*dimmIter);
		}


		std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator iter = channelDimmListMap.begin();
		for(; iter != channelDimmListMap.end(); iter++)
		{
			if (iter->second.size() == 1)
			{
				found = true;
				setReserveDimmForStorage(iter->second[0], layout);
				break;
			}
		}
	}

	return found;
}


bool core::memory_allocator::LayoutStepReserveDimm::oddlySizedDimmIsSelectedAsReserve(
		const std::map<NVM_UINT16, std::vector<Dimm> > &socketDimmListMap,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool found = false;
	std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator socketIter = socketDimmListMap.begin();
	for (; socketIter != socketDimmListMap.end() && !found; socketIter++)
	{
		// Key, value = capacity, vector of dimms that have the specific capacity
		std::map<NVM_UINT64,  std::vector<Dimm> > capacityDimmListMap;
		for (std::vector<struct Dimm>::const_iterator dimmIter = socketIter->second.begin();
				dimmIter != socketIter->second.end(); dimmIter++)
		{
			NVM_UINT64 capacity = dimmIter->capacity;
			capacityDimmListMap[capacity].push_back(*dimmIter);
		}

		std::map<NVM_UINT64, std::vector<Dimm> >::const_iterator iter = capacityDimmListMap.begin();
		for(; iter != capacityDimmListMap.end(); iter++)
		{
			// pick the odd one out
			if (iter->second.size() == 1)
			{
				found = true;
				setReserveDimmForStorage(iter->second[0], layout);
				break;
			}
		}
	}

	return found;
}

void core::memory_allocator::LayoutStepReserveDimm::setReserveDimmForStorage(struct Dimm reserveDimm,
		MemoryAllocationLayout& layout)
{
	layout.storageCapacity = reserveDimm.capacity / BYTES_PER_GB;
	layout.goals[reserveDimm.uid].memory_size = 0;
	layout.goals[reserveDimm.uid].app_direct_count = 0;
	layout.reservedimmUid = reserveDimm.uid;
}
