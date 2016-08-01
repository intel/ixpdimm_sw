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
 * Lay out the app direct region in memory.
 */

#ifndef _core_LOGIC_LAYOUTSTEPAPPDIRECT_H_
#define _core_LOGIC_LAYOUTSTEPAPPDIRECT_H_

#include <nvm_types.h>
#include <core/NvmLibrary.h>
#include "LayoutStep.h"

namespace core
{
namespace memory_allocator
{

class NVM_API LayoutStepAppDirect: public LayoutStep
{
	public:
		LayoutStepAppDirect(const struct nvm_capabilities &cap,
				const int appDirectExtentIndex,
				core::NvmLibrary &nvmLib);
		virtual ~LayoutStepAppDirect();

		virtual void execute(const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);

		virtual bool isRemainingStep(const MemoryAllocationRequest &request);

	protected:
		NVM_UINT64 getRequestedCapacityBytes(
				const NVM_UINT64 &requestedCapacity,
				const struct MemoryAllocationRequest& request,
				MemoryAllocationLayout &layout);
		NVM_UINT64 layoutByOneAd(
				const NVM_UINT64 &bytesToAllocate,
				const std::vector<Dimm> &dimms,
				const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);
		NVM_UINT64 layoutInterleavedAd(
				const NVM_UINT64 &bytesToAllocate,
				const std::vector<Dimm> &dimms,
				const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);
		NVM_UINT64 layoutInterleavedAdAcrossSocket(
				const NVM_UINT64 &bytesPerSocket,
				const std::vector<Dimm> &dimms,
				const MemoryAllocationRequest& request,
				MemoryAllocationLayout& layout);
		NVM_UINT64 layoutInterleaveSet(
				const NVM_UINT64 &bytesPerDimm,
				const std::vector<Dimm> &dimms,
				const MemoryAllocationRequest& request,
				MemoryAllocationLayout& layout);
		NVM_UINT64 getBestInterleaveBytesPerDimm(
				const std::vector<Dimm> &requestedDimms,
				std::map<std::string, struct config_goal> &goals,
				const NVM_UINT64 &requestedBytes,
				const MemoryAllocationRequest &request,
				std::vector<Dimm> &dimmsIncluded);
		bool canMapInterleavedCapacity(
				const std::vector<Dimm> &dimms,
				std::map<std::string, struct config_goal> &goals,
				const MemoryAllocationRequest &request);
		NVM_UINT64 getDimmUnallocatedAppDirectBytes(
				const struct Dimm &dimm, const struct config_goal &goal);
		NVM_UINT64 getRemainingAppDirectBytesFromDimms(
				const MemoryAllocationRequest &request,
				MemoryAllocationLayout &layout);
		bool isValidWay(const size_t &numDimms);
		enum interleave_ways getInterleaveWay(const size_t &numDimms);
		enum interleave_size getInterleaveChannelSize(
				const MemoryAllocationRequest &request, const size_t &numDimms);
		enum interleave_size getInterleaveImcSize(
				const MemoryAllocationRequest &request, const size_t &numDimms);
		void addInterleaveSetToGoal(const std::string &dimmUid,
				struct config_goal &goal, const NVM_UINT64 &sizeBytes,
				const NVM_UINT16 &setId, const struct app_direct_attributes &settings);
		bool interleaveSetFromPreviousExtent(
				const std::string &dimmUid,
				struct config_goal &goal);
		bool interleaveSetsMatch(const struct app_direct_attributes &oldSet, const struct app_direct_attributes &newSet);
		int getDimmPopulationMap(const std::vector<Dimm> &requestedDimms,
						std::map<std::string, struct config_goal> &goals);
		bool dimmPopulationMatchesInterleaveSet(const int &dimmMap, const int &interleaveSet);
		std::vector<Dimm> getDimmsMatchingInterleaveSet(const int &interleaveSet,
				const std::vector<Dimm> &requestedDimms);
		NVM_UINT64 removeEmptySockets(std::map<NVM_UINT16, std::vector<Dimm> > &sockets,
				const NVM_UINT64 &bytesToAllocate,
				const MemoryAllocationRequest& request,
				MemoryAllocationLayout& layout);
		std::vector<Dimm> getDimmsWithCapacity(const std::vector<Dimm> &dimms,
				MemoryAllocationLayout& layout);
		std::vector<Dimm> getRemainingDimms(const std::vector<Dimm> &dimms,
				const std::vector<Dimm> &dimmsIncluded);

		struct nvm_capabilities m_systemCap;
		unsigned int m_adExtentIndex;
		core::NvmLibrary &m_nvmLib;
		std::map<std::string, int> m_dimmExistingSets; // dimm UID to App Direct count map
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* _core_LOGIC_LAYOUTSTEPAPPDIRECT_H_ */
