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
 * This file contains the provider for the VolatileMemory instances
 * which represent all NVM-DIMM volatile capacity in the system.
 */

#ifndef _WBEM_MEMORY_VOLATILE_MEMORY_FACTORY_H_
#define _WBEM_MEMORY_VOLATILE_MEMORY_FACTORY_H_

#include <string>
#include <libinvm-cim/Types.h>
#include <server/BaseServerFactory.h>
#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace memory
{

// constants
static const std::string VOLATILEMEMORY_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "VolatileMemory"; //!< CreationClassName
static const std::string VOLATILEMEMORY_SYSTEMCREATIONCLASSNAME = wbem::server::BASESERVER_CREATIONCLASSNAME; //!< SystemCreationClassName
static const std::string VOLATILEMEMORY_DEVICEID = "NVDIMM Volatile Capacity"; //!< DeviceID

// blocksize values
static const wbem::framework::UINT64 VOLATILEMEMORY_BLOCKSIZE = 1;

// replication values
static const wbem::framework::UINT16 VOLATILEMEMORY_REPLICATION_UNKNOWN_VAL = 0;

// accessgranularity values
static const wbem::framework::UINT16 VOLATILEMEMORY_ACCESSGRANULARITY_BYTE_ADDRESSABLE = 2;

// processaffinity values
static const std::string VOLATILEMEMORY_PROCESSORAFFINITY_NONE = "";

// health state values
static const wbem::framework::UINT16 VOLATILEMEMORY_HEALTHSTATE_UNKNOWN = 0; //!< cannot report on healthstate
static const wbem::framework::UINT16 VOLATILEMEMORY_HEALTHSTATE_OK = 5; //!< fully functional
static const wbem::framework::UINT16 VOLATILEMEMORY_HEALTHSTATE_DEGRADED = 10; //!< in working order but element not at best of abilities ( optimal performance or recoverable errors)
static const wbem::framework::UINT16 VOLATILEMEMORY_HEALTHSTATE_CRITICAL_FAILURE = 25; //!< non-functional and recovery might not be possible

// operational status values
static const wbem::framework::UINT16 VOLATILEMEMORY_OPERATIONALSTATUS_UNKNOWN = 0;
static const wbem::framework::UINT16 VOLATILEMEMORY_OPERATIONALSTATUS_OK = 2;
static const wbem::framework::UINT16 VOLATILEMEMORY_OPERATIONALSTATUS_DEGRADED = 3; //!< operation completed but did not finish ok
static const wbem::framework::UINT16 VOLATILEMEMORY_OPERATIONALSTATUS_SUPPORTINGENTITYINERROR = 16; //!< this element is ok, but another element it depends on is in error

/*!
 * Represents NVM-DIMM Memory Mode capacity.
 */
class NVM_API VolatileMemoryFactory: public framework_interface::NvmInstanceFactory
{
	public:
		/*!
		 * Initialize a new VolatileMemoryFactory.
		 */
		VolatileMemoryFactory();

		/*!
		 * Clean up the VolatileMemoryFactory.
		 */
		virtual ~VolatileMemoryFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the memory capacity information.
		 * @return The instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * host server object paths.
		 * @remarks There is only one VolatileMemory class per server so return list
		 * should only contain one item.
		 * @return The object path of the instance.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*!
		 * Test to see if to instances are associated.
		 * @param[in] associationClass
		 * 		Pointer to the association class
		 * @param[in] pAntInstance
		 * 		Pointer to the antecedent instance
		 * @param pDepInstance
		 * 		Pointer to the dependent instance
		 * @return true if there is a antecedent/dependent relationship.
		 */
		bool isAssociated(const std::string &associationClass, framework::Instance* pAntInstance,
			 framework::Instance* pDepInstance);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		void validateObjectPath(framework::ObjectPath &path)
			throw (wbem::framework::Exception);

		wbem::framework::UINT64 getMemoryCapacity()
			throw (wbem::framework::Exception);

		wbem::framework::UINT64 getDimmMemoryCapacity(std::string uidStr)
			throw (wbem::framework::Exception);

		wbem::framework::UINT16 getHealthState()
			throw (wbem::framework::Exception);

		wbem::framework::UINT16 getOperationalStatus(NVM_UINT16 health_state)
			throw (wbem::framework::Exception);

		void updateHealthStateIncrementally(NVM_UINT16 &currentHealthState, const NVM_UINT16 dimmHealthState)
			throw (framework::Exception);

};

} /* namespace memory */
} /* namespace wbem */

#endif /* _WBEM_MEMORY_VOLATILE_MEMORY_FACTORY_H_ */
