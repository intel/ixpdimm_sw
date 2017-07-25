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
#ifndef CR_MGMT_MEMORYALLOCATIONREQUESTBUILDER_H
#define CR_MGMT_MEMORYALLOCATIONREQUESTBUILDER_H

#include <exception>
#include <vector>
#include <string>
#include <nvm_types.h>
#include <core/device/DeviceService.h>
#include "MemoryAllocationRequest.h"

namespace core
{
namespace memory_allocator
{

class NVM_API MemoryAllocationRequestBuilder
{
public:
	MemoryAllocationRequestBuilder(core::device::DeviceService &service =
			core::device::DeviceService::getService());

	class InvalidPercentageException : public std::exception {};

	virtual MemoryAllocationRequest build();

	void setPersistentTypeAppDirectNonInterleaved();
	void setPersistentTypeAppDirectInterleaved();

	void setMemoryModePercentage(const NVM_UINT32 percentage);

	void setReservedPercentage(const NVM_UINT32 percentage);

	void addDimmIds(const std::vector<std::string> &dimmIds);
	void addSocketIds(const std::vector<NVM_UINT16> &socketIds);

protected:
	enum PersistentType
	{
		AppDirect = 0,
		AppDirectNoInterleave = 1,
	};

	std::vector<std::string> m_dimmIds;
	std::vector<NVM_UINT16> m_sockets;
	PersistentType m_pmType;
	float m_memoryRatio;
	float m_reservedRatio;

private:
	core::device::DeviceService &m_service;
	MemoryAllocationRequest m_result;

	void buildRequestedDimms();
	void buildMemoryCapacity();
	void buildAppDirectCapacity();
	void buildReservedCapacity();

	std::vector<Dimm> getAllDimms();
	std::vector<Dimm> getRequestedDimms();
	std::vector<std::string> getRequestedUids();
	std::vector<std::string> getUniqueUidsFromList(const std::vector<std::string> &uids);
	std::vector<std::string> getUidsFromRequestedDimmIds();
	std::vector<std::string> getUidsFromRequestedSockets();
	Dimm getDimmFromDevice(core::device::Device &device);
	NVM_UINT64 getTotalCapacityBytesFromRequestDimms();
	NVM_UINT64 getPersistentCapacityGiBFromRequest();
	AppDirectExtent getAppDirectExtent();
};

}
}
#endif //CR_MGMT_MEMORYALLOCATIONREQUESTBUILDER_H
