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

NVM_UINT64 core::memory_allocator::LayoutStep::getCountOfDimmsWithUnallocatedCapacity(
		const std::vector<core::memory_allocator::Dimm> &dimms,
		std::map<std::string, struct config_goal> &goals)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 dimmCount = 0;
	for (std::vector<struct Dimm>::const_iterator dimmIter = dimms.begin();
				dimmIter != dimms.end(); dimmIter++)
	{
		if (getDimmUnallocatedGiBAlignedBytes(dimmIter->capacityBytes, goals[dimmIter->uid])
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
		round_down(remaining, BYTES_PER_GIB); // memory partition eats metadata
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

	NVM_UINT64 bytes = dimms.front().capacityBytes;
	for (std::vector<struct Dimm>::const_iterator dimmIter = dimms.begin();
				dimmIter != dimms.end(); dimmIter++)
	{
		NVM_UINT64 dimmMaxBytes = getDimmUnallocatedGiBAlignedBytes(
				dimmIter->capacityBytes, goals[dimmIter->uid]);
		if (dimmMaxBytes > 0)
		{
			dimmsIncluded.push_back(*dimmIter);
			if (dimmMaxBytes < bytes)
			{
				bytes = dimmMaxBytes;
			}
		}
	}

	NVM_UINT64 evenlyDividedBytes = requestedBytes / dimmsIncluded.size();
	if (evenlyDividedBytes < bytes)
	{
		bytes = evenlyDividedBytes;
	}

	bytes = round_down(bytes, BYTES_PER_GIB);
	return bytes;
}

NVM_UINT64 core::memory_allocator::LayoutStep::getRemainingBytesFromRequestedDimms(
		const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> dimms = request.getNonReservedDimms();
	return getRemainingBytesFromDimms(dimms, layout);
}

NVM_UINT64 core::memory_allocator::LayoutStep::getRemainingBytesFromDimms(
		const std::vector<Dimm>& dimms, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	for (std::vector<struct Dimm>::const_iterator dimmIter = dimms.begin();
			dimmIter != dimms.end(); dimmIter++)
	{
		bytes += getDimmUnallocatedGiBAlignedBytes(dimmIter->capacityBytes, layout.goals[dimmIter->uid]);
	}

	return bytes;
}

NVM_UINT64 core::memory_allocator::LayoutStep::getDimmUnallocatedGiBAlignedBytes(
		const NVM_UINT64& dimmCapacity, const struct config_goal& dimmGoal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 dimmRemainingBytes = getDimmUnallocatedBytes(dimmCapacity, dimmGoal);

	return round_down(dimmRemainingBytes, BYTES_PER_GIB);
}

std::vector<core::memory_allocator::Dimm> core::memory_allocator::LayoutStep::getAD2Dimms(
		const std::vector<core::memory_allocator::Dimm>& requestedDimms,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<core::memory_allocator::Dimm> dimms;

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			requestedDimms.begin(); dimm != requestedDimms.end(); dimm++)
	{
		if (dimmHasAppDirect2(dimm, layout))
		{
			dimms.push_back(*dimm);
		}
	}

	return dimms;
}

std::vector<core::memory_allocator::Dimm> core::memory_allocator::LayoutStep::getAD1Dimms(
		const std::vector<core::memory_allocator::Dimm>& requestedDimms,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<core::memory_allocator::Dimm> dimms;

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			requestedDimms.begin(); dimm != requestedDimms.end(); dimm++)
	{
		if (dimmHasAppDirect1(dimm, layout))
		{
			dimms.push_back(*dimm);
		}
	}

	return dimms;
}

std::vector<core::memory_allocator::Dimm> core::memory_allocator::LayoutStep::get2LMDimms(
		const std::vector<core::memory_allocator::Dimm>& requestedDimms,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<core::memory_allocator::Dimm> dimms;

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			requestedDimms.begin(); dimm != requestedDimms.end(); dimm++)
	{
		if (dimmHas2LM(dimm, layout))
		{
			dimms.push_back(*dimm);
		}
	}

	return dimms;
}

bool core::memory_allocator::LayoutStep::dimmHasAppDirect1(
		std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return layout.goals[dimm->uid].app_direct_1_size > 0;
}

bool core::memory_allocator::LayoutStep::dimmHasAppDirect2(
		std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return layout.goals[dimm->uid].app_direct_2_size > 0;
}

bool core::memory_allocator::LayoutStep::dimmHas2LM(
		std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return layout.goals[dimm->uid].memory_size > 0;
}

NVM_UINT64 core::memory_allocator::LayoutStep::getTotalAD2Capacity(
		const std::vector<core::memory_allocator::Dimm>& dimms,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 totalAD2Capacity = 0;
	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		totalAD2Capacity += layout.goals[dimm->uid].app_direct_2_size;
	}

	return totalAD2Capacity;
}

NVM_UINT64 core::memory_allocator::LayoutStep::getTotalAD1Capacity(
		const std::vector<core::memory_allocator::Dimm>& dimms,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 totalAD1Capacity = 0;
	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		totalAD1Capacity += layout.goals[dimm->uid].app_direct_1_size;
	}

	return totalAD1Capacity;
}

void core::memory_allocator::LayoutStep::killAllCapacityByType(
		const std::vector<core::memory_allocator::Dimm>& dimms,
		MemoryAllocationLayout& layout,
		enum capacity_type type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		config_goal& goal = layout.goals[dimm->uid];

		if (type == CAPACITY_TYPE_APPDIRECT1 || type == CAPACITY_TYPE_RESERVED_APPDIRECT_BYONE)
		{
			goal.app_direct_1_size = 0;
			killADIfSizeIsZero(goal, type);
		}
		else if (type == CAPACITY_TYPE_APPDIRECT2)
		{
			goal.app_direct_2_size = 0;
			killADIfSizeIsZero(goal, type);
		}
		else if (type == CAPACITY_TYPE_2LM)
		{
			goal.memory_size = 0;
		}
	}
}

void core::memory_allocator::LayoutStep::killADIfSizeIsZero(
		config_goal& goal, enum capacity_type type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if ((type == CAPACITY_TYPE_APPDIRECT1 || type == CAPACITY_TYPE_RESERVED_APPDIRECT_BYONE ) &&
		goal.app_direct_1_size == 0)
	{
		struct app_direct_attributes empty_settings;
		memset(&empty_settings, 0, sizeof(struct app_direct_attributes));
		goal.app_direct_1_settings = empty_settings;
		goal.app_direct_count--;
	}

	if (type == CAPACITY_TYPE_APPDIRECT2 && goal.app_direct_2_size == 0)
	{
		struct app_direct_attributes empty_settings;
		memset(&empty_settings, 0, sizeof(struct app_direct_attributes));
		goal.app_direct_2_settings = empty_settings;
		goal.app_direct_count--;
	}
}

NVM_UINT64 core::memory_allocator::LayoutStep::calculateCapacityToShrinkPerDimm(
		NVM_UINT64 capacityToShrink, int numDimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (NVM_UINT64) (ceil((double) (capacityToShrink) / (double) (numDimms)));
}

void core::memory_allocator::LayoutStep::shrinkSize(
		NVM_UINT64 &shrinkSizeBy, NVM_UINT64 reduceBy, NVM_UINT64 &size)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// reduce each DIMM evenly if possible
	if (size >= reduceBy && shrinkSizeBy >= reduceBy)
	{
		size -= reduceBy;
		shrinkSizeBy -= reduceBy;
	}
	// reduce little more than needed (because of ceil/rounding up)
	else if (size >= reduceBy && shrinkSizeBy < reduceBy)
	{
		size -= reduceBy;
		shrinkSizeBy = 0;
	}
	// reduce more than what's available per DIMM
	else
	{
		shrinkSizeBy -= size;
		size = 0;
	}
}

void core::memory_allocator::LayoutStep::shrinkAD2(
		const std::vector<core::memory_allocator::Dimm>& dimms,
		NVM_UINT64 &shrinkAD2By, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (shrinkAD2By > 0)
	{
		std::vector<core::memory_allocator::Dimm> ad2Dimms = getAD2Dimms(dimms, layout);
		if (!ad2Dimms.empty())
		{
			// If capacityToShrink exceeds AD2, eliminate all AD2, else shrink evenly on all AD2 DIMMs
			NVM_UINT64 totalAD2Capacity = getTotalAD2Capacity(ad2Dimms, layout);
			if (shrinkAD2By >= totalAD2Capacity)
			{
				killAllCapacityByType(ad2Dimms, layout, CAPACITY_TYPE_APPDIRECT2);

				shrinkAD2By -= totalAD2Capacity;
			}
			else
			{
				NVM_UINT64 reduceBy = calculateCapacityToShrinkPerDimm(shrinkAD2By, ad2Dimms.size());

				for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
						ad2Dimms.begin(); dimm != ad2Dimms.end(); dimm++)
				{
					if (shrinkAD2By > 0)
					{
						config_goal& goal = layout.goals[dimm->uid];

						shrinkSize(shrinkAD2By, reduceBy, goal.app_direct_2_size);

						killADIfSizeIsZero(goal, CAPACITY_TYPE_APPDIRECT2);
					}
				}
			}
		}
	}
}

void core::memory_allocator::LayoutStep::shrinkAD1(
		const std::vector<core::memory_allocator::Dimm>& dimms,
		NVM_UINT64 &shrinkAD1By,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (shrinkAD1By > 0)
	{
		std::vector<core::memory_allocator::Dimm> ad1Dimms = getAD1Dimms(dimms, layout);
		if (!ad1Dimms.empty())
		{
			// If capacityToShrink exceeds AD1, eliminate all AD1, else shrink evenly on all AD1 DIMMs
			NVM_UINT64 totalAD1Capacity = getTotalAD1Capacity(ad1Dimms, layout);
			if (shrinkAD1By >= totalAD1Capacity)
			{
				killAllCapacityByType(ad1Dimms, layout, CAPACITY_TYPE_APPDIRECT1);

				shrinkAD1By -= totalAD1Capacity;
			}
			else
			{
				NVM_UINT64 reduceBy = calculateCapacityToShrinkPerDimm(shrinkAD1By, ad1Dimms.size());

				for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
						ad1Dimms.begin(); dimm != ad1Dimms.end(); dimm++)
				{
					if (shrinkAD1By > 0)
					{
						config_goal& goal = layout.goals[dimm->uid];
						shrinkSize(shrinkAD1By, reduceBy, goal.app_direct_1_size);

						killADIfSizeIsZero(goal, CAPACITY_TYPE_APPDIRECT1);
					}
				}
			}
		}
	}
}
