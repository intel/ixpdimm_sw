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
 * This file contains the implementation of the CMPI Provider.
 * Each CIM intrinsic method is represented here by a Generic_
 * function and is called directly by the CMPI CIMOM. Associated Cleanup functions are called
 * shortly after the CIMOM has called any intrinsic method. The CIMOM may call multiple intrinsic
 * methods prior to executing the cleanup function.  Currently the cleanup function is used to
 * gather all the logs from the csv file to the database as the CIMOM process may not release the
 * libixpdimm-cim library (which would call lib_unload).
 */

#include <libinvm-cim/IntelCmpiProvider.h>
#ifdef __ESX__
#include "cmpiMonitor.h"
#endif

void InstanceProviderInit()
{
#ifdef __ESX__
	// TODO: fix ESX endless loop
	// cmpiMonitor::Init();
#endif
}


// Instance Provider Entry Points, provider names only
CMInstanceMIStubName(Intel_BaseServer);
CMInstanceMIStubName(Intel_NVDIMM);
CMInstanceMIStubName(Intel_RawMemory);
CMInstanceMIStubName(Intel_MemoryController);
CMInstanceMIStubName(Intel_SystemProcessor);
CMInstanceMIStubName(Intel_ServerChassis);
CMInstanceMIStubName(Intel_SupportDataService);
CMInstanceMIStubName(Intel_OpaqueSupportData);
CMInstanceMIStubName(Intel_NVDIMMFWVersion);
CMInstanceMIStubName(Intel_AvailableFW);
CMInstanceMIStubName(Intel_NVDIMMCollection);
CMInstanceMIStubName(Intel_NVDIMMSoftwareInstallationService);
CMInstanceMIStubName(Intel_ErasureCapabilities);
CMInstanceMIStubName(Intel_NVDIMMSensor);
CMInstanceMIStubName(Intel_ErasureService);
CMInstanceMIStubName(Intel_PerformanceMetric);
CMInstanceMIStubName(Intel_PerformanceMetricDefinition);
CMInstanceMIStubName(Intel_MemoryResources);
CMInstanceMIStubName(Intel_PersistentMemoryPool);
CMInstanceMIStubName(Intel_VolatileMemory);
CMInstanceMIStubName(Intel_MemoryCapabilities);
CMInstanceMIStubName(Intel_MemoryConfigurationService);
CMInstanceMIStubName(Intel_MemoryConfigurationCapabilities);
CMInstanceMIStubName(Intel_MemoryAllocationSettings);
CMInstanceMIStubName(Intel_FWEventLog);
CMInstanceMIStubName(Intel_HostSoftware);
CMInstanceMIStubName(Intel_NVDIMMDiagnostic);
CMInstanceMIStubName(Intel_DiagnosticIdentity);
CMInstanceMIStubName(Intel_DiagnosticLog);
CMInstanceMIStubName(Intel_DiagnosticCompletionRecord);
CMInstanceMIStubName(Intel_GenericDriverIdentity);
CMInstanceMIStubName(Intel_NVDIMMDriverIdentity);
CMInstanceMIStubName(Intel_ManagementSoftwareIdentity);
CMInstanceMIStubName(Intel_NVDIMMEventLog);
CMInstanceMIStubName(Intel_NVDIMMLogEntry);
CMInstanceMIStubName(Intel_PersistentMemoryNamespace);
CMInstanceMIStubName(Intel_NamespaceSettings);
CMInstanceMIStubName(Intel_PersistentMemoryCapabilities);
CMInstanceMIStubName(Intel_PersistentMemoryService);
CMInstanceMIStubName(Intel_PersistentConfigurationCapabilities);
CMInstanceMIStubName(Intel_PersistentMemory);
CMInstanceMIStubName(Intel_RegisteredProfile); // interop namespace only
CMInstanceMIStubName(Intel_StaticFilter);

// Association classes
CMInstanceMIStubName(Intel_SystemDevice);
CMInstanceMIStubName(Intel_ComputerSystemPackage);
CMInstanceMIStubName(Intel_ConcreteComponent);
CMInstanceMIStubName(Intel_ConcreteDependency);
CMInstanceMIStubName(Intel_ControlledBy);
CMInstanceMIStubName(Intel_Realizes);
CMInstanceMIStubName(Intel_MemberOfCollection);
CMInstanceMIStubName(Intel_InstalledSoftwareIdentity);
CMInstanceMIStubName(Intel_HostedCollection);
CMInstanceMIStubName(Intel_ServiceAffectsElement);
CMInstanceMIStubName(Intel_AssociatedSensor);
CMInstanceMIStubName(Intel_MetricForME);
CMInstanceMIStubName(Intel_MetricDefForME);
CMInstanceMIStubName(Intel_HostedResourcePool);
CMInstanceMIStubName(Intel_HostedStoragePool);
CMInstanceMIStubName(Intel_ElementCapabilities);
CMInstanceMIStubName(Intel_AllocatedFromStoragePool);
CMInstanceMIStubName(Intel_ElementSettingData);
CMInstanceMIStubName(Intel_ElementAllocatedFromPool);
CMInstanceMIStubName(Intel_BasedOn);
CMInstanceMIStubName(Intel_HostedService);
CMInstanceMIStubName(Intel_LogManagesRecord);
CMInstanceMIStubName(Intel_ElementSoftwareIdentity);
CMInstanceMIStubName(Intel_UseOfLog);
CMInstanceMIStubName(Intel_MetricInstance);
CMInstanceMIStubName(Intel_AssociatedMemory);
CMInstanceMIStubName(Intel_AvailableDiagnosticService);
CMInstanceMIStubName(Intel_ElementView);
CMInstanceMIStubName(Intel_ElementConformsToProfile);

// Association Provider Entry Points
CMAssociationMIStubName(Intel_SystemDevice);
CMAssociationMIStubName(Intel_ComputerSystemPackage);
CMAssociationMIStubName(Intel_ControlledBy);
CMAssociationMIStubName(Intel_ConcreteComponent);
CMAssociationMIStubName(Intel_ConcreteDependency);
CMAssociationMIStubName(Intel_AssociatedMemory);
CMAssociationMIStubName(Intel_Realizes);
CMAssociationMIStubName(Intel_MemberOfCollection);
CMAssociationMIStubName(Intel_InstalledSoftwareIdentity);
CMAssociationMIStubName(Intel_HostedCollection);
CMAssociationMIStubName(Intel_ServiceAffectsElement);
CMAssociationMIStubName(Intel_AssociatedSensor);
CMAssociationMIStubName(Intel_MetricForME);
CMAssociationMIStubName(Intel_MetricDefForME);
CMAssociationMIStubName(Intel_HostedResourcePool);
CMAssociationMIStubName(Intel_HostedStoragePool);
CMAssociationMIStubName(Intel_ElementCapabilities);
CMAssociationMIStubName(Intel_AllocatedFromStoragePool);
CMAssociationMIStubName(Intel_ElementSettingData);
CMAssociationMIStubName(Intel_ElementAllocatedFromPool);
CMAssociationMIStubName(Intel_BasedOn);
CMAssociationMIStubName(Intel_HostedService);
CMAssociationMIStubName(Intel_LogManagesRecord);
CMAssociationMIStubName(Intel_ElementSoftwareIdentity);
CMAssociationMIStubName(Intel_UseOfLog);
CMAssociationMIStubName(Intel_AvailableDiagnosticService);
CMAssociationMIStubName(Intel_ElementView);
CMAssociationMIStubName(Intel_ElementConformsToProfile);
CMAssociationMIStubName(Intel_MetricInstance);

// Method Provider Entry Points
CMMethodMIStubName(Intel_NVDIMM);
CMMethodMIStubName(Intel_SupportDataService);
CMMethodMIStubName(Intel_NVDIMMSoftwareInstallationService);
CMMethodMIStubName(Intel_ErasureService);
CMMethodMIStubName(Intel_MemoryConfigurationService);
CMMethodMIStubName(Intel_NVDIMMDiagnostic);
CMMethodMIStubName(Intel_DiagnosticLog);
CMMethodMIStubName(Intel_NVDIMMEventLog);
CMMethodMIStubName(Intel_PersistentMemoryService);
CMMethodMIStubName(Intel_PersistentMemoryCapabilities);
CMMethodMIStubName(Intel_PersistentMemoryPool);
CMMethodMIStubName(Intel_PersistentMemoryNamespace);

// Indication Provider Entry Points
CMIndicationMIStubName(Intel_NVDIMMEvent);
CMIndicationMIStubName(Intel_InstCreation);
CMIndicationMIStubName(Intel_InstDeletion);
CMIndicationMIStubName(Intel_InstModification);

