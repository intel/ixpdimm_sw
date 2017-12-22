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

#ifndef _core_LOGIC_LAYOUTSTEPMEMORY_H_
#define _core_LOGIC_LAYOUTSTEPMEMORY_H_

#include <nvm_management.h>
#include "LayoutStep.h"
#include <core/ExportCore.h>

namespace core
{
namespace memory_allocator
{

class NVM_CORE_API LayoutStepMemory : public LayoutStep
{
	public:
		LayoutStepMemory();
		virtual ~LayoutStepMemory();

		virtual void execute(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);

	protected:
		void layoutMemoryModeCapacity(const MemoryAllocationRequest& request,
				MemoryAllocationLayout& layout);
		NVM_UINT64 getBytesAllocatedFromLayout(const MemoryAllocationLayout& layout);
		void layoutMaximumSymmetricalBytesOnDimms(const NVM_UINT64 bytesToLayout,
				const std::vector<Dimm> &dimmsToLayout,
				MemoryAllocationLayout &layout);
		void alignPartitionBoundary(const MemoryAllocationRequest& request,
				MemoryAllocationLayout &layout);
		NVM_UINT64 getAlignedMemoryGoalSize(const Dimm &dimm, const config_goal &goal);
		NVM_UINT64 getAlignedPersistentPartitionCapacityGiB(const NVM_UINT64 persistentPartitionGiB,
				const NVM_UINT64 totalDimmGiB);
		bool newBytesWereAllocated(const MemoryAllocationLayout& updatedLayout,
				const NVM_UINT64 previousBytesAllocated);
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* _core_LOGIC_LAYOUTSTEPMEMORY_H_ */
