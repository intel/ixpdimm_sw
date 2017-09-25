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
 * Lay out the storage region in memory.
 */

#include "LayoutStepReserved.h"

#include <LogEnterExit.h>
#include <utility.h>
#include <core/exceptions/NvmExceptionBadRequest.h>

core::memory_allocator::LayoutStepReserved::LayoutStepReserved()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepReserved::~LayoutStepReserved()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepReserved::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 reservedCapacity = request.getReservedCapacityGiB();
	if (reservedCapacity > 0)
	{
		shrinkAppDirectPerReservedCapacity(request, reservedCapacity, layout);

		layout.appDirectCapacity = getTotalADCapacity(request, layout);
	}

	layout.remainingCapacity = B_TO_GiB(getRemainingBytesFromRequestedDimms(request, layout));

	// no remaining capacity left on any dimms
	if (layout.remainingCapacity == 0)
	{
		throw core::NvmExceptionBadRequestSize();
	}
}

void core::memory_allocator::LayoutStepReserved::shrinkAppDirectPerReservedCapacity(
		const MemoryAllocationRequest& request, NVM_UINT64 reservedCapacity,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<core::memory_allocator::Dimm> dimms = request.getNonReservedDimms();

	shrinkAD2(dimms, reservedCapacity, layout);
	shrinkAD1(dimms, reservedCapacity, layout);
}

NVM_UINT64 core::memory_allocator::LayoutStepReserved::getTotalADCapacity(
		const MemoryAllocationRequest& request, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 totalADCapacity = 0;

	std::vector<core::memory_allocator::Dimm> dimms = request.getDimms();
	for (std::vector<core::memory_allocator::Dimm>::const_iterator dimm =
			dimms.begin(); dimm != dimms.end(); dimm++)
	{
		totalADCapacity +=
				layout.goals[dimm->uid].app_direct_1_size +
				layout.goals[dimm->uid].app_direct_2_size;
	}

	return totalADCapacity;
}
