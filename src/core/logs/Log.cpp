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

#include "Log.h"
#include <LogEnterExit.h>

namespace core
{
namespace logs
{

Log::Log()
{
	memset(&m_log, 0, sizeof(m_log));
}

Log::Log(const struct nvm_log &templog)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	memmove(&m_log, &templog, sizeof(m_log));
}

Log::Log(const Log &other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	copy(other);
}

Log::~Log()
{
}

Log &Log::operator=(const Log &other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (&other == this)
		return *this;

	copy(other);

	return *this;
}

bool Log::operator>(Log rhs) const
{
	return (m_log.time > rhs.getTime());
}

void Log::copy(const Log &other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	this->m_log = other.m_log;
}

Log *Log::clone() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return new Log(*this);
}

std::string Log::getFileName()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return std::string(m_log.file_name);
}

NVM_UINT32 Log::getLineNumber()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_log.line_number;
}

std::string Log::getLogMessage()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return std::string(m_log.message);
}

time_t Log::getTime()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_log.time;
}

enum log_level Log::getLogLevel()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_log.level;
}

}
}
