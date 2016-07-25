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
 * This file contains the hooks into NVMCLI namespace related commands, and the
 * implementations of miscellaneous commands.
 */

#include <vector>
#include <string>
#include <cstring>

#include <LogEnterExit.h>
#include <mem_config/MemoryConfigurationServiceFactory.h>
#include <mem_config/InterleaveSet.h>
#include <pmem_config/NamespaceViewFactory.h>

#include <libinvm-cli/FeatureBase.h>
#include <libinvm-cli/SimpleResult.h>
#include <libinvm-cli/SimpleListResult.h>
#include <libinvm-cli/CommandSpec.h>
#include <libinvm-cli/PropertyListResult.h>
#include <libinvm-cli/NotImplementedErrorResult.h>

#include "CommandParts.h"
#include "NamespaceFeature.h"
#include <cr_i18n.h>
#include <mem_config/PoolViewFactory.h>
#include <exception/NvmExceptionLibError.h>
#include "FieldSupportFeature.h"

const std::string cli::nvmcli::NamespaceFeature::Name = "Namespace";

/*
 * Command Specs the Example Feature supports
 */
void cli::nvmcli::NamespaceFeature::getPaths(cli::framework::CommandSpecList &list)
{
	cli::framework::CommandSpec deleteConfigGoal(DELETE_CONFIG_GOAL,
			TR("Delete Memory Allocation Goal"), framework::VERB_DELETE,
			TR("Delete the memory allocation goal from one or more " NVM_DIMM_NAME "s."));
	deleteConfigGoal.addTarget(TARGET_DIMM.name, false, DIMMIDS_STR, true,
			TR("Delete the memory allocation goal from specific " NVM_DIMM_NAME "s by supplying one or more "
					"comma separated " NVM_DIMM_NAME " identifiers. The default is to delete the "
					"memory allocation goals from all manageable " NVM_DIMM_NAME "s."));
	deleteConfigGoal.addTarget(TARGET_GOAL_R);
	deleteConfigGoal.addTarget(TARGET_SOCKET.name, false, SOCKETIDS_STR, true,
			TR("Delete the memory allocation goal from the " NVM_DIMM_NAME "s on specific sockets "
			"by supplying the socket target and one or more comma-separated socket identifiers. "
			"The default is to delete the memory allocation goals from all manageable " NVM_DIMM_NAME "s on all sockets."));

	cli::framework::CommandSpec showConfigGoal(SHOW_CONFIG_GOAL,
			TR("Show Memory Allocation Goal"), framework::VERB_SHOW,
			TR("Show the memory allocation goal on one or more " NVM_DIMM_NAME "s. Once the goal is successfully applied "
					"by the BIOS, it is no longer displayed."));
	showConfigGoal.addOption(framework::OPTION_ALL);
	showConfigGoal.addOption(framework::OPTION_DISPLAY);
	showConfigGoal.addTarget(TARGET_DIMM.name, false, DIMMIDS_STR, true,
			TR("Restrict output to specific " NVM_DIMM_NAME "s by supplying one or more comma-separated " NVM_DIMM_NAME " "
			"identifiers. The default is to display all manageable " NVM_DIMM_NAME "s with memory allocation goals."));
	showConfigGoal.addTarget(TARGET_GOAL_R).isValueAccepted(false);
	showConfigGoal.addTarget(TARGET_SOCKET.name, false, SOCKETIDS_STR, true,
			TR("Restrict output to the " NVM_DIMM_NAME "s on specific sockets by supplying the socket target and "
				"one or more comma-separated socket identifiers. The default is to display all "
				"manageable " NVM_DIMM_NAME "s on all sockets with memory allocation goals."));

	framework::CommandSpec createGoal(CREATE_GOAL, TR("Create Memory Allocation Goal"),
			framework::VERB_CREATE, TR("Create a memory allocation goal on one or more " NVM_DIMM_NAME "s. "
				"This operation stores the specified goal on the " NVM_DIMM_NAME "(s) for the BIOS to "
				"read on the next reboot in order to map the " NVM_DIMM_NAME " capacity into the system "
				"address space."));
	createGoal.addOption(framework::OPTION_FORCE).helpText(TR("Reconfiguring " NVM_DIMM_NAME "s is a destructive operation "
			"which requires confirmation from the user. This option suppresses the confirmation."));;
	createGoal.addTarget(TARGET_DIMM)
			.isValueRequired(true)
			.helpText(TR("Create a memory allocation goal on specific " NVM_DIMM_NAME "s by "
				"supplying one or more comma-separated " NVM_DIMM_NAME " identifiers. "
				"This list must include all unconfigured " NVM_DIMM_NAME "s on the affected socket(s). "
				"The default is to configure all manageable " NVM_DIMM_NAME "s on all sockets."));
	createGoal.addTarget(TARGET_SOCKET)
			.isValueRequired(true)
			.helpText(TR("Create a memory allocation goal on the " NVM_DIMM_NAME "s on specific sockets by supplying the "
					"socket target and one or more comma-separated socket identifiers. The default is "
					"to configure all manageable " NVM_DIMM_NAME "s on all sockets."));
	createGoal.addTarget(TARGET_GOAL_R);
	createGoal.addProperty(MEMORYSIZE_PROPERTYNAME).isValueRequired(true)
		.valueText("GiB")
		.helpText(TR("Gibibytes of the requested " NVM_DIMM_NAME "s' capacity to use in Memory Mode or \"Remaining\" "
				"if all remaining unconfigured space is desired. Must be a multiple of the memory alignment "
				"size defined in Show System Capabilities."));
	createGoal.addProperty(APPDIRECTSIZE_PROPERTYNAME).isValueRequired(true)
		.valueText("GiB")
		.helpText(TR(APPDIRECT1SIZE_PROPERTYDESC.c_str()));
	createGoal.addProperty(APPDIRECTSETTINGS_PROPERTYNAME).isValueRequired(true)
		.valueText("value")
		.helpText(TR(APPDIRECT1SETTING_PROPERTYDESC.c_str()));
	createGoal.addProperty(APPDIRECT1SIZE_PROPERTYNAME).isValueRequired(true)
		.valueText("GiB")
		.helpText(TR(APPDIRECT1SIZE_PROPERTYDESC.c_str()));
	createGoal.addProperty(APPDIRECT1SETTINGS_PROPERTYNAME).isValueRequired(true)
		.valueText("value")
		.helpText(TR(APPDIRECT1SETTING_PROPERTYDESC.c_str()));
	createGoal.addProperty(APPDIRECT2SIZE_PROPERTYNAME).isValueRequired(true)
		.valueText("GiB")
		.helpText(TR(APPDIRECT2SIZE_PROPERTYDESC.c_str()));
	createGoal.addProperty(APPDIRECT2SETTINGS_PROPERTYNAME).isValueRequired(true)
		.valueText("value")
		.helpText(TR(APPDIRECT2SETTING_PROPERTYDESC.c_str()));
	createGoal.addProperty(RESERVEDIMM_PROPERTYNAME)
		.isValueRequired(true)
		.valueText("0|1")
		.helpText(TRS(RESERVEDIMM_PROPERTYDESC));
	createGoal.addProperty(STORAGECAPACITY_PROPERTYNAME)
		.isValueRequired(true)
		.valueText("Remaining")
		.helpText(TRS(STORAGECAPACITY_PROPERTYDESC));

	cli::framework::CommandSpec showNamespace(SHOW_NAMESPACE, TR("Show Namespace"), framework::VERB_SHOW,
			TR("Show information about one or more namespaces."));
	showNamespace.addOption(framework::OPTION_ALL);
	showNamespace.addOption(framework::OPTION_DISPLAY);
	showNamespace.addOption(framework::OPTION_UNITS).helpText(TR(NS_UNITS_OPTION_DESC.c_str()));
	showNamespace.addTarget(TARGET_NAMESPACE_R)
			.helpText(TR("Restrict output to specific namespaces by providing a comma separated list "
					"of one or more namespace identifiers. The default is to display all namespaces."));
	showNamespace.addTarget(TARGET_POOL)
			.isValueRequired(true)
			.helpText(TR("Restrict output to the namespaces on specific pools by supplying the pool "
					"target and one or more comma-separated pool identifiers. The default is to "
					"display namespaces on all pools."));
	showNamespace.addProperty(wbem::TYPE_KEY, false,
			wbem::pmem_config::NS_TYPE_STR_UNKNOWN + "|" + wbem::pmem_config::NS_TYPE_STR_APPDIRECT + "|" +
				wbem::pmem_config::NS_TYPE_STR_STORAGE, true,
			TR("Restrict output to namespaces of a specific type by supplying the Type property "
				"with the desired namespace type. The default is to display every type of namespace."));
	showNamespace.addProperty(wbem::HEALTHSTATE_KEY, false,
			wbem::pmem_config::NS_HEALTH_STR_UNKNOWN + "|" + wbem::pmem_config::NS_HEALTH_STR_NORMAL + "|" +
			wbem::pmem_config::NS_HEALTH_STR_WARN + "|" + wbem::pmem_config::NS_HEALTH_STR_ERR + "|" +
			wbem::pmem_config::NS_HEALTH_STR_BROKENMIRROR, true,
			TR("Restrict output to namespaces with a specific health state by supplying the "
				"HealthState property with the desired health state. The default is to display "
				"namespaces in every state."));

	cli::framework::CommandSpec createNamespace(CREATE_NAMESPACE, TR("Create Namespace"), framework::VERB_CREATE,
			TR("Create a new namespace from a persistent memory pool of " NVM_DIMM_NAME " capacity."));
	createNamespace.addOption(framework::OPTION_FORCE);
	createNamespace.addOption(framework::OPTION_UNITS).helpText(TR(NS_UNITS_OPTION_DESC.c_str()));
	createNamespace.addTarget(TARGET_NAMESPACE_R)
			.valueText("")
			.isValueAccepted(false)
			.helpText(TR("Create a new namespace. No filtering is supported on this target."));
	createNamespace.addTarget(TARGET_POOL).isValueRequired(false)
			.helpText(TR("The pool identifier on which to create the namespace."));
	createNamespace.addProperty(CREATE_NS_PROP_TYPE, true, "AppDirect|Storage", true,
			TR("The type of namespace to be created."));
	createNamespace.addProperty(CREATE_NS_PROP_BLOCKSIZE, false, "size", true,
			TR("The logical size in bytes for read/write operations. Must be one of the supported "
					"block sizes retrieved using the Show System Capabilities command."));
	createNamespace.addProperty(CREATE_NS_PROP_BLOCKCOUNT, false, "count", true,
			TR("The total number of blocks of memory that make up the namespace (BlockCount x BlockSize = Capacity)."));
	createNamespace.addProperty(CREATE_NS_PROP_FRIENDLYNAME, false, "string", true,
			TR("Optional user specified namespace name to more easily identify the namespace. Up to a maximum of 64 "
			"characters."));
	createNamespace.addProperty(CREATE_NS_PROP_OPTIMIZE, false, "CopyOnWrite|None", true,
			TR("If the namespace has CopyOnWrite optimization turned on after creation."));
	createNamespace.addProperty(CREATE_NS_PROP_ENABLED, false, "0|1|False|True", true,
			TR("Enable or disable the namespace after creation. "
					"A disabled namespace is hidden from the OS by the driver."));
	createNamespace.addProperty(CREATE_NS_PROP_ENCRYPTION, false, "0|1|False|True", true,
			TR("If the namespace has Encryption turned on after creation."));
	createNamespace.addProperty(CREATE_NS_PROP_ERASECAPABLE, false, "0|1|False|True", true,
			TR("If the namespace supports erase capability after creation."));
	createNamespace.addProperty(APPDIRECTSETTINGS_PROPERTYNAME, false, "value", true,
			TR("Create the namespace on persistent memory matching the "
					"specified quality of service attributes. Only applicable if the "
					"type is App Direct."));
	createNamespace.addProperty(CREATE_NS_PROP_CAPACITY, false, "capacity", true,
			TR("The size of the namespace in GB. Capacity and BlockCount are exclusive "
					"and therefore cannot be used together. Note: Capacity can only be provided "
					"as decimal gigabytes and not gibibytes (e.g. 16.7 GB vs 16 GiB)."));
	createNamespace.addProperty(CREATE_NS_PROP_MEMORYPAGEALLOCATION, false, "None|DRAM|AppDirect", true,
			TR("Support access to the AppDirect namespace capacity using legacy memory page protocols "
					"such as DMA/RDMA by specifying where to create the underlying OS structures."));

	cli::framework::CommandSpec modifyNamespace(MODIFY_NAMESPACE, TR("Modify Namespace"), framework::VERB_SET,
			TR("Modify one or more existing namespaces."));
	modifyNamespace.addOption(framework::OPTION_FORCE);
	modifyNamespace.addTarget(TARGET_NAMESPACE_R).helpText(TR("Modify the settings on specific namespaces by "
			"providing comma separated list of one or more namespace identifiers. The default is to modify all namespaces."));
	modifyNamespace.addProperty(CREATE_NS_PROP_FRIENDLYNAME, false, "string", true,
			TR("Change the user specified namespace name up to a maximum of 64 characters."));
	modifyNamespace.addProperty("BlockCount", false, "count", true,
			TR("Change the total number of blocks of memory that make up in the namespace "
			"(BlockCount x BlockSize = Capacity)."));
	modifyNamespace.addProperty(CREATE_NS_PROP_ENABLED, false, "0|1", true,
			TR("Enable or disable the namespace.  A disabled namespace is hidden from the OS by the "
			"driver."));
	modifyNamespace.addProperty(CREATE_NS_PROP_CAPACITY, false, "capacity", true,
			TR("Change the size of the namespace."));

	cli::framework::CommandSpec deleteNamespace(DELETE_NAMESPACE, TR("Delete Namespace"), framework::VERB_DELETE,
			TR("Delete one or more existing namespaces. All data on the deleted namespace(s) becomes "
					"inaccessible."));
	deleteNamespace.addOption(framework::OPTION_FORCE);
	deleteNamespace.addTarget(TARGET_NAMESPACE_R).helpText(TR("Delete specific namespaces by providing "
			"a comma separated list of one or more namespace identifiers. The default is to "
			"delete all namespaces."));


	framework::CommandSpec showPools(SHOW_POOLS, TR("Show Persistent Memory"), framework::VERB_SHOW,
			TR("Retrieve a list of persistent memory pools of " NVM_DIMM_NAME " capacity."));
	showPools.addOption(framework::OPTION_DISPLAY);
	showPools.addOption(framework::OPTION_ALL);
	showPools.addOption(framework::OPTION_UNITS).helpText(TR("Change the units the pool capacities are displayed in."));
	showPools.addTarget(TARGET_POOL_R).helpText(TR("Restrict output to specific persistent "
			"memory pools by providing one or more comma-separated pool identifiers. "
			"The default is to display the persistent memory pools across all "
			"manageable " NVM_DIMM_NAME "s."));
	showPools.addTarget(TARGET_SOCKET).helpText(TR("Restrict output to the persistent memory "
			"pools on specific sockets by supplying the socket target and one or more "
			"comma-separated socket identifiers. The default is to display all sockets."));


	framework::CommandSpec dumpConfig(DUMP_CONFIG, TR("Dump Memory Allocation Settings"), framework::VERB_DUMP,
			TR("Store the currently configured memory allocation settings for all " NVM_DIMM_NAME "s in the "
					"system to a file in order to replicate the configuration elsewhere. Apply the stored "
					"memory allocation settings using the Load Memory Allocation Goal command."));
	dumpConfig.addOption(framework::OPTION_DESTINATION_R).helpText(TR("The file path in which to store "
			"the memory allocation settings. The resulting file will contain the memory allocation settings "
			"for all configured " NVM_DIMM_NAME "s in the system."));
	dumpConfig.addTarget(TARGET_SYSTEM_R).helpText(TR("The host system."))
			.isValueAccepted(false);
	dumpConfig.addTarget(TARGET_CONFIG_R).helpText(TR("The current " NVM_DIMM_NAME " memory allocation settings."))
			.isValueAccepted(false);


	framework::CommandSpec loadGoal(LOAD_CONFIG_GOAL, TR("Load Memory Allocation Goal"), framework::VERB_LOAD,
			TR("Load a memory allocation goal from a file onto one or more " NVM_DIMM_NAME "s."));
	loadGoal.addOption(framework::OPTION_FORCE).helpText(TR("Reconfiguring " NVM_DIMM_NAME "s is a destructive operation "
			"which requires confirmation from the user. This option suppresses the confirmation."));
	loadGoal.addOption(framework::OPTION_SOURCE_R).helpText(TR("File path of the stored configuration "
			"settings to load as a memory allocation goal."));
	loadGoal.addTarget(TARGET_GOAL_R).isValueAccepted(false);
	loadGoal.addTarget(TARGET_DIMM)
			.helpText(TR("Load the memory allocation goal to specific " NVM_DIMM_NAME "s "
			"by supplying one or more comma-separated " NVM_DIMM_NAME " identifiers. The default is to load the "
			"memory allocation goal onto all manageable " NVM_DIMM_NAME "s."))
			.isValueRequired(true);
	loadGoal.addTarget(TARGET_SOCKET).helpText(TR("Load the memory allocation goal onto all manageable " NVM_DIMM_NAME "s on "
			"specific sockets by supplying the socket target and one or more comma-separated socket identifiers. "
			"The default is to load the memory allocation goal onto all manageable " NVM_DIMM_NAME "s on all sockets."));

	list.push_back(showNamespace);
	list.push_back(createNamespace);
	list.push_back(modifyNamespace);
	list.push_back(deleteNamespace);
	list.push_back(showConfigGoal);
	list.push_back(deleteConfigGoal);
	list.push_back(createGoal);
	list.push_back(showPools);
	list.push_back(dumpConfig);
	list.push_back(loadGoal);
 }


// Constructor, just calls super class
cli::nvmcli::NamespaceFeature::NamespaceFeature() : cli::framework::FeatureBase(),
	m_deleteNamespace(wbemDeleteNamespace),
	m_getSupportedBlockSizes(wbemGetSupportedBlockSizes),
	m_getSupportedSizeRange(wbemGetSupportedSizeRange),
	m_poolUid(""), m_blockSize(0), m_blockSizeExists(false),
	m_blockCount(0), m_blockCountExists(false), m_nsTypeStr(""),
	m_nsType(0), m_capacityExists(false), m_capacityGB(0),
	m_friendlyName(""), m_friendlyNameExists(false),
	m_enableState(0), m_enabledStateExists(false),
	m_optimize(0), m_encryption(0), m_eraseCapable(0), m_reserveDimm(false),
	m_channelSize(wbem::mem_config::MEMORYALLOCATIONSETTINGS_EXPONENT_UNKNOWN),
	m_controllerSize(wbem::mem_config::MEMORYALLOCATIONSETTINGS_EXPONENT_UNKNOWN),
	m_byOne(false), m_forceOption(false), m_appDirectIsRemaining(false),
	m_storageIsRemaining(false), m_memoryPageAllocation(0), m_optimizeExists(false),
	m_pPmNamespaceProvider(new wbem::pmem_config::PersistentMemoryNamespaceFactory()),
	m_pPmServiceProvider(new wbem::pmem_config::PersistentMemoryServiceFactory()),
	m_pPmPoolProvider(new wbem::pmem_config::PersistentMemoryPoolFactory()),
	m_pCapProvider(new wbem::pmem_config::PersistentMemoryCapabilitiesFactory()),
	m_pNsViewFactoryProvider(new wbem::pmem_config::NamespaceViewFactory()),
	m_pWbemToCli(new cli::nvmcli::WbemToCli())
{ }

cli::nvmcli::NamespaceFeature::~NamespaceFeature()
{
	delete m_pPmServiceProvider;
	delete m_pPmPoolProvider;
	delete m_pCapProvider;
	delete m_pNsViewFactoryProvider;
	delete m_pWbemToCli;
	delete m_pPmNamespaceProvider;
}


/*
 * Get all the BaseServer Instances from the wbem base server factory.
 */
cli::framework::ResultBase * cli::nvmcli::NamespaceFeature::run(
		const int &commandSpecId, const framework::ParsedCommand& parsedCommand)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::ResultBase *pResult = NULL;
	switch (commandSpecId)
	{
		case SHOW_CONFIG_GOAL:
			pResult = showConfigGoal(parsedCommand);
			break;
		case DELETE_CONFIG_GOAL:
			pResult = deleteConfigGoal(parsedCommand);
			break;
		case CREATE_GOAL:
			pResult = createGoal(parsedCommand);
			break;
		case SHOW_POOLS:
			pResult = showPools(parsedCommand);
			break;
		case DUMP_CONFIG:
			pResult = dumpConfig(parsedCommand);
			break;
		case LOAD_CONFIG_GOAL:
			pResult = loadGoal(parsedCommand);
			break;
		case SHOW_NAMESPACE:
			pResult = showNamespaces(parsedCommand);
			break;
		case DELETE_NAMESPACE:
			pResult = deleteNamespaces(parsedCommand);
			break;
		case CREATE_NAMESPACE:
			pResult = createNamespace(parsedCommand);
			break;
		case MODIFY_NAMESPACE:
			pResult = modifyNamespace(parsedCommand);
			break;
		default:
			pResult = new framework::NotImplementedErrorResult(commandSpecId, Name);
			break;
	}
	return pResult;

}

cli::framework::ResultBase *cli::nvmcli::NamespaceFeature::showPools(cli::framework::ParsedCommand const &parsedCommand)
{
	framework::ResultBase *pResult = NULL;
	wbem::mem_config::PoolViewFactory poolViewFactory;
	wbem::framework::attribute_names_t attributes;
	wbem::framework::instances_t *pInstances = NULL;
	try
	{
		// define default display attributes
		wbem::framework::attribute_names_t defaultAttributes;
		defaultAttributes.push_back(wbem::POOLID_KEY);
		defaultAttributes.push_back(wbem::POOLTYPE_KEY);
		defaultAttributes.push_back(wbem::CAPACITY_KEY);
		defaultAttributes.push_back(wbem::FREECAPACITY_KEY);

		// define all attributes
		wbem::framework::attribute_names_t allAttributes(defaultAttributes);
		allAttributes.push_back(wbem::ENCRYPTIONCAPABLE_KEY);
		allAttributes.push_back(wbem::ENCRYPTIONENABLED_KEY);
		allAttributes.push_back(wbem::ERASECAPABLE_KEY);
		allAttributes.push_back(wbem::SOCKETID_KEY);
		allAttributes.push_back(wbem::APPDIRECTNAMESPACE_MAX_SIZE_KEY);
		allAttributes.push_back(wbem::APPDIRECTNAMESPACE_MIN_SIZE_KEY);
		allAttributes.push_back(wbem::APPDIRECTNAMESPACE_COUNT_KEY);
		allAttributes.push_back(wbem::STORAGENAMESPACE_MAX_SIZE_KEY);
		allAttributes.push_back(wbem::STORAGENAMESPACE_MIN_SIZE_KEY);
		allAttributes.push_back(wbem::STORAGENAMESPACE_COUNT_KEY);
		allAttributes.push_back(wbem::HEALTHSTATE_KEY);
		allAttributes.push_back(wbem::APP_DIRECT_SETTINGS_KEY);

		// get the desired attributes
		wbem::framework::attribute_names_t attributes =
				GetAttributeNames(parsedCommand.options, defaultAttributes, allAttributes);

		// make sure we have the Pool  id in our display
		// this would cover the case the user asks for specific display attributes, but they
		// don't include the physical ID
		if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(wbem::POOLID_KEY,
				attributes))
		{
			attributes.insert(attributes.begin(), wbem::POOLID_KEY);
		}

		// create the display filters
		wbem::framework::attribute_names_t requestedAttributes = attributes;
		cli::nvmcli::filters_t filters;
		generateSocketFilter(parsedCommand, requestedAttributes, filters);
		generatePoolFilter(parsedCommand, requestedAttributes, filters);
		pInstances = poolViewFactory.getInstances(requestedAttributes);
		if (pInstances == NULL)
		{
			COMMON_LOG_ERROR("PoolViewFactory getInstances returned a NULL instances pointer");
			pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
				TRS(nvmcli::UNKNOWN_ERROR_STR));
		}
		else
		{
			// get the desired units of capacity
			std::string capacityUnits;
			pResult = getCapacityUnits(parsedCommand, &capacityUnits);
			if (pResult == NULL)
			{
				for (size_t i = 0; i < pInstances->size(); i++)
				{
					cli::nvmcli::convertCapacityAttribute((*pInstances)[i], wbem::CAPACITY_KEY, capacityUnits);
					cli::nvmcli::convertCapacityAttribute((*pInstances)[i], wbem::FREECAPACITY_KEY, capacityUnits);
					cli::nvmcli::convertCapacityAttribute((*pInstances)[i], wbem::APPDIRECTNAMESPACE_MAX_SIZE_KEY, capacityUnits);
					cli::nvmcli::convertCapacityAttribute((*pInstances)[i], wbem::APPDIRECTNAMESPACE_MIN_SIZE_KEY, capacityUnits);
					cli::nvmcli::convertCapacityAttribute((*pInstances)[i], wbem::STORAGENAMESPACE_MAX_SIZE_KEY, capacityUnits);
					cli::nvmcli::convertCapacityAttribute((*pInstances)[i], wbem::STORAGENAMESPACE_MIN_SIZE_KEY, capacityUnits);
				}
				pResult = NvmInstanceToObjectListResult(*pInstances, "Pool",
						wbem::POOLID_KEY, attributes, filters);
				// Set layout to table unless the -all or -display option is present
				if (!framework::parsedCommandContains(parsedCommand, framework::OPTION_DISPLAY) &&
					!framework::parsedCommandContains(parsedCommand, framework::OPTION_ALL))
				{
					pResult->setOutputType(framework::ResultBase::OUTPUT_TEXTTABLE);
				}
			}
		}
	}
	catch (wbem::framework::Exception &e)
	{
		if (pResult)
		{
			delete pResult;
			pResult = NULL;
		}
		pResult = NvmExceptionToResult(e);
	}

	if (pInstances)
	{
		delete pInstances;
	}

	return pResult;
}

/*
 * create filters for pool ID
 */
void cli::nvmcli::NamespaceFeature::generatePoolFilter(
		const cli::framework::ParsedCommand &parsedCommand,
		wbem::framework::attribute_names_t &attributes,
		cli::nvmcli::filters_t &filters)
{
	std::vector<std::string> poolTargets =
			cli::framework::Parser::getTargetValues(parsedCommand,
					cli::nvmcli::TARGET_POOL.name);
	if (!poolTargets.empty())
	{
		struct instanceFilter poolFilter;
		poolFilter.attributeName = wbem::POOLID_KEY;

		for (std::vector<std::string>::const_iterator poolTargetIter = poolTargets.begin();
			 poolTargetIter != poolTargets.end(); poolTargetIter++)
		{
			std::string target = (*poolTargetIter);
			poolFilter.attributeValues.push_back(target);
		}

		if (!poolFilter.attributeValues.empty())
		{
			filters.push_back(poolFilter);
			// make sure we have the Pool ID filter attribute
			if (!wbem::framework_interface::NvmInstanceFactory::containsAttribute(wbem::POOLID_KEY, attributes))
			{
				attributes.insert(attributes.begin(), wbem::POOLID_KEY);
			}
		}
	}
}

void cli::nvmcli::NamespaceFeature::setPersistentMemoryNamespaceProvider(
		wbem::pmem_config::PersistentMemoryNamespaceFactory *pProvider)
{
	SET_PROVIDER(m_pPmNamespaceProvider, pProvider);
}

void cli::nvmcli::NamespaceFeature::setPersistentMemoryServiceProvider(
		wbem::pmem_config::PersistentMemoryServiceFactory *pProvider)
{
	SET_PROVIDER(m_pPmServiceProvider, pProvider);
}

void cli::nvmcli::NamespaceFeature::setPmPoolProvider(
		wbem::pmem_config::PersistentMemoryPoolFactory *pProvider)
{
	SET_PROVIDER(m_pPmPoolProvider, pProvider);
}

void cli::nvmcli::NamespaceFeature::setCapabilitiesProvider(
		wbem::pmem_config::PersistentMemoryCapabilitiesFactory *pProvider)
{
	SET_PROVIDER(m_pCapProvider, pProvider);
}

void cli::nvmcli::NamespaceFeature::setNsViewProvider(
		wbem::pmem_config::NamespaceViewFactory *pProvider)
{
	SET_PROVIDER(m_pNsViewFactoryProvider, pProvider);
}

void cli::nvmcli::NamespaceFeature::setWbemToCli(
		cli::nvmcli::WbemToCli *pInstance)
{
	SET_PROVIDER(m_pWbemToCli, pInstance);
}

// if units option is not specified in the cli command, capacity units from
// configuration setting SQL_KEY_CLI_SIZE is chosen as display units
cli::framework::SyntaxErrorResult *cli::nvmcli::NamespaceFeature::getCapacityUnits(
		const cli::framework::ParsedCommand &parsedCommand, std::string *pCapacityUnits)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::SyntaxErrorResult *pResult = NULL;

	bool unitsOption = parsedCommand.options.find(framework::OPTION_UNITS.name)
				!= parsedCommand.options.end();
	if (unitsOption)
	{
		*pCapacityUnits = parsedCommand.options.at(framework::OPTION_UNITS.name);
		if (!(*pCapacityUnits).empty() &&
			(!framework::stringsIEqual(*pCapacityUnits, PREFERENCE_SIZE_GIB) &&
			!framework::stringsIEqual(*pCapacityUnits, cli::nvmcli::CAPACITY_UNITS_GB)))
		{
			pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_OPTION,
					framework::OPTION_UNITS.name, *pCapacityUnits);
		}
	}

	return pResult;
}
