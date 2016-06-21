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

#include "NvmProviderFactory.h"

#include <persistence/logging.h>
#include <LogEnterExit.h>

#include <libinvm-cim/ObjectPathBuilder.h>
#include <server/BaseServerFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include <memory/RawMemoryFactory.h>
#include <memory/MemoryControllerFactory.h>
#include <profile/RegisteredProfileFactory.h>
#include <memory/SystemProcessorFactory.h>
#include <support/SupportDataServiceFactory.h>
#include <support/OpaqueSupportDataFactory.h>
#include <support/NVDIMMDiagnosticFactory.h>
#include <support/DiagnosticIdentityFactory.h>
#include <server/ServerChassisFactory.h>
#include <software/AvailableFWFactory.h>
#include <software/NVDIMMFWVersionFactory.h>
#include <software/NVDIMMSoftwareInstallationServiceFactory.h>
#include <software/NVDIMMCollectionFactory.h>
#include <support/NVDIMMSensorFactory.h>
#include <performance/PerformanceMetricFactory.h>
#include <performance/PerformanceMetricDefinitionFactory.h>
#include <erasure/ErasureServiceFactory.h>
#include <erasure/ErasureCapabilitiesFactory.h>
#include <pmem_config/PersistentMemoryPoolFactory.h>
#include <mem_config/MemoryResourcesFactory.h>
#include <memory/VolatileMemoryFactory.h>
#include <memory/PersistentMemoryFactory.h>
#include <mem_config/MemoryConfigurationCapabilitiesFactory.h>
#include <mem_config/MemoryConfigurationServiceFactory.h>
#include <mem_config/MemoryCapabilitiesFactory.h>
#include <mem_config/MemoryAllocationSettingsFactory.h>
#include <support/FWEventLogFactory.h>
#include <software/HostSoftwareFactory.h>
#include <software/NVDIMMDriverIdentityFactory.h>
#include <software/ManagementSoftwareIdentityFactory.h>
#include <software/ElementSoftwareIdentityFactory.h>
#include <support/DiagnosticLogFactory.h>
#include <support/DiagnosticCompletionRecordFactory.h>
#include <support/SanitizeJobFactory.h>
#include <support/NVDIMMEventLogFactory.h>
#include <support/NVDIMMLogEntryFactory.h>
#include <pmem_config/PersistentMemoryNamespaceFactory.h>
#include <pmem_config/NamespaceSettingsFactory.h>
#include <pmem_config/PersistentMemoryCapabilitiesFactory.h>
#include <pmem_config/PersistentMemoryServiceFactory.h>
#include <pmem_config/PersistentConfigurationCapabilitiesFactory.h>
#include <pegasus/PegasusRegisteredProfileFactory.h>
#include <indication/StaticFilterFactory.h>

#include <framework_interface/NvmAssociationFactory.h>
#include <framework_interface/NvmInstanceFactory.h>
#include "NvmIndicationService.h"



wbem::framework_interface::NvmProviderFactory::NvmProviderFactory() //: ProviderFactory()
{
	setDefaultCimNamespace(NVM_NAMESPACE_CSTR);
}


wbem::framework::InstanceFactory *wbem::framework_interface::NvmProviderFactory::getInstanceFactory(
	const std::string &className)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	COMMON_LOG_DEBUG_F("Getting NVMInstanceFactory for CIM Class %s", className.c_str());

	wbem::framework::InstanceFactory *pFactory;

	if (className == wbem::server::BASESERVER_CREATIONCLASSNAME)
	{
		pFactory = new wbem::server::BaseServerFactory();
	}
	else if (className == wbem::physical_asset::NVDIMM_CREATIONCLASSNAME)
	{
		pFactory = new wbem::physical_asset::NVDIMMFactory();
	}
	else if (className == wbem::memory::RAWMEMORY_CREATIONCLASSNAME)
	{
		pFactory = new wbem::memory::RawMemoryFactory();
	}
	else if (className == wbem::memory::MEMORYCONTROLLER_CREATIONCLASSNAME)
	{
		pFactory = new wbem::memory::MemoryControllerFactory();
	}
	else if (className == wbem::profile::REGISTEREDPROFILE_CREATIONCLASSNAME)
	{
		pFactory = new wbem::profile::RegisteredProfileFactory();
	}
	else if (className == wbem::memory::SYSTEMPROCESSOR_CREATIONCLASSNAME)
	{
		pFactory = new wbem::memory::SystemProcessorFactory();
	}
	else if (className == wbem::support::SUPPORTDATASERVICE_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::SupportDataServiceFactory();
	}
	else if (className == wbem::support::OPAQUESUPPORTDATA_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::OpaqueSupportDataFactory();
	}
	else if (className == wbem::support::NVDIMMDIAGNOSTIC_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::NVDIMMDiagnosticFactory();
	}
	else if (className == wbem::support::DIAGNOSTICIDENTITY_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::DiagnosticIdentityFactory();
	}
	else if (className == wbem::server::SERVERCHASSIS_CREATIONCLASSNAME)
	{
		pFactory = new wbem::server::ServerChassisFactory();
	}
	else if (className == wbem::software::NVDIMMFWVERSION_CREATIONCLASSNAME)
	{
		pFactory = new wbem::software::NVDIMMFWVersionFactory();
	}
	else if (className == wbem::software::AVAILABLEFW_CREATIONCLASSNAME)
	{
		pFactory = new wbem::software::AvailableFWFactory();
	}
	else if (className == wbem::software::NVDIMMCOLLECTION_CREATIONCLASSNAME)
	{
		pFactory = new wbem::software::NVDIMMCollectionFactory();
	}
	else if (className == wbem::software::NVDIMMSOFTWAREINSTALLATIONSERVICE_CREATIONCLASSNAME)
	{
		pFactory = new wbem::software::NVDIMMSoftwareInstallationServiceFactory();
	}
	else if (className == wbem::erasure::ERASURESERVICE_CREATIONCLASSNAME)
	{
		pFactory = new wbem::erasure::ErasureServiceFactory();
	}
	else if (className == wbem::erasure::ERASURECAPABILITIES_CREATIONCLASSNAME)
	{
		pFactory = new wbem::erasure::ErasureCapabilitiesFactory();
	}
	else if (className == wbem::support::NVDIMMSENSOR_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::NVDIMMSensorFactory();
	}
	else if (className == wbem::performance::PERFORMANCE_METRIC_CREATIONCLASSNAME)
	{
		pFactory = new wbem::performance::PerformanceMetricFactory();
	}
	else if (className == wbem::performance::PERFORMANCEMETRICDEFINITION_CREATIONCLASSNAME)
	{
		pFactory = new wbem::performance::PerformanceMetricDefinitionFactory();
	}
	else if (className == wbem::mem_config::MEMORYRESOURCES_CREATIONCLASSNAME)
	{
		pFactory = new wbem::mem_config::MemoryResourcesFactory();
	}
	else if (className == wbem::pmem_config::PERSISTENTMEMORYPOOL_CREATIONCLASSNAME)
	{
		pFactory = new wbem::pmem_config::PersistentMemoryPoolFactory();
	}
	else if (className == wbem::memory::VOLATILEMEMORY_CREATIONCLASSNAME)
	{
		pFactory = new wbem::memory::VolatileMemoryFactory();
	}
	else if (className == wbem::memory::PERSISTENTMEMORY_CREATIONCLASSNAME)
	{
		pFactory = new wbem::memory::PersistentMemoryFactory();
	}
	else if (className == wbem::mem_config::MEMORYCONFIGURATIONCAPABILITIES_CREATIONCLASSNAME)
	{
		pFactory = new wbem::mem_config::MemoryConfigurationCapabilitiesFactory();
	}
	else if (className == wbem::mem_config::MEMORYCONFIGURATIONSERVICE_CREATIONCLASSNAME)
	{
		pFactory = new wbem::mem_config::MemoryConfigurationServiceFactory();
	}
	else if (className == wbem::mem_config::MEMORYCAPABILITIES_CREATIONCLASSNAME)
	{
		pFactory = new wbem::mem_config::MemoryCapabilitiesFactory();
	}
	else if (className == wbem::mem_config::MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME)
	{
		pFactory = new wbem::mem_config::MemoryAllocationSettingsFactory();
	}
	else if (className == wbem::support::FWEVENTLOG_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::FWEventLogFactory();
	}
	else if (className == wbem::software::HOSTSOFTWARE_CREATIONCLASSNAME)
	{
		pFactory = new wbem::software::HostSoftwareFactory();
	}
	else if (className == wbem::software::NVDIMMDRIVERIDENTITY_CREATIONCLASSNAME)
	{
		pFactory = new wbem::software::NVDIMMDriverIdentityFactory();
	}
	else if (className == wbem::software::MANAGEMENTSWIDENTITY_CREATIONCLASSNAME)
	{
		pFactory = new wbem::software::ManagementSoftwareIdentityFactory();
	}
	else if (className == wbem::support::DIAGNOSTICLOG_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::DiagnosticLogFactory();
	}
	else if (className == wbem::support::DIAGNOSTICCOMPLETION_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::DiagnosticCompletionRecordFactory();
	}
	else if (className == wbem::support::SANITIZEJOB_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::SanitizeJobFactory();
	}
	else if (className == wbem::support::NVDIMMEVENTLOG_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::NVDIMMEventLogFactory();
	}
	else if (className == wbem::support::NVDIMMLOGENTRY_CREATIONCLASSNAME)
	{
		pFactory = new wbem::support::NVDIMMLogEntryFactory();
	}
	else if (className == wbem::pmem_config::PMNS_CREATIONCLASSNAME)
	{
		pFactory = new wbem::pmem_config::PersistentMemoryNamespaceFactory();
	}
	else if (className == wbem::pmem_config::NSSETTINGS_CREATIONCLASSNAME)
	{
		pFactory = new wbem::pmem_config::NamespaceSettingsFactory();
	}
	else if (className == wbem::pmem_config::PMCAP_CREATIONCLASSNAME)
	{
		pFactory = new wbem::pmem_config::PersistentMemoryCapabilitiesFactory();
	}
	else if (className == wbem::pmem_config::PM_SERVICE_CLASSNAME)
	{
		pFactory = new wbem::pmem_config::PersistentMemoryServiceFactory();
	}
	else if (className == wbem::pmem_config::PM_CAP_CLASSNAME)
	{
		pFactory = new wbem::pmem_config::PersistentConfigurationCapabilitiesFactory();
	}
	else if (className == wbem::pegasus::PGREGISTEREDPROFILE_CLASSNAME)
	{
		pFactory = new wbem::pegasus::PegasusRegisteredProfileFactory();
	}
	else if (className == wbem::indication::STATICFILTER_CREATIONCLASSNAME)
	{
		pFactory = new wbem::indication::StaticFilterFactory();
	}
	else
	{
		// Should have only gotten one result when querying by class name
		std::vector<wbem::framework::InstanceFactory *> factories = getAssociationFactories(NULL, className);
		if (factories.size() == 1)
		{
			pFactory = factories.back();
		}
		else
		{
			COMMON_LOG_ERROR_F("Error getting instance factory: %s", className.c_str());
			pFactory = NULL;

			// Clean up
			while (!factories.empty())
			{
				wbem::framework::InstanceFactory *pAssocFactory = factories.back();
				factories.pop_back();
				delete pAssocFactory;
			}
		}
	}

	return pFactory;
}

std::vector<wbem::framework::InstanceFactory *> wbem::framework_interface::NvmProviderFactory::getAssociationFactories(
	wbem::framework::Instance *pInstance, const std::string &associationClassName,
	const std::string &resultClassName, const std::string &roleName,
	const std::string &resultRoleName)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	COMMON_LOG_DEBUG_F("Getting AssociationFactory for CIM Class '%s'", associationClassName.c_str());

	std::vector<wbem::framework::InstanceFactory *> factories;

	if (associationClassName.empty() || associationClassName == software::ELEMENTSOFTWAREIDENTITY_CREATIONCLASSNAME)
	{
		factories.push_back(new software::ElementSoftwareIdentityFactory(pInstance, resultClassName, roleName, resultRoleName));
	}

	wbem::framework::AssociationFactory *pFactory = new NvmAssociationFactory(
			pInstance, associationClassName, resultClassName, roleName, resultRoleName);
	if (associationClassName.empty() || pFactory->isAssociationClass(associationClassName))
	{
		factories.push_back(pFactory);
	}
	else
	{
		delete pFactory;
	}

	return factories;
}

wbem::framework::IndicationService *wbem::framework_interface::NvmProviderFactory::getIndicationService()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	return framework_interface::NvmIndicationService::getSingleton();
}

