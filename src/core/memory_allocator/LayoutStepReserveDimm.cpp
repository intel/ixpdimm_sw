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
 * Lay out the reserve dimm in memory.
 */

#include "LayoutStepReserveDimm.h"

#include <utility.h>
#include <LogEnterExit.h>
#include <core/exceptions/NvmExceptionBadRequest.h>
#include <core/memory_allocator/LayoutStepAppDirect.h>
#include <core/memory_allocator/LayoutStepStorage.h>

core::memory_allocator::LayoutStepReserveDimm::LayoutStepReserveDimm(MemoryAllocationUtil &util) :
	m_memAllocUtil(util)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::memory_allocator::LayoutStepReserveDimm::~LayoutStepReserveDimm()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void core::memory_allocator::LayoutStepReserveDimm::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.hasReservedDimm())
	{
		verifyEnoughDimmsInRequest(request);

		layoutReservedDimm(request, layout);
	}
}

void core::memory_allocator::LayoutStepReserveDimm::verifyEnoughDimmsInRequest(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (request.getNumberOfDimms() == 0)
	{
		throw NvmExceptionBadRequestNoDimms();
	}
	else if (request.getNumberOfDimms() == 1)
	{
		// Makes no sense to "reserve" a DIMM out of the request if there's only one
		throw NvmExceptionBadRequestReserveDimm();
	}
}

void core::memory_allocator::LayoutStepReserveDimm::layoutReservedDimm(
		const MemoryAllocationRequest& request, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	Dimm reservedDimm = getReservedDimmFromRequest(request);

	if (request.getReservedDimmCapacityType() == RESERVE_DIMM_STORAGE)
	{
		layoutReservedDimmForStorage(reservedDimm, layout);
	}
	else
	{
		layoutReservedDimmForAppDirect(reservedDimm, layout);
	}

	layout.reservedimmUid = reservedDimm.uid;
}

core::memory_allocator::Dimm
core::memory_allocator::LayoutStepReserveDimm::getReservedDimmFromRequest(
		const MemoryAllocationRequest& request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	Dimm reservedDimm;
	try
	{
		reservedDimm = request.getReservedDimm();
	}
	catch (MemoryAllocationRequest::NoReservedDimmException &)
	{
		throw NvmExceptionBadRequestReserveDimm();
	}

	return reservedDimm;
}

void core::memory_allocator::LayoutStepReserveDimm::layoutReservedDimmForStorage(const Dimm &reserveDimm,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	MemoryAllocationRequest reserveDimmRequest = getRequestForStorageReservedDimm(reserveDimm);

	LayoutStepStorage storageReserveDimmStep;
	storageReserveDimmStep.execute(reserveDimmRequest, layout);
}

core::memory_allocator::MemoryAllocationRequest
core::memory_allocator::LayoutStepReserveDimm::getRequestForStorageReservedDimm(
		const Dimm& reserveDimm)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	MemoryAllocationRequest reserveDimmRequest;

	reserveDimmRequest.addDimm(reserveDimm);
	reserveDimmRequest.setStorageRemaining(true);

	return reserveDimmRequest;
}

void core::memory_allocator::LayoutStepReserveDimm::layoutReservedDimmForAppDirect(
		const Dimm& reserveDimm, MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	MemoryAllocationRequest reserveDimmRequest = getRequestForAppDirectReservedDimm(reserveDimm);

	LayoutStepAppDirect appDirectReserveDimmStep(m_memAllocUtil);
	appDirectReserveDimmStep.execute(reserveDimmRequest, layout);
}

core::memory_allocator::MemoryAllocationRequest
core::memory_allocator::LayoutStepReserveDimm::getRequestForAppDirectReservedDimm(
		const Dimm& reserveDimm)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	MemoryAllocationRequest reserveDimmRequest;

	reserveDimmRequest.addDimm(reserveDimm);

	AppDirectExtent nonInterleavedExtent;
	nonInterleavedExtent.byOne = true;
	nonInterleavedExtent.capacityGiB = B_TO_GiB(reserveDimm.capacityBytes);
	reserveDimmRequest.setAppDirectExtent(nonInterleavedExtent);

	return reserveDimmRequest;
}
