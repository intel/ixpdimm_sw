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

#include "MemoryAllocationRequestBuilder.h"
#include <LogEnterExit.h>
#include <set>

namespace core
{
namespace memory_allocator
{

MemoryAllocationRequestBuilder::MemoryAllocationRequestBuilder(
		core::device::DeviceService &service) :
				m_pmType(AppDirect),
				m_memoryRatio(0.0),
				m_reservedRatio(0.0),
				m_service(service)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

MemoryAllocationRequest MemoryAllocationRequestBuilder::build()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	buildRequestedDimms();
	buildMemoryCapacity();
	buildReservedCapacity();
	buildAppDirectCapacity();

	return m_result;
}

void MemoryAllocationRequestBuilder::buildRequestedDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_dimmIds.empty() && m_sockets.empty())
	{
		m_result.setDimms(getAllDimms());
	}
	else
	{
		m_result.setDimms(getRequestedDimms());
	}
}

std::vector<Dimm> MemoryAllocationRequestBuilder::getAllDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> allDimms;

	device::DeviceCollection allDevices = m_service.getAllDevices();
	for (size_t i = 0; i < allDevices.size(); i++)
	{
		if (allDevices[i].isManageable())
		{
			Dimm d = getDimmFromDevice(allDevices[i]);
			allDimms.push_back(d);
		}
	}

	return allDimms;
}

Dimm MemoryAllocationRequestBuilder::getDimmFromDevice(core::device::Device &device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	Dimm d;
	d.uid = device.getUid();
	d.channel = device.getChannelId();
	d.capacityBytes = device.getTotalCapacityBytes();
	d.memoryController = device.getMemoryControllerId();
	d.socket = device.getSocketId();

	return d;
}

std::vector<Dimm> MemoryAllocationRequestBuilder::getRequestedDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<std::string> uids = getRequestedUids();

	std::vector<Dimm> dimms;
	for (std::vector<std::string>::const_iterator uid = uids.begin();
			uid != uids.end(); uid++)
	{
		Result<device::Device> device = m_service.getDevice(*uid);
		Dimm d = getDimmFromDevice(device.getValue());
		dimms.push_back(d);
	}

	return dimms;
}

std::vector<std::string> MemoryAllocationRequestBuilder::getRequestedUids()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<std::string> uidsFromIds = getUidsFromRequestedDimmIds();
	std::vector<std::string> uidsFromSockets = getUidsFromRequestedSockets();

	std::vector<std::string> allUids;
	allUids.insert(allUids.end(), uidsFromIds.begin(), uidsFromIds.end());
	allUids.insert(allUids.end(), uidsFromSockets.begin(), uidsFromSockets.end());

	return getUniqueUidsFromList(allUids);
}

std::vector<std::string> MemoryAllocationRequestBuilder::getUidsFromRequestedDimmIds()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_service.getUidsForDeviceIds(m_dimmIds);
}

std::vector<std::string> MemoryAllocationRequestBuilder::getUidsFromRequestedSockets()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<Dimm> allDimms = getAllDimms();
	std::vector<std::string> uids;

	for (std::vector<NVM_UINT16>::const_iterator socket = m_sockets.begin();
			socket != m_sockets.end(); socket++)
	{
		for (std::vector<Dimm>::const_iterator dimm = allDimms.begin();
				dimm != allDimms.end(); dimm++)
		{
			if (dimm->socket == *socket)
			{
				uids.push_back(dimm->uid);
			}
		}
	}

	return uids;
}

std::vector<std::string> MemoryAllocationRequestBuilder::getUniqueUidsFromList(
		const std::vector<std::string>& uids)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::set<std::string> uniqueUidsSet(uids.begin(), uids.end());
	std::vector<std::string> uniqueUids(uniqueUidsSet.begin(), uniqueUidsSet.end());

	return uniqueUids;
}

void MemoryAllocationRequestBuilder::buildMemoryCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 totalCapacityBytes = getTotalCapacityBytesFromRequestDimms();
	NVM_UINT64 memoryCapacityBytes = (NVM_UINT64)(totalCapacityBytes * m_memoryRatio);
	m_result.setMemoryModeCapacityGiB(B_TO_GiB(memoryCapacityBytes));
}

NVM_UINT64 MemoryAllocationRequestBuilder::getTotalCapacityBytesFromRequestDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 totalCapacity = 0;

	std::vector<Dimm> dimms = m_result.getDimms();
	for (size_t i = 0; i < dimms.size(); i++)
	{
		totalCapacity += dimms[i].capacityBytes;
	}

	return totalCapacity;
}

void MemoryAllocationRequestBuilder::buildAppDirectCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_result.setAppDirectExtent(getAppDirectExtent());
}

AppDirectExtent MemoryAllocationRequestBuilder::getAppDirectExtent()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	AppDirectExtent extent;

	NVM_UINT64 pmCapacityGiB = getPersistentCapacityGiBFromRequest();

	if (pmCapacityGiB > 0)
	{
		extent.capacityGiB = pmCapacityGiB;
		if (m_pmType == AppDirectNoInterleave)
		{
			extent.byOne = true;
		}
	}

	return extent;
}

NVM_UINT64 MemoryAllocationRequestBuilder::getPersistentCapacityGiBFromRequest()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 persistentCapacityGiB = 0;

	NVM_UINT64 totalCapacityGiB = B_TO_GiB(getTotalCapacityBytesFromRequestDimms());

	if (totalCapacityGiB >=
			(m_result.getMemoryModeCapacityGiB() + m_result.getReservedCapacityGiB()))
	{
		persistentCapacityGiB = totalCapacityGiB -
			m_result.getMemoryModeCapacityGiB() - m_result.getReservedCapacityGiB();
	}

	return  persistentCapacityGiB;
}

void MemoryAllocationRequestBuilder::buildReservedCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 totalCapacityBytes = getTotalCapacityBytesFromRequestDimms();
	NVM_UINT64 reservedGiB =
		B_TO_GiB((NVM_UINT64)(totalCapacityBytes * m_reservedRatio));

	m_result.setReservedCapacityGiB(reservedGiB);
}

void MemoryAllocationRequestBuilder::addDimmIds(const std::vector<std::string> &dimmIds)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_dimmIds.insert(m_dimmIds.end(), dimmIds.begin(), dimmIds.end());
}

void MemoryAllocationRequestBuilder::addSocketIds(const std::vector<NVM_UINT16>& socketIds)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_sockets.insert(m_sockets.end(), socketIds.begin(), socketIds.end());
}

void MemoryAllocationRequestBuilder::setPersistentTypeAppDirectNonInterleaved()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pmType = AppDirectNoInterleave;
}

void MemoryAllocationRequestBuilder::setPersistentTypeAppDirectInterleaved()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pmType = AppDirect;
}

void MemoryAllocationRequestBuilder::setMemoryModePercentage(const NVM_UINT32 percentage)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (percentage > 100)
	{
		COMMON_LOG_ERROR_F("Invalid percentage: %u", percentage);
		throw InvalidPercentageException();
	}

	m_memoryRatio = percentage / 100.0;
}

void MemoryAllocationRequestBuilder::setReservedPercentage(const NVM_UINT32 percentage)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (percentage > 100)
	{
		COMMON_LOG_ERROR_F("Invalid percentage: %u", percentage);
		throw InvalidPercentageException();
	}

	m_reservedRatio = percentage / 100.0;
}
}
}
