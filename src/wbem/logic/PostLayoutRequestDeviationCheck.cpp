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
 * Rule that checks that layout is within acceptable deviation from the request
 */

#include "PostLayoutRequestDeviationCheck.h"
#include <LogEnterExit.h>
#include <exception/NvmExceptionBadRequest.h>

#define ACCEPTED_PERCENT_DEVIATION 10

wbem::logic::PostLayoutRequestDeviationCheck::PostLayoutRequestDeviationCheck()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

wbem::logic::PostLayoutRequestDeviationCheck::~PostLayoutRequestDeviationCheck()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

double wbem::logic::PostLayoutRequestDeviationCheck::findPercentDeviation(
		NVM_UINT64 expectedValue, NVM_UINT64 observedValue)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int deviation =
	(observedValue > expectedValue) ? (observedValue - expectedValue) : (expectedValue - observedValue);
	return (double)100.0 * (deviation)/expectedValue;
}

bool wbem::logic::PostLayoutRequestDeviationCheck::layoutDeviationIsWithinBounds(
		double percentDeviation)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return (percentDeviation <= ACCEPTED_PERCENT_DEVIATION);
}

void wbem::logic::PostLayoutRequestDeviationCheck::checkIfMemoryCapacityLayoutIsAcceptable(
		const struct MemoryAllocationRequest& request,
		const MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.memoryCapacity)
	{
		double percentDeviation =
				findPercentDeviation(request.memoryCapacity, layout.memoryCapacity);
		if ((layout.memoryCapacity == 0)  ||
				(!layoutDeviationIsWithinBounds(percentDeviation)))
		{
			throw exception::NvmExceptionUnacceptableLayoutDeviation();
		}
	}
}

void wbem::logic::PostLayoutRequestDeviationCheck::checkAppDirectCapacityLayoutIsAcceptable(
		const struct MemoryAllocationRequest& request,
		const MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.appDirectExtents.size())
	{
		for (size_t i = 0;  i < request.appDirectExtents.size(); i++)
		{
			double percentDeviation =
					findPercentDeviation(request.appDirectExtents[i].capacity,
							layout.appDirectCapacities[i]);
			if ((layout.appDirectCapacities[i] == 0)  ||
					(!layoutDeviationIsWithinBounds(percentDeviation)))
			{
				throw exception::NvmExceptionUnacceptableLayoutDeviation();
			}
		}
	}
}

void wbem::logic::PostLayoutRequestDeviationCheck::verify(
		const struct MemoryAllocationRequest& request,
		const struct MemoryAllocationLayout &layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkIfMemoryCapacityLayoutIsAcceptable(request, layout);
	checkAppDirectCapacityLayoutIsAcceptable(request, layout);
}
