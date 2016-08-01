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
 * This file contains the provider for the MemoryConfiguration instances. This class is no longer
 * part of the actual CIM Schema, however, a lot depends on it so it still exists in the code.
 */

#ifndef	_WBEM_MEMCONFIG_MEMORYCONFIGURATION_FACTORY_H_
#define	_WBEM_MEMCONFIG_MEMORYCONFIGURATION_FACTORY_H_

#include <string>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <core/memory_allocator/MemoryAllocationTypes.h>

namespace wbem
{
namespace mem_config
{
// object path strings
static const std::string MEMORYCONFIGURATION_ELEMENTNAME = " NVM memory configuration"; //!< Element Name static
static const std::string MEMORYCONFIGURATION_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "MemoryConfiguration"; //!< CreationClassName

// Values for ChangeableType attribute
static const framework::UINT16 CHANGEABLETYPE_PERSISTENT = 0; //!< ChangeableType 0 - Persistent - current pool config
static const framework::UINT16 CHANGEABLETYPE_TRANSIENT = 3; //!< ChangeableType 3 - Transient - config goal

// Struct used to pass interleave set information as opposed to using individual parameters
struct InterleaveSetInfo
{
	NVM_UINT64 size;
	NVM_UINT32 settings;
	NVM_UINT32 setIndex;
	int mirrored;
};


/*!
 * Represents the support data maintained by the management software
 */
class NVM_API MemoryConfigurationFactory  : public framework_interface::NvmInstanceFactory
{
	public:
		// constructor
		MemoryConfigurationFactory();
		~MemoryConfigurationFactory();

		// methods required for CIM provider support, note that if
		// you don't see an expected method here the inherited version is used and
		// it throws an unsupported exception
		framework::Instance* getInstance(framework::ObjectPath &path,
				framework::attribute_names_t &attributes) throw (framework::Exception);
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		// Helper functions to retrieve just goal instances
		framework::instance_names_t* getGoalInstanceNames(const bool onlyUnappliedGoals)
			throw (framework::Exception);
		framework::instances_t* getGoalInstances(framework::attribute_names_t &attributes,
			const bool onlyUnappliedGoals)
			throw (framework::Exception);

		wbem::framework::instances_t *getInstancesFromLayout(
				const core::memory_allocator::MemoryAllocationLayout &layout,
				framework::attribute_names_t &attributes);

		static bool isValidInstanceId(std::string instanceId);

	private:
		/*
		 * Return true if the given dimm is in at least one pool
		 */
		bool dimmIsInAPool(NVM_UID uid, std::vector<struct pool> pools);

		/*
		 * Populate the attribute list with the attributes specific to this class
		 */
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*
		 * Return the index the given dimm uid in the array of dimms in this pool
		 * or return NOTFOUND.
		 */
		int getDimmIndexInPoolOrReturnNotFound(const NVM_UID uid, const struct pool *pool);

		/*
		 * Return true if the given dimm uid is in the list of dimms in this interleave set
		 */
		bool dimmIsInIlset(const NVM_UID uid, const struct interleave_set &ilset);

		/*
		 * Get the InterleaveSetInfo for the current configuration
		 */
		void getCurrentIlsetInfo
				(const NVM_UID uid, const std::vector<struct pool> &pools,
				std::vector<struct InterleaveSetInfo> &ilsetInfos);

		/*
		 * Return the Memory Mode capacity for the given dimm uid
		 */
		NVM_UINT64 getDimmMemoryCapacityFromCurrentConfig
				(const NVM_UID uid, const std::vector<struct pool> &pools)
						throw (framework::Exception);

		/*
		 * Return the Storage Mode capacity for the given dimm uid for the current config
		 */
		NVM_UINT64 getDimmStorageCapacityFromCurrentConfig
				(const NVM_UID uid, const std::vector<struct pool> &pools)
						throw (framework::Exception);

		/*
		 * Return the Storage Mode capacity for the given dimm for the goal config
		 */
		static NVM_UINT64 getDimmStorageCapacityFromGoal
			(const struct device_discovery *pDiscovery, const struct config_goal &goal);

		/*
		 * finish populating the goal instance
		 */
		void populateGoalInstance(
			const framework::attribute_names_t &attributes,
			const std::string &uidStr,
			wbem::framework::Instance* pInstance,
			const struct device_discovery *p_discovery);

		void configGoalToGoalInstance(const framework::attribute_names_t &attributes,
				const struct device_discovery *pDiscovery,
				const struct config_goal &goal, wbem::framework::Instance *pInstance);

		void populateInstanceDimmInfoFromDiscovery(
				framework::attribute_names_t &attributes,
				wbem::framework::Instance *pInstance,
				const struct device_discovery &discovery);

		/*
		 * finish populating the current config instance
		 */
		void populateCurrentConfigInstance(
			const framework::attribute_names_t &attributes,
			const std::string &uidStr,
			wbem::framework::Instance* pInstance,
			const struct device_discovery *p_discovery);

		/*
		 * Return true if the instanceId describes a goal instance
		 */
		static bool isGoalConfig(std::string instanceId);

		/*
		 * Return true if the instanceId describes a current config instance
		 */
		static bool isCurrentConfig(std::string instanceId);

		/*
		 * Translates a config goal status into a helpful string
		 */
		static std::string configGoalStatusToString(const enum config_goal_status status);
};

} // mem_config
} // wbem
#endif  // #ifndef _WBEM_MEMCONFIG_MEMORYCONFIGURATION_FACTORY_H_
