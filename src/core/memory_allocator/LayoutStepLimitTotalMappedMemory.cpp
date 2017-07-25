/*
 * Copyright (c) 2017 Intel Corporation
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
 * Reduce laid out capacity based on SKU mapped memory limit
 */

#include "LayoutStepLimitTotalMappedMemory.h"
#include <LogEnterExit.h>
#include <core/memory_allocator/LayoutStepAppDirect.h>
#include <core/memory_allocator/LayoutStepMemory.h>
#include <math.h>

core::memory_allocator::LayoutStepLimitTotalMappedMemory::LayoutStepLimitTotalMappedMemory() :
	m_limit(0), m_totalMappedSize(0), m_mappedCapacityExceedsLimit(0)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepLimitTotalMappedMemory::~LayoutStepLimitTotalMappedMemory()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::execute(
	const MemoryAllocationRequest& request, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool needLayoutChange = false;

	initializeDimmsSortedBySocket(request);

	for (std::map<NVM_UINT16, std::vector<Dimm> >::iterator socketDimms = m_dimmsSortedBySocket.begin();
			socketDimms != m_dimmsSortedBySocket.end(); socketDimms++)
	{
		initializeTotalMappedSizeVariablesPerSocket(request, layout, socketDimms->first);

		if (mappedSizeExceedsLimit())
		{
			needLayoutChange = true;
			initializeExceedsLimit();
			shrinkLayoutGoals(layout);
		}
	}

	if (needLayoutChange)
	{
		shrinkLayoutCapacities(request, layout);
		layout.warnings.push_back(LAYOUT_WARNING_SKU_MAPPED_MEMORY_LIMITED);
	}
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkLayoutGoals(
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	shrinkAppDirect2(layout);

	shrinkAppDirect1(layout);

	shrinkMemory(layout);
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkAppDirect2(
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	shrinkAD2(m_socketDimms, m_mappedCapacityExceedsLimit, layout);
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkAppDirect1(
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	shrinkAD1(m_socketDimms, m_mappedCapacityExceedsLimit, layout);
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkMemory(
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_mappedCapacityExceedsLimit > 0)
	{
		std::vector<core::memory_allocator::Dimm> memDimms = get2LMDimms(m_socketDimms, layout);
		if (!memDimms.empty())
		{
			// If limit exceeds 2LM, eliminate all 2LM, else shrink evenly on all 2LM DIMMs
			NVM_UINT64 total2LMapacity = getTotal2LMCapacity(memDimms, layout);
			if (m_mappedCapacityExceedsLimit >= total2LMapacity)
			{
				killAllCapacityByType(memDimms, layout, CAPACITY_TYPE_2LM);

				m_mappedCapacityExceedsLimit =
					m_mappedCapacityExceedsLimit - total2LMapacity;
			}
			else
			{
				NVM_UINT64 reduceBy = calculateCapacityToShrinkPerDimm(m_mappedCapacityExceedsLimit, memDimms.size());

				for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
						memDimms.begin(); dimm != memDimms.end(); dimm++)
				{
					if (m_mappedCapacityExceedsLimit > 0)
					{
						if (layout.goals.find(dimm->uid) != layout.goals.end())
						{
							config_goal& goal = layout.goals[dimm->uid];

							shrinkSizePerDimm(reduceBy, goal.memory_size);
						}
					}
				}
			}
		}
	}
}


// TODO: Get from PCAT for each socket (US20271)
NVM_UINT64 core::memory_allocator::LayoutStepLimitTotalMappedMemory::getLimit(const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return request.getSocketLimit();
}

bool core::memory_allocator::LayoutStepLimitTotalMappedMemory::mappedSizeExceedsLimit()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_totalMappedSize > m_limit;
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initializeExceedsLimit()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_mappedCapacityExceedsLimit = m_totalMappedSize - m_limit;
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initializeDimmsSortedBySocket(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_dimmsSortedBySocket =	getDimmsSortedBySocket(request);
}

// TODO: need to get m_totalMappedSize for all dimms on socket, not just on requested dimms
// Fixed by US20271
void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initializeTotalMappedSizeVariablesPerSocket(
		const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout, int socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_totalMappedSize = 0;
	m_limit = getLimit(request);

	m_socketDimms.assign(m_dimmsSortedBySocket[socketId].begin(), m_dimmsSortedBySocket[socketId].end());
	for (std::vector<core::memory_allocator::Dimm>::iterator dimm = m_socketDimms.begin();
			dimm != m_socketDimms.end(); dimm++)
	{
		if (layout.goals.find(dimm->uid) != layout.goals.end())
		{
			m_totalMappedSize += layout.goals[dimm->uid].memory_size;
			m_totalMappedSize +=
					layout.goals[dimm->uid].app_direct_1_size +
					layout.goals[dimm->uid].app_direct_2_size;
		}
	}
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkLayoutCapacities(
		const MemoryAllocationRequest& request,	MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 memoryCapacity = 0,  appDirectCapacity = 0, totalCapacity = 0;
	std::vector<core::memory_allocator::Dimm> dimms = request.getDimms();
	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		memoryCapacity += layout.goals[dimm->uid].memory_size;
		appDirectCapacity +=
			layout.goals[dimm->uid].app_direct_1_size + layout.goals[dimm->uid].app_direct_2_size;
		totalCapacity += B_TO_GiB(dimm->capacityBytes);
	}

	layout.memoryCapacity = memoryCapacity;
	layout.appDirectCapacity = appDirectCapacity;
}

NVM_UINT64 core::memory_allocator::LayoutStepLimitTotalMappedMemory::getTotal2LMCapacity(
		const std::vector<core::memory_allocator::Dimm>& dimms,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 total2LMCapacity = 0;

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		total2LMCapacity += layout.goals[dimm->uid].memory_size;
	}

	return total2LMCapacity;
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkSizePerDimm(
		NVM_UINT64 reduceBy, NVM_UINT64 &size)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	shrinkSize(m_mappedCapacityExceedsLimit, reduceBy, size);
}

std::map<NVM_UINT16, std::vector<core::memory_allocator::Dimm> >
core::memory_allocator::LayoutStepLimitTotalMappedMemory::getDimmsSortedBySocket(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT16, std::vector<Dimm> > dimmsSortedBySocket;

	std::vector<Dimm> dimms = request.getDimms();

	for (std::vector<Dimm>::const_iterator dimm = dimms.begin(); dimm != dimms.end(); dimm++)
	{
		dimmsSortedBySocket[dimm->socket].push_back(*dimm);
	}

	return dimmsSortedBySocket;
}
