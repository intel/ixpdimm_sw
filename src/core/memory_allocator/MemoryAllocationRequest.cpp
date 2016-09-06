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

#include "MemoryAllocationRequest.h"
#include <LogEnterExit.h>
#include <utility.h>

namespace core
{
namespace memory_allocator
{

MemoryAllocationRequest::MemoryAllocationRequest() :
		m_memoryCapacityGiB(0), m_appDirectExtent(), m_storageRemaining(false),
		m_reserveDimmUid(""), m_reserveDimmType(RESERVE_DIMM_NONE),
		m_dimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

MemoryAllocationRequest::~MemoryAllocationRequest()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

NVM_UINT64 core::memory_allocator::MemoryAllocationRequest::getMemoryModeCapacityGiB() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_memoryCapacityGiB;
}

void MemoryAllocationRequest::setMemoryModeCapacityGiB(const NVM_UINT64 capacityGiB)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_memoryCapacityGiB = capacityGiB;
}

NVM_UINT64 MemoryAllocationRequest::getAppDirectCapacityGiB() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_appDirectExtent.capacityGiB;
}

AppDirectExtent MemoryAllocationRequest::getAppDirectExtent() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_appDirectExtent;
}

void MemoryAllocationRequest::setAppDirectExtent(const AppDirectExtent& extent)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_appDirectExtent = extent;
}

bool MemoryAllocationRequest::isStorageRemaining() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_storageRemaining;
}

void MemoryAllocationRequest::setStorageRemaining(const bool storageIsRemaining)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_storageRemaining = storageIsRemaining;
}

std::string MemoryAllocationRequest::getReservedDimmUid() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_reserveDimmUid;
}

void MemoryAllocationRequest::setReservedDimmUid(const std::string& uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_reserveDimmUid = uid;
}

ReserveDimmType MemoryAllocationRequest::getReservedDimmCapacityType() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_reserveDimmType;
}

void MemoryAllocationRequest::setReservedDimmCapacityType(const ReserveDimmType type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_reserveDimmType = type;
}

bool MemoryAllocationRequest::hasReservedDimm() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (getReservedDimmCapacityType() != RESERVE_DIMM_NONE) &&
			!getReservedDimmUid().empty();
}

Dimm MemoryAllocationRequest::getReservedDimm() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	Dimm reservedDimm;
	bool found = false;

	for (std::vector<Dimm>::const_iterator dimmIter = m_dimms.begin();
			dimmIter != m_dimms.end(); dimmIter++)
	{
		if (isReservedDimm(*dimmIter))
		{
			reservedDimm = *dimmIter;
			found = true;
			break;
		}
	}

	if (!found)
	{
		throw NoReservedDimmException();
	}

	return reservedDimm;
}

bool MemoryAllocationRequest::isReservedDimm(const Dimm& dimm) const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return hasReservedDimm() && (dimm.uid == getReservedDimmUid());
}

std::vector<Dimm> MemoryAllocationRequest::getDimms() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_dimms;
}

size_t MemoryAllocationRequest::getNumberOfDimms() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_dimms.size();
}

void MemoryAllocationRequest::addDimm(const Dimm& dimm)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_dimms.push_back(dimm);
}

void MemoryAllocationRequest::setDimms(const std::vector<Dimm>& dimmList)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_dimms = dimmList;
}

std::vector<Dimm> MemoryAllocationRequest::getNonReservedDimms() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> nonReservedDimms;
	for (std::vector<Dimm>::const_iterator dimm = m_dimms.begin();
			dimm != m_dimms.end(); dimm++)
	{
		if (!isReservedDimm(*dimm))
		{
			nonReservedDimms.push_back(*dimm);
		}
	}

	return nonReservedDimms;
}

NVM_UINT64 MemoryAllocationRequest::getMappableDimmCapacityInBytes() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 usableCapacity = 0;
	for (std::vector<Dimm>::const_iterator dimm = m_dimms.begin();
			dimm != m_dimms.end(); dimm++)
	{
		if (!isReservedDimm(*dimm))
		{
			usableCapacity += USABLE_CAPACITY_BYTES(dimm->capacityBytes);
		}
	}

	return usableCapacity;
}

NVM_UINT64 MemoryAllocationRequest::getRequestedMappedCapacityInBytes() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 mappedCapacityBytes = 0;

	NVM_UINT64 mappedCapacityGiB = getMemoryModeCapacityGiB();
	mappedCapacityGiB += getAppDirectCapacityGiB();

	mappedCapacityBytes = mappedCapacityGiB * BYTES_PER_GIB;

	return mappedCapacityBytes;
}

} /* namespace memory_allocator */
} /* namespace core */
