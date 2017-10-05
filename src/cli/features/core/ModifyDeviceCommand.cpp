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

#include "ModifyDeviceCommand.h"
#include <LogEnterExit.h>
#include <libinvm-cli/cr_i18n.h>
#include <libinvm-cli/Parser.h>
#include <libinvm-cli/SyntaxErrorResult.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cli/SimpleListResult.h>
#include <cli/features/core/CommandParts.h>
#include <cli/features/core/WbemToCli_utilities.h>
#include <cli/features/core/ShowCommandUtilities.h>
#include <cli/features/core/framework/CliHelper.h>

namespace cli
{
namespace nvmcli
{

ModifyDeviceCommand::ModifyDeviceCommand(cli::framework::YesNoPrompt &prompt,
		core::device::DeviceService &deviceService) :
		framework::CommandBase(),
		m_prompt(prompt),
		m_deviceService(deviceService),
		m_forceOption(false),
		m_firstFastRefreshPropertyExists(false),
		m_firstFastRefreshPropValue(false),
		m_viralPolicyPropertyExists(false),
		m_viralPolicyPropValue(false)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

ModifyDeviceCommand::~ModifyDeviceCommand()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

framework::CommandSpec ModifyDeviceCommand::getCommandSpec(int id)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::CommandSpec modifySpec(id, TR("Modify Device"), framework::VERB_SET,
			TR("Change the configurable setting(s) on one or more " NVM_DIMM_NAME "s."));

	modifySpec.addOption(framework::OPTION_FORCE).helpText(
			TR("Changing " NVM_DIMM_NAME " setting(s) is a potentially destructive operation which requires "
					"confirmation from the user for each " NVM_DIMM_NAME ". This option suppresses the confirmation."));

	modifySpec.addTarget(TARGET_DIMM.name, true, DIMMIDS_STR, false,
			TR("Modify specific " NVM_DIMM_NAME "s by supplying one or more comma-separated " NVM_DIMM_NAME " identifiers. "
			"However, this is not recommended as it may put the system in an undesirable state. "
			"The default is to modify all manageable " NVM_DIMM_NAME "s."));

	modifySpec.addProperty(FIRSTFASTREFRESH_PROPERTYNAME, false, "0|1", true,
			TR("Whether acceleration of the first refresh cycle is enabled."));
	modifySpec.addProperty(VIRALPOLICY_PROPERTYNAME, false, "0|1", true,
			TR("Whether the viral policies are enabled."));

	return modifySpec;
}

framework::ResultBase* cli::nvmcli::ModifyDeviceCommand::execute(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;

	parseForceOption();
	parseSettings();
	parseDevices();

	modifyDevices();

	return m_pResult;
}

bool ModifyDeviceCommand::hasError()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_pResult &&
			m_pResult->getErrorCode() != cli::framework::ErrorResult::ERRORCODE_SUCCESS;
}

void ModifyDeviceCommand::parseForceOption()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_forceOption = m_parsedCommand.options.find(framework::OPTION_FORCE.name)
				!= m_parsedCommand.options.end();
}

void ModifyDeviceCommand::parseSettings()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkAtLeastOnePropertyExists();
	parseFirstFastRefresh();
	parseViralPolicy();
}

void ModifyDeviceCommand::checkAtLeastOnePropertyExists()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!hasError() && m_parsedCommand.properties.empty())
	{
		m_pResult = new framework::SyntaxErrorResult(TRS(NOMODIFIABLEPROPERTY_ERROR_STR));
	}
}

void ModifyDeviceCommand::parseFirstFastRefresh()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!hasError())
	{
		std::string firstFastRefresh = framework::Parser::getPropertyValue(m_parsedCommand,
				FIRSTFASTREFRESH_PROPERTYNAME, &m_firstFastRefreshPropertyExists);

		if (m_firstFastRefreshPropertyExists)
		{
			m_firstFastRefreshPropValue = parseBooleanPropertyValue(
					FIRSTFASTREFRESH_PROPERTYNAME, firstFastRefresh);
		}
	}
}

void ModifyDeviceCommand::parseViralPolicy()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!hasError())
	{
		std::string viralPolicy = framework::Parser::getPropertyValue(m_parsedCommand,
				VIRALPOLICY_PROPERTYNAME, &m_viralPolicyPropertyExists);

		if (m_viralPolicyPropertyExists)
		{
			m_viralPolicyPropValue = parseBooleanPropertyValue(
					VIRALPOLICY_PROPERTYNAME, viralPolicy);
		}
	}
}

bool ModifyDeviceCommand::parseBooleanPropertyValue(const std::string &propertyName,
		const std::string &propertyValue)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool result = false;

	if (cli::framework::stringsIEqual(propertyValue, "1"))
	{
		result = true;
	}
	else if (cli::framework::stringsIEqual(propertyValue, "0"))
	{
		result = false;
	}
	else
	{
		m_pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
				propertyName,
				propertyValue);
	}

	return result;
}

void ModifyDeviceCommand::parseDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!hasError())
	{
		try
		{
			m_devices = m_deviceService.getAllDevices();

			parseDimmIds();
			filterDevices();
		}
		catch (core::LibraryException &e)
		{
			m_pResult = CoreExceptionToResult(e);
		}
	}
}

void ModifyDeviceCommand::parseDimmIds()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_dimmIds = framework::CliHelper::splitCommaSeperatedString(
			m_parsedCommand.targets[TARGET_DIMM.name]);
}

void ModifyDeviceCommand::filterDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_dimmIds.empty())
	{
		filterManageableDimms();
		checkAtLeastOneDimmStillInList();
	}
	else if (dimmIdsAreValid())
	{
		cli::nvmcli::ShowCommandUtilities::filterDevicesOnDimmIds(m_devices, m_dimmIds);
	}
}

void ModifyDeviceCommand::filterManageableDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (size_t i = m_devices.size(); i > 0; i--)
	{
		if (!m_devices[i - 1].isManageable())
		{
			m_devices.removeAt(i - 1);
		}
	}
}

void ModifyDeviceCommand::checkAtLeastOneDimmStillInList()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_devices.size() == 0)
	{
		m_pResult = new framework::ErrorResult(
				framework::ErrorResult::ERRORCODE_UNKNOWN,
				TR("No manageable " NVM_DIMM_NAME "s found."));
	}
}

bool ModifyDeviceCommand::dimmIdsAreValid()
{
	m_pResult = ShowCommandUtilities::getInvalidDimmIdResult(
			m_dimmIds, m_devices);

	return m_pResult == NULL;
}

void ModifyDeviceCommand::modifyDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!hasError())
	{
		m_pResult = new framework::SimpleListResult();

		for (size_t i = 0; i < m_devices.size() && !hasError(); i++)
		{
			if (shouldModifyDevice(m_devices[i]))
			{
				modifyDevice(m_devices[i]);
			}
			else
			{
				insertUnchangedResultForDevice(m_devices[i]);
			}
		}
	}
}

bool ModifyDeviceCommand::shouldModifyDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return (m_forceOption ||
			m_prompt.prompt(getPromptMessageForDevice(device)));
}

void ModifyDeviceCommand::modifyDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		m_deviceService.modifyDeviceSettings(device.getUid(),
				getNewFirstFastRefreshForDevice(device),
				getNewViralPolicyForDevice(device));
		insertSuccessResultForDevice(device);
	}
	catch (core::LibraryException &e)
	{
		insertErrorResultForDeviceFromException(device, e);
	}
}

std::string ModifyDeviceCommand::getPromptMessageForDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string dimmId = cli::nvmcli::ShowCommandUtilities::getDimmId(device);

	// Keeps the string with format specifiers intact for translation purposes
	std::string message = framework::ResultBase::stringFromArgList(
			TR("Change settings for " NVM_DIMM_NAME " %s?"),
			dimmId.c_str());

	return message;
}

bool ModifyDeviceCommand::getNewFirstFastRefreshForDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool newFirstFastRefresh = device.isFirstFastRefresh();
	if (m_firstFastRefreshPropertyExists)
	{
		newFirstFastRefresh = m_firstFastRefreshPropValue;
	}

	return newFirstFastRefresh;
}

bool ModifyDeviceCommand::getNewViralPolicyForDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool newViralPolicy = device.isViralPolicyEnabled();
	if (m_viralPolicyPropertyExists)
	{
		newViralPolicy = m_viralPolicyPropValue;
	}

	return newViralPolicy;
}

void ModifyDeviceCommand::insertSuccessResultForDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::SimpleListResult *pListResults = dynamic_cast<framework::SimpleListResult *>(m_pResult);
	pListResults->insert(getResultPrefixForDevice(device)  + ": " + TR("Success"));
}

void ModifyDeviceCommand::insertErrorResultForDeviceFromException(core::device::Device& device,
		core::LibraryException& e)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::SimpleListResult *pListResults = dynamic_cast<framework::SimpleListResult *>(m_pResult);

	cli::framework::ErrorResult *pError = CoreExceptionToResult(e, getResultPrefixForDevice(device));
	pListResults->insert(pError);
	pListResults->setErrorCode(pError->getErrorCode());
}

void ModifyDeviceCommand::insertUnchangedResultForDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::SimpleListResult *pListResults = dynamic_cast<framework::SimpleListResult *>(m_pResult);
	pListResults->insert(getResultPrefixForDevice(device)  + ": " + TR("Unchanged"));
}

std::string ModifyDeviceCommand::getResultPrefixForDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string prefix = TR("Modify " NVM_DIMM_NAME);
	prefix += " " + ShowCommandUtilities::getDimmId(device);

	return prefix;
}

} /* namespace nvmcli */
} /* namespace cli */
