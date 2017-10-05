/*
 * Copyright (c) 2017, Intel Corporation
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

#ifndef MODIFYDEVICECOMMAND_H_
#define MODIFYDEVICECOMMAND_H_

#include <cli/features/core/framework/CommandBase.h>
#include <libinvm-cli/CommandSpec.h>
#include <cli/features/core/framework/YesNoPrompt.h>
#include <core/device/DeviceService.h>
#include <core/StringList.h>

namespace cli
{
namespace nvmcli
{

static const std::string FIRSTFASTREFRESH_PROPERTYNAME = "FirstFastRefresh";
static const std::string VIRALPOLICY_PROPERTYNAME = "ViralPolicy";

class ModifyDeviceCommand : public framework::CommandBase
{
	public:
		ModifyDeviceCommand(cli::framework::YesNoPrompt &prompt,
				core::device::DeviceService &deviceService = core::device::DeviceService::getService());
		virtual ~ModifyDeviceCommand();

		static framework::CommandSpec getCommandSpec(int id);

		virtual framework::ResultBase *execute(const framework::ParsedCommand &parsedCommand);

	private:
		cli::framework::YesNoPrompt &m_prompt;
		core::device::DeviceService &m_deviceService;
		core::device::DeviceCollection m_devices;

		bool m_forceOption;

		bool m_firstFastRefreshPropertyExists;
		bool m_firstFastRefreshPropValue;
		bool m_viralPolicyPropertyExists;
		bool m_viralPolicyPropValue;

		core::StringList m_dimmIds;

		bool hasError();

		void parseForceOption();

		void parseSettings();
		void checkAtLeastOnePropertyExists();
		void parseFirstFastRefresh();
		void parseViralPolicy();
		bool parseBooleanPropertyValue(const std::string &propertyName,
				const std::string &propertyValue);

		void parseDevices();
		void parseDimmIds();
		void filterDevices();
		void filterManageableDimms();
		void checkAtLeastOneDimmStillInList();
		bool dimmIdsAreValid();

		void modifyDevices();
		bool shouldModifyDevice(core::device::Device &device);
		void modifyDevice(core::device::Device &device);
		std::string getPromptMessageForDevice(core::device::Device &device);
		bool getNewFirstFastRefreshForDevice(core::device::Device &device);
		bool getNewViralPolicyForDevice(core::device::Device &device);
		void insertSuccessResultForDevice(core::device::Device &device);
		void insertErrorResultForDeviceFromException(core::device::Device &device, core::LibraryException &e);
		void insertUnchangedResultForDevice(core::device::Device &device);
		std::string getResultPrefixForDevice(core::device::Device &device);
};

} /* namespace nvmcli */
} /* namespace cli */

#endif /* MODIFYDEVICECOMMAND_H_ */
