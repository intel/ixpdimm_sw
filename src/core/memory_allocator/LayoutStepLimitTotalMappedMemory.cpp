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
			shrinkLayoutGoals(request, layout);
		}
	}

	if (needLayoutChange)
	{
		shrinkLayoutCapacities(request, layout);
		layout.warnings.push_back(LAYOUT_WARNING_SKU_MAPPED_MEMORY_LIMITED);
	}
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkLayoutGoals(
		const MemoryAllocationRequest& request,	MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	shrinkAppDirect2(request, layout);

	shrinkAppDirect1(request, layout);

	shrinkMemory(request, layout);

	shrinkReservedDimm(request, layout);
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkAppDirect2(
		const MemoryAllocationRequest& request,	MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_mappedCapacityExceedsLimit > 0)
	{
		std::vector<core::memory_allocator::Dimm> ad2Dimms = getDimmsByType(layout, TYPE_APPDIRECT2);
		if (!ad2Dimms.empty())
		{
			// If limit exceeds AD2, eliminate all AD2, else shrink evenly on all AD2 DIMMs
			NVM_UINT64 totalAD2Capacity = getTotalAD2Capacity(ad2Dimms, layout);
			if (m_mappedCapacityExceedsLimit >= totalAD2Capacity)
			{
				killAllCapacityByType(ad2Dimms, layout, TYPE_APPDIRECT2);

				m_mappedCapacityExceedsLimit =
					m_mappedCapacityExceedsLimit - totalAD2Capacity;
			}
			else
			{
				NVM_UINT64 reduceBy = reduceEachDimmBy(m_mappedCapacityExceedsLimit, ad2Dimms.size());

				for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
						ad2Dimms.begin(); dimm != ad2Dimms.end(); dimm++)
				{
					if (m_mappedCapacityExceedsLimit > 0)
					{
						config_goal& goal = layout.goals[dimm->uid];

						shrinkSizePerDimm(reduceBy, goal.app_direct_2_size);

						killADIfSizeIsZero(goal, TYPE_APPDIRECT2);
					}
				}
			}
		}
	}
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkAppDirect1(
		const MemoryAllocationRequest& request,	MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_mappedCapacityExceedsLimit > 0)
	{
		std::vector<core::memory_allocator::Dimm> ad1Dimms = getDimmsByType(layout, TYPE_APPDIRECT1);
		if (!ad1Dimms.empty())
		{
			// If limit exceeds AD1, eliminate all AD1, else shrink evenly on all AD1 DIMMs
			NVM_UINT64 totalAD1Capacity = getTotalAD1Capacity(ad1Dimms, layout);
			if (m_mappedCapacityExceedsLimit >= totalAD1Capacity)
			{
				killAllCapacityByType(ad1Dimms, layout, TYPE_APPDIRECT1);

				m_mappedCapacityExceedsLimit =
					m_mappedCapacityExceedsLimit - totalAD1Capacity;
			}
			else
			{
				NVM_UINT64 reduceBy = reduceEachDimmBy(m_mappedCapacityExceedsLimit, ad1Dimms.size());

				for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
						ad1Dimms.begin(); dimm != ad1Dimms.end(); dimm++)
				{
					if (m_mappedCapacityExceedsLimit > 0)
					{
						config_goal& goal = layout.goals[dimm->uid];

						shrinkSizePerDimm(reduceBy, goal.app_direct_1_size);

						killADIfSizeIsZero(goal, TYPE_APPDIRECT1);
					}
				}
			}
		}
	}
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkMemory(
		const MemoryAllocationRequest& request,	MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_mappedCapacityExceedsLimit > 0)
	{
		std::vector<core::memory_allocator::Dimm> memDimms = getDimmsByType(layout, TYPE_2LM);
		if (!memDimms.empty())
		{
			// If limit exceeds 2LM, eliminate all 2LM, else shrink evenly on all 2LM DIMMs
			NVM_UINT64 total2LMapacity = getTotal2LMCapacity(memDimms, layout);
			if (m_mappedCapacityExceedsLimit >= total2LMapacity)
			{
				killAllCapacityByType(memDimms, layout, TYPE_2LM);

				m_mappedCapacityExceedsLimit =
					m_mappedCapacityExceedsLimit - total2LMapacity;
			}
			else
			{
				NVM_UINT64 reduceBy = reduceEachDimmBy(m_mappedCapacityExceedsLimit, memDimms.size());

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

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkReservedDimm(
		const MemoryAllocationRequest& request,	MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_mappedCapacityExceedsLimit > 0)
	{
		// we'll only have one reserved DIMM per socket
		std::vector<core::memory_allocator::Dimm> reservedDimms =
				getDimmsByType(layout, TYPE_RESERVED_APPDIRECT_BYONE);

		for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
				reservedDimms.begin(); dimm != reservedDimms.end(); dimm++)
		{
			config_goal& goal = layout.goals[dimm->uid];
			if (m_mappedCapacityExceedsLimit <= goal.app_direct_1_size)
			{
				goal.app_direct_1_size -= m_mappedCapacityExceedsLimit;
				m_mappedCapacityExceedsLimit = 0;

				killADIfSizeIsZero(goal, TYPE_RESERVED_APPDIRECT_BYONE);
			}
		}
	}
}

// TODO: Get from PCAT for each socket (US20271)
NVM_UINT64 core::memory_allocator::LayoutStepLimitTotalMappedMemory::getLimit(const MemoryAllocationRequest& request)
{
	return request.getSocketLimit();
}

bool core::memory_allocator::LayoutStepLimitTotalMappedMemory::mappedSizeExceedsLimit()
{
	return m_totalMappedSize > m_limit;
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initializeExceedsLimit()
{
	m_mappedCapacityExceedsLimit = m_totalMappedSize - m_limit;
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::initializeDimmsSortedBySocket(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> dimms = request.getDimms();

	for (std::vector<Dimm>::const_iterator dimm = dimms.begin(); dimm != dimms.end(); dimm++)
	{
		m_dimmsSortedBySocket[dimm->socket].push_back(*dimm);
	}
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


NVM_UINT64 core::memory_allocator::LayoutStepLimitTotalMappedMemory::getTotalAD2Capacity(
		std::vector<core::memory_allocator::Dimm> dimms,
		MemoryAllocationLayout& layout)
{
	NVM_UINT64 totalAD2Capacity = 0;

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		totalAD2Capacity += layout.goals[dimm->uid].app_direct_2_size;
	}

	return totalAD2Capacity;
}

NVM_UINT64 core::memory_allocator::LayoutStepLimitTotalMappedMemory::getTotalAD1Capacity(
		std::vector<core::memory_allocator::Dimm> dimms,
		MemoryAllocationLayout& layout)
{
	NVM_UINT64 totalAD1Capacity = 0;

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		totalAD1Capacity += layout.goals[dimm->uid].app_direct_1_size;
	}

	return totalAD1Capacity;
}

NVM_UINT64 core::memory_allocator::LayoutStepLimitTotalMappedMemory::getTotal2LMCapacity(
		std::vector<core::memory_allocator::Dimm> dimms,
		MemoryAllocationLayout& layout)
{
	NVM_UINT64 total2LMCapacity = 0;

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		total2LMCapacity += layout.goals[dimm->uid].memory_size;
	}

	return total2LMCapacity;
}

std::vector<core::memory_allocator::Dimm> core::memory_allocator::LayoutStepLimitTotalMappedMemory::getDimmsByType(
		MemoryAllocationLayout& layout, NVM_UINT16 capacityType)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<core::memory_allocator::Dimm> dimms;

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			m_socketDimms.begin(); dimm != m_socketDimms.end(); dimm++)
	{
		switch(capacityType)
		{
		case TYPE_APPDIRECT1:
			if (dimmHasAppDirect1(dimm, layout))
			{
				dimms.push_back(*dimm);
			}
			break;
		case TYPE_APPDIRECT2:
			if (dimmHasAppDirect2(dimm, layout))
			{
				dimms.push_back(*dimm);
			}
			break;
		case TYPE_2LM:
			if (dimmHas2LM(dimm, layout))
			{
				dimms.push_back(*dimm);
			}
			break;
		case TYPE_RESERVED_APPDIRECT_BYONE:
			if (dimmisReservedAppDirectByOne(dimm, layout))
			{
				dimms.push_back(*dimm);
			}
			break;
		default:
			break;
		}
	}

	return dimms;
}

bool core::memory_allocator::LayoutStepLimitTotalMappedMemory::dimmHasAppDirect1(
		std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return layout.reservedimmUid != dimm->uid &&
			layout.goals[dimm->uid].app_direct_1_size > 0;
}

bool core::memory_allocator::LayoutStepLimitTotalMappedMemory::dimmHasAppDirect2(
		std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return layout.reservedimmUid != dimm->uid &&
			layout.goals[dimm->uid].app_direct_2_settings.interleave.ways == INTERLEAVE_WAYS_1 &&
			layout.goals[dimm->uid].app_direct_2_size > 0;
}

bool core::memory_allocator::LayoutStepLimitTotalMappedMemory::dimmHas2LM(
		std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return layout.goals[dimm->uid].memory_size > 0;
}

bool core::memory_allocator::LayoutStepLimitTotalMappedMemory::dimmisReservedAppDirectByOne(
		std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return layout.reservedimmUid == dimm->uid &&
			layout.goals[dimm->uid].app_direct_1_settings.interleave.ways == INTERLEAVE_WAYS_1 &&
			layout.goals[dimm->uid].app_direct_1_size > 0;
}

NVM_UINT64 core::memory_allocator::LayoutStepLimitTotalMappedMemory::reduceEachDimmBy(
		NVM_UINT64 mappedMemoryCapacityExceedsLimit, int numDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (NVM_UINT64) (ceil((double) (mappedMemoryCapacityExceedsLimit) / (double) (numDimms)));
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::killADIfSizeIsZero(
		config_goal& goal, NVM_UINT16 type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if ((type == TYPE_APPDIRECT1 || type == TYPE_RESERVED_APPDIRECT_BYONE ) &&
		goal.app_direct_1_size == 0)
	{
		struct app_direct_attributes empty_settings;
		memset(&empty_settings, 0, sizeof(struct app_direct_attributes));
		goal.app_direct_1_settings = empty_settings;
		goal.app_direct_count--;
	}

	if (type == TYPE_APPDIRECT2 && goal.app_direct_2_size == 0)
	{
		struct app_direct_attributes empty_settings;
		memset(&empty_settings, 0, sizeof(struct app_direct_attributes));
		goal.app_direct_2_settings = empty_settings;
		goal.app_direct_count--;
	}
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::killAllCapacityByType(
		std::vector<core::memory_allocator::Dimm> dimms,
		MemoryAllocationLayout& layout,
		NVM_UINT16 type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		config_goal& goal = layout.goals[dimm->uid];

		if (type == TYPE_APPDIRECT1 || type == TYPE_RESERVED_APPDIRECT_BYONE)
		{
			goal.app_direct_1_size = 0;
			killADIfSizeIsZero(goal, type);
		}
		else if (type == TYPE_APPDIRECT2)
		{
			goal.app_direct_2_size = 0;
			killADIfSizeIsZero(goal, type);
		}
		else if (type == TYPE_2LM)
		{
			goal.memory_size = 0;
		}
	}
}

void core::memory_allocator::LayoutStepLimitTotalMappedMemory::shrinkSizePerDimm(
		NVM_UINT64 reduceBy, NVM_UINT64 &size)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// reduce each DIMM evenly if possible
	if (size >= reduceBy && m_mappedCapacityExceedsLimit >= reduceBy)
	{
		size -=reduceBy;
		m_mappedCapacityExceedsLimit -= reduceBy;
	}
	// reduce little more than needed
	else if (size >= reduceBy && m_mappedCapacityExceedsLimit < reduceBy)
	{
		size -=reduceBy;
		m_mappedCapacityExceedsLimit = 0;
	}
	// reduce more than what's available per DIMM
	else
	{
		m_mappedCapacityExceedsLimit -= size;
		size = 0;
	}
}
