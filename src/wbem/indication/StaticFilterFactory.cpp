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

#include <server/BaseServerFactory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <LogEnterExit.h>
#include "StaticFilterFactory.h"

wbem::indication::StaticFilterFactory::StaticFilterFactory()
{
	m_nameQueryMap["SensorChanged"] =
		"SELECT * FROM CIM_InstModification WHERE SourceInstance ISA CIM_Sensor";
	m_nameQueryMap["NamespaceCreated"] =
		"SELECT * FROM CIM_InstCreation WHERE SourceInstance ISA CIM_PersistentMemoryNamespace";
	m_nameQueryMap["NamespaceDeleted"] =
		"SELECT * FROM CIM_InstDeletion WHERE SourceInstance ISA CIM_PersistentMemoryNamespace";
	m_nameQueryMap["DeviceCreated"] =
		"SELECT * FROM CIM_InstCreation WHERE SourceInstance ISA CIM_PhysicalMemory";
	m_nameQueryMap["DeviceDeleted"] =
		"SELECT * FROM CIM_InstDeletion WHERE SourceInstance ISA CIM_PhysicalMemory";
	m_nameQueryMap["AllAlerts"] =
		"SELECT * FROM CIM_AlertIndication";
}


void wbem::indication::StaticFilterFactory::populateAttributeList(
	wbem::framework::attribute_names_t &attributes) throw (framework::Exception)
{
	// Key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(NAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);

	// Non-Key attributes
	attributes.push_back(QUERY_KEY);
	attributes.push_back(QUERYLANGUAGE_KEY);
	attributes.push_back(SOURCENAMESPACE_KEY);
	attributes.push_back(INDIVIDUALSUBSCRIPTIONSUPPORTED_KEY);

}


wbem::framework::Instance *wbem::indication::StaticFilterFactory::getInstance(
	wbem::framework::ObjectPath &path, wbem::framework::attribute_names_t &attributes)
throw (framework::Exception)

{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	checkAttributes(attributes);

	std::string hostname = server::getHostName();
	path.checkKey(SYSTEMCREATIONCLASSNAME_KEY, server::BASESERVER_CREATIONCLASSNAME);
	path.checkKey(CREATIONCLASSNAME_KEY, STATICFILTER_CREATIONCLASSNAME);
	path.checkKey(SYSTEMNAME_KEY, hostname);

	std::string name = path.getKeyValue(NAME_KEY).stringValue();
	if (m_nameQueryMap.find(name) == m_nameQueryMap.end())
	{
		throw framework::ExceptionBadParameter("path");
	}

	std::string query = m_nameQueryMap[name];
	framework::Instance *instance = new framework::Instance(path);

	if (containsAttribute(QUERY_KEY, attributes))
	{
		instance->setAttribute(QUERY_KEY, framework::Attribute(query, false));
	}

	if (containsAttribute(SOURCENAMESPACE_KEY, attributes))
	{
		instance->setAttribute(SOURCENAMESPACE_KEY,
			framework::Attribute(NVM_NAMESPACE, false));
	}

	if (containsAttribute(QUERYLANGUAGE_KEY, attributes))
	{
		instance->setAttribute(QUERYLANGUAGE_KEY,
			framework::Attribute(STATICFILTER_QUERYLANGUAGE, false));
	}

	if (containsAttribute(INDIVIDUALSUBSCRIPTIONSUPPORTED_KEY, attributes))
	{
		instance->setAttribute(INDIVIDUALSUBSCRIPTIONSUPPORTED_KEY,
			framework::Attribute(true, false));
	}

	return instance;
}

wbem::framework::instance_names_t *wbem::indication::StaticFilterFactory::getInstanceNames()
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::instance_names_t *const instanceNames = new framework::instance_names_t();

	std::string hostname = server::getHostName();
	std::map<std::string, std::string>::iterator i;
	for (i = m_nameQueryMap.begin(); i != m_nameQueryMap.end(); i++)
	{
		std::string name = i->first;
		framework::attributes_t keys;
		keys[SYSTEMCREATIONCLASSNAME_KEY] =
			framework::Attribute(server::BASESERVER_CREATIONCLASSNAME, true);
		keys[CREATIONCLASSNAME_KEY] =
			framework::Attribute(STATICFILTER_CREATIONCLASSNAME, true);
		keys[NAME_KEY] = framework::Attribute(name, true);
		keys[SYSTEMNAME_KEY] = framework::Attribute(hostname, true);

		framework::ObjectPath path(hostname,
			NVM_NAMESPACE, STATICFILTER_CREATIONCLASSNAME, keys);

		instanceNames->push_back(path);
	}

	return instanceNames;
}


