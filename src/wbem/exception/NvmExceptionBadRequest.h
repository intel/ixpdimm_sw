/*
 * Copyright (c) 2015, Intel Corporation
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

#include <intel_cim_framework/Exception.h>
#include <nvm_types.h>

namespace wbem
{
namespace exception
{

class NvmExceptionBadRequest : public framework::Exception
{
	public:
		NvmExceptionBadRequest() :
			framework::Exception("The request was invalid.")
		{}

		NvmExceptionBadRequest(const std::string message) :
			framework::Exception(message.c_str())
		{}
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

class NvmExceptionBadRequestVolatileSize : public NvmExceptionBadRequestSize
{
	public:
		NvmExceptionBadRequestVolatileSize() :
			NvmExceptionBadRequestSize()
		{
			m_Message = "The requested volatile capacity was invalid.";
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

class NvmExceptionPersistentSettingsNotSupported : public NvmExceptionBadRequest
{
	public:
		NvmExceptionPersistentSettingsNotSupported() :
			NvmExceptionBadRequest("The requested persistent memory settings are not supported.")
		{}
};

class NvmExceptionVolatileNotSupported : public NvmExceptionBadRequest
{
	public:
		NvmExceptionVolatileNotSupported() :
			NvmExceptionBadRequest("The requested volatile memory settings are not supported.")
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

class NvmExceptionTooManyPersistentExtents : public NvmExceptionBadRequest
{
	public:
		NvmExceptionTooManyPersistentExtents() :
			NvmExceptionBadRequest("The request contains too many persistent extents.")
		{}
};

} /* namespace exception */
} /* namespace wbem */

#endif /* _WBEM_EXCEPTION_NVMEXCEPTIONBADREQUEST_H_ */
