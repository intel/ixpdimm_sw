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

#ifndef _core_LOGIC_LAYOUTSTEP_H_
#define _core_LOGIC_LAYOUTSTEP_H_

#include "MemoryAllocationTypes.h"
#include "MemoryAllocationRequest.h"

namespace core
{
namespace memory_allocator
{

class NVM_API LayoutStep
{
	public:
		virtual ~LayoutStep() {}

		virtual void execute(const MemoryAllocationRequest &request, MemoryAllocationLayout &layout) = 0;

	protected:

		enum capacity_type
		{
			CAPACITY_TYPE_APPDIRECT1 = 0,
			CAPACITY_TYPE_APPDIRECT2 = 1,
			CAPACITY_TYPE_2LM = 3,
			CAPACITY_TYPE_RESERVED_APPDIRECT_BYONE = 4
		};

		NVM_UINT64 getDimmUnallocatedBytes(
				const NVM_UINT64 &dimmCapacity,
				const struct config_goal &dimmGoal);
		NVM_UINT64 getDimmUnallocatedGiBAlignedBytes(
				const NVM_UINT64 &dimmCapacity,
				const struct config_goal &dimmGoal);
		NVM_UINT64 getCountOfDimmsWithUnallocatedCapacity(
				const std::vector<Dimm> &dimms,
				std::map<std::string, struct config_goal> &goals);
		NVM_UINT64 getLargestPerDimmSymmetricalBytes(
				const std::vector<Dimm> &dimms,
				std::map<std::string, struct config_goal> &goals,
				const NVM_UINT64 &requestedCapacity,
				std::vector<Dimm> &dimmsIncluded);
		NVM_UINT64 getRemainingBytesFromRequestedDimms(
				const struct MemoryAllocationRequest& request,
				MemoryAllocationLayout& layout);
		NVM_UINT64 getRemainingBytesFromDimms(
				const std::vector<Dimm> &dimms,
				MemoryAllocationLayout& layout);

		std::vector<core::memory_allocator::Dimm> getAD2Dimms(
				const std::vector<core::memory_allocator::Dimm>& requestedDimms,
				MemoryAllocationLayout& layout);

		std::vector<core::memory_allocator::Dimm> getAD1Dimms(
				const std::vector<core::memory_allocator::Dimm>& requestedDimms,
				MemoryAllocationLayout& layout);

		std::vector<core::memory_allocator::Dimm> get2LMDimms(
				const std::vector<core::memory_allocator::Dimm>& requestedDimms,
				MemoryAllocationLayout& layout);

		bool dimmHasAppDirect1(std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
				MemoryAllocationLayout& layout);

		bool dimmHasAppDirect2(std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
				MemoryAllocationLayout& layout);

		bool dimmHas2LM(std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
				MemoryAllocationLayout& layout);

		NVM_UINT64 getTotalAD2Capacity(const std::vector<core::memory_allocator::Dimm>& ad2Dimms,
				MemoryAllocationLayout& layout);

		NVM_UINT64 getTotalAD1Capacity(const std::vector<core::memory_allocator::Dimm>& ad1Dimms,
				MemoryAllocationLayout& layout);

		void killAllCapacityByType(const std::vector<core::memory_allocator::Dimm>& dimms,
				MemoryAllocationLayout& layout, enum capacity_type type);

		void killADIfSizeIsZero(config_goal& goal, enum capacity_type type);

		NVM_UINT64 calculateCapacityToShrinkPerDimm(NVM_UINT64 capacityToShrink, int numDimms);

		void shrinkSize(NVM_UINT64 &shrinkSizeBy,NVM_UINT64 reduceBy, NVM_UINT64 &size);

		void shrinkAD2(const std::vector<core::memory_allocator::Dimm>& dimms,
				NVM_UINT64 &shrinkAD2By, MemoryAllocationLayout& layout);

		void shrinkAD1(const std::vector<core::memory_allocator::Dimm>& dimms,
				NVM_UINT64 &shrinkAD1By, MemoryAllocationLayout& layout);
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* _core_LOGIC_LAYOUTSTEP_H_ */
