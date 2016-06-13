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

#include "ShowVersionCommand.h"
#include <LogEnterExit.h>
#include <libinvm-cli/ObjectListResult.h>
#include <core/Helper.h>

cli::nvmcli::ShowVersionCommand::ShowVersionCommand(core::system::SystemService &service) :
	framework::CommandBase(),
	m_service(service), m_softwareInfo(),
	m_mgmtSwVersionString(), m_driverVersionString()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

cli::nvmcli::ShowVersionCommand::~ShowVersionCommand()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

cli::framework::ResultBase* cli::nvmcli::ShowVersionCommand::execute(
		const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	createVersionStrings();
	createResult();

	return m_pResult;
}

void cli::nvmcli::ShowVersionCommand::createVersionStrings()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	try
	{
		m_softwareInfo = m_service.getSoftwareInfo().getValue();

		m_mgmtSwVersionString = m_softwareInfo.getMgmtSoftwareVersion();
		createDriverVersionStringFromSoftwareInfo();
	}
	catch (core::LibraryException &e)
	{
		// Couldn't get the versions - display error
		m_mgmtSwVersionString = getErrorMessage(e.getErrorCode());
		m_driverVersionString = getErrorMessage(e.getErrorCode());
	}
}

void cli::nvmcli::ShowVersionCommand::createDriverVersionStringFromSoftwareInfo()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (m_softwareInfo.isDriverInstalled())
	{
		m_driverVersionString = m_softwareInfo.getDriverVersion();

		// Exists but not supported - display error alongside version
		if (!m_softwareInfo.isDriverSupported())
		{
			m_driverVersionString += " - ";
			m_driverVersionString += getBadDriverErrorMessage();
		}
	}
	else
	{
		m_driverVersionString = getBadDriverErrorMessage();
	}
}

std::string cli::nvmcli::ShowVersionCommand::getBadDriverErrorMessage()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return getErrorMessage(NVM_ERR_BADDRIVER);
}

std::string cli::nvmcli::ShowVersionCommand::getErrorMessage(
		const int errorCode)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string message = core::Helper::getErrorMessage(errorCode);
	framework::ErrorResult result(framework::ErrorResult::ERRORCODE_UNKNOWN,
			message);

	return result.outputText();
}

void cli::nvmcli::ShowVersionCommand::createResult()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ObjectListResult *pVersionList = new framework::ObjectListResult();
	pVersionList->setRoot(SHOWVERSION_ROOT);
	pVersionList->setOutputType(framework::ResultBase::OUTPUT_TEXTTABLE);
	pVersionList->setErrorCode(getResultErrorCode());

	framework::PropertyListResult mgmtSwResult;
	mgmtSwResult.insert(SHOWVERSION_COMPONENT, SHOWVERSION_MGMTSW_KEY);
	mgmtSwResult.insert(SHOWVERSION_VERSION, m_mgmtSwVersionString);
	pVersionList->insert(SHOWVERSION_MGMTSW_KEY, mgmtSwResult);

	framework::PropertyListResult driverResult;
	driverResult.insert(SHOWVERSION_COMPONENT, SHOWVERSION_DRIVER_KEY);
	driverResult.insert(SHOWVERSION_VERSION, m_driverVersionString);
	pVersionList->insert(SHOWVERSION_DRIVER_KEY, driverResult);

	m_pResult = pVersionList;
}

cli::framework::ErrorResult::ErrorCode cli::nvmcli::ShowVersionCommand::getResultErrorCode()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	cli::framework::ErrorResult::ErrorCode code = cli::framework::ErrorResult::ERRORCODE_UNKNOWN;
	if (m_softwareInfo.isDriverInstalled() && m_softwareInfo.isDriverSupported())
	{
		code = cli::framework::ErrorResult::ERRORCODE_SUCCESS;
	}

	return code;
}
