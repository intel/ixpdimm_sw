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
#ifndef CR_MGMT_LIBRARYEXCEPTION_H
#define CR_MGMT_LIBRARYEXCEPTION_H

#include <exception>
#include <string>
#include <lib/nvm_types.h>
#include <LogEnterExit.h>
#include <lib/nvm_management.h>
#include <cr_i18n.h>
#include <common/string/s_str.h>

namespace core
{
class NVM_API LibraryException : public std::exception
{
public:
	LibraryException(int errorCode) : m_errorCode(errorCode)
	{
		NVM_ERROR_DESCRIPTION description;
		if (nvm_get_error((enum return_code) errorCode, description, NVM_ERROR_LEN) != NVM_SUCCESS)
		{
			s_snprintf(description, sizeof(description), N_TR("The Native API Library returned an unknown error code: %d."), errorCode);
		}

		m_message = description;
	}

	~LibraryException() throw () {}

	int getErrorCode()
	{
		LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
		return m_errorCode;
	}


	virtual const char *what() const throw ()
	{
		return m_message.c_str();
	}
private:
	int m_errorCode;
	std::string m_message;
};

}

#endif //CR_MGMT_LIBRARYEXCEPTION_H
