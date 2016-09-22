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

#ifndef MEMORYALLOCATIONGOAL_H_
#define MEMORYALLOCATIONGOAL_H_

#include <nvm_types.h>
#include <nvm_management.h>
#include <core/device/Device.h>
#include <core/NvmLibrary.h>
#include <vector>

namespace core
{
namespace configuration
{

class NVM_API MemoryAllocationGoal
{
	public:
		MemoryAllocationGoal();
		MemoryAllocationGoal(const config_goal &goal, device::Device &device,
				NvmLibrary &lib = NvmLibrary::getNvmLibrary());
		virtual ~MemoryAllocationGoal();

		MemoryAllocationGoal &operator=(const MemoryAllocationGoal &other);
		virtual MemoryAllocationGoal *clone() const;

		virtual std::string getDeviceUid();
		virtual NVM_UINT32 getDeviceHandle();
		virtual NVM_UINT16 getDeviceSocketId();

		virtual NVM_UINT64 getMemorySizeInBytes();
		virtual NVM_UINT64 getStorageSizeInBytes();

		virtual NVM_UINT16 getAppDirectCount();

		virtual bool hasAppDirect1();
		virtual NVM_UINT64 getAppDirect1SizeInBytes();
		virtual NVM_UINT16 getAppDirect1Id();
		virtual NVM_UINT16 getAppDirect1InterleaveWay();
		virtual interleave_size getAppDirect1ChannelInterleave();
		virtual interleave_size getAppDirect1MemoryControllerInterleave();

		virtual bool hasAppDirect2();
		virtual NVM_UINT64 getAppDirect2SizeInBytes();
		virtual NVM_UINT16 getAppDirect2Id();
		virtual NVM_UINT16 getAppDirect2InterleaveWay();
		virtual interleave_size getAppDirect2ChannelInterleave();
		virtual interleave_size getAppDirect2MemoryControllerInterleave();

		virtual config_goal_status getStatus();

		virtual bool isActionRequired();
		virtual std::vector<event> getActionRequiredEvents();

	private:
		NvmLibrary &m_lib;

		std::string m_deviceUid;
		NVM_UINT32 m_deviceHandle;
		NVM_UINT16 m_deviceSocketId;
		NVM_UINT64 m_deviceCapacityBytes;
		config_goal m_goal;

		NVM_UINT64 getGoalPersistentCapacityInBytes();
		NVM_UINT64 getDeviceCapacityInBytes();
		bool hasMemoryModePartition();
		NVM_UINT64 getGoalAppDirectCapacityInBytes();
		event_filter getActionRequiredEventFilter();
};

} /* namespace configuration */
} /* namespace core */

#endif /* MEMORYALLOCATIONGOAL_H_ */
