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
 * This file contains the provider for the SanitizeJob instances which
 * provide security santize operations on an NVM DIMM.
 */

#ifndef	_WBEM_SUPPORT__FACTORY_H_
#define	_WBEM_SUPPORT_SANITIZEJOB_FACTORY_H_

#include <string>
#include <framework_interface/NvmInstanceFactory.h>
#include <server/BaseServerFactory.h>
#include <nvm_management.h>




#include <stdio.h>


namespace wbem
{
namespace support
{
	static const std::string SANITIZEJOB_ELEMENTNAME = "SanitizeJob";
	static const std::string SANITIZEJOB_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "SanitizeJob";


/*!
 * Uniquely identifies a job instance
 */
class NVM_API SanitizeJobFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new SupportDataServiceFactory.
		 */
		SanitizeJobFactory() throw (framework::Exception);

		/*!
		 * Clean up the SupportDataServiceFactory
		 */
		~SanitizeJobFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the DIMM info
		 * @return The DiagnosticIdentity instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * DiagnosticIdentity object paths.
		 * @throw Exception if unable to retrieve the DIMM list
		 * @throw Exception if unable to retrieve the server name
		 * @return The object paths for each DiagnosticIdentify.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);
};

} // support
} // wbem
#endif  // #ifndef _WBEM_SUPPORT_SANITIZEJOB_FACTORY_H_





