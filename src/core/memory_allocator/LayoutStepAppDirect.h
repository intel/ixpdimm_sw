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
 * Lay out the app direct region in memory.
 */

#ifndef _core_LOGIC_LAYOUTSTEPAPPDIRECT_H_
#define _core_LOGIC_LAYOUTSTEPAPPDIRECT_H_

#include <vector>
#include <map>
#include <nvm_types.h>
#include "LayoutStep.h"
#include "MemoryAllocationUtil.h"

namespace core
{
namespace memory_allocator
{

class NVM_API LayoutStepAppDirect: public LayoutStep
{
	public:
		LayoutStepAppDirect(MemoryAllocationUtil &util);
		virtual ~LayoutStepAppDirect();

		virtual void execute(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);

	protected:
		void initNextInterleaveId(const MemoryAllocationLayout& layout);
		bool requestExtentIsInterleaved(const MemoryAllocationRequest &request);

		void layoutExtent(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);

		void layoutInterleavedExtentOnRequestedDimms(const std::vector<Dimm> &dimms,
				MemoryAllocationLayout &layout);
		std::map<NVM_UINT16, std::vector<Dimm> > getDimmsSortedBySocket(
				const std::vector<Dimm> &dimms);
		void layoutInterleavedExtentOnSocket(const std::vector<Dimm> &socketDimms,
				MemoryAllocationLayout &layout);
		std::vector<Dimm> getLargestSetOfInterleavableDimms(const std::vector<Dimm>& dimms);
		void layoutInterleaveSet(const std::vector<Dimm> &interleavedDimms,
				const NVM_UINT64 bytesPerDimm,
				MemoryAllocationLayout &layout);
		void removeDimmsFromList(const std::vector<Dimm> &dimmsToRemove,
				std::vector<Dimm> &dimmList);

		void layoutUnallocatedCapacityWithNonInterleaved(const std::vector<Dimm> &dimms,
				MemoryAllocationLayout &layout);

		void updateGoalWithInterleaveSet(config_goal &goal,
				const NVM_UINT64 bytesPerDimm,
				const std::vector<Dimm> &interleavedDimms);
		void updateGoalParametersWithInterleaveSet(NVM_UINT64 &goalSize,
				NVM_UINT16 &goalSetId,
				app_direct_attributes &goalSettings,
				const NVM_UINT64 bytesPerDimm,
				const std::vector<Dimm> &interleavedDimms);
		interleave_ways getInterleaveWaysFromNumDimms(const size_t numDimms);

		void addExtentCapacityToLayout(MemoryAllocationLayout &layout);
		NVM_UINT64 getExtentCapacityFromLayout(const MemoryAllocationLayout &layout);
		void checkTotalExtentCapacityAllocated(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);
		bool allRequestedCapacityAllocated(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);

		MemoryAllocationUtil &m_memAllocUtil;
		size_t m_extentIndex;
		NVM_UINT16 m_nextInterleaveId;
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* _core_LOGIC_LAYOUTSTEPAPPDIRECT_H_ */
