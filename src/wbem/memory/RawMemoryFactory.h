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
 * This file contains the provider for the RawMemory instances
 * which model the capacity of a given DIMM.
 */

#ifndef _WBEM_MEMORY_RAW_MEMORY_FACTORY_H_
#define _WBEM_MEMORY_RAW_MEMORY_FACTORY_H_

#include <string>

#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace memory
{
	static const std::string RAWMEMORY_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "RawMemory"; //!< CreationClassName static
	static const std::string RAWMEMORY_ELEMENTNAME_prefix = "NVDIMM Memory for "; //!< Element Name = prefix + UID
	static const NVM_UINT16 OPSTATUS_UNKNOWN = 0;
	static const NVM_UINT16 OPSTATUS_OK = 2;
	static const NVM_UINT16 OPSTATUS_PREDICTIVEFAILURE = 5;
	static const NVM_UINT16 OPSTATUS_NOCONTACT = 12;
	static const NVM_UINT16 OPSTATUS_DORMANT = 15;
	static const NVM_UINT16 HEALTHSTATE_UNKNOWN = 0;
	static const NVM_UINT16 HEALTHSTATE_OK = 5;
	static const NVM_UINT16 HEALTHSTATE_DEGRATEDWARNING = 10;
	static const NVM_UINT16 HEALTHSTATE_CRITICALFAILURE = 25;

/*!
 * Models the capacity of a given DIMM
 */
class NVM_API RawMemoryFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new IntelRawMemoryFactory.
		 */
		RawMemoryFactory() throw (framework::Exception);

		/*!
		 * Clean up the IntelRawMemoryFactory
		 */
		~RawMemoryFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the RawMemory info
		 * @return The NVDIMM instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * RawMemory object paths.
		 * @throw Exception if unable to retrieve the RawMemory list
		 * @throw Exception if unable to retrieve the server name
		 * @return The object paths for each RawMemory.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);
};

} // memory
} // wbem

#endif // _WBEM_MEMORY_RAW_MEMORY_FACTORY_H_
