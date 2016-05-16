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
 * This file contains the provider for the NVDIMMDriverIdentity instances
 * which represents the version of the NVM DIMM drivers.
 */

#ifndef	_WBEM_SOFTWARE_NVDIMMDRIVERIDENTITY_FACTORY_H_
#define	_WBEM_SOFTWARE_NVDIMMDRIVERIDENTITY_FACTORY_H_

#include <string>
#include <vector>

#include <framework_interface/NvmInstanceFactory.h>
#include <core/system/SystemService.h>

namespace wbem
{
namespace software
{
	static const std::string NVDIMMDRIVERIDENTITY_INSTANCEID = "ProductDriver"; //!< Instance ID static string
	static const std::string NVDIMMDRIVERIDENTITY_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "NVDIMMDriverIdentity"; //!< Creation class name
	static const NVM_UINT16 NVDIMMDRIVERIDENTITY_CLASSIFICATIONS_DRIVER = 2;

/*!
 * Provider Factory for NVDIMMDriverIdentity
 */
class NVM_API NVDIMMDriverIdentityFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new NVDIMMDriverIdentityFactory.
		 */
		NVDIMMDriverIdentityFactory(
				core::system::SystemService &systemService = core::system::SystemService::getService())
			throw (framework::Exception);

		/*!
		 * Clean up the NVDIMMDriverIdentityFactory
		 */
		~NVDIMMDriverIdentityFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the host information.
		 * @todo Should throw an exception if the object path doesn't match
		 * the results of getHostName.
		 * @return The instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * object paths.
		 * @return The object path.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		std::string getInstanceId();
		std::string getElementName();

		core::system::SystemService &m_systemService;
		std::string m_hostName;
};

} // software
} // wbem
#endif  // _WBEM_SOFTWARE_NVDIMMDRIVERIDENTITY_FACTORY_H_
