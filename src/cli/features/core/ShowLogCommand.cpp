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

#include "ShowLogCommand.h"
#include <LogEnterExit.h>
#include <cr_i18n.h>
#include <cli/features/core/WbemToCli_utilities.h>
#include <libinvm-cli/Parser.h>
#include "FieldSupportFeature.h"

namespace cli
{
namespace nvmcli
{
ShowLogCommand::ShowLogCommand(core::logs::LogService &service) :
		m_service(service)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_props.addOther("Time", &core::logs::Log::getTime, getTimeInFormat).setIsDefault();
	m_props.addStr("FileName", &core::logs::Log::getFileName).setIsDefault();
	m_props.addUint32("LineNumber", &core::logs::Log::getLineNumber).setIsDefault();
	m_props.addOther("LogLevel", &core::logs::Log::getLogLevel, convertLoglevel).setIsDefault();
	m_props.addStr("Message", &core::logs::Log::getLogMessage).setIsDefault();
}

ShowLogCommand::~ShowLogCommand()
{
}

framework::ResultBase *ShowLogCommand::execute(const framework::ParsedCommand &parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_parsedCommand = parsedCommand;
	try
	{
		int requestedLogCount = parseLogCountPropertyValue();

		if (!hasError())
		{
			int actualLogCount = m_service.getLogsCount();
			m_logs = m_service.getAllLogsInDescOrderbyTime();
			if (requestedLogCount < actualLogCount)
			{
				m_logs.resize(requestedLogCount);
			}

			createResults();
		}
	}
	catch (core::LibraryException &e)
	{
		m_pResult = cli::nvmcli::CoreExceptionToResult(e);
	}

	return m_pResult;
}

void ShowLogCommand::createResults()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::ObjectListResult *pLogList = new framework::ObjectListResult();
	pLogList->setOutputType(framework::ResultBase::OUTPUT_TEXTTABLE);
	pLogList->setRoot(SHOWLOG_ROOT);

	for (size_t i = 0; i < m_logs.size(); i++)
	{
		framework::PropertyListResult value;
		for (size_t j = 0; j < m_props.size(); j++)
		{
			framework::IPropertyDefinition<core::logs::Log> &p = m_props[j];

			value.insert(p.getName(), p.getValue(m_logs[i]));
		}

		pLogList->insert(SHOWLOG_ROOT, value);
	}

	m_pResult = pLogList;
}

std::string ShowLogCommand::convertLoglevel(log_level loglevel)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::map<log_level, std::string> map;
	map[LOG_LEVEL_ERROR] = TR("Error");
	map[LOG_LEVEL_WARN] = TR("Warning");
	map[LOG_LEVEL_INFO] = TR("Informational");
	map[LOG_LEVEL_DEBUG] = TR("Debug");

	return map[loglevel];
}

std::string ShowLogCommand::getTimeInFormat(time_t timeValue)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	char formattedTime[255];
	struct tm *p_localTime = localtime(&timeValue);
	if (!p_localTime)
	{
		// this should never happen
		COMMON_LOG_ERROR("Unable to get local time for log entry.");
		s_strcpy(formattedTime, TR("Unknown"), sizeof(formattedTime));
	}
	else
	{
		strftime(formattedTime, sizeof(formattedTime), "%m:%d:%Y:%H:%M:%S", p_localTime);
	}

	return std::string(formattedTime);
}

int ShowLogCommand::parseLogCountPropertyValue()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	int countValue = SHOWLOG_DEFAULT_LOG_COUNT;
	bool countPropExists = false;
	std::string countStr =
			framework::Parser::getPropertyValue(m_parsedCommand,SHOWLOGS_COUNT, &countPropExists);

	if (countPropExists &&
			(!stringToInt(countStr, &countValue) ||
					countValue < 1 ||
					countValue > SHOWLOG_MAX_LOG_COUNT))
	{
		m_pResult = new framework::SyntaxErrorBadValueResult(
				framework::TOKENTYPE_PROPERTY, SHOWLOGS_COUNT, countStr);
	}

	return countValue;
}

bool ShowLogCommand::hasError()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_pResult != NULL;
}

}
}
