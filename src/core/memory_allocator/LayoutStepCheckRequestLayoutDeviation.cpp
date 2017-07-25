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
 * Add a layout warning if layout is not within acceptable deviation from the request
 */

#include "LayoutStepCheckRequestLayoutDeviation.h"

#include <LogEnterExit.h>

#define ACCEPTED_PERCENT_DEVIATION 10

// using macro to avoid ambiguous overload call to abs function
#define ABSOLUTEDIFFERENCE(X, Y) (X > Y) ? (X - Y) : (Y - X)

core::memory_allocator::LayoutStepCheckRequestLayoutDeviation::LayoutStepCheckRequestLayoutDeviation()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepCheckRequestLayoutDeviation::~LayoutStepCheckRequestLayoutDeviation()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepCheckRequestLayoutDeviation::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isAcceptableDeviation = false;

	isAcceptableDeviation = isMemoryCapacityLayoutAcceptable(request, layout) &&
			isAppDirectCapacityLayoutAcceptable(request, layout);

	if(!isAcceptableDeviation)
	{
		layout.warnings.push_back(LAYOUT_WARNING_GOAL_ADJUSTED_MORE_THAN_10PERCENT);
	}
}

double core::memory_allocator::LayoutStepCheckRequestLayoutDeviation::findPercentDeviation(
		NVM_UINT64 expectedValue, NVM_UINT64 observedValue, NVM_UINT64 totalCapacity)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	int deviation = ABSOLUTEDIFFERENCE(observedValue, expectedValue);

	return (double) 100 * deviation / totalCapacity;
}

bool core::memory_allocator::LayoutStepCheckRequestLayoutDeviation::isLayoutDeviationWithinBounds(
		double percentDeviation)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return (percentDeviation <= ACCEPTED_PERCENT_DEVIATION);
}

bool core::memory_allocator::LayoutStepCheckRequestLayoutDeviation::isMemoryCapacityLayoutAcceptable(
		const struct MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isDeviationWithinBounds = true;
	if (request.getAllMappableDimmCapacityInGiB() > 0 && request.getMemoryModeCapacityGiB() > 0)
	{
		double percentDeviation =
				findPercentDeviation(request.getMemoryModeCapacityGiB(), layout.memoryCapacity, request.getAllMappableDimmCapacityInGiB());

		if ((layout.memoryCapacity == 0)  ||
				(!isLayoutDeviationWithinBounds(percentDeviation)))
		{
			isDeviationWithinBounds = false;
		}
	}

	return isDeviationWithinBounds;
}

bool core::memory_allocator::LayoutStepCheckRequestLayoutDeviation::isAppDirectCapacityLayoutAcceptable(
		const struct MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool isDeviationWithinBounds = true;
	if (request.getAllMappableDimmCapacityInGiB() > 0 && request.getAppDirectCapacityGiB() > 0)
	{
		NVM_UINT64 layoutAppDirectCapacity = getNonReservedAppDirectCapacityGiBFromLayout(request, layout);
		double percentDeviation = findPercentDeviation(request.getAppDirectCapacityGiB(),
						layoutAppDirectCapacity, request.getAllMappableDimmCapacityInGiB());

		if(!isLayoutDeviationWithinBounds(percentDeviation))
		{
			isDeviationWithinBounds = false;
		}
	}

	return isDeviationWithinBounds;
}

NVM_UINT64 core::memory_allocator::LayoutStepCheckRequestLayoutDeviation::getNonReservedAppDirectCapacityGiBFromLayout(
		const struct MemoryAllocationRequest& request, const MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return layout.appDirectCapacity;
}
