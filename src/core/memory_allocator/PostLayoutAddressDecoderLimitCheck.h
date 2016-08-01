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
 * Rule that checks that the address decoder limit is not exceeded
 */

#ifndef _core_LOGIC_POSTLAYOUTOVERADDRESSDECODERLIMITCHECK_H_
#define _core_LOGIC_POSTLAYOUTOVERADDRESSDECODERLIMITCHECK_H_

#include <nvm_types.h>
#include <vector>
#include <list>
#include "PostLayoutCheck.h"

static const NVM_UINT16 criticalNumberOfSockets = 8;
static const NVM_UINT16 criticalNumberOfIlsetsOnSocket = 7;

namespace core
{
namespace memory_allocator
{

class NVM_API PostLayoutAddressDecoderLimitCheck: public PostLayoutCheck
{
	public:
		PostLayoutAddressDecoderLimitCheck(const std::vector<struct device_discovery> &devices,
				const std::vector<struct pool> &pools,
				const NVM_UINT16 numSystemSockets);
		virtual ~PostLayoutAddressDecoderLimitCheck();
		virtual void verify(
				const struct MemoryAllocationRequest &request,
				const MemoryAllocationLayout &layout);

	protected:
		/*
		 * For a given dimm, get the socketId
		 */
		NVM_UINT16 getSocketIdForDimm(NVM_UID &uid);

		/*
		 * Get a list of all the sockets involved in the layout
		 */
		std::list<NVM_UINT16> getListOfSocketsInLayout(
				const struct MemoryAllocationLayout &layout);

		/*
		 * Get a list of all config_goals for the socket in the layout
		 */
		std::vector<struct config_goal> getConfigGoalsForSocket(
				const struct MemoryAllocationLayout &layout, const NVM_UINT16 socketId);

		/*
		 * Get the number of interleave sets on a given socket in the layout
		 */
		NVM_UINT16 getNumberOfIlsetsOnSocket(
				const struct MemoryAllocationLayout &layout, const NVM_UINT16 socketId);

		NVM_UINT16 getNumberOfConfigGoalInterleaveSetsOnSocket(const struct MemoryAllocationLayout &layout,
				const NVM_UINT16 socketId);
		NVM_UINT16 getNumberOfUnchangedPoolInterleaveSetsOnSocket(const struct MemoryAllocationLayout &layout,
				const NVM_UINT16 socketId);
		NVM_UINT16 getNumberOfUnchangedInterleaveSetsInPool(const struct MemoryAllocationLayout& layout,
				const struct pool &pool);
		bool isInterleaveSetOverwrittenByLayout(const struct MemoryAllocationLayout &layout,
				const struct interleave_set &interleave);

		std::vector<struct device_discovery> m_devices;
		std::vector<struct pool> m_pools;
		NVM_UINT16 m_numSystemSockets;
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* _core_LOGIC_POSTLAYOUTOVERADDRESSDECODERLIMITCHECK_H_ */
