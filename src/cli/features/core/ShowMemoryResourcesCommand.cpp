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
#include <intel_cli_framework/CliFrameworkTypes.h>
#include <cli/features/core/WbemToCli_utilities.h>
#include <cli/features/core/CommandParts.h>

#include "ShowMemoryResourcesCommand.h"

namespace cli
{
namespace nvmcli
{

ShowMemoryResourcesCommand::ShowMemoryResourcesCommand(core::system::SystemService &service)
: m_service(service)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	core::Result<core::system::SystemMemoryResources> s = m_service.getMemoryResources();
	m_memoryResourcesInfo = s.getValue();

	m_props.addUint64("Capacity", &core::system::SystemMemoryResources::getTotalCapacity).setIsDefault();
	m_props.addUint64("VolatileCapacity", &core::system::SystemMemoryResources::getTotalVolatileCapacity).setIsDefault();
	m_props.addUint64("PersistentCapacity", &core::system::SystemMemoryResources::getTotalPersistentCapacity).setIsDefault();
	m_props.addUint64("UnconfiguredCapacity", &core::system::SystemMemoryResources::getTotalUnconfiguredCapacity).setIsDefault();
	m_props.addUint64("InaccessibleCapacity", &core::system::SystemMemoryResources::getTotalInaccessibleCapacity).setIsDefault();
	m_props.addUint64("ReservedCapacity", &core::system::SystemMemoryResources::getTotalReservedCapacity).setIsDefault();
}

framework::ResultBase *ShowMemoryResourcesCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	m_displayOptions = framework::DisplayOptions(m_parsedCommand.options);

	if (displayOptionsAreValid())
	{
		try
		{
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

bool ShowMemoryResourcesCommand::displayOptionsAreValid()
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

bool ShowMemoryResourcesCommand::isPropertyDisplayed(
		framework::IPropertyDefinition<core::system::SystemMemoryResources> &p)
{
	return p.isRequired() ||
			m_displayOptions.isAll() ||
			(p.isDefault() && m_displayOptions.isDefault()) ||
			m_displayOptions.contains(p.getName());
}

void ShowMemoryResourcesCommand::createResults()
{
	framework::PropertyListResult *pList = new framework::PropertyListResult();
	pList->setName("MemoryResources");
	m_pResult = pList;

	for (size_t j = 0; j < m_props.size(); j++)
	{
		framework::IPropertyDefinition<core::system::SystemMemoryResources> &p = m_props[j];
		if (isPropertyDisplayed(p))
		{
			pList->insert(p.getName(), p.getValue(m_memoryResourcesInfo));
		}
	}

	m_pResult->setOutputType(
			m_displayOptions.isDefault() ?
					framework::ResultBase::OUTPUT_TEXTTABLE :
					framework::ResultBase::OUTPUT_TEXT);
}

}
}
