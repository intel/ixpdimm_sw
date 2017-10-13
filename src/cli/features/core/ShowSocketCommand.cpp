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

#include "ShowSocketCommand.h"

namespace cli
{
namespace nvmcli
{

ShowSocketCommand::ShowSocketCommand(core::system::SystemService &service)
: m_service(service), m_pResult(NULL)
{
	m_props.addCustom("SocketID", getSocketId).setIsDefault();
	m_props.addCustom("Type", getSocketType).setIsDefault();
	m_props.addCustom("Family", getSocketFamily).setIsDefault();
	m_props.addCustom("Manufacturer", getSocketManufacturer).setIsDefault();
	m_props.addCustom("MappedMemoryLimit", getMappedMemoryLimit).setIsDefault();
	m_props.addCustom("TotalMappedMemory", getTotalMappedMemory).setIsDefault();
}

framework::ResultBase *ShowSocketCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	validateInput();

	if (NULL == m_pResult)
	{
		try
		{
			m_sockets = m_service.getSocketsForSocketIds(m_socketIds);
			createResults();
		}
		catch (core::LibraryException &e)
		{
			int libRc = e.getErrorCode();
			if (libRc == NVM_ERR_NOMEMORY)
			{
				m_pResult = new framework::ErrorResult(
						framework::ErrorResult::ERRORCODE_OUTOFMEMORY, NOMEMORY_ERROR_STR);
			}
			else if (libRc == NVM_ERR_NOTSUPPORTED)
			{
				m_pResult = new framework::ErrorResult(
						framework::ErrorResult::ERRORCODE_NOTSUPPORTED, NOTSUPPORTED_ERROR_STR);
			}
			else
			{
				// return the library message
				m_pResult = new framework::ErrorResult(
						framework::ErrorResult::ERRORCODE_UNKNOWN, e.what());
			}
		}
	}

	return m_pResult;
}

void ShowSocketCommand::validateInput()
{
	m_displayOptions = framework::DisplayOptions(m_parsedCommand.options);
	m_unitsOption = framework::UnitsOption(m_parsedCommand.options);

	if (displayOptionsAreValid() && unitsOptionIsValid())
	{
		bool socket_exists;
		std::vector<std::string> parsed_socketIds = framework::Parser::getTargetValues(
				m_parsedCommand, TARGET_SOCKET_R.name, &socket_exists);
		std::vector<NVM_UINT16> all_socketIds = m_service.getAllSocketIds();
		if (!parsed_socketIds.empty())
		{
			bool socket_id_exists;
			NVM_UINT16 socketId;
			for (size_t i = 0; i < parsed_socketIds.size(); i++)
			{
				socket_id_exists = false;
				socketId = (NVM_UINT16)stringToUInt64(parsed_socketIds[i]);
				for (size_t j = 0; j < all_socketIds.size(); j++)
				{
					if (socketId == all_socketIds[j])
					{
						m_socketIds.push_back(socketId);
						socket_id_exists = true;
						break;
					}
				}

				if (!socket_id_exists)
				{
					m_pResult = new cli::framework::SyntaxErrorBadValueResult(
							cli::framework::TOKENTYPE_TARGET, TARGET_SOCKET_R.name,
							parsed_socketIds[i]);
					break;
				}
			}
		}
		else
		{
			m_socketIds = all_socketIds;
		}
	}
}

void ShowSocketCommand::createResults()
{
	framework::ObjectListResult *pList = new framework::ObjectListResult();
	pList->setRoot("SystemSocket");
	m_pResult = pList;

	for (size_t i = 0; i < m_sockets.size(); i++)
	{
		framework::PropertyListResult value;
		for (size_t j = 0; j < m_props.size(); j++)
		{
			framework::IPropertyDefinition<core::system::SystemSocket> &p = m_props[j];
			if (isPropertyDisplayed(p))
			{
				value.insert(p.getName(), p.getValue(m_sockets[i]));
			}
		}

		pList->insert("SystemSocket", value);
	}

	m_pResult->setOutputType(m_displayOptions.isDefault() ?
					framework::ResultBase::OUTPUT_TEXTTABLE : framework::ResultBase::OUTPUT_TEXT);
}

bool ShowSocketCommand::displayOptionsAreValid()
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

bool ShowSocketCommand::unitsOptionIsValid()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_pResult = ShowCommandUtilities::getInvalidUnitsOptionResult(m_unitsOption);

	return m_pResult == NULL;
}

bool ShowSocketCommand::isPropertyDisplayed(
		framework::IPropertyDefinition<core::system::SystemSocket> &p)
{
	return p.isRequired() ||
			(p.isDefault() && m_displayOptions.isDefault()) ||
			m_displayOptions.contains(p.getName());
}

std::string ShowSocketCommand::getSocketId(core::system::SystemSocket &socket)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream result;

	result << socket.getSocketId();

	return result.str();
}

std::string ShowSocketCommand::getSocketType(core::system::SystemSocket &socket)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return socket.getSocketTypeStr();
}

std::string ShowSocketCommand::getSocketFamily(core::system::SystemSocket &socket)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return socket.getSocketFamilyStr();
}

std::string ShowSocketCommand::getSocketManufacturer(core::system::SystemSocket &socket)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string result;

	result = socket.getSocketManufacturer();

	return result;
}

std::string ShowSocketCommand::getMappedMemoryLimit(core::system::SystemSocket &socket)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream result;

	if (socket.isCapacitySkuingSupported())
	{
		result << socket.getSocketMappedMemoryLimit();
	}
	else
	{
		result << "N/A";
	}

	return result.str();
}

std::string ShowSocketCommand::getTotalMappedMemory(core::system::SystemSocket &socket)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream result;

	if (socket.isCapacitySkuingSupported())
	{
		result << socket.getSocketTotalMappedMemory();
	}
	else
	{
		result << "N/A";
	}
	return result.str();
}

}
}
