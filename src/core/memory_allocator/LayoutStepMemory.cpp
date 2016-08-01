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

bool core::memory_allocator::LayoutStepMemory::isRemainingStep(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return request.memoryCapacity == REQUEST_REMAINING_CAPACITY;
}

void core::memory_allocator::LayoutStepMemory::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytesToAllocate = getRequestedCapacityBytes(request, layout);
	if (bytesToAllocate)
	{
		std::vector<Dimm> dimmsToLayout;
		for (std::vector<Dimm>::const_iterator dimmIter = request.dimms.begin();
				dimmIter != request.dimms.end(); dimmIter++)
		{
			if (layout.reservedimmUid != dimmIter->uid)
			{
				dimmsToLayout.push_back(*dimmIter);
			}
		}
		NVM_UINT64 bytesAllocated = 0;
		NVM_UINT64 alignedBytesAllocated = 0;
		while (bytesAllocated < bytesToAllocate)
		{
			NVM_UINT64 bytesRemaining = bytesToAllocate - bytesAllocated;
			try
			{
				std::vector<Dimm> dimmsIncluded;
				NVM_UINT64 bytesPerDimm = getLargestPerDimmSymmetricalBytes(
						dimmsToLayout, layout.goals, bytesRemaining, dimmsIncluded);
				for (std::vector<Dimm>::const_iterator dimmIter = dimmsIncluded.begin();
								dimmIter != dimmsIncluded.end(); dimmIter++)
				{
					NVM_UINT64 alignedBytes = getAlignedDimmBytes(request, *dimmIter, layout, bytesPerDimm);
					layout.goals[dimmIter->uid].memory_size += bytesToConfigGoalSize(alignedBytes);
					alignedBytesAllocated += alignedBytes;
					bytesAllocated += bytesPerDimm;
				}
			}
			catch (core::NvmExceptionBadRequestSize &)
			{
				// out of capacity, clean up and pass along the exception
				layout.memoryCapacity = bytesToConfigGoalSize(alignedBytesAllocated);
				throw core::NvmExceptionBadRequestMemorySize();
			}
		}
		layout.memoryCapacity = bytesToConfigGoalSize(alignedBytesAllocated);
	}
}

NVM_UINT64 core::memory_allocator::LayoutStepMemory::getRequestedCapacityBytes(
		const struct MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	if (request.memoryCapacity != REQUEST_REMAINING_CAPACITY)
	{
		bytes = configGoalSizeToBytes(request.memoryCapacity);
	}
	else
	{
		bytes = getRemainingBytesFromRequestedDimms(request, layout);
	}
	return bytes;
}

NVM_UINT64 core::memory_allocator::LayoutStepMemory::getAlignedDimmBytes(
		const MemoryAllocationRequest& request,
		const Dimm &dimm,
		MemoryAllocationLayout& layout,
		const NVM_UINT64 &requestedBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 existingMemoryBytes = bytesToConfigGoalSize(layout.goals[dimm.uid].memory_size);
	NVM_UINT64 totalMemoryBytes = getTotalMemoryBytes(requestedBytes, existingMemoryBytes);
	NVM_UINT64 dimmBytes = round_down(dimm.capacity, BYTES_PER_GB);

	NVM_UINT64 alignedTotalMemoryBytes = totalMemoryBytes;
	// Memory Mode layout is last step
	if (request.memoryCapacity == REQUEST_REMAINING_CAPACITY)
	{
		if (request.appDirectExtents.size() > 0)
		{
			alignedTotalMemoryBytes = roundDownMemoryToPMAlignment(
					dimm, layout, totalMemoryBytes, dimmBytes);
		}
	}
	// Memory Mode is first step
	else
	{
		// round up by consuming storage
		alignedTotalMemoryBytes = roundMemoryToNearestPMAlignment(
				dimm, layout, totalMemoryBytes, dimmBytes);
	}

	if (alignedTotalMemoryBytes <= existingMemoryBytes)
	{
		throw core::NvmExceptionBadRequestSize();
	}
	NVM_UINT64 alignedBytes = alignedTotalMemoryBytes - existingMemoryBytes;
	if (alignedBytes < BYTES_PER_GB)
	{
		throw core::NvmExceptionBadRequestSize();
	}
	return alignedBytes;
}

NVM_UINT64 core::memory_allocator::LayoutStepMemory::getTotalMemoryBytes(
		const NVM_UINT64 &requestedBytes, const NVM_UINT64 &existingBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	if (requestedBytes < BYTES_PER_GB)
	{
		throw core::NvmExceptionBadRequestSize();
	}
	NVM_UINT64 totalMemoryBytes = existingBytes + requestedBytes;
	bytes = round_down(totalMemoryBytes, BYTES_PER_GB); // always 1 GiB aligned
	return bytes;
}

NVM_UINT64 core::memory_allocator::LayoutStepMemory::roundDownMemoryToPMAlignment(
		const Dimm &dimm, MemoryAllocationLayout& layout,
		const NVM_UINT64 &memoryBytes, const NVM_UINT64 dimmBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 pmBytes = dimmBytes - memoryBytes;
	NVM_UINT64 pmAlignedBytes = pmBytes;
	if (pmBytes > 0)
	{
		pmAlignedBytes = round_up(pmBytes, PM_ALIGNMENT_GIB * BYTES_PER_GB);
		if (pmAlignedBytes > dimmBytes)
		{
			throw core::NvmExceptionBadRequestSize();
		}
	}
	NVM_UINT64 alignedMemoryBytes = dimmBytes - pmAlignedBytes;
	if (alignedMemoryBytes < BYTES_PER_GB)
	{
		throw core::NvmExceptionBadRequestSize();
	}
	return alignedMemoryBytes;
}

NVM_UINT64 core::memory_allocator::LayoutStepMemory::roundUpMemoryToPMAlignment(
		const Dimm &dimm, MemoryAllocationLayout& layout,
		const NVM_UINT64 &memoryBytes, const NVM_UINT64 dimmBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 pmBytes = dimmBytes - memoryBytes;
	NVM_UINT64 pmAlignedBytes = pmBytes;
	if (pmBytes > 0)
	{
		pmAlignedBytes = round_down(pmBytes, PM_ALIGNMENT_GIB * BYTES_PER_GB);
		if (pmAlignedBytes == 0)
		{
			throw core::NvmExceptionBadRequestSize();
		}
	}
	NVM_UINT64 alignedMemoryBytes = dimmBytes - pmAlignedBytes;
	return alignedMemoryBytes;
}

NVM_UINT64 core::memory_allocator::LayoutStepMemory::roundMemoryToNearestPMAlignment(
		const Dimm &dimm, MemoryAllocationLayout& layout,
		const NVM_UINT64 &memoryBytes, const NVM_UINT64 dimmBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool canRoundUp = true;
	bool canRoundDown = true;
	NVM_UINT64 roundedDownBytes = 0;
	NVM_UINT64 roundedUpBytes = 0;
	NVM_UINT64 roundedUpDiff = 0;
	NVM_UINT64 roundedDownDiff = 0;
	try
	{
		roundedUpBytes = roundUpMemoryToPMAlignment(dimm, layout, memoryBytes, dimmBytes);
		roundedUpDiff = roundedUpBytes - memoryBytes;
		if (roundedUpDiff < BYTES_PER_GB || roundedUpBytes > dimmBytes)
		{
			canRoundUp = false;
		}
	}
	catch (core::NvmExceptionBadRequestSize &)
	{
		canRoundUp = false;
	}

	try
	{
		roundedDownBytes = roundDownMemoryToPMAlignment(dimm, layout, memoryBytes, dimmBytes);
		roundedDownDiff = memoryBytes - roundedDownBytes;
		if (roundedDownDiff < BYTES_PER_GB || roundedDownBytes > dimmBytes)
		{
			canRoundUp = false;
		}
	}
	catch (core::NvmExceptionBadRequestSize &)
	{
		canRoundDown = false;
	}


	NVM_UINT64 alignedBytes = 0;
	if (canRoundUp && canRoundDown)
	{
		alignedBytes = roundedUpDiff < roundedDownDiff ? roundedUpBytes : roundedDownBytes;
	}
	else if (canRoundUp)
	{
		alignedBytes = roundedUpBytes;
	}
	else if (canRoundDown)
	{
		alignedBytes = roundedDownBytes;
	}
	else
	{
		throw core::NvmExceptionBadRequestSize();
	}

	return alignedBytes;
}
