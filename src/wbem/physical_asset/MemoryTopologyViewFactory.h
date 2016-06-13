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
 * This file defines a factory for instances of MemoryTopologyView, an internal-only
 * WBEM view class that represents basic topology information about all memory devices.
 * This includes NVM-DIMMs and other devices, such as DRAM.
 */

#ifndef MEMORYTOPOLOGYVIEWFACTORY_H_
#define MEMORYTOPOLOGYVIEWFACTORY_H_

#include <string>

#include <nvm_types.h>
#include <nvm_management.h>
#include <libinvm-cim/Attribute.h>
#include <libinvm-cim/Instance.h>
#include <libinvm-cim/ObjectPath.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <NvmStrings.h>

namespace wbem
{
namespace physical_asset
{

static const std::string MEMORYTOPOLOGYVIEW_CREATIONCLASSNAME =
		std::string(NVM_WBEM_PREFIX) + "MemoryTopologyView";

// MemoryType
static const std::string MEMORYTOPOLOGYVIEW_MEMORYTYPE_NVMDIMM = "NVM-DIMM";
static const std::string MEMORYTOPOLOGYVIEW_MEMORYTYPE_DDR4DRAM = "DDR4";
static const std::string MEMORYTOPOLOGYVIEW_MEMORYTYPE_UNKNOWN = "Unknown";

// Used if the attribute was requested but doesn't apply to the given memory type
static const std::string MEMORYTOPOLOGYVIEW_NOTAPPLICABLE = "N/A";

class NVM_API MemoryTopologyViewFactory : public framework_interface::NvmInstanceFactory
{
	public:
		/*
		 * Initialize the factory.
		 */
		MemoryTopologyViewFactory() throw (framework::Exception);

		/*
		 * Clean up the factory.
		 */
		virtual ~MemoryTopologyViewFactory();

		/*
		 * Retrieve an instance of MemoryTopologyView with a specific object path.
		 */
		virtual framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes)
				throw (framework::Exception);

		/*
		 * Retrieve a list of object paths for all MemoryTopologyView instances.
		 */
		virtual framework::instance_names_t* getInstanceNames()
				throw (framework::Exception);

		/*
		 * Returns the string value for the memory type.
		 */
		static std::string memoryTypeToString(const enum memory_type memoryType);

	protected:
		/*
		 * Fill out the list of valid attributes.
		 */
		virtual void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*
		 * Verify the PhysicalID attribute is a valid type/range and return it converted to uint16
		 */
		NVM_UINT16 getPhysicalIdValue(framework::Attribute& physicalIdAttr)
			throw (framework::Exception);

		/*
		 * Returns a copy of the memory_topology structure for the memory device with a given
		 * physical ID.
		 * Throws an exception if the physical ID is not found.
		 */
		void getMemoryTopologyForPhysicalId(const NVM_UINT16 physicalId, struct memory_topology &memTopo)
			throw (framework::Exception);

		/*
		 * Returns a copy of the device_discovery structure fo rthe memory device with a given
		 * physical ID.
		 * Throws an exception if a matching device_discovery isn't found for the physical ID.
		 */
		void getDeviceDiscoveryForPhysicalId(const NVM_UINT16 physicalId, struct device_discovery &device)
			throw (framework::Exception);

		/*
		 * Populate the list of all actual instance names.
		 */
		void populateInstanceNames(framework::instance_names_t &instanceNames)
			throw (framework::Exception);

		/*
		 * Populate all requested attributes of the instance.
		 */
		void populateInstanceFromMemoryTopology(framework::Instance &instance,
				framework::attribute_names_t &attributes,
				const struct memory_topology &memTopology)
			throw (framework::Exception);

		/*
		 * Populate only the attributes that are different for NVM-DIMMs
		 */
		void populateNvmDimmInstanceAttributes(framework::Instance &instance,
				framework::attribute_names_t &attributes,
				const struct memory_topology &memTopology)
			throw (framework::Exception);

		/*
		 * Populate only the attributes that are different for DRAM DIMMs
		 */
		void populateDramDimmInstanceAttributes(framework::Instance &instance,
				framework::attribute_names_t &attributes,
				const struct memory_topology &memTopology)
			throw (framework::Exception);
};

} /* namespace physical_asset */
} /* namespace wbem */

#endif /* MEMORYTOPOLOGYVIEWFACTORY_H_ */
