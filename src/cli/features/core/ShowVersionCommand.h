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

#ifndef SHOWVERSIONCOMMAND_H_
#define SHOWVERSIONCOMMAND_H_

#include <nvm_types.h>
#include <cli/features/core/framework/CommandBase.h>
#include <core/system/SystemService.h>
#include <core/system/SoftwareInfo.h>
#include <libinvm-cli/ErrorResult.h>

namespace cli
{
namespace nvmcli
{

const std::string SHOWVERSION_ROOT = "Software";
const std::string SHOWVERSION_COMPONENT = "Component";
const std::string SHOWVERSION_VERSION = "Version";

const std::string SHOWVERSION_MGMTSW_KEY = N_TR(NVM_SYSTEM" Software Version");
const std::string SHOWVERSION_DRIVER_KEY = N_TR(NVM_SYSTEM" Driver Version");

class NVM_API ShowVersionCommand : public framework::CommandBase
{
	public:
		ShowVersionCommand(
				core::system::SystemService &service = core::system::SystemService::getService());
		virtual ~ShowVersionCommand();

		framework::ResultBase *execute(const framework::ParsedCommand &parsedCommand);

	protected:
		core::system::SystemService &m_service;
		core::system::SoftwareInfo m_softwareInfo;
		std::string m_mgmtSwVersionString;
		std::string m_driverVersionString;

		void createVersionStrings();
		void createDriverVersionStringFromSoftwareInfo();
		std::string getBadDriverErrorMessage();
		std::string getErrorMessage(const int errorCode);
		void createResult();
		cli::framework::ErrorResult::ErrorCode getResultErrorCode();
};

} /* namespace nvmcli */
} /* namespace cli */

#endif /* SHOWVERSIONCOMMAND_H_ */
