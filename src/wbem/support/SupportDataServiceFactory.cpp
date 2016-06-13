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
 * This file contains the provider for the SupportDataService instance
 * that provides the capability to capture and export support data.
 */

#include <string.h>
#include <file_ops/file_ops_adapter.h>
#include <nvm_management.h>
#include <LogEnterExit.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include "SupportDataServiceFactory.h"
#include "OpaqueSupportDataFactory.h"
#include <server/BaseServerFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>

wbem::support::SupportDataServiceFactory::SupportDataServiceFactory()
	throw (wbem::framework::Exception)
{
	m_SaveStateProvider = nvm_save_state;
	m_GatherSupportProvider = nvm_gather_support;
	m_PurgeStateData = nvm_purge_state_data;
}

wbem::support::SupportDataServiceFactory::~SupportDataServiceFactory()
{
}

void wbem::support::SupportDataServiceFactory::populateAttributeList(
		framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(NAME_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::SupportDataServiceFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	checkPath(path);
	checkAttributes(attributes);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);

	return pInstance;
}

/*
 Return the object paths for the SupportDataService class
 */
wbem::framework::instance_names_t* wbem::support::SupportDataServiceFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string hostName = wbem::server::getHostName();
	framework::instance_names_t *pNames = new framework::instance_names_t();

	framework::attributes_t keys;
	keys.insert(std::pair<std::string, framework::Attribute>(
			SYSTEMCREATIONCLASSNAME_KEY,
			framework::Attribute(SUPPORTDATASERVICE_SYSTEMCREATIONCLASSNAME, true)));
	keys.insert(std::pair<std::string, framework::Attribute>(
			SYSTEMNAME_KEY, framework::Attribute(hostName, true)));
		keys.insert(std::pair<std::string, framework::Attribute>(
			CREATIONCLASSNAME_KEY,
			framework::Attribute(SUPPORTDATASERVICE_CREATIONCLASSNAME, true)));
	keys.insert(std::pair<std::string, framework::Attribute>(
			NAME_KEY, framework::Attribute(SUPPORTDATASERVICE_NAME, true)));

	framework::ObjectPath path(hostName, NVM_NAMESPACE,
			SUPPORTDATASERVICE_CREATIONCLASSNAME, keys);
	pNames->push_back(path);

	return pNames;
}


wbem::framework::UINT32 wbem::support::SupportDataServiceFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = framework::MOF_ERR_SUCCESS;

	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %d", method.c_str(), (int)(inParms.size()));

	try
	{
		if (method == SUPPORTDATASERVICE_EXPORTTOURI)
		{
			// verify host
			std::string hostName;
			try
			{
				hostName = wbem::server::getHostName();
			}
			// eat the exception
			catch (exception::NvmExceptionLibError&)
			{
				httpRc = framework::CIM_ERR_FAILED;
			}

			if (std::string::npos == inParms[SUPPORTDATASERVICE_OMD_OBJPATH].stringValue().find(hostName))
			{
				// not looking for this host
				httpRc = framework::CIM_ERR_INVALID_PARAMETER;
			}
			else
			{
				// call ExportToUri
				exportToUri(inParms[SUPPORTDATASERVICE_EXPORTTOURI].stringValue());
			}
		}
		else if (method == SUPPORTDATASERVICE_CREATE)
		{
			framework::ObjectPath opaqueManagementData;
			create(inParms[SUPPORTDATASERVICE_ELEMNAME].stringValue(), opaqueManagementData);
			outParms[SUPPORTDATASERVICE_OMD_OBJPATH] =
					framework::Attribute(opaqueManagementData.asString(false),false);
		}
		else if (method == SUPPORTDATASERVICE_CLEAR)
		{
			clear();
		}
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
		}
	}
	catch(wbem::framework::ExceptionBadParameter &)
	{
		httpRc = framework::CIM_ERR_INVALID_PARAMETER;
	}
	catch(exception::NvmExceptionLibError &)
	{
		wbemRc = SUPPORTDATASERVICE_ERR_UNKNOWN;
	}
	catch(framework::Exception &)
	{
		wbemRc = SUPPORTDATASERVICE_ERR_UNKNOWN;
	}

	COMMON_LOG_EXIT_RETURN("httpRc: %u, wbemRc: %u", httpRc, wbemRc);
	return httpRc;
}

// ------------------------------------------------------------------------------------------------
// EXTRINSIC METHODS
// Note: these extrinsic methods deviate from the CIM schema in that they throw exceptions instead
//		 of returning error codes.  The out parameters are returned instead.
// ------------------------------------------------------------------------------------------------

/*
 * Create a snapshot
 * elementName is optional name used in the db to describe this snapshot
 */
void wbem::support::SupportDataServiceFactory::create(const std::string elementName,
		const framework::ObjectPath& opaqueManagementData)
const throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;
	if ((rc = m_SaveStateProvider(elementName.c_str(), elementName.length())) != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}

	OpaqueSupportDataFactory opaqueFactory;
}

// Save history to file
void wbem::support::SupportDataServiceFactory::exportToUri(const std::string exportUri)
const throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	COMMON_PATH absPath;
	if (get_absolute_path(exportUri.c_str(), exportUri.length() + 1, absPath) != COMMON_SUCCESS)
	{
		throw framework::ExceptionBadParameter(exportUri.c_str());
	}

	int rc;
	if ((rc = m_GatherSupportProvider(absPath, strlen(absPath))) != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

void wbem::support::SupportDataServiceFactory::clear()
throw (framework::Exception)
{
	int rc;
	if ((rc = m_PurgeStateData()) != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}
