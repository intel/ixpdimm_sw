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
 * This file contains the provider for the SystemCapabilities instance
 * which is an internal only view class for the CLI that combines
 * MemoryCapabilities and PersistentMemoryCapabilities.
 */

#ifndef	_WBEM_SERVER_SYSTEMCAPABILITIES_FACTORY_H_
#define	_WBEM_SERVER_SYSTEMCAPABILITIES_FACTORY_H_

#include <string>

#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace server
{

static const std::string SYSTEMCAPABILITIES_CREATIONCLASSNAME = "SystemCapabilities";
// Memory mode strings
static const std::string MEMORYMODE_UNKNOWN_STR = "Unknown";
static const std::string MEMORYMODE_1LM_STR = "1LM";
static const std::string MEMORYMODE_MEMORY_STR = "Memory Mode";
static const std::string MEMORYMODE_AUTO_STR = "Auto";
static const std::string MEMORYMODE_APP_DIRECT_STR = "App Direct";
static const std::string MEMORYMODE_APP_DIRECT_DISABLED_STR = "Disabled";
static const std::string MEMORYMODE_STORAGE_STR = "Storage Mode";

// Reliability strings
static const std::string RELIABILITY_DIMM_SPARING = "DIMM Sparing";
static const std::string RELIABILITY_MEMORY_MIGRATION = "Memory Migration";

/*!
 * Represents the support data maintained by the management software
 */
class NVM_API SystemCapabilitiesFactory  : public framework_interface::NvmInstanceFactory
{
	public:
		// constructor
	SystemCapabilitiesFactory();
	~SystemCapabilitiesFactory();

	// methods required for CIM provider support, note that if
	// you don't see an expected method here the inherited version is used and
	// it throws an unsupported exception
	framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);
	framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	static framework::STR_LIST getSupportedMemoryModes(const struct nvm_capabilities &nvmCaps);
	static std::string getCurrentVolatileMode(const struct nvm_capabilities &nvmCaps);
	static std::string getCurrentAppDirectMode(const struct nvm_capabilities &nvmCaps);
	static framework::STR_LIST getSupportedSettings(const struct nvm_capabilities &nvmCaps);
	static framework::STR_LIST getRecommendedSettings(const struct nvm_capabilities &nvmCaps);
	static std::string getInterleaveSetFormatStr(const struct interleave_format &format, bool mirrorSupported);
	static framework::UINT64 getAppDirectAlignment(const struct nvm_capabilities &nvmCaps);

	static void addFormatStringIfNotInList(wbem::framework::STR_LIST &list,
			const struct interleave_format &format, bool mirrorSupported);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);
		void addCapabilitySupportedAttribute(framework::Instance *pInstance,
				framework::attribute_names_t attributes,
				std::string key, bool value);
};

} // server
} // wbem
#endif  // #ifndef _WBEM_SERVER_SYSTEMCAPABILITIES_FACTORY_H_
