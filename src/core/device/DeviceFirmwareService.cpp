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

#include "DeviceFirmwareService.h"
#include <core/exceptions/NoMemoryException.h>
#include <LogEnterExit.h>

core::device::DeviceFirmwareService *core::device::DeviceFirmwareService::m_pSingleton =
		new core::device::DeviceFirmwareService();

core::device::DeviceFirmwareService &core::device::DeviceFirmwareService::getService()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (!m_pSingleton)
	{
		throw NoMemoryException();
	}

	return *m_pSingleton;
}

core::Result<core::device::DeviceFirmwareInfo> core::device::DeviceFirmwareService::getFirmwareInfo(
		const std::string &deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	device_fw_info fwInfo =  m_lib.getDeviceFwInfo(deviceUid);

	DeviceFirmwareInfo firmwareInfo(deviceUid, fwInfo);
	core::Result<DeviceFirmwareInfo> result(firmwareInfo);
	return result;
}

core::device::DeviceFirmwareService::~DeviceFirmwareService()
{
	if (this == m_pSingleton)
	{
		m_pSingleton = NULL;
	}
}
