/*
 * Copyright (c) 2015 2016, Intel Corporation
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

/*
 * Helper class that interprets and wraps CLI inputs for memory provisioning
 */

#include <string.h>

#include "WbemToCli_utilities.h"
#include <string/x_str.h>
#include <string/s_str.h>
#include <libinvm-cli/SyntaxErrorMissingValueResult.h>
#include <mem_config/MemoryCapabilitiesFactory.h>
#include "MemoryProperty.h"
#include <utility.h>

namespace cli
{

cli::nvmcli::MemoryProperty::MemoryProperty(
		const framework::ParsedCommand& parsedCommand,
		const std::string& sizePropertyName,
		const std::string& settingsPropertyName) :
	m_size(0),
	m_sizePropertyExists(false),
	m_settingsPropertyExists(false)
{
	memset(&m_format, 0, sizeof (m_format));

	setParsedCommand(parsedCommand, sizePropertyName, settingsPropertyName);
	m_settingsValid = validateSettings();
}

void cli::nvmcli::MemoryProperty::setParsedCommand(
		const framework::ParsedCommand& parsedCommand,
		const std::string& sizePropertyName,
		const std::string& settingsPropertyName)
{
	m_sizePropertyName = sizePropertyName;
	m_settingPropertyName = settingsPropertyName;
	m_sizeString = framework::Parser::getPropertyValue(parsedCommand,
			sizePropertyName, &m_sizePropertyExists);

	m_settingsPropertyValue = framework::Parser::getPropertyValue(parsedCommand,
			settingsPropertyName, &m_settingsPropertyExists);

	if (!getIsRemaining() && m_sizePropertyExists)
	{
		m_size = stringToUInt64(m_sizeString);
	}
}

cli::framework::SyntaxErrorResult* cli::nvmcli::MemoryProperty::validate() const
{
	framework::SyntaxErrorResult *pResult = NULL;
	if (m_sizePropertyExists)
	{
		if (!getIsSizeValidNumber())
		{
			pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
					m_sizePropertyName, m_sizeString);
		}
		else if (m_settingsPropertyExists && !getIsSettingsValid())
		{
			pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_PROPERTY,
					m_settingPropertyName, m_settingsPropertyValue);
		}
	}
	else if (m_settingsPropertyExists) // AppDirectSettings requires corresponding AppDirectSize
	{
		pResult = new framework::SyntaxErrorMissingValueResult(framework::TOKENTYPE_PROPERTY,
				m_sizePropertyName);
	}
	return pResult;
}

bool cli::nvmcli::MemoryProperty::getIsSizeValidNumber() const
{
	bool result = true;
	if (!getIsRemaining())
	{
		result = isStringValidNumber(m_sizeString);
	}
	return result;
}

bool cli::nvmcli::MemoryProperty::convertSettingStringToInterleaveSize(
		const std::string& setting, enum interleave_size &size)
{
	bool result = true;
	if (framework::stringsIEqual(setting, APP_DIRECTSETTING_64B))
	{
		size = INTERLEAVE_SIZE_64B;
	}
	else if (framework::stringsIEqual(setting, APP_DIRECTSETTING_128B))
	{
		size = INTERLEAVE_SIZE_128B;
	}
	else if (framework::stringsIEqual(setting, APP_DIRECTSETTING_256B))
	{
		size = INTERLEAVE_SIZE_256B;
	}
	else if (framework::stringsIEqual(setting, APP_DIRECTSETTING_4KB))
	{
		size = INTERLEAVE_SIZE_4KB;
	}
	else if (framework::stringsIEqual(setting, APP_DIRECTSETTING_1GB))
	{
		size = INTERLEAVE_SIZE_1GB;
	}
	else // invalid interleave size
	{
		result = false;
	}
	return result;
}

bool cli::nvmcli::MemoryProperty::parseSettings()
{
	bool result = true;

	bool imcFound = false;
	bool channelFound = false;
	if (m_settingsPropertyExists && (result = tokenizeSettings()))
	{
		const size_t MAX_TOKENS = 2;
		size_t numTokens = m_settingsTokens.size();
		size_t tokenIdx = 0;
		// expect tokens in the format:
		//  IMC_CHANNEL
		if (numTokens > 0 && numTokens <= MAX_TOKENS)
		{
			// Check IMC/channel sizes
			for (; result && (tokenIdx < numTokens); tokenIdx++)
			{
				if (imcFound) // IMC was previously found, this is the channel setting
				{
					channelFound = true;
					result = convertSettingStringToInterleaveSize(
							m_settingsTokens[tokenIdx], m_format.channel);
				}
				else // This is the first size token - IMC setting
				{
					imcFound = true;
					result = convertSettingStringToInterleaveSize(
							m_settingsTokens[tokenIdx], m_format.imc);
				}
			}

			if (imcFound && !channelFound) // only IMC size was specified - channel should be same
			{
				m_format.channel = m_format.imc;
			}
		}
		else // wrong number of tokens - invalid
		{
			result = false;
		}
	}
	// either no settings provided at all, or no interleave sizes specified - use defaults (if possible)
	if (result && !imcFound && !channelFound)
	{
		// If there's no viable default, user needs to supply interleave sizes
		result = wbem::mem_config::MemoryCapabilitiesFactory::getRecommendedInterleaveSizes(
				m_format.imc, m_format.channel);
	}
	return result;
}

bool cli::nvmcli::MemoryProperty::validateSettings()
{
	bool result = parseSettings();

	if (result && m_format.imc < m_format.channel)
	{
		result = false;
	}

	return result;
}

bool cli::nvmcli::MemoryProperty::tokenizeSettings()
{
	bool result = true;

	if (m_settingsPropertyExists)
	{
		m_settingsTokens.clear();

		// Create a mutable copy to tokenize
		char tmp[m_settingsPropertyValue.size() + 1];
		s_strcpy(tmp, m_settingsPropertyValue.c_str(), sizeof (tmp));

		char *pRemainder = tmp;
		char *pToken = x_strtok(&pRemainder, wbem::mem_config::MEMORYPROP_TOKENSEP.c_str());
		while (pToken)
		{
			if (strlen(pToken) == 0) // delimiters arranged in an invalid way
			{
				result = false;
				break;
			}
			m_settingsTokens.push_back(std::string(pToken));

			pToken = x_strtok(&pRemainder, wbem::mem_config::MEMORYPROP_TOKENSEP.c_str());
		}
	}

	return result;
}

bool cli::nvmcli::MemoryProperty::getIsSizePart(const std::string& value) const
{
	std::string setting;
	size_t underscorePos = m_settingsPropertyValue.find("_");
	if (underscorePos != std::string::npos)
	{
		underscorePos ++; // after "_"
		setting = m_settingsPropertyValue.substr(underscorePos, m_settingsPropertyValue.size() - underscorePos);
	}
	else
	{
		setting = m_settingsPropertyValue;
	}
	return  framework::stringsIEqual(setting, value);
}

bool cli::nvmcli::MemoryProperty::getIsMirrored() const
{
	return getIsFirstSettingPart(wbem::mem_config::APP_DIRECT_SETTING_MIRROR);
}

bool cli::nvmcli::MemoryProperty::getIsByOne() const
{
	return getIsFirstSettingPart(wbem::mem_config::APP_DIRECT_SETTING_BYONE);
}

bool cli::nvmcli::MemoryProperty::getIsFirstSettingPart(
		const std::string& value) const
{
	std::string setting;
	size_t underscorePos = m_settingsPropertyValue.find("_");
	if (underscorePos != std::string::npos)
	{
		setting = m_settingsPropertyValue.substr(0, underscorePos);
	}
	else
	{
		setting = m_settingsPropertyValue;
	}
	return  framework::stringsIEqual(setting, value);
}

} /* namespace cli */
