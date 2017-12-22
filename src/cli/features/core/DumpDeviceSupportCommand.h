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

#ifndef SRC_CLI_FEATURES_CORE_DUMPDEVICESUPPORTCOMMAND_H_
#define SRC_CLI_FEATURES_CORE_DUMPDEVICESUPPORTCOMMAND_H_

#include "WbemToCli_utilities.h"
#include <physical_asset/NVDIMMFactory.h>
#include <cli/features/core/framework/CommandBase.h>
#include <cli/features/core/framework/CliHelper.h>
#include <cli/features/core/CommandParts.h>
#include <lib/nvm_types.h>
#include <libinvm-cli/CommandSpec.h>
#include <libinvm-cli/Parser.h>
#include <libinvm-cli/CliFrameworkTypes.h>
#include <libinvm-cli/SimpleListResult.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cli/SyntaxErrorMissingValueResult.h>
#include <libinvm-cli/SyntaxErrorUnexpectedValueResult.h>
#include <core/device/DeviceService.h>
#include <cli/features/core/StringList.h>
#include <LogEnterExit.h>
#include <cli/features/ExportCli.h>

namespace cli
{
namespace nvmcli
{

static const std::string DUMPDEVICESUPPORT_HEADER = "Retrieving device support data from ";
static const std::string DUMPDEVICESUPPORT_MSG = "Dump device support data from ";
static const std::string WRITE_SUCCESS = "Successfully written to ";

class NVM_CLI_API DumpDeviceSupportCommand : public framework::CommandBase
{
	public:

		DumpDeviceSupportCommand(core::device::DeviceService &service =
				core::device::DeviceService::getService());

		virtual ~DumpDeviceSupportCommand();

		framework::ResultBase *execute(const framework::ParsedCommand &parsedCommand);

	private:

		std::string m_destination;
		core::device::DeviceService &m_service;
		core::device::DeviceCollection m_devices;
		framework::ParsedCommand m_parsedCommand;
		framework::ResultBase *m_pResult;
		std::vector<std::string> m_uids, m_dimmIds;

		virtual void validateInput();
		virtual void createResults();
};

}
}

#endif /* SRC_CLI_FEATURES_CORE_DUMPDEVICESUPPORTCOMMAND_H_ */
