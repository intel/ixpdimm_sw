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

namespace core
{
namespace memory_allocator
{

// variables defined only for the purpose of this layout step
static const NVM_UINT16 TYPE_APPDIRECT1 = 0;
static const NVM_UINT16 TYPE_APPDIRECT2 = 1;
static const NVM_UINT16 TYPE_2LM = 3;
static const NVM_UINT16 TYPE_RESERVED_APPDIRECT_BYONE = 4;

class NVM_API LayoutStepLimitTotalMappedMemory : public LayoutStep
{
	public:
		LayoutStepLimitTotalMappedMemory();
		virtual ~LayoutStepLimitTotalMappedMemory();

		virtual void execute(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);

		virtual NVM_UINT64 getLimit(const MemoryAllocationRequest& request);

	private:

		void initializeExceedsLimit();

		void initializeDimmsSortedBySocket(const MemoryAllocationRequest& request);

		void initializeTotalMappedSizeVariablesPerSocket(const MemoryAllocationRequest& request,
			MemoryAllocationLayout& layout, int socketId);

		bool mappedSizeExceedsLimit();

		void shrinkLayoutCapacities(const MemoryAllocationRequest& request,
			MemoryAllocationLayout& layout);

		void shrinkLayoutGoals(const MemoryAllocationRequest& request, MemoryAllocationLayout& layout);

		void shrinkAppDirect2(const MemoryAllocationRequest& request, MemoryAllocationLayout& layout);

		void shrinkAppDirect1(const MemoryAllocationRequest& request, MemoryAllocationLayout& layout);

		void shrinkReservedDimm(const MemoryAllocationRequest& request, MemoryAllocationLayout& layout);

		void shrinkMemory(const MemoryAllocationRequest& request, MemoryAllocationLayout& layout);

		void shrinkSizePerDimm(NVM_UINT64 reduceBy, NVM_UINT64 &size);

		NVM_UINT64 reduceEachDimmBy(NVM_UINT64 mappedMemoryCapacityExceedsLimit, int numDimms);

		std::vector<core::memory_allocator::Dimm> getDimmsByType(
			MemoryAllocationLayout& layout, NVM_UINT16 capacityType);

		bool dimmHasAppDirect1(
			std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
			MemoryAllocationLayout& layout);

		bool dimmHasAppDirect2(
			std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
			MemoryAllocationLayout& layout);

		bool dimmHas2LM(
			std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
			MemoryAllocationLayout& layout);

		bool dimmisReservedAppDirectByOne(
			std::vector<core::memory_allocator::Dimm>::const_iterator dimm,
			MemoryAllocationLayout& layout);

		void killADIfSizeIsZero(config_goal& goal, NVM_UINT16 type);

		void killAllCapacityByType(std::vector<core::memory_allocator::Dimm> dimms,
			MemoryAllocationLayout& layout, NVM_UINT16 type);

		NVM_UINT64 getTotalAD2Capacity(std::vector<core::memory_allocator::Dimm> ad2Dimms,
			MemoryAllocationLayout& layout);

		NVM_UINT64 getTotalAD1Capacity(std::vector<core::memory_allocator::Dimm> ad1Dimms,
			MemoryAllocationLayout& layout);

		NVM_UINT64 getTotal2LMCapacity(std::vector<core::memory_allocator::Dimm> ad1Dimms,
			MemoryAllocationLayout& layout);

		NVM_UINT64 m_limit;

		NVM_UINT64 m_totalMappedSize;
		NVM_UINT64 m_mappedCapacityExceedsLimit;

		std::vector<core::memory_allocator::Dimm> m_socketDimms;
		std::map<NVM_UINT16, std::vector<Dimm> > m_dimmsSortedBySocket;
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* _core_LOGIC_LAYOUTSTEPLIMITTOTALMAPPEDMEMORY_H_ */
