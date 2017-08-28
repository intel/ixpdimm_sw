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
#include "DeleteDevicePcdCommand.h"
#include "uid/uid.h"
#include <libinvm-cli/Parser.h>

namespace cli
{
namespace nvmcli
{

bool DeleteDevicePcdCommand::promptUserToDeletePCD(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream message;

	message << TR("This is a destructive operation which will clear the entire namespace label storage area including all namespace labels"
		"and the namespace label index block in order to re-purpose the " NVM_DIMM_NAME "s for use in a different operating system. ");
	message << NVM_DIMM_NAME << " " << ShowCommandUtilities::getDimmId(device) << ". ";
	message << TR("Do you want to continue?");

	framework::YesNoPrompt m_prompt;

	return m_prompt.prompt(message.str());
}

cli::nvmcli::DeleteDevicePcdCommand::DeleteDevicePcdCommand(core::device::DeviceService &service, core::NvmLibrary &nvmLib) :
		m_checkPromptResponseToDeletePCD(&promptUserToDeletePCD), m_service(service), m_pResult(NULL), m_nvmLib(nvmLib)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

cli::nvmcli::DeleteDevicePcdCommand::~DeleteDevicePcdCommand()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

framework::CommandSpec DeleteDevicePcdCommand::getCommandSpec(int id)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::CommandSpec result(id, "Delete Device Platform Configuration Data", framework::VERB_DELETE);
	result.help = TR(
		"Delete the platform configuration data for one or more " NVM_DIMM_NAME "s.");

	result.addTarget(TARGET_DIMM.name, true, DIMMIDS_STR, false,
			TR("Restrict output to the platform configuration data for specific " NVM_DIMM_NAME "s by "
			"supplying one or more comma-separated " NVM_DIMM_NAME " identifiers. The default "
			"is to delete the platform configuration data for all manageable " NVM_DIMM_NAME "s."));

	result.addOption("-force", false, "", false,
			TR("DeleteDevicePcd " NVM_DIMM_NAME "s is a destructive operation which requires "
			 "confirmation from the user for each " NVM_DIMM_NAME ". This option "
			 "suppresses the confirmation."),
			"-f").isValueAccepted(false);

	result.addTarget(TARGET_PCD_STR, true, "LSA", true,
			TR("Clear the namespace label storage area partition in the Platform Configuration Data "
			"in the " NVM_DIMM_NAME "s.")).isValueAccepted(true);

	return result;
}

framework::ResultBase *DeleteDevicePcdCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	m_dimmIds = framework::CliHelper::splitCommaSeperatedString(m_parsedCommand.targets[TARGET_DIMM.name]);

	// verify "-pcd" target value is "LSA"
	std::string valueString = cli::framework::Parser::getTargetValue(parsedCommand, TARGET_PCD_STR);
	const std::string expectedValue = "LSA" ;

	if (expectedValue != valueString)
	{
		// input error
		m_pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
				TARGET_PCD_STR, valueString);
	}
	else
	{
		// delete PCD LSA
		try
		{
			m_devices = m_service.getAllDevices();

			if (dimmIdsAreValid())
			{
				filterDevicesOnDimmIds();

				deleteDevicesPcdLsa();
			}
		}
		catch (core::LibraryException &e)
		{
			m_pResult = cli::nvmcli::CoreExceptionToResult(e);
		}
	}

	return m_pResult;
}

bool DeleteDevicePcdCommand::dimmIdsAreValid()
{
	m_pResult = ShowCommandUtilities::getInvalidDimmIdResult(
			m_dimmIds, m_devices);

	return m_pResult == NULL;
}

void DeleteDevicePcdCommand::filterDevicesOnDimmIds()
{
	ShowCommandUtilities::filterDevicesOnDimmIds(m_devices, m_dimmIds);
}

void DeleteDevicePcdCommand::deleteDevicesPcdLsa()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	int rc;
	NVM_UID uid;

	std::string result;
	framework::SimpleListResult *pListResult = new framework::SimpleListResult();
	for (size_t i = 0; i < m_devices.size(); i++)
	{
		if (hasForceOption() || m_checkPromptResponseToDeletePCD(m_devices[i]))
		{
			std::string dimmId = ShowCommandUtilities::getDimmId(m_devices[i]);
			std::string resultPerDimm = "\nOK: cleared LSA on DimmID: " + dimmId + '\n';
			try
			{
				uid_copy(m_devices[i].getUid().c_str(), uid);
				rc = m_nvmLib.clearDimmLsa(uid);

				if (rc == NVM_SUCCESS)
				{
					result = resultPerDimm;
				}
				else
				{
					NVM_ERROR_DESCRIPTION errStr;
					nvm_get_error((enum return_code) rc, errStr, NVM_ERROR_LEN);
					result = "\nErr: " + std::string(TRS(DELETEDEVICEPCD_MSG)) + dimmId + ": " + errStr + '\n';
				}
			}
			catch (core::LibraryException &e)
			{
				result += cli::nvmcli::CoreExceptionToResult(e)->outputText();
			}
		}
		else
		{
			result = "\nNot changed: " + std::string(TRS(DELETEDEVICEPCD_MSG)) + '\n';
		}

		pListResult->insert(result);
	}

	m_pResult = pListResult;
}

bool DeleteDevicePcdCommand::hasForceOption()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_parsedCommand.options.find("-force") != m_parsedCommand.options.end();
}

}
}
