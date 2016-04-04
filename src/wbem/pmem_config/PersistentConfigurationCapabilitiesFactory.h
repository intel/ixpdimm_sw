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
 * This file contains the provider for the PersistentConfigurationCapabilities instances.
 */

#include <server/BaseServerFactory.h>
#include <framework_interface/NvmInstanceFactory.h>

#ifndef	_WBEM_PMEMCONFIG_PERSISTENTCONFIGURATIONCAPABILITIES_H_
#define	_WBEM_PMEMCONFIG_PERSISTENTCONFIGURATIONCAPABILITIES_H_
namespace wbem
{
namespace pmem_config
{

static const std::string PM_CAP_CLASSNAME =  std::string(NVM_WBEM_PREFIX) + "PersistentConfigurationCapabilities";
static const std::string PM_CAP_INSTANCEID = "Persistent Memory Service Capabilities";
static const std::string PM_CAP_ELEMENTNAME = PM_CAP_INSTANCEID;


class NVM_API PersistentConfigurationCapabilitiesFactory : public framework_interface::NvmInstanceFactory
{
public:
	/*!
	 * Initialize a new PersistentConfigurationCapabilitiesFactory.
	 */
	PersistentConfigurationCapabilitiesFactory() throw (framework::Exception);

	/*!
	 * Clean up the PersistentConfigurationCapabilitiesFactory
	 */
	~PersistentConfigurationCapabilitiesFactory();

	/*!
	 * Implementation of the standard CIM method to retrieve a specific instance
	 * @param[in] path
	 * 		The object path of the instance to retrieve.
	 * @param[in] attributes
	 * 		The attributes to retrieve.
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
};
}

}

#endif /* _WBEM_PMEMCONFIG_PERSISTENTCONFIGURATIONCAPABILITIES_H_ */
