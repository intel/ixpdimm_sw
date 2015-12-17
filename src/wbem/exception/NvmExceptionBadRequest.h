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
};

class NvmExceptionBadRequestNoDimms : public NvmExceptionBadRequest
{
};

class NvmExceptionBadRequestSize : public NvmExceptionBadRequest
{
};

class NvmExceptionBadRequestVolatileSize : public NvmExceptionBadRequestSize
{
};

class NvmExceptionInvalidDimm : public NvmExceptionBadRequest
{
};

class NvmExceptionBadDimmList : public NvmExceptionBadRequest
{
};

class NvmExceptionRequestNotSupported : public NvmExceptionBadRequest
{
};

class NvmExceptionBadRequestRemaining : public NvmExceptionBadRequest
{
};

class NvmExceptionPersistentSettingsNotSupported : public NvmExceptionBadRequest
{
};

class NvmExceptionVolatileNotSupported : public NvmExceptionBadRequest
{
};

class NvmExceptionNamespacesExist : public NvmExceptionBadRequest
{
};

class NvmExceptionDimmHasConfigGoal : public NvmExceptionBadRequest
{
};

class NvmExceptionBadRequestDoesntContainRequiredDimms : public NvmExceptionBadRequest
{
};

class NvmExceptionOverAddressDecoderLimit : public NvmExceptionBadRequest
{
};

class NvmExceptionStorageNotSupported : public NvmExceptionBadRequest
{
};

class NvmExceptionBadRequestReserveDimm : public NvmExceptionBadRequest
{
};

class NvmExceptionUnacceptableLayoutDeviation : public NvmExceptionBadRequest
{
};

class NvmExceptionTooManyPersistentExtents : public NvmExceptionBadRequest
{
};

} /* namespace exception */
} /* namespace wbem */

#endif /* _WBEM_EXCEPTION_NVMEXCEPTIONBADREQUEST_H_ */
