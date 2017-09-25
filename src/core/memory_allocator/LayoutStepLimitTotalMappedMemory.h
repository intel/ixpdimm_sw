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

#ifndef _core_LOGIC_LAYOUTSTEPLIMITTOTALMAPPEDMEMORY_H_
#define _core_LOGIC_LAYOUTSTEPLIMITTOTALMAPPEDMEMORY_H_

#include <core/memory_allocator/LayoutStep.h>
#include "MemoryAllocationUtil.h"

namespace core
{
namespace memory_allocator
{

class NVM_API LayoutStepLimitTotalMappedMemory : public LayoutStep
{
	public:
		LayoutStepLimitTotalMappedMemory(MemoryAllocationUtil &util);
		virtual ~LayoutStepLimitTotalMappedMemory();

		virtual void execute(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);

		virtual NVM_UINT64 getLimit(const int socketId);

	private:

		std::map<NVM_UINT16, std::vector<core::memory_allocator::Dimm> >
		getDimmsSortedBySocket(const MemoryAllocationRequest& request);

		void initializeExceedsLimit();

		void initializeDimmsSortedBySocket(const MemoryAllocationRequest& request);

		void initializeTotalMappedSizeVariablesPerSocket(const MemoryAllocationRequest& request,
			MemoryAllocationLayout& layout, int socketId);

		bool mappedSizeExceedsLimit();

		void shrinkLayoutCapacities(const MemoryAllocationRequest& request,
			MemoryAllocationLayout& layout);

		void shrinkLayoutGoals(MemoryAllocationLayout& layout);

		void shrinkAppDirect2(MemoryAllocationLayout& layout);

		void shrinkAppDirect1(MemoryAllocationLayout& layout);

		void shrinkMemory(MemoryAllocationLayout& layout);

		void shrinkSizePerDimm(NVM_UINT64 reduceBy, NVM_UINT64 &size);

		NVM_UINT64 getTotal2LMCapacity(
			const std::vector<core::memory_allocator::Dimm>& memoryDimms,
			MemoryAllocationLayout& layout);

		NVM_UINT64 m_limit;
		NVM_UINT64 m_totalMappedSize;
		NVM_UINT64 m_mappedCapacityExceedsLimit;
		std::vector<core::memory_allocator::Dimm> m_socketDimms;
		std::map<NVM_UINT16, std::vector<Dimm> > m_dimmsSortedBySocket;
		MemoryAllocationUtil &m_memAllocUtil;
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* _core_LOGIC_LAYOUTSTEPLIMITTOTALMAPPEDMEMORY_H_ */
