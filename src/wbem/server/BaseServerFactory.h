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
 * This file contains the provider for the BaseServer instances which models
 * the server containing the NVM DIMMs.
 */

#ifndef	_WBEM_SERVER_BASESERVER_FACTORY_H_
#define	_WBEM_SERVER_BASESERVER_FACTORY_H_

#include <string>

#include <framework_interface/NvmInstanceFactory.h>

#include <core/system/SystemService.h>

namespace wbem
{
namespace server
{
	static const std::string BASESERVER_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "BaseServer"; //!< Creation ClassName static

	static const NVM_UINT16 BASESERVER_OPSTATUS_OK = 2;
	static const NVM_UINT16 BASESERVER_OPSTATUS_MIXEDSKU = 32768;
	static const NVM_UINT16 BASESERVER_OPSTATUS_SKUVIOLATION = 32769;

	/*!
	 * Utility function to retrieve the host name as a standard string.
	 * @return - std::string version of the network name of the host.
	 */
	const NVM_API std::string getHostName() throw (wbem::framework::Exception);

/*!
 * The SNIA base server profile models the host server.
 */
class NVM_API BaseServerFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new BaseServerFactory.
		 */
		BaseServerFactory() throw (framework::Exception);

		/*!
		 * Clean up the BaseServerFactory
		 */
		~BaseServerFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the host information.
		 * @todo Should throw an exception if the object path doesn't match
		 * the results of getHostName.
		 * @return The host server instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * host server object paths.
		 * @remarks There is only one host server so return list should only contain one item.
		 * @return The object path of the host server.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*
		 * Internal extrinsic methods
		 */

		/*!
		 * Add a default simulator to be loaded when the nvm library is loaded
		 * @param[in] fileName Absolute path to the simulator to load
		 */
		void addDefaultSimulator(std::string fileName) throw (framework::Exception);

		/*!
		 * Remove the default simulator setting
		 */
		void removeDefaultSimulator() throw (framework::Exception);

		/*!
		 * Change the debug logging level of the management software
		 * @param[in] logSetting
		 * 		0 = Off (Critical Errors Only) @n
		 * 		1 = Full Debug Logging
		 */
		void setDebugLogging(int logSetting) throw (framework::Exception);

		/*
		 * Helper function to convert the host information into operational status
		 */
		static wbem::framework::UINT16_LIST hostToOpStatus(bool mixedSku, bool skuViolation);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);
		void validateObjectPath(framework::ObjectPath &path)
			throw (wbem::framework::Exception);
		void toInstance(core::system::SystemInfo &hostInfo,
				framework::Instance &instance, wbem::framework::attribute_names_t attributes);

};

} // server
} // wbem
#endif  // #ifndef _WBEM_SERVER_BASESERVER_FACTORY_H_
