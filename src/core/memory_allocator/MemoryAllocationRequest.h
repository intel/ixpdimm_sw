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

#ifndef MEMORYALLOCATIONREQUEST_H_
#define MEMORYALLOCATIONREQUEST_H_

#include <vector>
#include <string>
#include <exception>
#include <nvm_types.h>
#include <core/memory_allocator/MemoryAllocationTypes.h>

namespace core
{
namespace memory_allocator
{

struct AppDirectExtent
{
	AppDirectExtent() : capacityGiB(0), mirrored(false), byOne(false),
			channel(REQUEST_DEFAULT_INTERLEAVE_FORMAT),
			imc(REQUEST_DEFAULT_INTERLEAVE_FORMAT) {}

	NVM_UINT64 capacityGiB; // total in GiB
	bool mirrored;
	bool byOne;
	int channel; // channel interleave size
	int imc; // memory controller interleave size
};

enum ReserveDimmType
{
	RESERVE_DIMM_NONE,
	RESERVE_DIMM_STORAGE,
	RESERVE_DIMM_APP_DIRECT_X1
};

class NVM_API MemoryAllocationRequest
{
	public:
		MemoryAllocationRequest();
		virtual ~MemoryAllocationRequest();

		class NoReservedDimmException : public std::exception {};

		/*
		 * All requested capacity is in GiB.
		 */

		NVM_UINT64 getMemoryModeCapacityGiB() const;
		void setMemoryModeCapacityGiB(const NVM_UINT64 capacity);

		NVM_UINT64 getAppDirectCapacityGiB() const;
		AppDirectExtent getAppDirectExtent() const;
		void setAppDirectExtent(const AppDirectExtent &extent);

		bool isStorageRemaining() const;
		void setStorageRemaining(const bool storageIsRemaining);
		bool hasStorage() const;

		NVM_UINT64 getReserveStorageCapacityGiB() const;
		void setReserveStorageCapacityGiB(const NVM_UINT64 capacity);

		std::string getReservedDimmUid() const;
		void setReservedDimmUid(const std::string &uid);
		ReserveDimmType getReservedDimmCapacityType() const;
		void setReservedDimmCapacityType(const ReserveDimmType type);
		bool hasReservedDimm() const;
		Dimm getReservedDimm() const;

		std::vector<Dimm> getDimms() const;
		size_t getNumberOfDimms() const;
		void addDimm(const Dimm &dimm);
		void setDimms(const std::vector<Dimm> &dimmList);
		std::vector<Dimm> getNonReservedDimms() const;

		NVM_UINT64 getAllMappableNonReservedCapacity() const;
		NVM_UINT64 getAllMappableDimmCapacityInGiB() const;
		NVM_UINT64 getRequestedMappedCapacityInBytes() const;

		// TODO: Update this function and move it to utils when nvm_get_socket
		// is updated to get mapped memory limit (US20271)
		virtual NVM_UINT64 getSocketLimit() const;

	private:
		NVM_UINT64 m_memoryCapacityGiB;
		AppDirectExtent m_appDirectExtent;
		bool m_storageRemaining;
		NVM_UINT64 m_reserveStorageCapacityGiB;

		std::string m_reserveDimmUid;
		ReserveDimmType m_reserveDimmType;

		std::vector<Dimm> m_dimms;

		bool isReservedDimm(const Dimm &dimm) const;
		bool isReservedAppDirectByOneDimm(const Dimm &dimm) const;
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* MEMORYALLOCATIONREQUEST_H_ */
