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
#ifndef CR_MGMT_DEVICEFIRMWAREINFO_H
#define CR_MGMT_DEVICEFIRMWAREINFO_H


#include <string>
#include <nvm_management.h>


namespace core
{
namespace device
{
class NVM_API DeviceFirmwareInfo
{
public:
	DeviceFirmwareInfo(std::string uid, device_fw_info fw_info) : m_uid(uid), m_info(fw_info) { }

	std::string getUid() const { return m_uid; }

	std::string getActiveRevision() const;
	enum device_fw_type getActiveType() const;
	std::string getActiveCommitId() const;
	std::string getActiveBuildConfiguration() const;

	std::string getStagedRevision() const;
	enum device_fw_type getStagedType() const;
	bool isStagedPending() const;

	DeviceFirmwareInfo * clone() const;
private:
	std::string m_uid;
	device_fw_info m_info;
};
}
}
#endif //CR_MGMT_DEVICEFIRMWAREINFO_H
