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
 * This file contains the NVMCLI namespace related commands.
 */

#ifndef _CLI_NVMCLI_NAMESPACEFEATURE_H_
#define _CLI_NVMCLI_NAMESPACEFEATURE_H_

#include "WbemToCli.h"
#include "WbemToCli_utilities.h"
#include "MemoryProperty.h"
#include "VerboseFeatureBase.h"
#include <mem_config/MemoryConfigurationServiceFactory.h>
#include <mem_config/InterleaveSet.h>
#include <nvm_types.h>
#include <pmem_config/PersistentMemoryServiceFactory.h>
#include <pmem_config/PersistentMemoryPoolFactory.h>
#include <pmem_config/PersistentMemoryCapabilitiesFactory.h>
#include <pmem_config/PersistentMemoryNamespaceFactory.h>
#include <pmem_config/NamespaceViewFactory.h>
#include <core/memory_allocator/MemoryAllocationTypes.h>
#include <mem_config/PoolViewFactory.h>

namespace cli
{
namespace nvmcli
{

static const std::string MEGABYTES_UNITS = " MB";
static const std::string MIRRORED_CAPACITY = " Mirrored";

static const std::string MEMORYSIZE_PROPERTYNAME = "MemorySize";
static const std::string APPDIRECTSIZE_PROPERTYNAME = "AppDirectSize";
static const std::string APPDIRECTSETTINGS_PROPERTYNAME = "AppDirectSetting";
static const std::string APPDIRECT1SIZE_PROPERTYNAME = "AppDirect1Size";
static const std::string APPDIRECT1SETTINGS_PROPERTYNAME = "AppDirect1Setting";
static const std::string APPDIRECT2SIZE_PROPERTYNAME = "AppDirect2Size";
static const std::string APPDIRECT2SETTINGS_PROPERTYNAME = "AppDirect2Setting";
static const std::string RESERVEDIMM_PROPERTYNAME = "ReserveDimm";
static const std::string STORAGECAPACITY_PROPERTYNAME = wbem::STORAGECAPACITY_KEY;
static const std::string APPDIRECT1SIZE_PROPERTYDESC =
		"The size in gibibytes of the first App Direct interleave set or \"Remaining\" "
		"if all remaining unconfigured space is desired. Must be a multiple of the memory alignment "
		"size defined in Show System Capabilities.";
static const std::string APPDIRECT2SIZE_PROPERTYDESC =
		"The size in gibibytes of the second App Direct interleave set or \"Remaining\" "
		"if all remaining unconfigured space is desired. Only applicable if AppDirect1Size "
		"is specified. Must be a multiple of the memory alignment "
		"size defined in Show System Capabilities.";
static const std::string APPDIRECT1SETTING_PROPERTYDESC =
		"The quality of service attributes for the first App Direct interleave set. "
		"Only applicable if AppDirect1Size is specified.";
static const std::string APPDIRECT2SETTING_PROPERTYDESC =
		"The quality of service attributes for the second App Direct interleave set. "
		"Only applicable if AppDirect2Size is specified.";
static const std::string RESERVEDIMM_PROPERTYDESC = "Reserve one " NVM_DIMM_NAME " across the "
		"specified target (the system, socket or the " NVM_DIMM_NAME ") for use in Storage Mode. "
		"If this property is set when creating a memory allocation goal on a single " NVM_DIMM_NAME
		", the only valid property is StorageCapacity=Remaining.";
static const std::string STORAGECAPACITY_PROPERTYDESC =
		"Utilize the remaining capacity on the " NVM_DIMM_NAME " in Storage Mode."
		"By default, all the remaining capacity after the Memory Mode and "
		"App Direct interleave sets are allocated is left as Storage. Therefore "
		"this property is only necessary if neither MemorySize or "
		"AppDirectSize are specified (i.e. set the entire "
		NVM_DIMM_NAME " to Storage Mode.";

static const std::string DELETECONFIGGOAL_FROMDIMM_MSG = "Delete configuration goal from " NVM_DIMM_NAME;
static const std::string DUMPSYSTEMCONFIG_FROMDIMM_MSG = "Dump system configuration to file";

static const std::string DELETE_NS_PROMPT = TR(
		"Delete namespace %s?"); //!< prompt for user if not forced
static const std::string MODIFY_CONFIG_GOAL_MSG = TR("Modify configuration goal: ");
static const std::string MODIFY_NS_PROMPT = TR(
		"Modify namespace %s?"); //!< prompt for user if not forced

static const std::string NS_ALIGNMENT_PROMPT = TR(
		"The requested namespace capacity %llu will be changed to %llu to align properly. Do you want to continue?");
static const std::string CREATE_NS_SMALL_BLOCK_SIZE_PROMPT = TR(
		"The requested namespace capacity %llu requires %llu physical space due to "
		"the selected block size. Are you sure you want to continue?");
static const std::string DELETE_CONFIG_GOAL_MSG = TR("Delete configuration goal: ");
static const std::string DELETE_GOAL_PROMPT = TR(
		"Delete the current configuration goal so that it is not applied by the BIOS on the next reboot?"); //!< prompt for user if not forced
static const std::string LOAD_CONFIG_GOAL_MSG = TR("Load configuration goal: ");
static const std::string LOAD_GOAL_PROMPT = TR(
		"Load the configuration goal from '%s' which will delete existing data and provision "
		"the capacity of the " NVM_DIMM_NAME "s on the next reboot?"); //!< prompt for user if not forced

/*
 * Command spec parts for namespace feature
 */
static const std::string CREATE_NS_PM_TYPE = "PersistentMemoryType";
static const std::string CREATE_NS_PROP_FRIENDLYNAME = "Name";
static const std::string CREATE_NS_PROP_ENABLED = "Enabled";
static const std::string CREATE_NS_PROP_CAPACITY = "Capacity";
static const std::string CREATE_NS_PROP_OPTIMIZE = "Optimize";
static const std::string CREATE_NS_PROP_ENCRYPTION = "Encryption";
static const std::string CREATE_NS_PROP_ERASECAPABLE = "EraseCapable";
static const std::string CREATE_NS_PROP_MEMORYPAGEALLOCATION = "MemoryPageAllocation";

static const std::string CREATE_NS_PROP_PM_TYPE_APPDIRECT_NOTINTERLEAVED = "AppDirectNotInterleaved";
static const std::string CREATE_NS_PROP_PM_TYPE_APPDIRECT = "AppDirect";
static const std::string CREATE_NS_PROP_READONLY = "ReadOnly";
static const std::string CREATE_NS_PROP_OPTIMIZE_COPYONWRITE = "CopyOnWrite";
static const std::string CREATE_NS_PROP_MEMORYPAGEALLOCATION_DRAM = "DRAM";
static const std::string CREATE_NS_PROP_MEMORYPAGEALLOCATION_APPDIRECT = "AppDirect";

static const size_t CREATE_NS_PROP_FRIENDLYNAME_MAX = 64u;

static const std::string NS_UNITS_OPTION_DESC = "Change the units that capacities are input and displayed in.";

/*!
 * Implements namespace related commands
 */
class NVM_API NamespaceFeature : public cli::nvmcli::VerboseFeatureBase
{
	public:
		NamespaceFeature();
		virtual ~NamespaceFeature();

		/*!
		 *
	 	* @param commandSpecId
	 	* @param parsedCommand
	 	* @return
	 	*/
		framework::ResultBase * run(const int &commandSpecId,
				const framework::ParsedCommand &parsedCommand);

		// Every feature must have this static members for registration
		void getPaths(cli::framework::CommandSpecList &list); //!< Required for Feature registration
		static const std::string Name; //!< Required for Feature registration
		enum
		{
			SHOW_NAMESPACE,
			CREATE_NAMESPACE,
			MODIFY_NAMESPACE,
			DELETE_NAMESPACE,
			SHOW_CONFIG_GOAL,
			DELETE_CONFIG_GOAL,
			CREATE_GOAL,
			SHOW_POOLS,
			DUMP_CONFIG,
			LOAD_CONFIG_GOAL
		};

		// Interface for WBEM deleteNamespace functionality
		void (*m_deleteNamespace) (const std::string & namespaceUid);

		// Interface for WBEM getSupportedSizeRange functionaliry
		void (*m_getSupportedSizeRange)(const std::string &poolUid,
				COMMON_UINT64 &largestPossibleAppDirectNs,
				COMMON_UINT64 &smallestPossibleAppDirectNs,
				COMMON_UINT64 &appDirectIncrement,
				COMMON_UINT64 &largestPossibleStorageNs,
				COMMON_UINT64 &smallestPossibleStorageNs,
				COMMON_UINT64 &storageIncrement);

		// Setters for WBEM Providers
		void setPersistentMemoryServiceProvider(wbem::pmem_config::PersistentMemoryServiceFactory *pProvider);
		void setPmPoolProvider(wbem::pmem_config::PersistentMemoryPoolFactory *pProvider);
		void setCapabilitiesProvider(wbem::pmem_config::PersistentMemoryCapabilitiesFactory *pProvider);
		void setNsViewProvider(wbem::pmem_config::NamespaceViewFactory *pProvider);
		void setPersistentMemoryNamespaceProvider(wbem::pmem_config::PersistentMemoryNamespaceFactory *pProvider);
		void setPoolViewProvider(wbem::mem_config::PoolViewFactory *pProvider);

		// Setter for WbemToCli
		void setWbemToCli(cli::nvmcli::WbemToCli *pInstance);

		// don't allow copies
		NamespaceFeature(const NamespaceFeature &);
		NamespaceFeature& operator=(const NamespaceFeature&);

protected:
		virtual framework::ResultBase *showConfigGoal(const framework::ParsedCommand &parsedCommand);
		virtual framework::ResultBase *deleteConfigGoal(const framework::ParsedCommand &parsedCommand);
		virtual framework::ResultBase *createGoal(const framework::ParsedCommand &parsedCommand);
		virtual framework::ResultBase *showPools(const framework::ParsedCommand &parsedCommand);
		virtual framework::ResultBase *showNamespaces(const framework::ParsedCommand &parsedCommand);
		virtual framework::ResultBase *deleteNamespaces(const framework::ParsedCommand &parsedCommand);
		virtual framework::ResultBase *createNamespace(const framework::ParsedCommand &parsedCommand);
		virtual framework::ResultBase *modifyNamespace(const framework::ParsedCommand &parsedCommand);

		// Namespace specific exception handling
		framework::ErrorResult *nsNvmExceptionToResult(wbem::framework::Exception &e,
				std::string prefix = "");

		// member variables for storing parsed information
		std::string m_poolUid;
		COMMON_UINT64 m_blockSize;
		COMMON_UINT64 m_blockCount;
		std::string m_capacityUnits;
		bool m_capacityExists;
		NVM_UINT64 m_capacityBytes; // Advertised capacity
		std::string m_friendlyName;
		bool m_friendlyNameExists;
		COMMON_UINT16 m_enableState;
		bool m_enabledStateExists;
		COMMON_UINT16 m_optimize;
		COMMON_UINT16 m_encryption;
		COMMON_UINT16 m_eraseCapable;
		wbem::mem_config::MemoryAllocationSettingsInterleaveSizeExponent m_channelSize;
		wbem::mem_config::MemoryAllocationSettingsInterleaveSizeExponent m_controllerSize;
		bool m_byOne;
		bool m_forceOption;
		std::string m_prefix;
		COMMON_UINT16 m_memoryPageAllocation;
		bool m_optimizeExists;

		/*
		 * Return true if the namespace block count has the correct alignment
		 */
		bool isBlockCountAligned(std::string namespaceUidStr);

		/*
		 * Return true if the modification request is supported
		 */
		bool isNamespaceModificationSupported(const namespace_details &details);
		bool namespaceCapacityModificationIsSupported(const namespace_details &details);

		void atomicModifyNamespace(
				const std::string namespaceUidStr, const struct namespace_details &details);

		/*
		 * Attempt the complete modifyNamespace operation
		 */
		void modifyNamespace(const std::string namespaceUidStr);

		/*
		 * Undo the complete modifyNamespace operation
		 */
		void undoModifyNamespace(const std::string namespaceUidStr,
				const struct namespace_details &details, const int originalError);

		/*
		 * Load a config goal from a config file.
		 */
		framework::ResultBase *loadGoal(const framework::ParsedCommand &parsedCommand);

		/*
		 * Dump the current config into a file that can be used to re-construct the config
		 * on other systems.
		 */
		framework::ResultBase *dumpConfig(const framework::ParsedCommand &parsedCommand);
		framework::ResultBase* dumpConfigNvmExceptionToResult(wbem::framework::Exception& e);

		/*
		 * Helper function to create a filter based on user input
		 */
		static void generatePoolFilter(
			const cli::framework::ParsedCommand& parsedCommand,
			wbem::framework::attribute_names_t &attributes,
			cli::nvmcli::filters_t &filters);

		/*
		 * Filter namespaces based on namespace ID
		 */
		static void generateNamespaceFilter(
				const cli::framework::ParsedCommand &parsedCommand,
				wbem::framework::attribute_names_t &attributes,
				cli::nvmcli::filters_t &filters);

		/*
		 * Filter namespaces based on namespace type
		 */
		static cli::framework::ResultBase *generateNamespaceTypeFilter(
				const cli::framework::ParsedCommand &parsedCommand,
				wbem::framework::attribute_names_t &attributes,
				cli::nvmcli::filters_t &filters);

		/*
		 * Filter namespaces based on namespace health
		 */
		static cli::framework::ResultBase *generateNamespaceHealthFilter(
				const cli::framework::ParsedCommand &parsedCommand,
				wbem::framework::attribute_names_t &attributes,
				cli::nvmcli::filters_t &filters);

		/*
		 * Helper function to check for valid deletion request
		 */
		bool validRequest(const wbem::framework::instances_t &allInstances,
						const wbem::framework::instances_t &requestedInstances);

		
		static void wbemDeleteNamespace (const std::string & namespaceUid)
				throw (wbem::framework::Exception);

		static void wbemGetSupportedSizeRange(const std::string &poolUid,
				COMMON_UINT64 &largestPossibleAdNs,
				COMMON_UINT64 &smallestPossibleAdNs,
				COMMON_UINT64 &adIncrement,
				COMMON_UINT64 &largestPossibleStorageNs,
				COMMON_UINT64 &smallestPossibleStorageNs,
				COMMON_UINT64 &storageIncrement);

		/*
		 * Parsing helpers for creating a namespace
		 */
		cli::framework::ResultBase *getCreateNsBlockCount();
		framework::ResultBase * parseNsFriendlyName(const framework::ParsedCommand& parsedCommand);
		framework::ResultBase * parseCreateNsEnabled(const framework::ParsedCommand& parsedCommand);
		framework::ResultBase * parseCreateNsOptimize(const framework::ParsedCommand& parsedCommand);
		framework::ResultBase * parseCreateNsEncryption(const framework::ParsedCommand& parsedCommand);
		framework::ResultBase * parseCreateNsEraseCapable(const framework::ParsedCommand& parsedCommand);
		framework::ResultBase * parseCreateNsCapacity(const framework::ParsedCommand& parsedCommand);
		framework::ResultBase * parseCreateNsMemoryPageAllocation(const framework::ParsedCommand& parsedCommand);
		framework::ResultBase * parsePersistentMemoryType(const framework::ParsedCommand &command);
		/*
		 * WBEM wrappers for modify namespace
		 */
		static void getNamespaceDetails(const std::string namespaceUidStr,
				struct namespace_details &details);

		static void modifyNamespaceName(const std::string namespaceUidStr,
				const std::string friendlyNameStr);

		static void modifyNamespaceBlockCount(const std::string namespaceUidStr,
				const NVM_UINT64 blockCount);

		static void requestStateChange(const std::string &namespaceUidStr,
				const NVM_UINT16 blockCount);

		static bool isModifyNamespaceNameSupported();

		static bool isModifyNamespaceBlockCountSupported(const namespace_details &details,
				const NVM_UINT64 blockCount);

		static bool isModifyNamespaceEnabledSupported(const enum namespace_enable_state enabled);

		/*
		 * Parsing helpers for modify namespaces
		 */
		framework::ResultBase* parseModifyNsCapacity(const framework::ParsedCommand& parsedCommand);

		static void convertSecurityAttributes(wbem::framework::Instance &wbemInstance);
		static void convertCapacityAndAddIsMirroredText(wbem::framework::Instance &instance,
				const std::string capacityUnits = "");
		static void generateBlockSizeAttributeValue(wbem::framework::Instance &instance);
		static void convertEnabledStateAttributes(wbem::framework::Instance &wbemInstance);
		static void populateNamespaceAttributes(wbem::framework::attribute_names_t &attributes,
			cli::framework::ParsedCommand const &parsedCommand);
		static void convertActionRequiredEventsToNAIfEmpty(wbem::framework::Instance &wbemInstance);

		/*
		 * Helper for adjusting modifyNamespace blockCount if needed for alignment purposes and if
		 * user allows
		 */
		bool adjustNamespaceBlockCount(NVM_UINT64 adjustedBlockCount);

		// helper functions for create namespace
		bool confirmNamespaceBlockSizeUsage();
		bool optimizePropertyExists();
		framework::ResultBase* parseMemoryPageAllocationForAppDirectNS(
			const std::string& requestedMode);

		// WBEM Dependencies
		wbem::pmem_config::PersistentMemoryNamespaceFactory *m_pPmNamespaceProvider;
		wbem::pmem_config::PersistentMemoryServiceFactory *m_pPmServiceProvider;
		wbem::pmem_config::PersistentMemoryPoolFactory *m_pPmPoolProvider;
		wbem::pmem_config::PersistentMemoryCapabilitiesFactory *m_pCapProvider;
		wbem::pmem_config::NamespaceViewFactory *m_pNsViewFactoryProvider;
		wbem::mem_config::PoolViewFactory *m_pPoolViewProvider;
		// WbemToCli dependency
		cli::nvmcli::WbemToCli *m_pWbemToCli;
};

}
}
#endif // _CLI_NVMCLI_NAMESPACEFEATURE_H_
