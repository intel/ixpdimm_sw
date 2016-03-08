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
#include "SystemInfo.h"

namespace core
{
namespace system
{

SystemInfo::SystemInfo() :
		m_info(),
		m_logLevel(0)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

SystemInfo::SystemInfo(struct host& host, int logLevel) :
		m_info(host),
		m_logLevel(logLevel)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

SystemInfo::SystemInfo(const SystemInfo &other) :
		m_info(other.m_info),
		m_logLevel(other.m_logLevel)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	copy(other);
}

SystemInfo &SystemInfo::operator=(const SystemInfo &other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (&other == this)
		return *this;

	copy(other);

	return *this;
}

void SystemInfo::copy(const SystemInfo &other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	this->m_info = other.m_info;
	this->m_logLevel = other.m_logLevel;
}

SystemInfo::~SystemInfo()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

SystemInfo *SystemInfo::clone()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return new SystemInfo(*this);
}

std::string SystemInfo::getHostName()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return std::string(m_info.name);
}

enum os_type SystemInfo::getOsType()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_info.os_type;
}

std::string SystemInfo::getOsName()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return std::string(m_info.os_name);
}

std::string SystemInfo::getOsVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return std::string(m_info.os_version);
}

bool SystemInfo::getMixedSku()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_info.mixed_sku;
}

bool SystemInfo::getSkuViolation()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_info.sku_violation;
}

NVM_UINT16 SystemInfo::getLogLevel()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return (NVM_UINT16)m_logLevel;
}

}
}
