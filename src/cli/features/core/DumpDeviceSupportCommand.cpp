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

#include "DumpDeviceSupportCommand.h"

namespace cli
{
namespace nvmcli
{

cli::nvmcli::DumpDeviceSupportCommand::DumpDeviceSupportCommand(
		core::device::DeviceService &service) :	m_service(service), m_pResult(NULL)
{ }

cli::nvmcli::DumpDeviceSupportCommand::~DumpDeviceSupportCommand()
{ }

void DumpDeviceSupportCommand::validateInput()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string result, dimmId;
	bool dimm_exists, support_exists, destination_exists;

	m_dimmIds = framework::Parser::getTargetValues(m_parsedCommand, TARGET_DIMM_R.name,
			&dimm_exists);
	m_destination = framework::Parser::getOptionValue(m_parsedCommand,
			framework::OPTION_DESTINATION_R.name, &destination_exists);
	result = framework::Parser::getTargetValue(m_parsedCommand, TARGET_SUPPORT_R.name,
			&support_exists);

	if (!dimm_exists)
	{
		m_pResult = new cli::framework::SyntaxErrorMissingValueResult(
				cli::framework::TOKENTYPE_TARGET, "dimm");
	}
	else if (!destination_exists || m_destination.empty())
	{
		m_pResult = new cli::framework::SyntaxErrorMissingValueResult(
				cli::framework::TOKENTYPE_OPTION, "destination");
	}
	else if (!support_exists)
	{
		m_pResult = new cli::framework::SyntaxErrorMissingValueResult(
				cli::framework::TOKENTYPE_TARGET, "support");
	}
	else if (!result.empty())
	{
		m_pResult = new cli::framework::SyntaxErrorUnexpectedValueResult(
				cli::framework::TOKENTYPE_TARGET, "support");
	}
	else
	{
		// Check for valid input dimmIds
		for (size_t i = 0; i < m_dimmIds.size(); i++)
		{
			dimmId = m_dimmIds[i];

			if (!isStringValidNumber(dimmId))
			{
				m_pResult = new cli::framework::SyntaxErrorBadValueResult(
						cli::framework::TOKENTYPE_TARGET, "dimm", dimmId);
				break;
			}
		}
	}
}

framework::ResultBase *DumpDeviceSupportCommand::
		execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Get command parameters
	m_parsedCommand = parsedCommand;
	validateInput();

	if (NULL == m_pResult) // Input parameters are valid
	{
		try
		{
			if (m_dimmIds.empty())
			{
				m_devices = m_service.getAllDevices();
				for (size_t i = 0; i < m_devices.size(); i++)
				{
					m_dimmIds.push_back(uint64ToString(m_devices[i].getDeviceHandle()));
				}
			}

			m_uids = m_service.getUidsForDeviceIds(m_dimmIds);
		}
		catch (core::LibraryException &e)
		{
			m_pResult = cli::nvmcli::CoreExceptionToResult(e);
		}

		if (!m_uids.empty())
		{
			createResults();
		}
	}

	return m_pResult;
}

void DumpDeviceSupportCommand::createResults()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::SimpleListResult *pListResult;
	pListResult = new framework::SimpleListResult();

	std::string result, dimmId;
	NVM_PATH files[NVM_MAX_EAFD_FILES];
	NVM_SIZE file_length;
	NVM_UID uid;
	NVM_PATH file;

	memset(files, 0, sizeof (files)); // Will get populated in NVM API layer
	for (size_t i = 0; i < m_dimmIds.size(); i++)
	{
		dimmId = m_dimmIds[i];
		if (!cli::nvmcli::isStringHex(dimmId))
		{
			dimmId = "0x" + dimmId;
		}

		file_length = m_destination.size();
		uid_copy(m_uids[i].c_str(), uid);
		s_strcpy(file, m_destination.c_str(), NVM_PATH_LEN);
		result = DUMPDEVICESUPPORT_HEADER + NVM_DIMM_NAME + " (" + dimmId + ").\n";

		try
		{
			int rc = m_service.dumpDeviceSupport(uid, file, file_length, files);

			// Print all blob files for current dimm.
			if (NVM_MIN_EAFD_FILES <= rc)
			{
				for (int j = 0; j < rc; j++)
				{
					result += DUMPDEVICESUPPORT_MSG + NVM_DIMM_NAME + " (" + dimmId + "): " +
							WRITE_SUCCESS + files[j] + "\n";
				}
			}
		}
		catch (core::LibraryException &e)
		{
			result += cli::nvmcli::CoreExceptionToResult(e)->outputText();
		}

		result += "\n";
		pListResult->insert(result);
	}

	m_pResult = pListResult;
}

}
}
