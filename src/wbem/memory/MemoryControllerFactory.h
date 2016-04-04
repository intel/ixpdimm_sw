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
 * This file contains the provider for the MemoryController instances that
 * model the memory controller associated with a set of NVM DIMMs.
 */


#ifndef _WBEM_MEMORY_MEMORY_CONTROLLER_FACTORY_H_
#define _WBEM_MEMORY_MEMORY_CONTROLLER_FACTORY_H_

#include <string>

#include <nvm_management.h>

#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace memory
{
	static const std::string MEMORYCONTROLLER_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "MemoryController"; //!< CreationClassName static

/*!
 * Models the NVDIMM Memory Controller Configuration
 */
class NVM_API MemoryControllerFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new IntelMemoryControllerFactory
		 */
		MemoryControllerFactory() throw (framework::Exception);

		/*!
		 * Clean up the IntelMemoryControllerFactory
		 */
		~MemoryControllerFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the MemoryController info
		 * @return The MemoryController instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * MemoryController object paths.
		 * @throw Exception if unable to retrieve the MemoryController list
		 * @throw Exception if unable to retrieve the server name
		 * @return The object paths for each MemoryController.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*!
		 * Overload of standard CIM method to retrieve a list of instances in this factory.
		 * @param[in] attributes
		 * 		The list of attribute names to retrieve for each instance.
		 * @remarks Default implementation that uses getInstanceNames, getInstance and
		 * populateAttributeList
		 * @return
		 * 		The list of instances.
		 */
		framework::instances_t* getInstances(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*!
		 * Static helper function to generate the MemoryController Device ID
		 * @param pDiscovery
		 * @return
		 */
		static std::string generateUniqueMemoryControllerID(
				struct device_discovery *pDiscovery);

		/*!
		 *
		 * @param associationClass
		 * @param pAntInstance
		 * @param pDepInstance
		 * @return
		 */
		bool isAssociated(const std::string &associationClass, framework::Instance *pAntInstance,
				framework::Instance *pDepInstance);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);


		void addNonKeyAttributesToInstance(
				framework::Instance *pInstance,
				framework::attribute_names_t *pAttrNames,
				struct device_discovery *pDiscovery);

		framework::ObjectPath getInstanceObjectPath(
				std::string &hostServerName,
				std::string &memory_controller_id);

		int getInstancesHelperLoop(
				framework::instance_names_t *pNames,
				framework::instances_t *pInstList,
				framework::attribute_names_t *pAttrNames);

};

} // memory
} // wbem

#endif // _WBEM_MEMORY_MEMORY_CONTROLLER_FACTORY_H_
