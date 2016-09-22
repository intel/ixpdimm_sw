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

#include "MemoryAllocationGoalService.h"
#include <LogEnterExit.h>
#include <core/exceptions/NoMemoryException.h>
#include <core/exceptions/LibraryException.h>

namespace core
{
namespace configuration
{

MemoryAllocationGoalService::MemoryAllocationGoalService(device::DeviceService &deviceService,
		NvmLibrary &lib) : m_deviceService(deviceService), m_lib(lib)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

MemoryAllocationGoalService::~MemoryAllocationGoalService()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

MemoryAllocationGoalService& MemoryAllocationGoalService::getService()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Creating the singleton on class init as a static class member
	// can lead to static initialization order issues.
	// This is a thread-safe form of lazy initialization.
	static MemoryAllocationGoalService *pSingleton = new MemoryAllocationGoalService();
	return *pSingleton;
}

MemoryAllocationGoalCollection MemoryAllocationGoalService::getAllGoals()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	MemoryAllocationGoalCollection goals;

	std::vector<std::string> manageableUids = m_deviceService.getManageableUids();
	for (std::vector<std::string>::const_iterator uid = manageableUids.begin();
			uid != manageableUids.end(); uid++)
	{
		addGoalForDeviceToCollection(*uid, goals);
	}

	return goals;
}

void MemoryAllocationGoalService::addGoalForDeviceToCollection(const std::string& deviceUid,
		MemoryAllocationGoalCollection& collection)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		Result<MemoryAllocationGoal> goal = getGoalForDevice(deviceUid);
		collection.push_back(goal.getValue());
	}
	catch (NoGoalOnDevice &)
	{
		// Nothing to add
	}
}

Result<MemoryAllocationGoal> MemoryAllocationGoalService::getGoalForDevice(
		const std::string& deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	Result<device::Device> device = m_deviceService.getDevice(deviceUid);
	config_goal libGoal = getConfigGoalForDeviceFromLibrary(deviceUid);
	MemoryAllocationGoal goal(libGoal, device.getValue());
	return Result<MemoryAllocationGoal>(goal);
}

config_goal MemoryAllocationGoalService::getConfigGoalForDeviceFromLibrary(
		const std::string& deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	config_goal configGoal;
	try
	{
		configGoal = m_lib.getConfigGoal(deviceUid);
	}
	catch (LibraryException &e)
	{
		if (e.getErrorCode() == NVM_ERR_NOTFOUND)
		{
			throw NoGoalOnDevice();
		}
		else
		{
			throw;
		}
	}

	return configGoal;
}

MemoryAllocationGoalCollection MemoryAllocationGoalService::getGoalsFromMemoryAllocationLayout(
		const memory_allocator::MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	MemoryAllocationGoalCollection goalCollection;

	for (std::map<std::string, config_goal>::const_iterator layoutGoal = layout.goals.begin();
			layoutGoal != layout.goals.end(); layoutGoal++)
	{
		std::string deviceUid = layoutGoal->first;
		core::Result<device::Device> device = m_deviceService.getDevice(deviceUid);
		MemoryAllocationGoal goal(layoutGoal->second, device.getValue());
		goalCollection.push_back(goal);
	}

	return goalCollection;
}

} /* namespace configuration */
} /* namespace core */
