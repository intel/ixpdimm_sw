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

#ifndef FORMATDEVICECOMMAND_H_
#define FORMATDEVICECOMMAND_H_

#include <nvm_types.h>
#include <string>
#include <map>
#include <cli/features/core/StringList.h>
#include <cli/features/core/framework/CommandBase.h>
#include <cli/features/core/framework/YesNoPrompt.h>
#include <libinvm-cli/CommandSpec.h>
#include <libinvm-cli/SimpleListResult.h>
#include <core/device/DeviceService.h>
#include <core/device/FormatDeviceService.h>
#include <cli/features/ExportCli.h>

namespace cli
{
namespace nvmcli
{

class NVM_CLI_API FormatDeviceCommand : public framework::CommandBase
{
	public:

		class NVM_CLI_API UserPrompt
		{
			public:
				UserPrompt(const framework::YesNoPrompt &prompt);
				virtual ~UserPrompt() {}

				virtual bool promptUserToFormatDevice(core::device::Device &device);

			private:
				const framework::YesNoPrompt &m_prompt;
		};

		FormatDeviceCommand(UserPrompt &prompt,
				core::device::DeviceService &deviceService = core::device::DeviceService::getService(),
				core::device::FormatDeviceService &formatService = core::device::FormatDeviceService::getService());
		virtual ~FormatDeviceCommand();

		static framework::CommandSpec getCommandSpec(int commandId);

		virtual framework::ResultBase *execute(const framework::ParsedCommand &parsedCommand);

	protected:
		virtual void parseDevices();
		virtual void filterDevices();
		virtual void filterManageableDevices();
		virtual bool dimmIdsAreValid();
		virtual void filterDevicesOnDimmIds();
		virtual void errorIfNoDevicesRemain();

		virtual bool hasError();

		virtual void setErrorResult(const std::string &errorMessage);

		virtual void formatDevices();
		virtual bool hasForceOption();
		virtual void startFormatForDevice(core::device::Device &device);
		virtual void insertSuccessResultForDevice(framework::SimpleListResult *pResults, core::device::Device &device);
		virtual void insertErrorResultForDevice(framework::SimpleListResult *pResults, core::device::Device &device);
		virtual void insertNonFormattableDimmMessage(framework::SimpleListResult *pResults, core::device::Device& device, const std::string &errorMessage);
		virtual std::string getErrorMessage(const int libError);
        virtual std::string getMessagePrefix(core::device::Device &device);

		virtual void waitUntilFormatCompleteForAllDimms();
		virtual bool isFormatCompleteForAllDimms();
		virtual void sleep(const NVM_UINT32 seconds);

		virtual void addPowerCycleNoticeToResultList();

		UserPrompt &m_prompt;
		core::device::DeviceService &m_deviceService;
		core::device::FormatDeviceService &m_formatService;
		core::device::DeviceCollection m_devices;

		core::StringList m_dimmIds;
		bool m_needsPowerCycle;
		std::map<NVM_UINT32, bool> m_waitingForCompletion;
};

} /* namespace nvmcli */
} /* namespace cli */

#endif /* FORMATDEVICECOMMAND_H_ */
