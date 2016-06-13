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

#include <LogEnterExit.h>
#include <libinvm-cli/CliFrameworkTypes.h>
#include <cli/features/core/WbemToCli_utilities.h>
#include <cli/features/core/CommandParts.h>

#include "ShowHostServerCommand.h"

namespace cli
{
namespace nvmcli
{

ShowHostServerCommand::ShowHostServerCommand(core::system::SystemService &service)
	: m_service(service)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_props.addStr("Name", &core::system::SystemInfo::getHostName).setIsDefault();
	m_props.addStr("OsName", &core::system::SystemInfo::getOsName).setIsDefault();
	m_props.addStr("OsVersion", &core::system::SystemInfo::getOsVersion).setIsDefault();
	m_props.addBool("MixedSKU", &core::system::SystemInfo::getMixedSku).setIsDefault();
	m_props.addBool("SKUViolation", &core::system::SystemInfo::getSkuViolation).setIsDefault();
	m_props.addUint16("LogLevel", &core::system::SystemInfo::getLogLevel);
}

framework::ResultBase *ShowHostServerCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	m_displayOptions = framework::DisplayOptions(m_parsedCommand.options);

	if (displayOptionsAreValid())
	{
		try
		{
			core::Result<core::system::SystemInfo> s = m_service.getHostInfo();
			m_hostInfo = s.getValue();

			createResults();
		}
		catch (core::LibraryException &e)
		{
			int libRc = e.getErrorCode();
			if (libRc == NVM_ERR_NOMEMORY)
			{
				m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_OUTOFMEMORY, NOMEMORY_ERROR_STR);
			}
			else if (libRc == NVM_ERR_NOTSUPPORTED)
			{
				m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_NOTSUPPORTED, NOTSUPPORTED_ERROR_STR);
			}
			else
			{ // return the library message
				m_pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN, e.what());
			}
		}
	}

	return m_pResult;
}

bool ShowHostServerCommand::displayOptionsAreValid()
{
	std::string invalidDisplay;
	const std::vector<std::string> &display = m_displayOptions.getDisplay();
	for (size_t i = 0; i < display.size() && invalidDisplay.empty(); i++)
	{
		if (!m_props.contains(display[i]))
		{
			invalidDisplay = display[i];
		}
	}

	if (!invalidDisplay.empty())
	{
		m_pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_OPTION,
			framework::OPTION_DISPLAY.name, invalidDisplay);
	}
	return m_pResult == NULL;
}

bool ShowHostServerCommand::isPropertyDisplayed(
	framework::IPropertyDefinition<core::system::SystemInfo> &p)
{
	return p.isRequired() ||
		   m_displayOptions.isAll() ||
		   (p.isDefault() && m_displayOptions.isDefault()) ||
		   m_displayOptions.contains(p.getName());
}

void ShowHostServerCommand::createResults()
{
	framework::PropertyListResult *pList = new framework::PropertyListResult();
	pList->setName("Host");
	m_pResult = pList;

	for (size_t j = 0; j < m_props.size(); j++)
	{
		framework::IPropertyDefinition<core::system::SystemInfo> &p = m_props[j];
		if (isPropertyDisplayed(p))
		{
			pList->insert(p.getName(), p.getValue(m_hostInfo));
		}
	}

	m_pResult->setOutputType(
			m_displayOptions.isDefault() ?
					framework::ResultBase::OUTPUT_TEXTTABLE :
					framework::ResultBase::OUTPUT_TEXT);
}

}
}
