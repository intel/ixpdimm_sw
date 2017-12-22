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
 * Reduce laid out capacity based on SKU mapped memory limit
 */

#ifndef _core_LOGIC_LAYOUTSTEPLIMITTOTALMAPPEDMEMORY_H_
#define _core_LOGIC_LAYOUTSTEPLIMITTOTALMAPPEDMEMORY_H_

#include <uid/uid.h>
#include <core/exceptions/LibraryException.h>
#include <core/memory_allocator/LayoutStep.h>
#include "MemoryAllocationUtil.h"
#include <core/ExportCore.h>

namespace core
{
namespace memory_allocator
{

class NVM_CORE_API LayoutStepLimitTotalMappedMemory : public LayoutStep
{
	public:
		LayoutStepLimitTotalMappedMemory(const std::vector<struct device_details> deviceDetailsList, MemoryAllocationUtil &util);
		virtual ~LayoutStepLimitTotalMappedMemory();

		virtual void execute(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);
	private:
		void initCurrentDeviceDetailsMap(const MemoryAllocationRequest& request);
		void initReqConfigGoalMap (const MemoryAllocationRequest& request,
				const MemoryAllocationLayout& layout, const std::vector<Dimm> reqDimms);
		void initPcatType6info();

		void initSocketDimms(NVM_UINT16 socketId, std::vector<Dimm> reqDimms);

		void initializeExceedsLimit();
		bool mappedSizeExceedsLimit();

		void calculateTotalMappedCapacityPerSocket( NVM_UINT16 socketId,
				std::vector<struct device_details> curDeviceDetailsVector,
				struct socket sktType6Info, std::vector<struct config_goal> reqConfigGoalVector, std::vector<Dimm> reqDimms);

		void shrinkLayoutCapacities(const MemoryAllocationRequest& request,
			MemoryAllocationLayout& layout);

		void shrinkLayoutGoals(MemoryAllocationLayout& layout);

		void shrinkAppDirect2(MemoryAllocationLayout& layout);

		void shrinkAppDirect1(MemoryAllocationLayout& layout);

		void shrinkMemory(MemoryAllocationLayout& layout);

		void shrinkSizePerDimm(NVM_UINT64 reduceBy, NVM_UINT64 &size);

		NVM_UINT64 getTotal2LMCapacity(
			const std::vector<core::memory_allocator::Dimm>& memoryDimms,
			MemoryAllocationLayout& layout);

		NVM_UINT64 m_newTotalMappedSizeOnSocketInGib;
		NVM_UINT64 m_limitOnSocketInGib;
		NVM_UINT64 m_exceededSocketLimitByInGib;
		std::vector<core::memory_allocator::Dimm> m_socketDimms;
		MemoryAllocationUtil &m_memAllocUtil;
		std::vector<struct device_details> m_DeviceDetailsList;
		std::map<NVM_UINT16, std::vector<struct device_details> > m_curSocketIdDeviceDetailsMap;
		std::map<NVM_UINT16, std::vector<struct config_goal> > m_reqSocketIdConfigGoalMap;
		std::map<NVM_UINT16, struct socket> m_curSocketInfoFromTyep6Map;
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* _core_LOGIC_LAYOUTSTEPLIMITTOTALMAPPEDMEMORY_H_ */
