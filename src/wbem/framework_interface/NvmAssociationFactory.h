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
#ifndef CR_MGMT_NVMASSOCIATIONFACTORY_H
#define CR_MGMT_NVMASSOCIATIONFACTORY_H

#include <libinvm-cim/AssociationFactory.h>
#include <NvmStrings.h>
#include <nvm_types.h>

namespace wbem
{
namespace framework_interface
{

// Association class names
static const std::string ASSOCIATION_CLASS_SYSTEMDEVICE = std::string(NVM_WBEM_PREFIX) + "SystemDevice"; //!< System Device
static const std::string ASSOCIATION_CLASS_COMPUTERSYSTEMPACKAGE = std::string(NVM_WBEM_PREFIX) + "ComputerSystemPackage"; //!< Computer System Package
static const std::string ASSOCIATION_CLASS_CONTROLLEDBY = std::string(NVM_WBEM_PREFIX) + "ControlledBy"; //!< Controlled By
static const std::string ASSOCIATION_CLASS_ASSOCIATEDMEMORY = std::string(NVM_WBEM_PREFIX) + "AssociatedMemory"; //!< Associated Memory
static const std::string ASSOCIATION_CLASS_CONCRETECOMPONENT = std::string(NVM_WBEM_PREFIX) + "ConcreteComponent"; //!< Concrete Component
static const std::string ASSOCIATION_CLASS_CONCRETEDEPENDENCY = std::string(NVM_WBEM_PREFIX) + "ConcreteDependency"; //!< Concrete Dependency
static const std::string ASSOCIATION_CLASS_REALIZES = std::string(NVM_WBEM_PREFIX) + "Realizes"; //!< Realizes
static const std::string ASSOCIATION_CLASS_MEMBEROFCOLLECTION = std::string(NVM_WBEM_PREFIX) + "MemberOfCollection"; //!< MemberOfCollection
static const std::string ASSOCIATION_CLASS_INSTALLEDSOFTWAREIDENTITY = std::string(NVM_WBEM_PREFIX) + "InstalledSoftwareIdentity"; //!< MemberOfCollection
static const std::string ASSOCIATION_CLASS_HOSTEDCOLLECTION = std::string(NVM_WBEM_PREFIX) + "HostedCollection"; //!< HostedCollection
static const std::string ASSOCIATION_CLASS_SERVICEAFFECTSELEMENT = std::string(NVM_WBEM_PREFIX) + "ServiceAffectsElement"; //!< ServiceAffectsElement
static const std::string ASSOCIATION_CLASS_ASSOCIATEDSENSOR = std::string(NVM_WBEM_PREFIX) + "AssociatedSensor"; //!< Associated Sensor
static const std::string ASSOCIATION_CLASS_METRIC_FOR_ME = std::string(NVM_WBEM_PREFIX) + "MetricForME"; //!< Associated For Managed Element.
static const std::string ASSOCIATION_CLASS_METRIC_DEF_FOR_ME = std::string(NVM_WBEM_PREFIX) + "MetricDefForME"; //!< Associated Metric definition for a managed element.
static const std::string ASSOCIATION_CLASS_METRIC_INSTANCE = std::string(NVM_WBEM_PREFIX) + "MetricInstance"; //!< Associated For Managed Element.
static const std::string ASSOCIATION_CLASS_HOSTEDRESOURCEPOOL = std::string(NVM_WBEM_PREFIX) + "HostedResourcePool"; //!< Hosted Resource Pool
static const std::string ASSOCIATION_CLASS_HOSTEDSTORAGEPOOL = std::string(NVM_WBEM_PREFIX) + "HostedStoragePool"; //<! Hosted Storage Pool
static const std::string ASSOCIATION_CLASS_ELEMENTCAPABILITIES = std::string(NVM_WBEM_PREFIX) + "ElementCapabilities"; //<! Element Capabilities
static const std::string ASSOCIATION_CLASS_ALLOCATEDFROMSTORAGEPOOL = std::string(NVM_WBEM_PREFIX) + "AllocatedFromStoragePool"; //<! Allocated From Storage Pool
static const std::string ASSOCIATION_CLASS_ELEMENTSETTINGDATA = std::string(NVM_WBEM_PREFIX) + "ElementSettingData"; //<! Element Setting Data
static const std::string ASSOCIATION_CLASS_ELEMENTALLOCATEDFROMPOOL = std::string(NVM_WBEM_PREFIX) + "ElementAllocatedFromPool"; //<! Element Allocated From Pool
static const std::string ASSOCIATION_CLASS_BASEDON = std::string(NVM_WBEM_PREFIX) + "BasedOn"; //<! BasedOn
static const std::string ASSOCIATION_CLASS_HOSTEDSERVICE = std::string(NVM_WBEM_PREFIX) + "HostedService"; //<! HostedService
static const std::string ASSOCIATION_CLASS_LOGMANAGESRECORD = std::string(NVM_WBEM_PREFIX) + "LogManagesRecord"; //!< Log Manages Record
static const std::string ASSOCIATION_CLASS_USEOFLOG = std::string(NVM_WBEM_PREFIX) + "UseOfLog"; //!< UseOfLog
static const std::string ASSOCIATION_CLASS_AVAILABLEDIAGNOSTICSERVICE = std::string(NVM_WBEM_PREFIX) + "AvailableDiagnosticService"; //!< AvailableDiagnosticSevice
static const std::string ASSOCIATION_CLASS_ELEMENTVIEW = std::string(NVM_WBEM_PREFIX) + "ElementView"; //!< ElementView
static const std::string ASSOCIATION_CLASS_ELEMENTCONFORMSTOPROFILE = std::string(NVM_WBEM_PREFIX) + "ElementConformsToProfile"; //!< ElementConformsToProfile
static const std::string ASSOCIATION_CLASS_REFERENCEDPROFILE = std::string(NVM_WBEM_PREFIX) + "ReferencedProfile"; //!< ReferencedProfile



class NVM_API NvmAssociationFactory : public framework::AssociationFactory
{

protected:

public:
	NvmAssociationFactory(framework::Instance *pInstance = NULL, const std::string &associationClassName = "",
			const std::string &resultClassName = "", const std::string &roleName = "",
			const std::string &resultRoleName = "");

protected:
	virtual void initClassMap();

	virtual void initAssociationTable();
};
}
}



#endif //CR_MGMT_NVMASSOCIATIONFACTORY_H
