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
 * This file contains the provider for the MemoryConfigurationService instances.
 * The MemoryConfigurationService supports changing how
 * NVM DIMMs are mapped into the system address space.
 */

#ifndef	_WBEM_MEMCONFIG_MEMORYCONFIGSERVICE_FACTORY_H_
#define	_WBEM_MEMCONFIG_MEMORYCONFIGSERVICE_FACTORY_H_

#include <string>
#include <server/BaseServerFactory.h>
#include "MemoryAllocationSettingsFactory.h"
#include <nvm_management.h>

#include <framework_interface/NvmInstanceFactory.h>
#include <core/memory_allocator/MemoryAllocationTypes.h>

namespace wbem
{
namespace mem_config
{
	static const std::string MEMORYCONFIGURATIONSERVICE_ELEMENTNAME = " Memory Resource Configuration Service"; //!< element name static
	static const std::string MEMORYCONFIGURATIONSERVICE_SYSTEMCREATIONCLASSNAME = wbem::server::BASESERVER_CREATIONCLASSNAME; //!< System Creation ClassName static
	static const std::string MEMORYCONFIGURATIONSERVICE_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "MemoryConfigurationService"; //!< Creation ClassName static
	static const std::string MEMORYCONFIGURATIONSERVICE_NAME = " Memory Configuration Service"; //!< Name static

	// extrinsic methods and params
	static const std::string MEMORYCONFIGURATIONSERVICE_ALLOCATEFROMPOOL = "AllocateFromPool"; //!< extrinsic method name
	static const std::string MEMORYCONFIGURATIONSERVICE_PARENTPOOL = "Pool"; //!< method param
	static const std::string MEMORYCONFIGURATIONSERVICE_SYSTEMPROCESSOR = "Processor";
	static const std::string MEMORYCONFIGURATIONSERVICE_MEMORYRESOURCES = "MemoryResources"; //!< method param
	static const std::string MEMORYCONFIGURATIONSERVICE_SETTINGS = "Goal"; //!< method param // 
	static const std::string MEMORYCONFIGURATIONSERVICE_EXPORTTOURI = "ExportToURI"; //!< extrinsic method name
	static const std::string MEMORYCONFIGURATIONSERVICE_EXPORTURI = "ExportURI"; //!< method param
	static const std::string MEMORYCONFIGURATIONSERVICE_IMPORTFROMURI = "ImportFromURI"; //!< extrinsic method name
	static const std::string MEMORYCONFIGURATIONSERVICE_IMPORTURI = "ImportURI"; //!< method param
	static const std::string MEMORYCONFIGURATIONSERVICE_REMOVEGOAL = "RemoveGoal"; //!< method name
	static const std::string MEMORYCONFIGURATIONSERVICE_TARGETS = "Targets"; //!< method param

	static const NVM_UINT32 MEMORYCONFIGURATIONSERVICE_ERR_FAILED = 4;
	static const NVM_UINT32 MEMORYCONFIGURATIONSERVICE_ERR_INVALID_PARAMETER = 5;
	static const NVM_UINT32 MEMORYCONFIGURATIONSERVICE_ERR_SUCCESSFULLY_STAGED_FOR_FUTURE = 4096;
	static const NVM_UINT32 MEMORYCONFIGURATIONSERVICE_ERR_INSUFFICIENT_RESOURCES = 4097;
	static const NVM_UINT32 MEMORYCONFIGURATIONSERVICE_ERR_INCONSISTENT_PARAMETERS = 4098;
	static const NVM_UINT32 MEMORYCONFIGURATIONSERVICE_ERR_DID_NOT_COMPLETE = 4099;

/*!
 * A service that provides access to memory configuration methods
 */
class NVM_API MemoryConfigurationServiceFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new MemoryConfigurationServiceFactory.
		 */
		MemoryConfigurationServiceFactory() throw (framework::Exception);

		/*!
		 * Copy Constructor
		 */
		MemoryConfigurationServiceFactory(const MemoryConfigurationServiceFactory &config);

		/*!
		 * Assignment Operator
		 */
		MemoryConfigurationServiceFactory &operator=(const MemoryConfigurationServiceFactory &config);

		/*!
		 * Clean up the MemoryConfigurationServiceFactory
		 */
		~MemoryConfigurationServiceFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the DIMM info
		 * @return The NVDIMM instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * NVDIMM object paths.
		 * @throw Exception if unable to retrieve the DIMM list
		 * @throw Exception if unable to retrieve the server name
		 * @return The object paths for each NVDIMM.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*!
		 * Implementation of the method to export the current memory allocation settings
		 * for all manageable NVM-DIMMs within a platform to the specified URI.
		 * @param exportURI
		 * 		A file URI writable by the CIM provider with sufficient free capacity to
		 * 		hold the exported data.
		 * @throw Exception if unable to dump the configuration settings to the file.
		 */
		void exportSystemConfigToUri(const std::string &exportUri) throw (framework::Exception);

		/*!
		 * Implementation of the method to export the current memory allocation settings
		 * for all manageable NVM-DIMMs within a platform to the specified filesystem path.
		 * @param path
		 * 		A file path writable by the CIM provider with sufficient free capacity to
		 * 		hold the exported data.
		 * @throw Exception if unable to dump the configuration settings to the file.
		 */
		void exportSystemConfigToPath(const std::string &path) throw (wbem::framework::Exception);

		/*!
		 * Implementation of the method to import memory allocation settings from the specified
		 * URI and modifies how the BIOS configures the specified targets.
		 * @param importURI
		 * 		A file URI to a locally accessible settings file.
		 * @param dimms
		 * 		One or more NVM-DIMMs to apply the settings to.
		 */
		void importDimmConfigsFromURI(const std::string &importUri, framework::STR_LIST dimms)
			throw (framework::Exception);

		/*!
		 * Import memory allocation settings from a file and use them to modify the configuration goal.
		 * @param path
		 * 		Path to a local settings configuration file.
		 * @param dimms
		 * 		One or more NVM-DIMMs to apply the settings to.
		 */
		void importDimmConfigsFromPath(const std::string &path, std::vector<std::string> dimms)
			throw (wbem::framework::Exception);

		/*!
		 * Hook for extrinsic methods.
		 */
		wbem::framework::UINT32 executeMethod(
					wbem::framework::UINT32 &wbemRc,
					const std::string method,
					wbem::framework::ObjectPath &object,
					wbem::framework::attributes_t &inParms,
					wbem::framework::attributes_t &outParms);

		wbem::framework::UINT32 executeMethodAllocateFromPool(
					wbem::framework::UINT32 &wbemRc,
					wbem::framework::ObjectPath &object,
					wbem::framework::attributes_t &inParms,
					wbem::framework::attributes_t &outParms);

		wbem::framework::UINT32 executeMethodExportUri(
					wbem::framework::UINT32 &wbemRc,
					wbem::framework::ObjectPath &object,
					wbem::framework::attributes_t &inParms,
					wbem::framework::attributes_t &outParms);

		wbem::framework::UINT32 executeMethodImportFromUri(
					wbem::framework::UINT32 &wbemRc,
					wbem::framework::ObjectPath &object,
					wbem::framework::attributes_t &inParms,
					wbem::framework::attributes_t &outParms);
	
		wbem::framework::UINT32 executeMethodRemoveGoal(
					wbem::framework::UINT32 &wbemRc,
					wbem::framework::ObjectPath &object,
					wbem::framework::attributes_t &inParms,
					wbem::framework::attributes_t &outParms);

		/*
		 * Delete a configuration goal from an NVM-DIMM
		 */
		wbem::framework::Instance* deleteInstance(framework::ObjectPath &path)
						throw (wbem::framework::Exception);

		/*
		 * Returns true if the DIMM already has a config goal.
		 * Throws an exception if it encounters a lib error.
		 */
		static bool dimmHasGoal(const NVM_UID dimmUid);

		/*
		 * Get all manageable dimm uids for a socket
		 */
		static std::vector<std::string> getManageableDimmIDsForSocket(NVM_UINT32 socketId);

	protected:
		/*!
		 * Provider for nvm_delete_config_goal
		 */
		int (*m_DeleteConfigGoalProvider)(const NVM_UID device_uid);

		/*
		 * Adds all valid attributes to the attribute list.
		 */
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*
		 * Verifies the MemoryConfigurationService object path is valid.
		 */
		bool validatePath(wbem::framework::ObjectPath &path, std::string hostName);

		/*
		 * Verifies the Pool reference refers to a real MemoryResources instance.
		 * Throws an exception if not.
		 */
		void validatePool(std::string parentPoolRef) throw (framework::Exception);

		/*
		 * Return the newMemoryOnly attribute of the settings string
		 */
		bool getNewMemorySetting(std::string &setting);

		/*
		 * Basic validation of settings strings, make sure class name is correct
		 * and the all the fields are filled in
		 */
		void validateSettingsStrings(framework::STR_LIST settingsStrings)
			throw(wbem::framework::Exception);

		/*
		 * Given a goal settings string return the socketId
		 */
		static NVM_UINT16 getSocketIdForSettingsString(std::string setting)
			throw(wbem::framework::Exception);

		/*
		 * For a given list of MemoryAllocationSettings, return a list of DIMMs for those settings
		 */
		void getDimmsForMemAllocSettings(framework::STR_LIST settings,
				std::vector<core::memory_allocator::Dimm> &dimms);

		/*
		 * Translate MemoryAllocationSettings strings into a MemoryAllocationRequest
		 */
		core::memory_allocator::MemoryAllocationRequest memAllocSettingsToRequest(
				const framework::STR_LIST &memoryAllocationSettings);

		/*
		 * Return a list of MemoryAllocationSettings tha belong to the given socket
		 */
		framework::STR_LIST getSettingsStringsForSocket(framework::STR_LIST &settingsStrings,
				NVM_UINT16 socketId) throw (wbem::framework::Exception);

		/*
		 * From a list of setting string sorted by socketId, remove all the setting strings that match
		 * the given socketId
		 */
		void removeSettingsWithSocketId(framework::STR_LIST &settingStrings, NVM_UINT16 socketId);

		/*
		 * Converts a list of embedded MemoryAllocationSettings instances (CIM XML) to a quantity of
		 * memory (in GiB) and a list of AppDirect extents
		 s Throws an exception if the settings are invalid MemoryAllocationSettings instances
		 * or if the capacity of the requested config doesn't add up to the total capacity of
		 * DIMMs requested.
		 */
		void settingsStringsToRequestedExtents(const framework::STR_LIST &settingsStrings,
				NVM_UINT64 &memoryCapacity,
				std::vector<struct core::memory_allocator::AppDirectExtent> &appDirectCapacities)
			throw (wbem::framework::Exception);

		/*
		 * Return true if all the settings strings are for NewMemoryOnly or if all
		 * the settings strings are not for NewMemoryOnly
		 */
		bool areNewMemoryOnlySettingsAllTheSame
				(framework::STR_LIST settingsStrings);

		/*
		 * Verifies that the DIMM list won't break an existing config or leave
		 * a socket partially unconfigured.
		 */
		void validateDimmList(const std::vector<std::string> dimmUids);

		/*
		 * Extract the socketId from the SystemProcessorRef
		 */
		NVM_UINT16 getSocketIdForProcessorRef(std::string processorRef);

		/*
		 * Remove the config_goals from the dimms in the given list
		 */
		void removeGoalFromDimms(std::vector<std::string> dimmUids);

		/*
		 * Return true if the given socketId is valid, false otherwise
		 */
		bool socketIdIsValid(NVM_UINT16 socketId);

		/*
		 * Validate the system processor reference
		 */
		void validateSystemProcessorRef(std::string processorRef);

		/*!
		 * Convert a URI to an absolute path
		 */
		void uriToPath(const std::string &uriParamName, const std::string &uri,
				std::string &path, bool check_exists = false) const
			throw (wbem::framework::Exception);

		/*
		 * convert NvmExceptionLibError to extrinsic return code
		 */
		wbem::framework::UINT32 getReturnCodeFromLibException(const exception::NvmExceptionLibError &e);
};

} // resource
} // wbem
#endif  // #ifndef _WBEM_MEMCONFIG_MEMORYCONFIGSERVICE_FACTORY_H_
