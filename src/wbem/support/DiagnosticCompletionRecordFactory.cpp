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
 * This file contains the provider for the DiagnosticCompletionRecord instances
 * which model an individual diagnostic test result.
 */

#include <sstream>
#include <uid/uid.h>
#include "DiagnosticLogFactory.h"
#include "DiagnosticCompletionRecordFactory.h"
#include "NVDIMMDiagnosticFactory.h"
#include <LogEnterExit.h>
#include <server/BaseServerFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include <software/NVDIMMCollectionFactory.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <NvmStrings.h>

wbem::support::DiagnosticCompletionRecordFactory::DiagnosticCompletionRecordFactory()
throw (wbem::framework::Exception)
{ }

wbem::support::DiagnosticCompletionRecordFactory::~DiagnosticCompletionRecordFactory()
{ }


void wbem::support::DiagnosticCompletionRecordFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(SERVICENAME_KEY);
	attributes.push_back(MANAGEDELEMENTNAME_KEY);
	attributes.push_back(CREATIONTIMESTAMP_KEY);
	attributes.push_back(ERRORCODE_KEY);
	attributes.push_back(COMPLETIONSTATE_KEY);

}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::DiagnosticCompletionRecordFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{
		checkAttributes(attributes);
		std::string hostName = wbem::server::getHostName();

		// gather results
		bool diagFound = false;
		wbem::support::diagnosticResults_t results;
		DiagnosticLogFactory::gatherDiagnosticResults(&results);

		// match the instance ID
		framework::Attribute instanceID = path.getKeyValue(INSTANCEID_KEY);
		for (diagnosticResults_t::iterator iter = results.begin(); iter != results.end(); iter++)
		{
			struct diagnosticResult diag = *iter;
			std::stringstream instanceIdStr;
			instanceIdStr << DIAGNOSTICCOMPLETION_INSTANCEID << diag.id;
			if (instanceID.stringValue() == instanceIdStr.str())
			{
				diagFound = true;

				// ServiceName = Diagnostic Test Name
				if (containsAttribute(SERVICENAME_KEY, attributes))
				{
					std::string testName;
					switch (diag.type)
					{
						case EVENT_TYPE_DIAG_QUICK:
							testName = NVDIMMDIAGNOSTIC_TEST_QUICK;
							break;
						case EVENT_TYPE_DIAG_PLATFORM_CONFIG:
							testName = NVDIMMDIAGNOSTIC_TEST_PLATFORM;
							break;
						case EVENT_TYPE_DIAG_PM_META:
							testName = NVDIMMDIAGNOSTIC_TEST_STORAGE;
							break;
						case EVENT_TYPE_DIAG_SECURITY:
							testName = NVDIMMDIAGNOSTIC_TEST_SECURITY;
							break;
						case EVENT_TYPE_DIAG_FW_CONSISTENCY:
							testName = NVDIMMDIAGNOSTIC_TEST_SETTING;
							break;
						default:
							testName = "Unknown";
							break;
					}
					framework::Attribute serviceNameAttr(testName, false);
					pInstance->setAttribute(SERVICENAME_KEY, serviceNameAttr, attributes);
				}

				// ManagedElementName - Intel NVDIMM + UUID or DIMM Collection for + hostname
				if (containsAttribute(MANAGEDELEMENTNAME_KEY, attributes))
				{
					std::string elementName = "";
					if (strlen(diag.device_uid) > 0)
					{
						elementName = wbem::physical_asset::NVDIMM_ELEMENTNAME_PREFIX + diag.device_uid;
					}
					else
					{
						elementName = wbem::software::NVDIMMCOLLECTION_ELEMENTNAME_PREFIX + hostName;
					}
					framework::Attribute dimmAttr(elementName, false);
					pInstance->setAttribute(MANAGEDELEMENTNAME_KEY, dimmAttr, attributes);
				}

				// CreationTimeStamp - record time stamp
				if (containsAttribute(CREATIONTIMESTAMP_KEY, attributes))
				{
					framework::Attribute timeAttr(diag.time,
									wbem::framework::DATETIME_SUBTYPE_DATETIME, false);
					pInstance->setAttribute(CREATIONTIMESTAMP_KEY, timeAttr, attributes);
				}

				// ErrorCode - Array of diagnostic result messages
				if (containsAttribute(ERRORCODE_KEY, attributes))
				{
					framework::Attribute msgsAttr(diag.messages, false);
					pInstance->setAttribute(ERRORCODE_KEY, msgsAttr, attributes);
				}

				// CompletionState - Combined result of diagnostic
				if (containsAttribute(COMPLETIONSTATE_KEY, attributes))
				{
					std::string stateStr;
					switch (diag.result)
					{
						case DIAGNOSTIC_RESULT_OK:
							stateStr = "OK";
							break;
						case DIAGNOSTIC_RESULT_WARNING:
							stateStr = "Warning";
							break;
						case DIAGNOSTIC_RESULT_FAILED:
							stateStr = "Failed";
							break;
						case DIAGNOSTIC_RESULT_ABORTED:
							stateStr = "Aborted";
							break;
						case DIAGNOSTIC_RESULT_UNKNOWN:
						default:
							stateStr = "Unknown";
							break;
					}
					framework::Attribute stateAttr((NVM_UINT16)diag.result, stateStr, false);
					pInstance->setAttribute(COMPLETIONSTATE_KEY, stateAttr, attributes);
				}

				break;
			} // end record found
		} // for

		if (!diagFound)
		{
			throw framework::ExceptionBadParameter(INSTANCEID_KEY.c_str());
		}
	}
	catch (framework::Exception) // clean up and re-throw
	{
		delete pInstance;
		throw;
	}

	return pInstance;
}

/*
 * Return the object path for the single instance for this server
 */
wbem::framework::instance_names_t* wbem::support::DiagnosticCompletionRecordFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		// get the host server name
		std::string hostName = wbem::server::getHostName();
		framework::attributes_t keys;

		// get the results
		diagnosticResults_t results;
		DiagnosticLogFactory::gatherDiagnosticResults(&results);
		for (diagnosticResults_t::iterator iter = results.begin(); iter != results.end(); iter++)
		{
			// Instance ID = "NVDIMM Diagnostic Results" + id
			std::stringstream instanceIdStr;
			instanceIdStr << DIAGNOSTICCOMPLETION_INSTANCEID << (*iter).id;
			keys[INSTANCEID_KEY] =
					framework::Attribute(instanceIdStr.str(), true);

			// create the object path
			framework::ObjectPath path(hostName, NVM_NAMESPACE,
					DIAGNOSTICCOMPLETION_CREATIONCLASSNAME, keys);
			pNames->push_back(path);
		}
	}
	catch (framework::Exception) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	return pNames;
}
