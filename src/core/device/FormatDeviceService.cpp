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

#include "FormatDeviceService.h"
#include <LogEnterExit.h>
#include <core/exceptions/LibraryException.h>
#include <core/firmware_interface/FwCommands.h>

namespace core
{
namespace device
{

FormatDeviceService::FormatDeviceService(firmware_interface::FwCommandsWrapper &fwCommands) :
		m_fwCommands(fwCommands)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

FormatDeviceService::~FormatDeviceService()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

core::device::FormatDeviceService &core::device::FormatDeviceService::getService()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	// Creating the singleton on class init as a static class member
	// can lead to static initialization order issues.
	// This is a thread-safe form of lazy initialization.
	static FormatDeviceService *pSingleton = new FormatDeviceService();
	return *pSingleton;
}

void FormatDeviceService::startFormatForDevice(const NVM_UINT32 deviceHandle)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	struct fwcmd_format_result result =
			m_fwCommands.FwcmdCallFormat(deviceHandle, 0, 0);

	if (!result.success)
	{
		// Format is a blocking command - will time out.
		// Our nvm error code depends on _DSM error code returned.
		int rc = getLibraryErrorCode(result.error_code);
		if (rc != NVM_SUCCESS && rc != NVM_ERR_TIMEOUT)
		{
			throw core::LibraryException(rc);
		}
	}
}

NVM_UINT8 FormatDeviceService::isFormatComplete(const NVM_UINT32 deviceHandle)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT8 format_success = NVM_FORMAT_BSR_POLL_INCOMPLETE;

	struct fwcmd_bsr_result result = m_fwCommands.FwcmdAllocBsr(deviceHandle);

	if (result.success)
	{
		if(result.p_data->rest1_mailbox_ready == 0 && result.p_data->rest1_assertion == 0 )
		{
			format_success = NVM_FORMAT_SUCCESS;
		}
		else
		{
			format_success = NVM_FORMAT_FAILURE;
		}
	}
	else
	{
		int rc = getLibraryErrorCode(result.error_code);
		m_fwCommands.FwcmdFreeBsr(&result);
		throw core::LibraryException(rc);
	}

	m_fwCommands.FwcmdFreeBsr(&result);
	return format_success;
}

int FormatDeviceService::getLibraryErrorCode(struct fwcmd_error_code& fwError)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	firmware_interface::FwCommands fwCommands(m_fwCommands);
	return fwCommands.convertFwcmdErrorCodeToNvmErrorCode(fwError);
}

bool FormatDeviceService::isDeviceInFormattableState(core::device::Device& device)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	bool formattableState = false;

	std::vector<NVM_UINT16> bootStatus = device.getBootStatus();
	for (std::vector<NVM_UINT16>::iterator statusItem = bootStatus.begin();
			statusItem != bootStatus.end(); statusItem++)
	{
		if (*statusItem == DEVICE_BOOT_STATUS_MEDIA_DISABLED)
		{
			formattableState = true;
			break;
		}
	}

	return formattableState;
}

} /* namespace device */
} /* namespace core */
