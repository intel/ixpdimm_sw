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
#include "DeviceFirmwareInfoCollection.h"

#include <logic/NvmApi.h>
#include <logic/exceptions/NoMemoryException.h>


logic::device::DeviceFirmwareService *logic::device::DeviceFirmwareService::m_pSingleton =
		new logic::device::DeviceFirmwareService();

logic::device::DeviceFirmwareService &logic::device::DeviceFirmwareService::getService()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (!m_pSingleton)
	{
		throw NoMemoryException();
	}

	return *m_pSingleton;
}

logic::device::DeviceFirmwareInfo *logic::device::DeviceFirmwareService::getFirmwareInfo(
		const std::string &deviceGuid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_GUID guid;
	Helper::stringToGuid(deviceGuid, guid);
	device_fw_info fwInfo;
	int rc = m_pApi.getDeviceFwInfo(guid, &fwInfo);
	if (rc != NVM_SUCCESS)
	{
		throw LibraryException(rc);
	}

	DeviceFirmwareInfo *result = new DeviceFirmwareInfo(deviceGuid, fwInfo);
	return result;
}
