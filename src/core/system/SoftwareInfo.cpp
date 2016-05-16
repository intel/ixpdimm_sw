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

#include "SoftwareInfo.h"
#include <LogEnterExit.h>
#include <string/revision.h>

core::system::SoftwareInfo::SoftwareInfo() :
		m_swInventory(),
		m_mgmtMajorVersion(0), m_mgmtMinorVersion(0),
		m_mgmtHotfixVersion(0), m_mgmtBuildVersion(0),
		m_driverMajorVersion(0), m_driverMinorVersion(0),
		m_driverHotfixVersion(0), m_driverBuildVersion(0)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::system::SoftwareInfo::SoftwareInfo(const SoftwareInfo& other) :
		m_swInventory(other.m_swInventory),
		m_mgmtMajorVersion(other.m_mgmtMajorVersion),
		m_mgmtMinorVersion(other.m_mgmtMinorVersion),
		m_mgmtHotfixVersion(other.m_mgmtHotfixVersion),
		m_mgmtBuildVersion(other.m_mgmtBuildVersion),
		m_driverMajorVersion(other.m_driverMajorVersion),
		m_driverMinorVersion(other.m_driverMinorVersion),
		m_driverHotfixVersion(other.m_driverHotfixVersion),
		m_driverBuildVersion(other.m_driverBuildVersion)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::system::SoftwareInfo::SoftwareInfo(const struct sw_inventory& swInv) :
		m_swInventory(swInv),
		m_mgmtMajorVersion(0), m_mgmtMinorVersion(0),
		m_mgmtHotfixVersion(0), m_mgmtBuildVersion(0),
		m_driverMajorVersion(0), m_driverMinorVersion(0),
		m_driverHotfixVersion(0), m_driverBuildVersion(0)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	parse_main_revision(&m_mgmtMajorVersion, &m_mgmtMinorVersion,
			&m_mgmtHotfixVersion, &m_mgmtBuildVersion,
			m_swInventory.mgmt_sw_revision,
			sizeof (m_swInventory.mgmt_sw_revision));

	parse_main_revision(&m_driverMajorVersion, &m_driverMinorVersion,
			&m_driverHotfixVersion, &m_driverBuildVersion,
			m_swInventory.vendor_driver_revision,
			sizeof (m_swInventory.vendor_driver_revision));
}

core::system::SoftwareInfo::~SoftwareInfo()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::system::SoftwareInfo& core::system::SoftwareInfo::operator=(
		const SoftwareInfo& other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (&other != this)
	{
		m_swInventory = other.m_swInventory;
		m_mgmtMajorVersion = other.m_mgmtMajorVersion;
		m_mgmtMinorVersion = other.m_mgmtMinorVersion;
		m_mgmtHotfixVersion = other.m_mgmtHotfixVersion;
		m_mgmtBuildVersion = other.m_mgmtBuildVersion;
		m_driverMajorVersion = other.m_driverMajorVersion;
		m_driverMinorVersion = other.m_driverMinorVersion;
		m_driverHotfixVersion = other.m_driverHotfixVersion;
		m_driverBuildVersion = other.m_driverBuildVersion;
	}

	return *this;
}

std::string core::system::SoftwareInfo::getMgmtSoftwareVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_swInventory.mgmt_sw_revision;
}

NVM_UINT16 core::system::SoftwareInfo::getMgmtSoftwareMajorVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_mgmtMajorVersion;
}

NVM_UINT16 core::system::SoftwareInfo::getMgmtSoftwareMinorVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_mgmtMinorVersion;
}

NVM_UINT16 core::system::SoftwareInfo::getMgmtSoftwareHotfixVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_mgmtHotfixVersion;
}

NVM_UINT16 core::system::SoftwareInfo::getMgmtSoftwareBuildVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_mgmtBuildVersion;
}

std::string core::system::SoftwareInfo::getDriverVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_swInventory.vendor_driver_revision;
}

bool core::system::SoftwareInfo::isDriverSupported()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_swInventory.vendor_driver_compatible;
}

bool core::system::SoftwareInfo::isDriverInstalled()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool driverInstalled = false;
	if (strlen(m_swInventory.vendor_driver_revision) > 0)
	{
		driverInstalled = true;
	}

	return driverInstalled;
}

NVM_UINT16 core::system::SoftwareInfo::getDriverMajorVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_driverMajorVersion;
}

NVM_UINT16 core::system::SoftwareInfo::getDriverMinorVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_driverMinorVersion;
}

NVM_UINT16 core::system::SoftwareInfo::getDriverHotfixVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_driverHotfixVersion;
}

NVM_UINT16 core::system::SoftwareInfo::getDriverBuildVersion()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return m_driverBuildVersion;
}

core::system::SoftwareInfo* core::system::SoftwareInfo::clone()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return new SoftwareInfo(*this);
}
