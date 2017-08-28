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

#include "SystemSocket.h"

/*core::system::SystemSocket::SystemSocket(const socket &in_socket)
	: m_socket(in_socket), m_smbiosUtility(SmbiosType4Utility())
{

}*/

core::system::SystemSocket::SystemSocket(const socket &in_socket, SmbiosType4Utility &smbiosUtility)
		: m_socket(in_socket), m_smbiosUtility(smbiosUtility)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

core::system::SystemSocket *core::system::SystemSocket::clone()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return new SystemSocket(*this);
}

NVM_UINT16 core::system::SystemSocket::getSocketId()
{
    LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
    return m_socket.id;
}

NVM_UINT8 core::system::SystemSocket::getSocketType()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_socket.type;
}

NVM_UINT8 core::system::SystemSocket::getSocketFamily()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_socket.family;
}

std::string core::system::SystemSocket::getSocketManufacturer()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string result;

	result = m_socket.manufacturer;
	return result;
}

NVM_UINT64 core::system::SystemSocket::getSocketMappedMemoryLimit()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_socket.mapped_memory_limit;
}

NVM_UINT64 core::system::SystemSocket::getSocketTotalMappedMemory()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_socket.total_mapped_memory;
}

std::string core::system::SystemSocket::getSocketFamilyStr()
{
	return m_smbiosUtility.getProcessorFamily((unsigned int)getSocketFamily());
}

std::string core::system::SystemSocket::getSocketTypeStr()
{
	return m_smbiosUtility.getProcessorType((unsigned char)getSocketType());
}