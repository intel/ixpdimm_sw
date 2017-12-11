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

#include "FormatDeviceCommand.h"
#include <LogEnterExit.h>
#include <cr_i18n.h>
#include <libinvm-cli/ErrorResult.h>
#include <sstream>
#include <os/os_adapter.h>
#include <core/NvmLibrary.h>
#include <cli/features/core/framework/CliHelper.h>
#include <cli/features/core/CommandParts.h>
#include <cli/features/core/ShowCommandUtilities.h>

namespace cli
{
namespace nvmcli
{

FormatDeviceCommand::UserPrompt::UserPrompt(const framework::YesNoPrompt &prompt) :
		m_prompt(prompt)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

bool FormatDeviceCommand::UserPrompt::promptUserToFormatDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream message;
	message << TR("This operation will take several minutes to complete and will erase all data on ");
	message << NVM_DIMM_NAME << " " << ShowCommandUtilities::getDimmId(device) << ". ";
	message << TR("Do you want to continue?");

	return m_prompt.prompt(message.str());
}

FormatDeviceCommand::FormatDeviceCommand(UserPrompt &prompt,
		core::device::DeviceService &deviceService,
		core::device::FormatDeviceService &formatService) :
				m_prompt(prompt),
				m_deviceService(deviceService),
				m_formatService(formatService),
				m_needsPowerCycle(false)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

FormatDeviceCommand::~FormatDeviceCommand()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

framework::CommandSpec FormatDeviceCommand::getCommandSpec(int commandId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::CommandSpec result(commandId,
			TR("Format Device"),
			"start",
			TR("Format one or more " NVM_DIMM_NAME "s in an attempt to recover them."));

	result.addOption("-force", false, "", false,
			TR("Formatting " NVM_DIMM_NAME "s is a destructive operation which requires "
					"confirmation from the user for each " NVM_DIMM_NAME ". This option "
					"suppresses the confirmation."),
			"-f").isValueAccepted(false);

	result.addTarget("-format", true, "", false,
			TR("Format the " NVM_DIMM_NAME "s.")).isValueAccepted(false);
	result.addTarget("-dimm", true, "DimmIDs", false,
			TR("Format specific " NVM_DIMM_NAME "s by supplying one or more comma-separated "
					NVM_DIMM_NAME " identifiers. The default is to format all manageable "
					NVM_DIMM_NAME "s."));

	return result;
}

framework::ResultBase* cli::nvmcli::FormatDeviceCommand::execute(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;

	try
	{
		parseDevices();
		if (!hasError())
		{
			formatDevices();
		}
	}
	catch (core::LibraryException &e)
	{
		setErrorResult(getErrorMessage(e.getErrorCode()));
	}
	return m_pResult;
}

void FormatDeviceCommand::parseDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_dimmIds = framework::CliHelper::splitCommaSeperatedString(
			m_parsedCommand.targets[TARGET_DIMM.name]);
	m_devices = m_deviceService.getAllDevices();

	filterDevices();
	errorIfNoDevicesRemain();
}

void FormatDeviceCommand::filterDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_dimmIds.empty())
	{
		filterManageableDevices();
	}
	else if (dimmIdsAreValid())
	{
		filterDevicesOnDimmIds();
	}
}

void FormatDeviceCommand::filterManageableDevices()
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

bool FormatDeviceCommand::dimmIdsAreValid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pResult = ShowCommandUtilities::getInvalidDimmIdResult(
			m_dimmIds, m_devices);

	return m_pResult == NULL;
}

void FormatDeviceCommand::filterDevicesOnDimmIds()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	ShowCommandUtilities::filterDevicesOnDimmIds(m_devices, m_dimmIds);
}

void FormatDeviceCommand::errorIfNoDevicesRemain()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (!hasError() && m_devices.size() == 0)
	{
		setErrorResult(N_TR("No manageable " NVM_DIMM_NAME "s found."));
	}
}

bool FormatDeviceCommand::hasError()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_pResult != NULL;
}

void FormatDeviceCommand::setErrorResult(const std::string& errorMessage)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	delete m_pResult;
	m_pResult = new framework::ErrorResult(framework::ResultBase::ERRORCODE_UNKNOWN,
			TRS(errorMessage));
}

void FormatDeviceCommand::formatDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pResult = new framework::SimpleListResult;

	for (size_t i = 0; i < m_devices.size(); i++)
	{
		if (hasForceOption() || m_prompt.promptUserToFormatDevice(m_devices[i]))
		{
			startFormatForDevice(m_devices[i]);
		}
	}

	waitUntilFormatCompleteForAllDimms();
	addPowerCycleNoticeToResultList();
}

bool FormatDeviceCommand::hasForceOption()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_parsedCommand.options.find("-force") != m_parsedCommand.options.end();
}

void FormatDeviceCommand::startFormatForDevice(core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::SimpleListResult *pResults =
			dynamic_cast<framework::SimpleListResult *>(m_pResult);

	if (device.isManageable())
	{
		try
		{
			if (!m_formatService.isDeviceInFormattableState(device))
			{
				throw core::LibraryException(NVM_ERR_NOTSUPPORTED);
			}

			m_formatService.startFormatForDevice(device.getDeviceHandle());
			m_waitingForCompletion[device.getDeviceHandle()] = true;
			m_needsPowerCycle = true;
		}
		catch (core::LibraryException &e)
		{
			m_waitingForCompletion[device.getDeviceHandle()] = false;
			insertNonFormattableDimmMessage(pResults,device,getErrorMessage(e.getErrorCode()));
		}
	}
	else
	{
		m_waitingForCompletion[device.getDeviceHandle()] = false;
		insertNonFormattableDimmMessage(pResults, device, getErrorMessage(NVM_ERR_NOTMANAGEABLE));
	}
}

void FormatDeviceCommand::insertSuccessResultForDevice(framework::SimpleListResult* pResults,core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string message =
			getMessagePrefix(device) + ": " + TR("Success");
	pResults->insert(message);
}
void FormatDeviceCommand::insertErrorResultForDevice(framework::SimpleListResult* pResults,core::device::Device& device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string message =
			getMessagePrefix(device) + ": " + TR("Failure");
	pResults->insert(message);
}
std::string FormatDeviceCommand::getMessagePrefix(core::device::Device &device)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::stringstream prefix;
	prefix << TR("Format " NVM_DIMM_NAME " ");
	prefix << ShowCommandUtilities::getDimmId(device);
	return prefix.str();
}


void FormatDeviceCommand::insertNonFormattableDimmMessage(framework::SimpleListResult* pResults, core::device::Device& device,
const std::string& errorMessage)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	cli::framework::ErrorResult *pError = new framework::ErrorResult(
			framework::ResultBase::ERRORCODE_UNKNOWN,
			TRS(errorMessage),
			getMessagePrefix(device));
	pResults->insert(pError);
}

std::string FormatDeviceCommand::getErrorMessage(const int libError)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	core::NvmLibrary nvmLib;
	return nvmLib.getErrorMessage(libError);
}

void FormatDeviceCommand::waitUntilFormatCompleteForAllDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	while (!isFormatCompleteForAllDimms())
	{
		sleep(10);
	}
}

bool FormatDeviceCommand::isFormatCompleteForAllDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool completeForAll = true;
	NVM_UINT32 handle = 0;
	size_t i = 0;

	for (std::map<NVM_UINT32, bool>::iterator stillWaiting =
		m_waitingForCompletion.begin();
		stillWaiting != m_waitingForCompletion.end() && i < m_devices.size();
		stillWaiting++,i++)
	{
		bool done = false;
		NVM_UINT8 format_success = 0;
		try
			{
				handle= stillWaiting->first;
				if (m_waitingForCompletion.at(handle) == true)
				{
					format_success = m_formatService.isFormatComplete(handle);
				}
				if(format_success == NVM_FORMAT_SUCCESS)
				{
					framework::SimpleListResult *pResults =
							dynamic_cast<framework::SimpleListResult *>(m_pResult);
					insertSuccessResultForDevice(pResults,m_devices[i]);
				}
				else if(format_success == NVM_FORMAT_FAILURE)
				{
					framework::SimpleListResult *pResults =
							dynamic_cast<framework::SimpleListResult *>(m_pResult);
					insertErrorResultForDevice(pResults, m_devices[i]);
				}
				else
				{
					//do nothing
				}
				done = !stillWaiting->second || format_success;
			}
			catch (core::LibraryException &)
			{
					// Ignore
			}
			if (!done)
			{
				completeForAll = false;
				break;
			}
	}
	return completeForAll;
}

void FormatDeviceCommand::sleep(const NVM_UINT32 seconds)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	nvm_sleep(seconds * 1000);
}

void FormatDeviceCommand::addPowerCycleNoticeToResultList()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_needsPowerCycle)
	{
		framework::SimpleListResult *pResults =
				dynamic_cast<framework::SimpleListResult *>(m_pResult);
		pResults->insert(TR("A power cycle is required after a device format."));
	}
}

} /* namespace nvmcli */
} /* namespace cli */
