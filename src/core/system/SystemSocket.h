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
#ifndef CR_MGMT_SYSTEM_SOCKET_H
#define CR_MGMT_SYSTEM_SOCKET_H

#include <iostream>
#include <LogEnterExit.h>
#include <string>
#include <nvm_management.h>
#include <core/Helper.h>
#include <common_types.h>
#include <system/jedec_manufacturer.h>
#include <utility.h>
#include <nvm_types.h>
#include <vector>
#include <string/s_str.h>
#include <sstream>
#include <core/exceptions/LibraryException.h>
#include <core/Collection.h>
#include <core/NvmLibrary.h>
#include "SmbiosType4Utility.h"

namespace core
{
namespace system
{

class NVM_API SystemSocket
{
public:
	// [Comment:RYON]: No need for the lib parameter
	//SystemSocket(const socket &in_socket);
	SystemSocket(const socket &in_socket,
			SmbiosType4Utility &smbiosUtility = core::system::SmbiosType4Utility::getUtility());

	SystemSocket &operator=(const SystemSocket &other)
	{
		if (&other == this)
			return *this;
		this->m_socket = other.m_socket;
		return *this;
	}

	virtual ~SystemSocket() { }

	virtual SystemSocket *clone();
	virtual NVM_UINT16 getSocketId();
	virtual NVM_UINT8 getSocketType();
	virtual std::string getSocketTypeStr();
	virtual NVM_UINT8 getSocketFamily();
	virtual std::string getSocketFamilyStr();
	virtual std::string getSocketManufacturer();
	virtual NVM_UINT64 getSocketMappedMemoryLimit();
	virtual NVM_UINT64 getSocketTotalMappedMemory();
	virtual NVM_BOOL isCapacitySkuingSupported();

private:
	socket m_socket;
	const SmbiosType4Utility &m_smbiosUtility;
};

class NVM_API SystemSocketCollection : public Collection<SystemSocket>
{
};

}
}
#endif //CR_MGMT_SYSTEM_SOCKET_H
