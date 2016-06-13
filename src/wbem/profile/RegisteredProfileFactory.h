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
 * This file contains the provider for the RegisteredProfile instances.
 */

#ifndef REGISTEREDPROFILEFACTORY_H_
#define REGISTEREDPROFILEFACTORY_H_

#include <libinvm-cim/Instance.h>
#include <libinvm-cim/Exception.h>
#include <libinvm-cim/Types.h>
#include <string>
#include <vector>
#include <map>
#include <framework_interface/NvmInstanceFactory.h>
#include <nvm_types.h>

namespace wbem
{
namespace profile
{

// Generic class constants
static const std::string REGISTEREDPROFILE_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "RegisteredProfile";
static const std::string REGISTEREDPROFILE_NAMESPACE = NVM_INTEROP_NAMESPACE;

// RegisteredOrganization
static const framework::UINT16 REGISTEREDPROFILE_REGISTEREDORG_OTHER = 1; //!< Intel-defined profiles
static const framework::UINT16 REGISTEREDPROFILE_REGISTEREDORG_DMTF = 2; //!< DMTF-defined profiles
static const framework::UINT16 REGISTEREDPROFILE_REGISTEREDORG_SNIA = 11; //!< SNIA-defined profiles

// OtherRegisteredOrganization
static const std::string REGISTEREDPROFILE_OTHERREGISTEREDORG_INTEL = "Intel";

// AdvertiseTypes
static const framework::UINT16 REGISTEREDPROFILE_ADVERTISETYPES_NOTADVERTISED = 2;

// RegisteredName for all supported profiles
static const std::string REGISTEREDPROFILE_REGISTEREDNAME_BASESERVER = "Base Server";
static const std::string REGISTEREDPROFILE_REGISTEREDNAME_SWINVENTORY = "Software Inventory";
static const std::string REGISTEREDPROFILE_REGISTEREDNAME_MULTITYPESYSTEMMEMORY = "Multi-type System Memory";

// Version for all supported profiles
static const std::string REGISTEREDPROFILE_VERSION_BASESERVER = "1.0.1";
static const std::string REGISTEREDPROFILE_VERSION_SWINVENTORY = "1.0.1";
static const std::string REGISTEREDPROFILE_VERSION_MULTITYPESYSTEMMEMORY = "1.0.0";

/*!
 * CIM provider for RegisteredProfile instances.
 * This provider is only exposed in the interop namespace
 * The list of supported profiles is effectively static.
 */
class NVM_API RegisteredProfileFactory : public framework_interface::NvmInstanceFactory
{
	public:
		/*!
		 * Constructor
		 */
		RegisteredProfileFactory();

		/*!
		 * Destructor
		 */
		virtual ~RegisteredProfileFactory();

		/*!
		 * Standard CIM method to retrieve a single instance.
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if not implemented.
		 * @return The requested instance.
		 */
		virtual framework::Instance* getInstance(framework::ObjectPath &path,
				framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Standard CIM method to retrieve a list of object paths for the instances in this factory.
		 * @throw Exception if not implemented.
		 * @return The list of object paths.
		 */
		virtual framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*!
		 * Determines if the two instances should be associated by the Association Class.
		 * @param associationClass
		 * @param pAntInstance
		 * @param pDepInstance
		 * @return true if associated
		 */
		virtual bool isAssociated(const std::string &associationClass,
				framework::Instance *pAntInstance, framework::Instance *pDepInstance);

	protected:
		// Wraps up info for all profile instances - strictly internal
		struct ProfileInfo
		{
			std::string registeredName;
			std::string version;
			framework::UINT16 registeredOrg;
			std::string otherRegisteredOrg;
			framework::UINT16_LIST advertiseTypes;
		};
		std::map<std::string, struct ProfileInfo> m_profileInfoMap; // Map from InstanceID to ProfileInfo
		std::vector<std::string> m_referencingProfileInstanceIDs; // profiles that reference Base Server profile

		/*
		 * Construct the internal map used to build the static instances.
		 */
		void buildProfileInfoMap();

		/*
		 * Populate attributes of an Instance based on the object path, attributes list, and m_profileInfoMap
		 */
		void buildInstanceFromProfileInfoMap(framework::Instance &instance, const framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*
		 * Populate attributes of an Instance based on the attributes list and ProfileInfo
		 */
		void buildInstanceFromProfileInfo(framework::Instance &instance, const framework::attribute_names_t &attributes,
				const struct ProfileInfo &info);

		/*!
		 * Create a default list of attributes names to retrieve.
		 * @param[in,out] attributes
		 * 		The list of attribute names to populate.
		 */
		virtual void populateAttributeList(framework::attribute_names_t &attributes) throw (framework::Exception);

		/*
		 * Determines if the two instances are associated with an ElementConformsToProfile association.
		 */
		bool isElementConformsToProfileAssociation(const framework::Instance &antecedentInstance,
				const framework::Instance &dependentInstance) throw (framework::Exception);
};

} /* namespace profile */
} /* namespace wbem */

#endif /* REGISTEREDPROFILEFACTORY_H_ */
