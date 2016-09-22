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

#include "ShowCommandUtilities.h"
#include <LogEnterExit.h>
#include <cli/features/core/CommandParts.h>
#include "WbemToCli_utilities.h"
#include <libinvm-cli/ErrorResult.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cli/SyntaxErrorMissingValueResult.h>
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>

namespace cli
{
namespace nvmcli
{

framework::ResultBase* ShowCommandUtilities::getInvalidDimmIdResult(
		const core::StringList &dimmIds, core::device::DeviceCollection& devices)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ResultBase *pResult = NULL;

	std::string badDimmId = getFirstBadDimmId(dimmIds, devices);
	if (!badDimmId.empty())
	{
		pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
			getInvalidDimmIdErrorString(badDimmId));
	}

	return pResult;
}

std::string ShowCommandUtilities::getFirstBadDimmId(
		const core::StringList &dimmIds, core::device::DeviceCollection& devices)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string badDimmId = "";

	for (size_t i = 0; i < dimmIds.size() && badDimmId.empty(); i++)
	{
		bool dimmIdFound = false;
		for (size_t j = 0; j < devices.size() && !dimmIdFound; j++)
		{
			if (framework::stringsIEqual(dimmIds[i], devices[j].getUid()) ||
				dimmIds[i] == uint64ToString(devices[j].getDeviceHandle()))
			{
				dimmIdFound = true;
			}
		}
		if (!dimmIdFound)
		{
			badDimmId = dimmIds[i];
		}
	}
	return badDimmId;
}

framework::ResultBase* ShowCommandUtilities::getInvalidSocketIdResult(
		const core::StringList& socketIds, core::device::DeviceCollection& devices)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ResultBase *pResult = NULL;

	std::string badSocketId = getFirstBadSocketId(socketIds, devices);
	if (!badSocketId.empty())
	{
		pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
			TARGET_SOCKET.name, badSocketId);
	}

	return pResult;
}

std::string ShowCommandUtilities::getFirstBadSocketId(const core::StringList &socketIds,
		core::device::DeviceCollection &devices)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string badsocketId = "";
	for (size_t i = 0; i < socketIds.size() && badsocketId.empty(); i++)
	{
		bool socketIdFound = false;
		for (size_t j = 0; j < devices.size() && !socketIdFound; j++)
		{
			if (socketIds[i] == uint64ToString(devices[j].getSocketId()))
			{
				socketIdFound = true;
			}
		}
		if (!socketIdFound)
		{
			badsocketId = socketIds[i];
		}
	}
	return badsocketId;
}

void ShowCommandUtilities::filterDevicesOnDimmIds(core::device::DeviceCollection& devices,
		core::StringList &dimmIds)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (dimmIds.size() > 0)
	{
		for (size_t i = devices.size(); i > 0; i--)
		{
			core::device::Device &device = devices[i - 1];

			std::string deviceHandle = uint64ToString(device.getDeviceHandle());

			if (!dimmIds.contains(device.getUid()) && !dimmIds.contains(deviceHandle))
			{
				devices.removeAt(i - 1);
			}
		}
	}
}

void ShowCommandUtilities::filterDevicesOnSocketIds(core::device::DeviceCollection& devices,
		core::StringList& socketIds)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (socketIds.size() > 0)
	{
		for (size_t i = devices.size(); i > 0; i--)
		{
			core::device::Device &device = devices[i - 1];

			std::string socketId = uint64ToString(device.getSocketId());

			if (!socketIds.contains(socketId))
			{
				devices.removeAt(i - 1);
			}
		}
	}
}

framework::ResultBase* ShowCommandUtilities::getInvalidUnitsOptionResult(
		const framework::UnitsOption& unitsOption)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ResultBase *pResult = NULL;
	if (unitsOption.isEmpty())
	{
		pResult = new framework::SyntaxErrorMissingValueResult(framework::TOKENTYPE_OPTION,
				framework::OPTION_UNITS.name);
	}
	else if (!unitsOption.isValid())
	{
		pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_OPTION,
				framework::OPTION_UNITS.name, unitsOption.getCapacityUnits());
	}

	return pResult;
}

std::string ShowCommandUtilities::getDimmId(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return getDimmIdFromDeviceUidAndHandle(device.getUid(), device.getDeviceHandle());
}

std::string ShowCommandUtilities::getDimmIdFromDeviceUidAndHandle(const std::string& uid,
		const NVM_UINT32 handle)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream result;
	bool useHandle = true;
	char value[CONFIG_VALUE_LEN];
	if (get_config_value(SQL_KEY_CLI_DIMM_ID, value) == COMMON_SUCCESS)
	{
		// switch to uid
		if (s_strncmpi("UID", value, strlen("UID")) == 0)
		{
			useHandle = false;
		}
	}

	if (useHandle)
	{
		result << handle;
	}
	else
	{
		result << uid;
	}
	return result.str();
}

std::string ShowCommandUtilities::getFormattedEvent(const event &event)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream eventMsg;
	eventMsg << TR("Event") << " " << event.event_id;

	size_t msgLen = NVM_EVENT_MSG_LEN + (3 * NVM_EVENT_ARG_LEN);
	char msg[msgLen];
	s_snprintf(msg, msgLen,
		event.message,
		event.args[0],
		event.args[1],
		event.args[2]);
	eventMsg << " - " << msg;

	return eventMsg.str();
}

std::string ShowCommandUtilities::getFormattedEventList(const std::vector<event> &events)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream eventList;
	for (std::vector<event>::const_iterator eventIter = events.begin();
			eventIter != events.end(); eventIter++)
	{
		// Not the first item
		if (eventIter != events.begin())
		{
			eventList << ", ";
		}
		eventList << getFormattedEvent(*eventIter);
	}

	return eventList.str();
}

}
}
