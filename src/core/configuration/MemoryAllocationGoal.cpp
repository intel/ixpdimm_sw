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

#include "MemoryAllocationGoal.h"
#include <LogEnterExit.h>
#include <utility.h>

namespace core
{
namespace configuration
{

MemoryAllocationGoal::MemoryAllocationGoal() :
		m_lib(NvmLibrary::getNvmLibrary()),
		m_deviceUid(),
		m_deviceHandle(0),
		m_deviceSocketId(0),
		m_deviceCapacityBytes(0)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	memset(&m_goal, 0, sizeof (config_goal));
}

MemoryAllocationGoal::MemoryAllocationGoal(const config_goal &goal,
		device::Device &device,
		NvmLibrary &lib) :
				m_lib(lib),
				m_deviceUid(device.getUid()),
				m_deviceHandle(device.getDeviceHandle()),
				m_deviceSocketId(device.getSocketId()),
				m_deviceCapacityBytes(device.getRawCapacity()),
				m_goal(goal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

MemoryAllocationGoal::~MemoryAllocationGoal()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

MemoryAllocationGoal& MemoryAllocationGoal::operator=(const MemoryAllocationGoal& other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (this != &other)
	{
		this->m_lib = other.m_lib;
		this->m_deviceUid = other.m_deviceUid;
		this->m_deviceHandle = other.m_deviceHandle;
		this->m_deviceSocketId = other.m_deviceSocketId;
		this->m_deviceCapacityBytes = other.m_deviceCapacityBytes;
		this->m_goal = other.m_goal;
	}

	return *this;
}

MemoryAllocationGoal* MemoryAllocationGoal::clone() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return new MemoryAllocationGoal(*this);
}

std::string MemoryAllocationGoal::getDeviceUid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_deviceUid;
}

NVM_UINT32 MemoryAllocationGoal::getDeviceHandle()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_deviceHandle;
}

NVM_UINT16 MemoryAllocationGoal::getDeviceSocketId()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_deviceSocketId;
}

NVM_UINT64 MemoryAllocationGoal::getMemorySizeInBytes()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return configGoalSizeToBytes(m_goal.memory_size);
}

NVM_UINT64 MemoryAllocationGoal::getStorageSizeInBytes()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 storageBytes = getGoalPersistentCapacityInBytes();
	if (getGoalAppDirectCapacityInBytes() <= storageBytes)
	{
		storageBytes -= getGoalAppDirectCapacityInBytes();
	}
	else
	{
		storageBytes = 0;
	}

	return storageBytes;
}

NVM_UINT64 MemoryAllocationGoal::getGoalPersistentCapacityInBytes()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 persistentBytes = 0;
	if (hasMemoryModePartition())
	{
		// Memory Mode partition absorbs capacity not aligned to 1 GiB
		NVM_UINT64 alignedDeviceCapacity = USABLE_CAPACITY_BYTES(getDeviceCapacityInBytes());
		if (getMemorySizeInBytes() <= alignedDeviceCapacity)
		{
			persistentBytes = alignedDeviceCapacity - getMemorySizeInBytes();
		}
	}
	else
	{
		persistentBytes = getDeviceCapacityInBytes();
	}

	return persistentBytes;
}

bool MemoryAllocationGoal::hasMemoryModePartition()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (getMemorySizeInBytes() > 0);
}

NVM_UINT64 MemoryAllocationGoal::getDeviceCapacityInBytes()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_deviceCapacityBytes;
}

NVM_UINT64 MemoryAllocationGoal::getGoalAppDirectCapacityInBytes()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return getAppDirect1SizeInBytes() + getAppDirect2SizeInBytes();
}

NVM_UINT16 MemoryAllocationGoal::getAppDirectCount()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.app_direct_count;
}

bool MemoryAllocationGoal::hasAppDirect1()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (getAppDirectCount() > 0);
}

NVM_UINT64 MemoryAllocationGoal::getAppDirect1SizeInBytes()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	if (hasAppDirect1())
	{
		bytes = configGoalSizeToBytes(m_goal.app_direct_1_size);
	}

	return bytes;
}

NVM_UINT16 MemoryAllocationGoal::getAppDirect1Id()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.app_direct_1_set_id;
}

NVM_UINT16 MemoryAllocationGoal::getAppDirect1InterleaveWay()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.app_direct_1_settings.interleave.ways;
}

interleave_size MemoryAllocationGoal::getAppDirect1ChannelInterleave()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.app_direct_1_settings.interleave.channel;
}

interleave_size MemoryAllocationGoal::getAppDirect1MemoryControllerInterleave()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.app_direct_1_settings.interleave.imc;
}


bool MemoryAllocationGoal::hasAppDirect2()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (getAppDirectCount() > 1);
}

NVM_UINT64 MemoryAllocationGoal::getAppDirect2SizeInBytes()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	if (hasAppDirect2())
	{
		bytes = configGoalSizeToBytes(m_goal.app_direct_2_size);
	}

	return bytes;
}

NVM_UINT16 MemoryAllocationGoal::getAppDirect2Id()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.app_direct_2_set_id;
}

NVM_UINT16 MemoryAllocationGoal::getAppDirect2InterleaveWay()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.app_direct_2_settings.interleave.ways;
}

interleave_size MemoryAllocationGoal::getAppDirect2ChannelInterleave()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.app_direct_2_settings.interleave.channel;
}

interleave_size MemoryAllocationGoal::getAppDirect2MemoryControllerInterleave()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.app_direct_2_settings.interleave.imc;
}

config_goal_status MemoryAllocationGoal::getStatus()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_goal.status;
}

bool MemoryAllocationGoal::isActionRequired()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	int actionRequiredCount = 0;
	try
	{
		event_filter filter = getActionRequiredEventFilter();
		actionRequiredCount = m_lib.getEventCount(filter);
	}
	catch (core::LibraryException &)
	{
		// Ignore errors
	}

	return actionRequiredCount > 0;
}

event_filter MemoryAllocationGoal::getActionRequiredEventFilter()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	event_filter configEventFilter;
	memset(&configEventFilter, 0, sizeof (configEventFilter));

	configEventFilter.filter_mask |= NVM_FILTER_ON_AR;
	configEventFilter.action_required = true;

	configEventFilter.filter_mask |= NVM_FILTER_ON_UID;
	Helper::stringToUid(getDeviceUid(), configEventFilter.uid);

	configEventFilter.filter_mask |= NVM_FILTER_ON_TYPE;
	configEventFilter.type = EVENT_TYPE_CONFIG;

	return configEventFilter;
}

std::vector<event> MemoryAllocationGoal::getActionRequiredEvents()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::vector<event> actionRequiredEvents;
	try
	{
		event_filter filter = getActionRequiredEventFilter();
		actionRequiredEvents = m_lib.getEvents(filter);
	}
	catch (core::LibraryException &)
	{
		// Ignore errors
	}

	return actionRequiredEvents;
}

} /* namespace configuration */
} /* namespace core */
