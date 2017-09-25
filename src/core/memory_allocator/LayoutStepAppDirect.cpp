/*
 * Copyright (c) 2016, Intel Corporation
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
 * Lay out the app direct extent.
 */

#include "LayoutStepAppDirect.h"

#include <LogEnterExit.h>
#include <utility.h>
#include <core/Helper.h>
#include <core/exceptions/NvmExceptionBadRequest.h>
#include "InterleaveableDimmSetBuilder.h"

core::memory_allocator::LayoutStepAppDirect::LayoutStepAppDirect(
		MemoryAllocationUtil &util) :
		m_memAllocUtil(util),
		m_nextInterleaveId(0)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepAppDirect::~LayoutStepAppDirect()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepAppDirect::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.getAppDirectCapacityGiB() > 0)
	{
		initNextInterleaveId(layout);

		layoutExtent(request, layout);
		addExtentCapacityToLayout(layout);

		checkTotalExtentCapacityAllocated(request, layout);
	}
}

void core::memory_allocator::LayoutStepAppDirect::initNextInterleaveId(
		const MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_nextInterleaveId = m_memAllocUtil.getNextAvailableInterleaveSetId(layout);
}

void core::memory_allocator::LayoutStepAppDirect::layoutExtent(
		const MemoryAllocationRequest& request, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> dimms = request.getNonReservedDimms();
	if (requestExtentIsInterleaved(request))
	{
		layoutInterleavedExtentOnRequestedDimms(dimms, layout);
	}

	layoutUnallocatedCapacityWithNonInterleaved(dimms, layout);
}

bool core::memory_allocator::LayoutStepAppDirect::requestExtentIsInterleaved(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return !request.getAppDirectExtent().byOne;
}

void core::memory_allocator::LayoutStepAppDirect::layoutUnallocatedCapacityWithNonInterleaved(
		const std::vector<Dimm>& dimms, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<Dimm>::const_iterator dimm = dimms.begin();
			dimm != dimms.end(); dimm++)
	{
		config_goal &goal = layout.goals[dimm->uid];

		NVM_UINT64 mappableDimmCapacity = USABLE_CAPACITY_BYTES(dimm->capacityBytes);
		NVM_UINT64 remainingBytes = getDimmUnallocatedBytes(mappableDimmCapacity, goal);
		if (remainingBytes > 0)
		{
			std::vector<Dimm> singleDimm;
			singleDimm.push_back(*dimm);
			layoutInterleaveSet(singleDimm, remainingBytes, layout);
		}
	}
}

void core::memory_allocator::LayoutStepAppDirect::layoutInterleavedExtentOnRequestedDimms(
		const std::vector<Dimm> &dimms, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT16, std::vector<Dimm> > sortedBySocket = getDimmsSortedBySocket(dimms);

	for (std::map<NVM_UINT16, std::vector<Dimm> >::const_iterator socket = sortedBySocket.begin();
			socket != sortedBySocket.end(); socket++)
	{
		const std::vector<Dimm> &socketDimms = socket->second;
		layoutInterleavedExtentOnSocket(socketDimms, layout);
	}
}

std::map<NVM_UINT16, std::vector<core::memory_allocator::Dimm> >
core::memory_allocator::LayoutStepAppDirect::getDimmsSortedBySocket(const std::vector<Dimm> &dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT16, std::vector<Dimm> > sortedBySocket;
	for (std::vector<Dimm>::const_iterator dimm = dimms.begin();
			dimm != dimms.end(); dimm++)
	{
		sortedBySocket[dimm->socket].push_back(*dimm);
	}

	return sortedBySocket;
}

void core::memory_allocator::LayoutStepAppDirect::layoutInterleavedExtentOnSocket(
		const std::vector<Dimm>& socketDimms,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> remainingDimms = socketDimms;
	removeUnavailableDimmsFromList(layout, remainingDimms);
	while (!remainingDimms.empty())
	{
		std::vector<Dimm> interleaveDimms = getLargestSetOfInterleavableDimms(
				remainingDimms);
		NVM_UINT64 remainingCapacity = getRemainingBytesFromDimms(interleaveDimms, layout);

		// no remaining capacity left on any dimms
		if (remainingCapacity == 0)
		{
			throw core::NvmExceptionBadRequestSize();
		}

		std::vector<Dimm> includedDimms;
		NVM_UINT64 bytesPerDimm = getLargestPerDimmSymmetricalBytes(interleaveDimms,
				layout.goals, remainingCapacity, includedDimms);
		layoutInterleaveSet(interleaveDimms, bytesPerDimm, layout);

		removeDimmsFromList(interleaveDimms, remainingDimms);
	}
}


void core::memory_allocator::LayoutStepAppDirect::removeUnavailableDimmsFromList(
		MemoryAllocationLayout& layout, std::vector<Dimm>& dimmList)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm>::iterator dimm = dimmList.begin();
	while (dimm != dimmList.end())
	{
		if (getDimmUnallocatedGiBAlignedBytes(dimm->capacityBytes,
				layout.goals[dimm->uid]) == 0)
		{
			dimm = dimmList.erase(dimm);
		}
		else
		{
			dimm++;
		}
	}
}

std::vector<core::memory_allocator::Dimm>
core::memory_allocator::LayoutStepAppDirect::getLargestSetOfInterleavableDimms(
		const std::vector<Dimm>& dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	InterleaveableDimmSetBuilder builder;
	builder.setDimms(dimms);

	return builder.getLargestSetOfInterleavableDimms();
}

void core::memory_allocator::LayoutStepAppDirect::removeDimmsFromList(
		const std::vector<Dimm>& dimmsToRemove, std::vector<Dimm>& dimmList)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<Dimm>::const_iterator dimmToRemove = dimmsToRemove.begin();
			dimmToRemove != dimmsToRemove.end(); dimmToRemove++)
	{
		for (std::vector<Dimm>::iterator dimm = dimmList.begin();
				dimm != dimmList.end(); dimm++)
		{
			if (dimm->uid == dimmToRemove->uid)
			{
				dimmList.erase(dimm);
				break;
			}
		}
	}
}

void core::memory_allocator::LayoutStepAppDirect::layoutInterleaveSet(
		const std::vector<Dimm>& interleavedDimms,
		const NVM_UINT64 bytesPerDimm,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<Dimm>::const_iterator dimm = interleavedDimms.begin();
			dimm != interleavedDimms.end(); dimm++)
	{
		config_goal &goal = layout.goals[dimm->uid];

		updateGoalWithInterleaveSet(goal, bytesPerDimm, interleavedDimms);
	}

	m_nextInterleaveId++;
}

void core::memory_allocator::LayoutStepAppDirect::updateGoalWithInterleaveSet(config_goal& goal,
		const NVM_UINT64 bytesPerDimm,
		const std::vector<Dimm> &interleavedDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	goal.app_direct_count++;
	if (goal.app_direct_count == 1)
	{
		updateGoalParametersWithInterleaveSet(goal.app_direct_1_size,
				goal.app_direct_1_set_id, goal.app_direct_1_settings,
				bytesPerDimm, interleavedDimms);
	}
	else
	{
		updateGoalParametersWithInterleaveSet(goal.app_direct_2_size,
				goal.app_direct_2_set_id, goal.app_direct_2_settings,
				bytesPerDimm, interleavedDimms);
	}
}

void core::memory_allocator::LayoutStepAppDirect::updateGoalParametersWithInterleaveSet(
		NVM_UINT64& goalSize, NVM_UINT16& goalSetId, app_direct_attributes& goalSettings,
		const NVM_UINT64 bytesPerDimm, const std::vector<Dimm>& interleavedDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	goalSize = bytesToConfigGoalSize(bytesPerDimm);
	goalSetId = m_nextInterleaveId;

	for (size_t i = 0; i < interleavedDimms.size(); i++)
	{
		core::Helper::stringToUid(interleavedDimms[i].uid, goalSettings.dimms[i]);
	}

	interleave_ways ways = getInterleaveWaysFromNumDimms(interleavedDimms.size());
	goalSettings.interleave = m_memAllocUtil.getRecommendedInterleaveFormatForWays(ways);
}

interleave_ways core::memory_allocator::LayoutStepAppDirect::getInterleaveWaysFromNumDimms(
		const size_t numDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (interleave_ways)numDimms;
}

void core::memory_allocator::LayoutStepAppDirect::addExtentCapacityToLayout(
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 capacityLaidOut = getExtentCapacityFromLayout(layout);
	if (capacityLaidOut > 0)
	{
		layout.appDirectCapacity += capacityLaidOut;
	}
}

NVM_UINT64 core::memory_allocator::LayoutStepAppDirect::getExtentCapacityFromLayout(
		const MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 extentCapacityGiB = 0;

	for (std::map<std::string, config_goal>::const_iterator goalPair = layout.goals.begin();
			goalPair != layout.goals.end(); goalPair++)
	{
		const config_goal &goal = goalPair->second;

		extentCapacityGiB += configGoalSizeToGiB(goal.app_direct_1_size);
		extentCapacityGiB += configGoalSizeToGiB(goal.app_direct_2_size);
	}

	return extentCapacityGiB;
}

void core::memory_allocator::LayoutStepAppDirect::checkTotalExtentCapacityAllocated(
		const MemoryAllocationRequest& request, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!allRequestedCapacityAllocated(request, layout))
	{
		throw core::NvmExceptionBadRequestSize();
	}
}

bool core::memory_allocator::LayoutStepAppDirect::allRequestedCapacityAllocated(
		const MemoryAllocationRequest& request, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 requestedCapacity = request.getAppDirectCapacityGiB();
	NVM_UINT64 allocatedCapacity = getExtentCapacityFromLayout(layout);

	return (allocatedCapacity >= requestedCapacity);
}
