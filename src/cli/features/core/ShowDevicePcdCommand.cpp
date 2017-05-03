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

#include <LogEnterExit.h>
#include "ShowCommandUtilities.h"
#include "WbemToCli_utilities.h"
#include <libinvm-cli/CliFrameworkTypes.h>
#include <libinvm-cli/SimpleListResult.h>
#include <cli/features/core/framework/CliHelper.h>
#include <cli/features/core/CommandParts.h>
#include <core/firmware_interface/FwCommands.h>
#include "ShowDevicePcdCommand.h"

namespace cli
{
namespace nvmcli
{

cli::nvmcli::ShowDevicePcdCommand::ShowDevicePcdCommand(core::device::DeviceService &service)
: m_service(service), m_pResult(NULL)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

cli::nvmcli::ShowDevicePcdCommand::~ShowDevicePcdCommand()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

framework::CommandSpec ShowDevicePcdCommand::getCommandSpec(int id)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::CommandSpec result(id, "Show Device Platform Configuration Data", framework::VERB_SHOW);
	result.help = TR(
		"Show the platform configuration data for one or more " NVM_DIMM_NAME "s.");

	result.addTarget(TARGET_DIMM.name, true, DIMMIDS_STR, false,
			TR("Restrict output to the platform configuration data for specific " NVM_DIMM_NAME "s by "
			"supplying one or more comma-separated " NVM_DIMM_NAME " identifiers. The default "
			"is to display the platform configuration data for all manageable " NVM_DIMM_NAME "s."));

	result.addTarget("-pcd")
		.isRequired(true)
		.isValueRequired(false)
		.isValueAccepted(false)
		.helpText(TR("The Platform Configuration Data information."));

	return result;
}

framework::ResultBase *ShowDevicePcdCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	m_dimmIds = framework::CliHelper::splitCommaSeperatedString(m_parsedCommand.targets[TARGET_DIMM.name]);

	try
	{
		m_devices = m_service.getAllDevices();

		if (dimmIdsAreValid())
		{
			filterDevicesOnDimmIds();

			createResults();
		}
	}
	catch (core::LibraryException &e)
	{
		m_pResult = cli::nvmcli::CoreExceptionToResult(e);
	}

	return m_pResult;
}

bool ShowDevicePcdCommand::dimmIdsAreValid()
{
	m_pResult = ShowCommandUtilities::getInvalidDimmIdResult(
			m_dimmIds, m_devices);

	return m_pResult == NULL;
}

void ShowDevicePcdCommand::filterDevicesOnDimmIds()
{
	ShowCommandUtilities::filterDevicesOnDimmIds(m_devices, m_dimmIds);
}

void ShowDevicePcdCommand::createResults()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	enum return_code rc = NVM_SUCCESS;

	core::firmware_interface::FwCommands fwCmds = core::firmware_interface::FwCommands::getFwCommands();

	std::string result;
	framework::SimpleListResult *pListResult = new framework::SimpleListResult();
	for (size_t i = 0; i < m_devices.size(); i++)
	{
		std::string dimmId = ShowCommandUtilities::getDimmId(m_devices[i]);
		std::string resultPerDimm = "\nDimmID: " + dimmId + '\n';
		rc = fwCmds.fwGetPayload_PlatformConfigDataConfigurationHeaderTable(m_devices[i].getDeviceHandle(), 1, 0, 0, resultPerDimm);
		if (rc == NVM_SUCCESS)
		{
			result = resultPerDimm;
		}
		else
		{
			NVM_ERROR_DESCRIPTION errStr;
			nvm_get_error(rc, errStr, NVM_ERROR_LEN);
			result = std::string(TRS(SHOWDEVICEPCD_MSG)) + dimmId + ": " + errStr;
		}

		pListResult->insert(result);
	}

	m_pResult = pListResult;
}

}
}
