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
 * Base class for memory allocation layout steps.
 */

#include "LayoutStep.h"

#include <utility.h>
#include <LogEnterExit.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

bool core::memory_allocator::LayoutStep::isRemainingStep(const MemoryAllocationRequest &request)
{
	return false;
}

NVM_UINT64 core::memory_allocator::LayoutStep::getCountOfDimmsWithUnallocatedCapacity(
		const std::vector<core::memory_allocator::Dimm> &dimms,
		std::map<std::string, struct config_goal> &goals)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 dimmCount = 0;
	for (std::vector<struct Dimm>::const_iterator dimmIter = dimms.begin();
				dimmIter != dimms.end(); dimmIter++)
	{
		if (getDimmUnallocatedGiBAlignedBytes(dimmIter->capacity, goals[dimmIter->uid])
				> 0)
		{
			dimmCount++;
		}
	}
	return dimmCount;
}

NVM_UINT64 core::memory_allocator::LayoutStep::getDimmUnallocatedBytes(
		const NVM_UINT64 &dimmCapacity,
		const struct config_goal &dimmGoal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 remaining = dimmCapacity;
	if (dimmGoal.memory_size)
	{
		remaining -= configGoalSizeToBytes(dimmGoal.memory_size);
		round_down(remaining, BYTES_PER_GB); // memory partition eats metadata
	}
	if (dimmGoal.app_direct_count >= 2)
	{
		remaining -= configGoalSizeToBytes(dimmGoal.app_direct_2_size);
	}
	if (dimmGoal.app_direct_count >= 1)
	{
		remaining -= configGoalSizeToBytes(dimmGoal.app_direct_1_size);
	}
	return remaining;
}

NVM_UINT64 core::memory_allocator::LayoutStep::getLargestPerDimmSymmetricalBytes(
		const std::vector<core::memory_allocator::Dimm> &dimms,
		std::map<std::string, struct config_goal> &goals,
		const NVM_UINT64 &requestedBytes,
		std::vector<core::memory_allocator::Dimm> &dimmsIncluded)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 dimmCount = getCountOfDimmsWithUnallocatedCapacity(dimms, goals);
	if (dimmCount == 0)
	{
		throw core::NvmExceptionBadRequestSize();
	}

	NVM_UINT64 bytes = requestedBytes / dimmCount;
	for (std::vector<struct Dimm>::const_iterator dimmIter = dimms.begin();
				dimmIter != dimms.end(); dimmIter++)
	{
		NVM_UINT64 dimmMaxBytes = getDimmUnallocatedGiBAlignedBytes(
				dimmIter->capacity, goals[dimmIter->uid]);
		if (dimmMaxBytes > 0)
		{
			dimmsIncluded.push_back(*dimmIter);
			if (dimmMaxBytes < bytes)
			{
				bytes = dimmMaxBytes;
			}
		}
	}

	// has to be 1 GiB aligned
	if (bytes < BYTES_PER_GB)
	{
		throw core::NvmExceptionBadRequestSize();
	}
	bytes = round_down(bytes, BYTES_PER_GB);
	return bytes;
}

NVM_UINT64 core::memory_allocator::LayoutStep::getRemainingBytesFromRequestedDimms(
		const struct MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	for (std::vector<struct Dimm>::const_iterator dimmIter = request.dimms.begin();
			dimmIter != request.dimms.end(); dimmIter++)
	{
		bytes += getDimmUnallocatedGiBAlignedBytes(dimmIter->capacity, layout.goals[dimmIter->uid]);
	}

	// no remaining capacity left on any dimms
	if (bytes == 0)
	{
		throw core::NvmExceptionBadRequestSize();
	}
	return bytes;
}

NVM_UINT64 core::memory_allocator::LayoutStep::getDimmUnallocatedGiBAlignedBytes(
		const NVM_UINT64& dimmCapacity, const struct config_goal& dimmGoal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 dimmRemainingBytes = getDimmUnallocatedBytes(dimmCapacity, dimmGoal);

	return round_down(dimmRemainingBytes, BYTES_PER_GB);
}
