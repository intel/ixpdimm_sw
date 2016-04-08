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
 * This file contains the provider for the MemoryCapabilities instances.
 */

#ifndef	_WBEM_MEMCONFIG_MEMORYCAPABILITIES_FACTORY_H_
#define	_WBEM_MEMCONFIG_MEMORYCAPABILITIES_FACTORY_H_

#include <string>
#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace mem_config
{
// object path strings
static const std::string MEMORYCAPABILITIES_ELEMENTNAME = " NVM Capabilities"; //!< Element Name static
static const std::string MEMORYCAPABILITIES_INSTANCEID = "NVM Capabilities"; //!< Instance ID static

static const std::string MEMORYCAPABILITIES_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "MemoryCapabilities"; //!< CreationClassName
static const std::string MEMORYCAPABILITIES_SYSTEMCREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "MemoryCapabilities"; //!< CreationClassName

// replication support values
static const NVM_UINT16 REPLICATION_NONE = 2;
static const NVM_UINT16 REPLICATION_LOCAL = 3;

// supported memory mode values
static const NVM_UINT16 MEMORYMODE_1LM = 2;
static const NVM_UINT16 MEMORYMODE_MEMORY = 3;
static const NVM_UINT16 MEMORYMODE_STORAGE = 4;
static const NVM_UINT16 MEMORYMODE_APP_DIRECT = 5;

// current values
static const NVM_UINT16 MEMORYMODE_UNKNOWN = 0;
static const NVM_UINT16 MEMORYMODE_AUTO = 32768;
static const NVM_UINT16 MEMORYMODE_APP_DIRECT_DISABLED = 32769;

// memory reliability values
static const NVM_UINT16 RELIABILITY_DIMMSPARING = 5;
static const NVM_UINT16 RELIABILITY_MEMORYMIGRATION = 6;

/*!
 * Represents the support data maintained by the management software
 */
class NVM_API MemoryCapabilitiesFactory  : public framework_interface::NvmInstanceFactory
{
	public:
		// constructor
	MemoryCapabilitiesFactory();
	~MemoryCapabilitiesFactory();

	// methods required for CIM provider support, note that if
	// you don't see an expected method here the inherited version is used and
	// it throws an unsupported exception
	framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);
	framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	/*!
	 * From interleave formats, harvest the recommended IMC and channel sizes.
	 * @param imcSize - value set on return
	 * @param channelSize - value set on return
	 * @return true if successful, false otherwise
	 * @throw Exception if an error occurs while getting capabilities instance
	 */
	static bool getRecommendedInterleaveSizes(interleave_size &imcSize,
			interleave_size &channelSize);

	static framework::UINT16_LIST getMemoryModes(const struct nvm_capabilities &nvmCaps);
	static framework::UINT16_LIST getReplicationSupport(const struct nvm_capabilities &nvmCaps);
	static framework::UINT16_LIST getReliabilitySupport(const struct nvm_capabilities &nvmCaps);
	static framework::UINT16 translateVolatileMode(const struct nvm_capabilities &cap);
	static framework::UINT16 translateAppDirectMode(const struct nvm_capabilities &cap);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);
};

} // mem_config
} // wbem
#endif  // #ifndef _WBEM_MEMCONFIG_MEMORYCAPABILITIES_FACTORY_H_
