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
 * This file contains the provider for the PersistentConfigurationCapabilities instances.
 */


#include "PersistentConfigurationCapabilitiesFactory.h"
#include <libinvm-cim/ExceptionBadParameter.h>

wbem::pmem_config::PersistentConfigurationCapabilitiesFactory::PersistentConfigurationCapabilitiesFactory()
throw (framework::Exception)
{

}

wbem::pmem_config::PersistentConfigurationCapabilitiesFactory::~PersistentConfigurationCapabilitiesFactory()
{

}


void wbem::pmem_config::PersistentConfigurationCapabilitiesFactory::populateAttributeList(std::vector<std::string> &attributes)
throw (framework::Exception)
{
	// keys
	attributes.push_back(INSTANCEID_KEY);

	// non-keys
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(SUPPORTEDASYNCHRONOUSOPERATIONS_KEY);
	attributes.push_back(SUPPORTEDSYNCHRONOUSOPERATIONS_KEY);
}

wbem::framework::Instance *wbem::pmem_config::PersistentConfigurationCapabilitiesFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (framework::Exception)
{
	// Inspect key attributes from object path
	// InstanceID
	if (path.getKeyValue(INSTANCEID_KEY).stringValue()
			!= PM_CAP_INSTANCEID)
	{
		throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
	}

	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);

		framework::UINT16_LIST asyncActions;
		framework::UINT16_LIST syncActions;
		syncActions.push_back(2); // AllocateFromPool
		syncActions.push_back(3); // ReturnToPool
		syncActions.push_back(4); // ModifyNamespace

		pInstance->setAttribute(ELEMENTNAME_KEY,
				framework::Attribute(PM_CAP_ELEMENTNAME, false), attributes);
		pInstance->setAttribute(SUPPORTEDASYNCHRONOUSOPERATIONS_KEY,
			framework::Attribute(asyncActions, false), attributes);
		pInstance->setAttribute(SUPPORTEDSYNCHRONOUSOPERATIONS_KEY,
			framework::Attribute(syncActions, false), attributes);
	}
	catch(framework::Exception &)
	{
		if (pInstance)
		{
			delete pInstance;
			pInstance = NULL;
		}
		throw;
	}

	return pInstance;
}

wbem::framework::instance_names_t *wbem::pmem_config::PersistentConfigurationCapabilitiesFactory::getInstanceNames()
throw (framework::Exception)
{
	wbem::framework::instance_names_t *pNames = new wbem::framework::instance_names_t ();

	framework::attributes_t keys;
	keys[INSTANCEID_KEY] = framework::Attribute(PM_CAP_INSTANCEID, true);

	pNames->push_back(framework::ObjectPath(server::getHostName(), NVM_NAMESPACE,
			PM_CAP_CLASSNAME, keys));

	return pNames;
}
