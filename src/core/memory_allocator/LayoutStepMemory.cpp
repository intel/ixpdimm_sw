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
 * Lay out the Memory Mode region.
 */

#include "LayoutStepMemory.h"

#include <utility.h>
#include <LogEnterExit.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

core::memory_allocator::LayoutStepMemory::LayoutStepMemory()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepMemory::~LayoutStepMemory()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepMemory::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.getMemoryModeCapacityGiB() > 0)
	{
		try
		{
			layoutMemoryModeCapacity(request, layout);
			alignPartitionBoundary(request, layout);
			layout.memoryCapacity = B_TO_GiB(getBytesAllocatedFromLayout(layout));
		}
		catch (core::NvmExceptionBadRequestSize &)
		{
			layout.memoryCapacity = B_TO_GiB(getBytesAllocatedFromLayout(layout));
			throw core::NvmExceptionBadRequestMemorySize();
		}
	}
}

void core::memory_allocator::LayoutStepMemory::layoutMemoryModeCapacity(
		const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> dimmsToLayout = request.getNonReservedDimms();
	NVM_UINT64 bytesToAllocate = GiB_TO_B(request.getMemoryModeCapacityGiB());
	NVM_UINT64 bytesAllocated = 0;

	while (bytesAllocated < bytesToAllocate)
	{
		NVM_UINT64 remainingBytes = bytesToAllocate - bytesAllocated;
		layoutMaximumSymmetricalBytesOnDimms(remainingBytes, dimmsToLayout, layout);

		if (!newBytesWereAllocated(layout, bytesAllocated))
		{
			break;
		}
		bytesAllocated = getBytesAllocatedFromLayout(layout);
	}
}

NVM_UINT64 core::memory_allocator::LayoutStepMemory::getBytesAllocatedFromLayout(
		const MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytesAllocated = 0;
	for (std::map<std::string, config_goal>::const_iterator goalPair = layout.goals.begin();
			goalPair != layout.goals.end(); goalPair++)
	{
		bytesAllocated += configGoalSizeToBytes(goalPair->second.memory_size);
	}

	return bytesAllocated;
}

void core::memory_allocator::LayoutStepMemory::layoutMaximumSymmetricalBytesOnDimms(
		const NVM_UINT64 bytesToLayout, const std::vector<Dimm>& dimmsToLayout,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> dimmsIncluded;
	NVM_UINT64 bytesPerDimm = getLargestPerDimmSymmetricalBytes(
			dimmsToLayout, layout.goals, bytesToLayout, dimmsIncluded);

	for (std::vector<Dimm>::const_iterator dimmIter = dimmsIncluded.begin();
					dimmIter != dimmsIncluded.end(); dimmIter++)
	{
		layout.goals[dimmIter->uid].memory_size += bytesToConfigGoalSize(bytesPerDimm);
	}
}

bool core::memory_allocator::LayoutStepMemory::newBytesWereAllocated(
		const MemoryAllocationLayout& updatedLayout,
		const NVM_UINT64 previousBytesAllocated)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 updatedBytesAllocated = getBytesAllocatedFromLayout(updatedLayout);
	return (previousBytesAllocated != updatedBytesAllocated);
}

void core::memory_allocator::LayoutStepMemory::alignPartitionBoundary(
		const MemoryAllocationRequest& request, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> dimms = request.getNonReservedDimms();
	for (std::vector<Dimm>::const_iterator dimm = dimms.begin(); dimm != dimms.end(); dimm++)
	{
		config_goal &goal = layout.goals[dimm->uid];
		goal.memory_size = getAlignedMemoryGoalSize(*dimm, goal);
	}
}

NVM_UINT64 core::memory_allocator::LayoutStepMemory::getAlignedMemoryGoalSize(const Dimm& dimm,
		const config_goal& goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 memoryBytes = configGoalSizeToBytes(goal.memory_size);
	NVM_UINT64 persistentGiB = B_TO_GiB(dimm.capacityBytes - memoryBytes);
	NVM_UINT64 dimmGiB = B_TO_GiB(dimm.capacityBytes);

	NVM_UINT64 newPersistentGiB = getAlignedPersistentPartitionCapacityGiB(persistentGiB, dimmGiB);
	NVM_UINT64 newVolatilePartitionBytes = dimm.capacityBytes - GiB_TO_B(newPersistentGiB);

	return bytesToConfigGoalSize(newVolatilePartitionBytes);
}

NVM_UINT64 core::memory_allocator::LayoutStepMemory::getAlignedPersistentPartitionCapacityGiB(
		const NVM_UINT64 persistentPartitionGiB,
		const NVM_UINT64 totalDimmGiB)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 roundedUpPersistentGiB = round_up(persistentPartitionGiB, PM_ALIGNMENT_GIB);
	NVM_UINT64 roundedUpDiff = roundedUpPersistentGiB - persistentPartitionGiB;
	NVM_UINT64 roundedDownPersistentGiB = round_down(persistentPartitionGiB, PM_ALIGNMENT_GIB);
	NVM_UINT64 roundedDownDiff = persistentPartitionGiB - roundedDownPersistentGiB;

	NVM_UINT64 alignedPersistentPartitionGiB = 0;
	if ((roundedUpPersistentGiB <= totalDimmGiB) && (roundedUpDiff <= roundedDownDiff))
	{
		alignedPersistentPartitionGiB = roundedUpPersistentGiB;
	}
	else
	{
		alignedPersistentPartitionGiB = roundedDownPersistentGiB;
	}

	return alignedPersistentPartitionGiB;
}


