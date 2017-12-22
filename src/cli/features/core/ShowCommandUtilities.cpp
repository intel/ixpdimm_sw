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
#include <exception/NvmExceptionLibError.h>
#include <exception/NvmExceptionBadTarget.h>
#include <exception/NvmExceptionNotManageable.h>
#include <cli/features/core/StringList.h>
#include <memory>

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
		std::string dimmId = dimmIds[i];
		bool dimmIdFound = false;
		bool isHandle = cli::nvmcli::isStringValidNumber(dimmId);
		bool isValidUid = isUid(dimmId);
		for (size_t j = 0; j < devices.size() && !dimmIdFound; j++)
		{
			if ((isValidUid && framework::stringsIEqual(dimmIds[i], devices[j].getUid())) ||
					(isHandle && (NVM_UINT32)stringToUInt64(dimmIds[i]) == devices[j].getDeviceHandle()))
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

// TODO: Temporarily moved the functionality to core to support PoolViewFactory
std::string ShowCommandUtilities::getFormattedEvent(const event& event)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return core::Helper::getFormattedEvent(event);
}

// TODO: Temporarily moved the functionality to core to support PoolViewFactory
std::string ShowCommandUtilities::getFormattedEventList(
		const std::vector<event>& events)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return core::Helper::getFormattedEventList(events);
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
		std::vector<NVM_UINT32> input_handles;
		for (size_t i = 0; i < dimmIds.size(); i++)
		{
			input_handles.push_back((NVM_UINT32)stringToUInt64(dimmIds[i]));
		}

		for (size_t i = devices.size(); i > 0; i--)
		{
			core::device::Device &device = devices[i - 1];
			NVM_UINT32 expected_handle = device.getDeviceHandle();
			std::string expected_uid = device.getUid();

			bool device_found = false;
			for (size_t j = 0; j < dimmIds.size(); j++)
			{
				std::string dimmId = dimmIds[j];
				NVM_UINT32 input_handle = (NVM_UINT32)stringToUInt64(dimmId);
				bool isHandle = cli::nvmcli::isStringValidNumber(dimmId);
				bool isValidUid = isUid(dimmId);
				if ((isHandle && (input_handle == expected_handle))
						|| (isValidUid && (dimmIds[j] == expected_uid)))
				{
					device_found = true;
					break;
				}
			}

			if (!device_found)
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

std::string ShowCommandUtilities::getHexFormatFromDeviceHandle(const NVM_UINT32 device_handle)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	char output_handle[NVM_MAX_DIMMID_STR_LEN];
	snprintf(output_handle, NVM_MAX_DIMMID_STR_LEN, "0x%04X", device_handle);

	return output_handle;
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
	bool useHandle = !ShowCommandUtilities::isUserPreferenceDimmIdUid();
	if (useHandle)
	{
		result << getHexFormatFromDeviceHandle(handle);
	}
	else
	{
		result << uid;
	}

	return result.str();
}


template<typename Out>
void ShowCommandUtilities::split(const std::string &s, char delim, Out result) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> ShowCommandUtilities::split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

std::string ShowCommandUtilities::listToString(std::vector<std::string> list)
{
	std::ostringstream final_str;
	int list_cnt = list.size();

	for (int i = 0; i < list_cnt; ++i)
	{
		final_str << list[i];
		if (i < list_cnt - 1)
			final_str << ", ";
	}
	return final_str.str();
}

// Is the user preferences setting for the printout style for the dimm identifier
// set to UID? (If no, it's set to the default of HANDLE)
bool ShowCommandUtilities::isUserPreferenceDimmIdUid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	char value[CONFIG_VALUE_LEN];
	int rc;
	if ((rc = get_config_value(SQL_KEY_CLI_DIMM_ID, value)) == COMMON_SUCCESS)
	{
		if (s_strncmpi("UID", value, strlen("UID")) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		throw wbem::exception::NvmExceptionLibError(rc);
	}
}



bool ShowCommandUtilities::isUid(std::string id)
{
	return id.find('-') != std::string::npos;
}

/*
 * Given a string of comma-separated dimm identifiers (UID or handle), initialize
 * and return the corresponding C++ Devices.
 *
 * populate_all_device_properties indicates whether to call nvm_get_devices (TRUE)
 * or nvm_get_devices_nfit (FALSE) for the device discovery data.
 *
 * Returns a vector of Devices
 */
std::vector<core::device::Device> ShowCommandUtilities::populateDevicesFromDimmsString(
		std::string dimms_string, bool populate_all_device_properties)
{

	//discover device topology
	int dev_count = nvm_get_device_count();
	if (dev_count < 0)
	{
		throw wbem::exception::NvmExceptionLibError(dev_count);
	}
    std::unique_ptr<struct device_discovery[]> devices(new device_discovery[dev_count]);
	memset(devices.get(), 0, dev_count * sizeof(struct device_discovery));

	std::vector<core::device::Device> devs;
	core::NvmLibrary &lib = core::NvmLibrary::getNvmLibrary();
	std::vector<std::string> dimm_strings = split(dimms_string, ',');

	bool dimms_string_contains_uid = 0;
	int rc;

	// Determine if a dimm UID or a handle is used as a device identifier.
	// A handle doesn't need the additional DSM calls that generating a UID
	// requires
	std::vector<std::string>::const_iterator id;
	for (id = dimm_strings.begin(); id != dimm_strings.end(); ++id)
	{
		// Determine if there are any UIDs in the list
		if (ShowCommandUtilities::isUid(*id))
		{
			dimms_string_contains_uid = 1;
			break;
		}
	}

	if (dimms_string_contains_uid || populate_all_device_properties ||
			ShowCommandUtilities::isUserPreferenceDimmIdUid())
	{
		rc = nvm_get_devices(devices.get(), dev_count);
	}
	else
	{
		rc = nvm_get_devices_nfit(devices.get(), dev_count);
	}
	if (rc < NVM_SUCCESS)
	{
		throw wbem::exception::NvmExceptionLibError(rc);
	}


	// Return all dimms if none are specified
	if (dimm_strings.empty())
	{
		for (int i = 0; i < dev_count; i++)
		{
			core::device::Device dev(lib, devices[i]);
			devs.push_back(dev);
		}
		return devs;
	}

	// Get the handle from each of the identified dimms, whether they used
	// UID or handle.
	// We have enough properties to identify a UID because of the
	// pre-parsing step using dimmsStringContainsUid above.
	NVM_UINT32 handle;
	bool idIsUid;
	int i;
	for (id = dimm_strings.begin();	id != dimm_strings.end(); ++id)
	{
		// Maybe save some work by pre-computing these values
		idIsUid = ShowCommandUtilities::isUid(*id);
		handle = (!idIsUid) ? (NVM_UINT32)stringToUInt64(*id) : 0;

		// Compare ids as appropriate
		for (i = 0; i < dev_count; i++)
		{
			// Match the handle or UID
			if ((!idIsUid && (handle == devices[i].device_handle.handle)) ||
				(idIsUid && ((*id).compare(devices[i].uid) == 0)))
			{
				// We matched a handle or uid, create a C++ device
				core::device::Device dev(lib, devices[i]);

				// Raise an exception if a specified dimm is unmanageable
				// Note: In the other cases, we skip over the dimm
				if (!dev.isManageable())
				{
					throw wbem::exception::NvmExceptionNotManageable((*id).c_str());
				}

				devs.push_back(dev);
				break;
			}
		}
		// If we never found a matching UID / handle for the ID, notify the user
		if (i == dev_count)
		{
			// Will get converted to a better-named error by NvmExceptionToResult
			throw wbem::exception::NvmExceptionBadTarget("-dimm", (*id).c_str());
		}

	}
	return devs;
}

}
}
