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
 * This file contains the definition of the exception class for an invalid parameter.
 */


#ifndef	_WBEM_FRAMEWORK_NVMEXCEPTION_BADTARGET_H_
#define	_WBEM_FRAMEWORK_NVMEXCEPTION_BADTARGET_H_

#include <libinvm-cim/Exception.h>
#include <nvm_types.h>


namespace wbem
{
namespace exception
{

/*!
 * An exception for an invalid target
 */
class NVM_API NvmExceptionBadTarget: public framework::Exception
{
	public:
	/*!
	 * Initialize a bad target exception
	 * @param[in] pTarget
	 * 		The WBEM target key.
	 * @param[in] pTargetValue
	 * 		The bad value of the target key type
	 */
	NvmExceptionBadTarget(const char *pTarget, const char *pBadTargetValue);
	~NvmExceptionBadTarget() throw () {}

	/*
	 * return the bad target value that this exception was thrown with
	 */
	std::string getBadTargetValue();

	/*
	 * return the target key for this exception (ie: "dimmID"
	 */
	std::string getTarget();

	private:
		std::string m_badTargetValue;
		std::string m_target;
};

} // framework
} // wbem

#endif // _WBEM_FRAMEWORK_NVMEXCEPTION_BADTARGET_H_
