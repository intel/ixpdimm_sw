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

#ifndef SHOWCOMMANDUTILITIES_H_
#define SHOWCOMMANDUTILITIES_H_

#include <nvm_types.h>
#include <libinvm-cli/ResultBase.h>
#include <libinvm-cli/ErrorResult.h>
#include <core/device/Device.h>
#include <cli/features/core/StringList.h>
#include "framework/UnitsOption.h"
#include <persistence/lib_persistence.h>
#include <persistence/config_settings.h>
#include <cli/features/ExportCli.h>

namespace cli
{
namespace nvmcli
{

class NVM_CLI_API ShowCommandUtilities
{
	public:
		ShowCommandUtilities() {}
		virtual ~ShowCommandUtilities() {}

		static framework::ResultBase *getInvalidDimmIdResult(const core::StringList &dimmIds,
				core::device::DeviceCollection &devices);
		static framework::ResultBase *getInvalidSocketIdResult(const core::StringList &socketIds,
				core::device::DeviceCollection &devices);
		static void filterDevicesOnDimmIds(core::device::DeviceCollection &devices,
				core::StringList &dimmIds);
		static void filterDevicesOnSocketIds(core::device::DeviceCollection &devices,
				core::StringList &socketIds);
		static framework::ResultBase *getInvalidUnitsOptionResult(const framework::UnitsOption &unitsOption);

		static std::string getHexFormatFromDeviceHandle(const NVM_UINT32 device_handle);
		static std::string getDimmId(core::device::Device &device);
		static std::string getDimmIdFromDeviceUidAndHandle(const std::string &uid, const NVM_UINT32 handle);
		static std::string getFormattedEvent(const event &event);
		static std::string getFormattedEventList(const std::vector<event> &events);

		static void setUserPreferenceDimmIdToHandle();
		static void setUserPreferenceDimmIdToUid();
		static bool isUserPreferenceDimmIdUid();
		static std::vector<core::device::Device> populateDevicesFromDimmsString(
				std::string dimms_string, bool populate_all_device_properties);
		static std::vector<std::string> split(const std::string &s, char delim);
		template<typename Out>
		static void split(const std::string &s, char delim, Out result);
		static std::string listToString(std::vector<std::string> list);
		static const std::size_t min_hex_id_length = 6;
		static bool isUid(std::string id);

	protected:
		static std::string getFirstBadDimmId(const core::StringList &dimmIds,
				core::device::DeviceCollection &devices);
		static std::string getFirstBadSocketId(const core::StringList &socketIds,
				core::device::DeviceCollection &devices);
};

}
}

#endif /* SHOWCOMMANDUTILITIES_H_ */
