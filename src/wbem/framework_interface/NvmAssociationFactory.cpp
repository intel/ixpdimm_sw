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

#include "NvmAssociationFactory.h"

#include <LogEnterExit.h>

#include <memory/MemoryControllerFactory.h>
#include <memory/SystemProcessorFactory.h>
#include <memory/RawMemoryFactory.h>
#include <memory/VolatileMemoryFactory.h>
#include <memory/PersistentMemoryFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include <server/BaseServerFactory.h>
#include <server/ServerChassisFactory.h>
#include <software/AvailableFWFactory.h>
#include <software/NVDIMMFWVersionFactory.h>
#include <software/NVDIMMSoftwareInstallationServiceFactory.h>
#include <software/NVDIMMCollectionFactory.h>

#include <support/SupportDataServiceFactory.h>
#include <support/OpaqueSupportDataFactory.h>
#include <support/NVDIMMSensorFactory.h>
#include <performance/PerformanceMetricFactory.h>
#include <performance/PerformanceMetricDefinitionFactory.h>
#include <profile/RegisteredProfileFactory.h>
#include <mem_config/MemoryAllocationSettingsFactory.h>
#include <mem_config/MemoryCapabilitiesFactory.h>
#include <mem_config/MemoryConfigurationServiceFactory.h>
#include <mem_config/MemoryConfigurationCapabilitiesFactory.h>
#include <mem_config/MemoryResourcesFactory.h>
#include <support/FWEventLogFactory.h>
#include <software/HostSoftwareFactory.h>
#include <software/NVDIMMDriverIdentityFactory.h>
#include <software/ManagementSoftwareIdentityFactory.h>
#include <support/DiagnosticLogFactory.h>
#include <support/DiagnosticCompletionRecordFactory.h>
#include <support/NVDIMMDiagnosticFactory.h>
#include <support/DiagnosticIdentityFactory.h>
#include <support/NVDIMMEventLogFactory.h>
#include <support/NVDIMMLogEntryFactory.h>
#include <pmem_config/PersistentMemoryNamespaceFactory.h>
#include <pmem_config/NamespaceSettingsFactory.h>
#include <pmem_config/PersistentMemoryCapabilitiesFactory.h>
#include <pmem_config/PersistentMemoryServiceFactory.h>
#include <pmem_config/PersistentConfigurationCapabilitiesFactory.h>
#include <pmem_config/PersistentMemoryPoolFactory.h>
#include <erasure/ErasureServiceFactory.h>
#include <erasure/ErasureCapabilitiesFactory.h>


// Association types
static const std::string ASSOCIATION_ANTECEDENT = "Antecedent"; //!< Antecedent
static const std::string ASSOCIATION_DEPENDENT = "Dependent"; //!< Dependent

static const std::string ASSOCIATION_GROUPCOMPONENT = "GroupComponent"; //!< Group Component
static const std::string ASSOCIATION_PARTCOMPONENT = "PartComponent"; //!< Part Component

static const std::string ASSOCIATION_MEMBER = "Member"; //!< Member
static const std::string ASSOCIATION_COLLECTION = "Collection"; //!< Collection

static const std::string ASSOCIATION_SYSTEM = "System"; //!< System
static const std::string ASSOCIATION_INSTALLEDSOFTWARE = "InstalledSoftware"; //!< InstalledSoftware

static const std::string ASSOCIATION_AFFECTEDELEMENT = "AffectedElement"; //!< AffectingElement
static const std::string ASSOCIATION_AFFECTINGELEMENT = "AffectingElement"; //!< AffectingElement

static const std::string ASSOCIATION_CAPABILITIES = "Capabilities"; //!< Capabilities
static const std::string ASSOCIATION_MANAGEDELEMENT = "ManagedElement"; //!< ManagedElement
static const std::string ASSOCIATION_SETTINGDATA = "SettingData"; //!< SettingData
static const std::string ASSOCIATION_CONFORMANTSTANDARD = "ConformantStandard"; //!< ConformantStandard

static const std::string ASSOCIATION_SERVICEPROVIDED = "ServiceProvided"; //!< ServiceProvided
static const std::string ASSOCIATION_USEROFSERVICE = "UserOfService"; //!< UserOfservice

static const std::string ASSOCIATION_LOG = "Log";
static const std::string ASSOCIATION_RECORD = "Record";

void wbem::framework_interface::NvmAssociationFactory::initClassMap()
{
	LogEnterExit log(__FUNCTION__, __FILE__, __LINE__);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_SYSTEMDEVICE, ASSOCIATION_GROUPCOMPONENT,
		ASSOCIATION_PARTCOMPONENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_COMPUTERSYSTEMPACKAGE, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_CONCRETECOMPONENT, ASSOCIATION_GROUPCOMPONENT,
		ASSOCIATION_PARTCOMPONENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_CONCRETEDEPENDENCY, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_CONTROLLEDBY, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_REALIZES, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_MEMBEROFCOLLECTION, ASSOCIATION_COLLECTION,
		ASSOCIATION_MEMBER);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_INSTALLEDSOFTWAREIDENTITY, ASSOCIATION_SYSTEM,
		ASSOCIATION_INSTALLEDSOFTWARE);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDCOLLECTION, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_SERVICEAFFECTSELEMENT,
		ASSOCIATION_AFFECTEDELEMENT, ASSOCIATION_AFFECTINGELEMENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_ASSOCIATEDSENSOR, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_METRIC_FOR_ME, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_METRIC_DEF_FOR_ME, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDRESOURCEPOOL, ASSOCIATION_GROUPCOMPONENT,
		ASSOCIATION_PARTCOMPONENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDSTORAGEPOOL, ASSOCIATION_GROUPCOMPONENT,
		ASSOCIATION_PARTCOMPONENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCAPABILITIES,
		ASSOCIATION_MANAGEDELEMENT, ASSOCIATION_CAPABILITIES);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_ALLOCATEDFROMSTORAGEPOOL,
		ASSOCIATION_ANTECEDENT, ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTSETTINGDATA, ASSOCIATION_MANAGEDELEMENT,
		ASSOCIATION_SETTINGDATA);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTALLOCATEDFROMPOOL,
		ASSOCIATION_ANTECEDENT, ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_BASEDON, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDSERVICE, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_LOGMANAGESRECORD, ASSOCIATION_LOG,
		ASSOCIATION_RECORD);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_USEOFLOG, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_METRIC_INSTANCE, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_ASSOCIATEDMEMORY, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_AVAILABLEDIAGNOSTICSERVICE,
		ASSOCIATION_SERVICEPROVIDED, ASSOCIATION_USEROFSERVICE);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTVIEW, ASSOCIATION_ANTECEDENT,
		ASSOCIATION_DEPENDENT);
	addClassToMap(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCONFORMSTOPROFILE,
		ASSOCIATION_CONFORMANTSTANDARD, ASSOCIATION_MANAGEDELEMENT);
}

void wbem::framework_interface::NvmAssociationFactory::initAssociationTable()
{
	LogEnterExit log(__FUNCTION__, __FILE__, __LINE__);
	/*
	 * Association Class Name, Association Type,
	 * Associated Antecedent Class Name, Associated Dependent Class name,
	 * Optional Antecedent FK, Optional Dependent FK
	 */
	// Base Server Associations
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SYSTEMDEVICE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::server::BASESERVER_CREATIONCLASSNAME,
		wbem::memory::PERSISTENTMEMORY_CREATIONCLASSNAME,
		wbem::NAME_KEY, wbem::SYSTEMNAME_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SYSTEMDEVICE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::server::BASESERVER_CREATIONCLASSNAME, wbem::memory::VOLATILEMEMORY_CREATIONCLASSNAME,
		wbem::NAME_KEY, wbem::SYSTEMNAME_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SYSTEMDEVICE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::server::BASESERVER_CREATIONCLASSNAME, wbem::memory::SYSTEMPROCESSOR_CREATIONCLASSNAME,
		wbem::NAME_KEY, wbem::SYSTEMNAME_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SYSTEMDEVICE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::server::BASESERVER_CREATIONCLASSNAME, wbem::pmem_config::PMNS_CREATIONCLASSNAME,
		wbem::NAME_KEY, wbem::SYSTEMNAME_KEY);

	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDSERVICE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::server::BASESERVER_CREATIONCLASSNAME, wbem::pmem_config::PM_SERVICE_CLASSNAME,
		wbem::NAME_KEY, wbem::SYSTEMNAME_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDSERVICE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::server::BASESERVER_CREATIONCLASSNAME,
		wbem::software::NVDIMMSOFTWAREINSTALLATIONSERVICE_CREATIONCLASSNAME,
		wbem::NAME_KEY, wbem::SYSTEMNAME_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDSERVICE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::server::BASESERVER_CREATIONCLASSNAME, wbem::erasure::ERASURESERVICE_CREATIONCLASSNAME,
		wbem::NAME_KEY, wbem::SYSTEMNAME_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDSERVICE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::server::BASESERVER_CREATIONCLASSNAME,
		wbem::mem_config::MEMORYCONFIGURATIONSERVICE_CREATIONCLASSNAME,
		wbem::NAME_KEY, wbem::SYSTEMNAME_KEY);

	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDCOLLECTION,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::server::BASESERVER_CREATIONCLASSNAME, wbem::software::AVAILABLEFW_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDCOLLECTION,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::server::BASESERVER_CREATIONCLASSNAME,
		wbem::software::NVDIMMCOLLECTION_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDCOLLECTION,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::server::BASESERVER_CREATIONCLASSNAME, wbem::software::HOSTSOFTWARE_CREATIONCLASSNAME);

	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_COMPUTERSYSTEMPACKAGE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::server::SERVERCHASSIS_CREATIONCLASSNAME, wbem::server::BASESERVER_CREATIONCLASSNAME,
		wbem::TAG_KEY, wbem::NAME_KEY);

	// Topology Profile
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ASSOCIATEDMEMORY,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::memory::RAWMEMORY_CREATIONCLASSNAME, wbem::memory::MEMORYCONTROLLER_CREATIONCLASSNAME,
		wbem::MEMORYCONTROLLERID_KEY, wbem::DEVICEID_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_CONCRETEDEPENDENCY,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::memory::MEMORYCONTROLLER_CREATIONCLASSNAME,
		wbem::memory::SYSTEMPROCESSOR_CREATIONCLASSNAME);


	// Software Profile
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_MEMBEROFCOLLECTION,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::software::AVAILABLEFW_CREATIONCLASSNAME,
		wbem::software::NVDIMMFWVERSION_CREATIONCLASSNAME);

	// NVDIMM Associations
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_REALIZES,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME, wbem::memory::RAWMEMORY_CREATIONCLASSNAME,
		wbem::TAG_KEY, wbem::DEVICEID_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SERVICEAFFECTSELEMENT,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME,
		wbem::software::NVDIMMSOFTWAREINSTALLATIONSERVICE_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SERVICEAFFECTSELEMENT,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME,
		wbem::support::SUPPORTDATASERVICE_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SERVICEAFFECTSELEMENT,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME,
		wbem::support::NVDIMMDIAGNOSTIC_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_USEOFLOG,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::support::NVDIMMEVENTLOG_CREATIONCLASSNAME,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_USEOFLOG,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::support::FWEVENTLOG_CREATIONCLASSNAME, wbem::physical_asset::NVDIMM_CREATIONCLASSNAME,
		wbem::INSTANCEID_KEY, wbem::TAG_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ASSOCIATEDSENSOR,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::support::NVDIMMSENSOR_CREATIONCLASSNAME,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_AVAILABLEDIAGNOSTICSERVICE,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::support::NVDIMMDIAGNOSTIC_CREATIONCLASSNAME,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_METRIC_FOR_ME,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME,
		wbem::performance::PERFORMANCE_METRIC_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_METRIC_DEF_FOR_ME,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME,
		wbem::performance::PERFORMANCEMETRICDEFINITION_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_MEMBEROFCOLLECTION,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::software::NVDIMMCOLLECTION_CREATIONCLASSNAME,
		wbem::physical_asset::NVDIMM_CREATIONCLASSNAME);

	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SERVICEAFFECTSELEMENT,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::software::NVDIMMCOLLECTION_CREATIONCLASSNAME,
		wbem::software::NVDIMMSOFTWAREINSTALLATIONSERVICE_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_MEMBEROFCOLLECTION,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::software::HOSTSOFTWARE_CREATIONCLASSNAME,
		wbem::software::NVDIMMDRIVERIDENTITY_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_MEMBEROFCOLLECTION,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::software::HOSTSOFTWARE_CREATIONCLASSNAME,
		wbem::software::MANAGEMENTSWIDENTITY_CREATIONCLASSNAME);

	// Support Profile
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SERVICEAFFECTSELEMENT,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::support::OPAQUESUPPORTDATA_CREATIONCLASSNAME,
		wbem::support::SUPPORTDATASERVICE_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_LOGMANAGESRECORD,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::support::DIAGNOSTICLOG_CREATIONCLASSNAME,
		wbem::support::DIAGNOSTICCOMPLETION_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_USEOFLOG,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::support::DIAGNOSTICLOG_CREATIONCLASSNAME,
		wbem::support::NVDIMMDIAGNOSTIC_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_LOGMANAGESRECORD,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::support::NVDIMMEVENTLOG_CREATIONCLASSNAME,
		wbem::support::NVDIMMLOGENTRY_CREATIONCLASSNAME);

	// Performance Profile
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_METRIC_INSTANCE,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::performance::PERFORMANCEMETRICDEFINITION_CREATIONCLASSNAME,
		wbem::performance::PERFORMANCE_METRIC_CREATIONCLASSNAME,
		wbem::ID_KEY, wbem::METRICDEFINITION_ID_KEY);

	// Resource Profile
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDRESOURCEPOOL,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::server::BASESERVER_CREATIONCLASSNAME,
		wbem::mem_config::MEMORYRESOURCES_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SERVICEAFFECTSELEMENT,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::mem_config::MEMORYRESOURCES_CREATIONCLASSNAME,
		wbem::mem_config::MEMORYCONFIGURATIONSERVICE_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_CONCRETECOMPONENT,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::mem_config::MEMORYRESOURCES_CREATIONCLASSNAME,
		wbem::memory::RAWMEMORY_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTALLOCATEDFROMPOOL,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::mem_config::MEMORYRESOURCES_CREATIONCLASSNAME,
		wbem::memory::VOLATILEMEMORY_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTALLOCATEDFROMPOOL,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::mem_config::MEMORYRESOURCES_CREATIONCLASSNAME,
		wbem::memory::PERSISTENTMEMORY_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTSETTINGDATA,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::mem_config::MEMORYRESOURCES_CREATIONCLASSNAME,
		wbem::mem_config::MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCAPABILITIES,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::mem_config::MEMORYRESOURCES_CREATIONCLASSNAME,
		wbem::mem_config::MEMORYCAPABILITIES_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCAPABILITIES,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::mem_config::MEMORYCONFIGURATIONSERVICE_CREATIONCLASSNAME,
		wbem::mem_config::MEMORYCONFIGURATIONCAPABILITIES_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTSETTINGDATA,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::memory::PERSISTENTMEMORY_CREATIONCLASSNAME,
		wbem::mem_config::MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME);


	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTSETTINGDATA,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::memory::VOLATILEMEMORY_CREATIONCLASSNAME,
		wbem::mem_config::MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME);

	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_BASEDON,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::memory::RAWMEMORY_CREATIONCLASSNAME, wbem::memory::VOLATILEMEMORY_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_BASEDON,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::memory::RAWMEMORY_CREATIONCLASSNAME,
		wbem::memory::PERSISTENTMEMORY_CREATIONCLASSNAME);

	// Persistent Memory Profile
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTSETTINGDATA,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::pmem_config::PMNS_CREATIONCLASSNAME, wbem::pmem_config::NSSETTINGS_CREATIONCLASSNAME,
		wbem::DEVICEID_KEY, wbem::INSTANCEID_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_BASEDON,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::memory::PERSISTENTMEMORY_CREATIONCLASSNAME,
		wbem::pmem_config::PMNS_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_HOSTEDRESOURCEPOOL,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::server::BASESERVER_CREATIONCLASSNAME,
		wbem::pmem_config::PERSISTENTMEMORYPOOL_CREATIONCLASSNAME);

	// Persistent Memory Pool
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTALLOCATEDFROMPOOL,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::pmem_config::PERSISTENTMEMORYPOOL_CREATIONCLASSNAME,
		wbem::pmem_config::PMNS_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCAPABILITIES,
		wbem::framework::ASSOCIATIONTYPE_SIMPLEFK,
		wbem::pmem_config::PERSISTENTMEMORYPOOL_CREATIONCLASSNAME,
		wbem::pmem_config::PMCAP_CREATIONCLASSNAME,
		wbem::INSTANCEID_KEY, wbem::INSTANCEID_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_SERVICEAFFECTSELEMENT,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::pmem_config::PERSISTENTMEMORYPOOL_CREATIONCLASSNAME,
		wbem::pmem_config::PM_SERVICE_CLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_CONCRETECOMPONENT,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::pmem_config::PERSISTENTMEMORYPOOL_CREATIONCLASSNAME,
		wbem::memory::PERSISTENTMEMORY_CREATIONCLASSNAME,
		wbem::INSTANCEID_KEY, wbem::INSTANCEID_KEY);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCAPABILITIES,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::pmem_config::PM_SERVICE_CLASSNAME, wbem::pmem_config::PM_CAP_CLASSNAME);

	// Erasure profile
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCAPABILITIES,
		wbem::framework::ASSOCIATIONTYPE_BASIC,
		wbem::erasure::ERASURESERVICE_CREATIONCLASSNAME,
		wbem::erasure::ERASURECAPABILITIES_CREATIONCLASSNAME);

	// RegisteredProfile associations
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCONFORMSTOPROFILE,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::profile::REGISTEREDPROFILE_CREATIONCLASSNAME,
		wbem::server::BASESERVER_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCONFORMSTOPROFILE,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::profile::REGISTEREDPROFILE_CREATIONCLASSNAME,
		wbem::memory::PERSISTENTMEMORY_CREATIONCLASSNAME);
	addAssociationToTable(wbem::framework_interface::ASSOCIATION_CLASS_ELEMENTCONFORMSTOPROFILE,
		wbem::framework::ASSOCIATIONTYPE_COMPLEX,
		wbem::profile::REGISTEREDPROFILE_CREATIONCLASSNAME,
		wbem::memory::VOLATILEMEMORY_CREATIONCLASSNAME);
}

wbem::framework_interface::NvmAssociationFactory::NvmAssociationFactory(wbem::framework::Instance *pInstance,
		const std::string &associationClassName, const std::string &resultClassName,
		const std::string &roleName, const std::string &resultRoleName)
		:
		AssociationFactory(pInstance, associationClassName,
		resultClassName, roleName, resultRoleName)
{
	setCimNamespace(NVM_NAMESPACE);

	initClassMap();
	initAssociationTable();
}
