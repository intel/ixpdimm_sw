/*
 * Copyright (c) 2015 2016, Intel Corporation
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

/*
 * Exception classes for bad memory allocation requests
 */

#ifndef _WBEM_EXCEPTION_NVMEXCEPTIONBADREQUEST_H_
#define _WBEM_EXCEPTION_NVMEXCEPTIONBADREQUEST_H_

#include <exception>
#include <string>
#include <nvm_types.h>

namespace core
{

class NvmExceptionBadRequest : public std::exception
{
	public:
		NvmExceptionBadRequest() :
			m_message("The request was invalid.")
		{}

		NvmExceptionBadRequest(const std::string message) :
			m_message(message)
		{}

		virtual ~NvmExceptionBadRequest() throw() {}

		virtual const char *what() const throw()
		{
			return m_message.c_str();
		}

	protected:
		std::string m_message;
};

class NvmExceptionBadRequestNoDimms : public NvmExceptionBadRequest
{
	public:
		NvmExceptionBadRequestNoDimms() :
			NvmExceptionBadRequest("No " NVM_DIMM_NAME "s were included in the request.")
		{}
};

class NvmExceptionBadRequestSize : public NvmExceptionBadRequest
{
	public:
		NvmExceptionBadRequestSize() :
			NvmExceptionBadRequest("The requested capacity was invalid.")
		{}
};

class NvmExceptionBadRequestMemorySize : public NvmExceptionBadRequestSize
{
	public:
		NvmExceptionBadRequestMemorySize() :
			NvmExceptionBadRequestSize()
		{
			m_message = "The requested Memory Mode capacity was invalid.";
		}
};

class NvmExceptionInvalidDimm : public NvmExceptionBadRequest
{
	public:
		NvmExceptionInvalidDimm() :
			NvmExceptionBadRequest("The request contained an invalid " NVM_DIMM_NAME ".")
		{}
};

class NvmExceptionBadDimmList : public NvmExceptionBadRequest
{
	public:
		NvmExceptionBadDimmList() :
			NvmExceptionBadRequest("The list of " NVM_DIMM_NAME
					"s in the request was invalid.")
		{}
};

class NvmExceptionRequestNotSupported : public NvmExceptionBadRequest
{
	public:
		NvmExceptionRequestNotSupported() :
			NvmExceptionBadRequest("The request is not supported.")
		{}
};

class NvmExceptionBadRequestRemaining : public NvmExceptionBadRequest
{
	public:
		NvmExceptionBadRequestRemaining() :
			NvmExceptionBadRequest("The request contained more than one request for "
					"remaining capacity.")
		{}
};

class NvmExceptionProvisionCapacityNotSupported : public NvmExceptionBadRequest
{
	public:
		NvmExceptionProvisionCapacityNotSupported() :
			NvmExceptionBadRequest("Provisioning capacity is not supported.")
		{}
};

class NvmExceptionAppDirectSettingsNotSupported : public NvmExceptionBadRequest
{
	public:
		NvmExceptionAppDirectSettingsNotSupported() :
			NvmExceptionBadRequest("The requested app direct memory settings are not supported.")
		{}
};

class NvmExceptionMemoryModeNotSupported : public NvmExceptionBadRequest
{
	public:
		NvmExceptionMemoryModeNotSupported() :
			NvmExceptionBadRequest("The requested Memory Mode settings are not supported.")
		{}
};

class NvmExceptionNamespacesExist : public NvmExceptionBadRequest
{
	public:
		NvmExceptionNamespacesExist() :
			NvmExceptionBadRequest("Namespaces exist on the requested " NVM_DIMM_NAME "s.")
		{}
};

class NvmExceptionDimmHasConfigGoal : public NvmExceptionBadRequest
{
	public:
		NvmExceptionDimmHasConfigGoal() :
			NvmExceptionBadRequest("A requested " NVM_DIMM_NAME " already has a memory allocation goal.")
		{}
};

class NvmExceptionBadRequestDoesntContainRequiredDimms : public NvmExceptionBadRequest
{
	public:
		NvmExceptionBadRequestDoesntContainRequiredDimms() :
			NvmExceptionBadRequest("The request does not contain all required " NVM_DIMM_NAME "s.")
		{}
};

class NvmExceptionOverAddressDecoderLimit : public NvmExceptionBadRequest
{
	public:
		NvmExceptionOverAddressDecoderLimit() :
			NvmExceptionBadRequest("The request would exceed address decoder resources.")
		{}
};

class NvmExceptionStorageNotSupported : public NvmExceptionBadRequest
{
	public:
		NvmExceptionStorageNotSupported() :
			NvmExceptionBadRequest("The requested storage settings are not supported.")
		{}
};

class NvmExceptionBadRequestReserveDimm : public NvmExceptionBadRequest
{
	public:
		NvmExceptionBadRequestReserveDimm() :
			NvmExceptionBadRequest("The request was unable to reserve a " NVM_DIMM_NAME
				" for storage with requested settings.")
		{}
};

class NvmExceptionUnacceptableLayoutDeviation : public NvmExceptionBadRequest
{
	public:
		NvmExceptionUnacceptableLayoutDeviation() :
			NvmExceptionBadRequest("Requested capacities were adjusted beyond the maximum "
					"acceptable layout deviation.")
		{}
};

class NvmExceptionTooManyAppDirectExtents : public NvmExceptionBadRequest
{
	public:
		NvmExceptionTooManyAppDirectExtents() :
			NvmExceptionBadRequest("The request contains too many app direct extents.")
		{}
};

class NvmExceptionRequestedDimmLocked : public NvmExceptionBadRequest
{
	public:
		NvmExceptionRequestedDimmLocked() :
			NvmExceptionBadRequest("One or more requested " NVM_DIMM_NAME "s are locked.")
		{}
};

class NvmExceptionPartialResultsCouldNotBeUndone : public NvmExceptionBadRequest
{
	public:
		NvmExceptionPartialResultsCouldNotBeUndone() :
			NvmExceptionBadRequest("Request did not complete in its entirety and partial"
					"results could not be undone.")
		{}
};

}

#endif /* _CORE_EXCEPTION_NVMEXCEPTIONBADREQUEST_H_ */
