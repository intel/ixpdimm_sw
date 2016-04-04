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
 * Lay out the volatile region in memory.
 */

#ifndef _WBEM_LOGIC_LAYOUTSTEPVOLATILE_H_
#define _WBEM_LOGIC_LAYOUTSTEPVOLATILE_H_

#include "LayoutStep.h"
#include <nvm_management.h>

namespace wbem
{
namespace logic
{

class NVM_API LayoutStepVolatile : public LayoutStep
{
	public:
		LayoutStepVolatile();
		virtual ~LayoutStepVolatile();

		virtual void execute(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);

		virtual bool isRemainingStep(const MemoryAllocationRequest &request);

	protected:
		NVM_UINT64 getRequestedCapacityBytes(
				const struct MemoryAllocationRequest& request,
				MemoryAllocationLayout &layout);
		NVM_UINT64 getAlignedDimmBytes(const MemoryAllocationRequest& request, const Dimm &dimm,
				MemoryAllocationLayout& layout, const NVM_UINT64 &requestedBytes);
		NVM_UINT64 getTotalVolatileBytes(const NVM_UINT64 &requestedBytes,
				const NVM_UINT64 &existingBytes);
		NVM_UINT64 roundDownVolatileToPMAlignment(
				const Dimm &dimm, MemoryAllocationLayout& layout,
				const NVM_UINT64 &requestedBytes, const NVM_UINT64 dimmBytes);
		NVM_UINT64 roundUpVolatileToPMAlignment(
				const Dimm &dimm, MemoryAllocationLayout& layout,
				const NVM_UINT64 &requestedBytes, const NVM_UINT64 dimmBytes);
		NVM_UINT64 roundVolatileToNearestPMAlignment(
				const Dimm &dimm, MemoryAllocationLayout& layout,
				const NVM_UINT64 &requestedBytes, const NVM_UINT64 dimmBytes);

};

} /* namespace logic */
} /* namespace wbem */

#endif /* _WBEM_LOGIC_LAYOUTSTEPVOLATILE_H_ */
