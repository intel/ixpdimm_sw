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
 * This file contains the provider for the NVDIMMDiagnostic instances
 * which represent an diagnostic test.
 */

#include <string.h>
#include <nvm_management.h>
#include <LogEnterExit.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/ObjectPathBuilder.h>
#include <libinvm-cim/CimXml.h>
#include <uid/uid.h>
#include "NVDIMMDiagnosticFactory.h"
#include "DiagnosticIdentityFactory.h"
#include <server/BaseServerFactory.h>
#include <exception/NvmExceptionLibError.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include <software/ElementSoftwareIdentityFactory.h>

wbem::support::NVDIMMDiagnosticFactory::NVDIMMDiagnosticFactory()
	throw (wbem::framework::Exception)
{
	m_RunDiagProvider = nvm_run_diagnostic;
}

wbem::support::NVDIMMDiagnosticFactory::~NVDIMMDiagnosticFactory()
{
}

void wbem::support::NVDIMMDiagnosticFactory::populateAttributeList(
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
wbem::framework::Instance* wbem::support::NVDIMMDiagnosticFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// get the host server name
	const std::string& hostName = wbem::server::getHostName();

	// verify path is correct before creating instance
	std::string systemCreationClassName = path.getKeyValue(SYSTEMCREATIONCLASSNAME_KEY).stringValue();
	if (systemCreationClassName != wbem::server::BASESERVER_CREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(SYSTEMCREATIONCLASSNAME_KEY.c_str());
	}
	std::string systemName = path.getKeyValue(SYSTEMNAME_KEY).stringValue();
	if (systemName != std::string(hostName))
	{
		throw framework::ExceptionBadParameter(SYSTEMNAME_KEY.c_str());
	}
	std::string creationClass = path.getKeyValue(CREATIONCLASSNAME_KEY).stringValue();
	if (creationClass != NVDIMMDIAGNOSTIC_CREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(CREATIONCLASSNAME_KEY.c_str());
	}
	// throw bad parameter if not one of the 5 defined test types
	std::string name = path.getKeyValue(NAME_KEY).stringValue();
	if (!testTypeValid(name))
	{
			throw framework::ExceptionBadParameter(NAME_KEY.c_str());
	}

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);

	return pInstance;
}

/*
 Return the object paths for the SupportDataService class
 There should be 5 possible instances, one for each type of diagnostic test.
 */
wbem::framework::instance_names_t* wbem::support::NVDIMMDiagnosticFactory::getInstanceNames()
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::instance_names_t *pNames = new framework::instance_names_t();

	// get the host server name
	const std::string& hostName = wbem::server::getHostName();

	int numTests = 5;
	for (int i = 0; i < numTests; i++)
	{
		framework::attributes_t keys;

		// add non-varying keys
		keys[SYSTEMCREATIONCLASSNAME_KEY] =
				framework::Attribute(wbem::server::BASESERVER_CREATIONCLASSNAME, true);
		keys[SYSTEMNAME_KEY] =
				framework::Attribute(hostName, true);
		keys[CREATIONCLASSNAME_KEY] =
				framework::Attribute(NVDIMMDIAGNOSTIC_CREATIONCLASSNAME, true);

		// add name keys for each test type
		switch (i)
		{
		case 0:
			// add health check test
			keys[NAME_KEY] =
					framework::Attribute(NVDIMMDIAGNOSTIC_TEST_QUICK, true);
			break;
		case 1:
			// add platform test
			keys[NAME_KEY] =
					framework::Attribute(NVDIMMDIAGNOSTIC_TEST_PLATFORM, true);
			break;
		case 2:
			// add storage test
			keys[NAME_KEY] =
					framework::Attribute(NVDIMMDIAGNOSTIC_TEST_STORAGE, true);
			break;
		case 3:
			// add security test
			keys[NAME_KEY] =
					framework::Attribute(NVDIMMDIAGNOSTIC_TEST_SECURITY, true);
			break;
		case 4:
			// add settings test
			keys[NAME_KEY] =
					framework::Attribute(NVDIMMDIAGNOSTIC_TEST_SETTING, true);
			break;
		}
		// Remove if once all are implemented above.  This just keeps names from being added
		// on the tests above that are not yet implemented, cases 1-4
		if (keys[NAME_KEY].stringValue().size() > 0)
		{
			framework::ObjectPath path(hostName, NVM_NAMESPACE,
					NVDIMMDIAGNOSTIC_CREATIONCLASSNAME, keys);
			pNames->push_back(path);
		}
	}

	return pNames;
}

wbem::framework::UINT32 wbem::support::NVDIMMDiagnosticFactory::executeMethod(
		wbem::framework::UINT32 &wbemRc,
		const std::string method,
		wbem::framework::ObjectPath &object,
		wbem::framework::attributes_t &inParms,
		wbem::framework::attributes_t &outParms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::UINT32 httpRc = framework::MOF_ERR_SUCCESS;
	wbemRc = framework::MOF_ERR_SUCCESS;

	COMMON_LOG_ENTRY_PARAMS("methodName: %s, number of in params: %d", method.c_str(), (int)(inParms.size()));

	try
	{

		if (method == NVDIMMDIAGNOSTIC_RUNDIAGNOSTICSERVICE)
		{
			validateObjectHostName(object);

			// which test?
			std::string testType = object.getKeyValue(NAME_KEY).stringValue();
			framework::UINT16_LIST ignoreResults = getDiagnosticIgnoreList(inParms);

			// fill uid
			COMMON_UID uid;
			getUidFromManagedElement(inParms, testType, uid);

			RunDiagnosticService(uid, ignoreResults, testType);
		}	// if NVDIMMDIAGNOSTIC_RUNDIAGNOSTICSERVICE
		else
		{
			httpRc = framework::CIM_ERR_METHOD_NOT_AVAILABLE;
		}

	} // try
	catch(wbem::framework::ExceptionBadParameter &)
	{
		wbemRc = NVDIMMDIAGNOSTIC_ERR_INVALID_PARAMETER;
	}
	catch(exception::NvmExceptionLibError &e)
	{
		if (e.getLibError() == NVM_ERR_NOTSUPPORTED)
		{
			wbemRc = NVDIMMDIAGNOSTIC_ERR_NOT_SUPPORTED;
		}
		else
		{
			wbemRc = NVDIMMDIAGNOSTIC_ERR_FAILED;
		}
	}
	catch(framework::Exception &)
	{
		wbemRc = NVDIMMDIAGNOSTIC_ERR_FAILED;
	}

	COMMON_LOG_EXIT_RETURN("httpRc: %u, wbemRc: %u", httpRc, wbemRc);
	return httpRc;
}

// ------------------------------------------------------------------------------------------------
// EXTRINSIC METHODS
// Note: these extrinsic methods deviate from the CIM schema in that they throw exceptions instead
//		 of returning error codes.  The out parameters are returned instead.
// ------------------------------------------------------------------------------------------------

void wbem::support::NVDIMMDiagnosticFactory::RunDiagnosticService(NVM_UID device_uid,
		framework::UINT16_LIST ignoreList, std::string testType)
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct diagnostic diags = getDiagnosticStructure(testType, ignoreList);

	int rc = NVM_SUCCESS;
	NVM_UINT32 results = 0;
	if ((rc = m_RunDiagProvider(device_uid,
			&diags, &results)) != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}
}

bool wbem::support::NVDIMMDiagnosticFactory::testTypeValid(std::string testType)
{
	bool rc = true;

	if ((testType.compare(NVDIMMDIAGNOSTIC_TEST_QUICK) != 0) &&
		(testType.compare(NVDIMMDIAGNOSTIC_TEST_PLATFORM) != 0) &&
		(testType.compare(NVDIMMDIAGNOSTIC_TEST_STORAGE) != 0) &&
		(testType.compare(NVDIMMDIAGNOSTIC_TEST_SECURITY) != 0) &&
		(testType.compare(NVDIMMDIAGNOSTIC_TEST_SETTING) != 0))
	{
		rc = false;
	}

	return rc;
}

bool wbem::support::NVDIMMDiagnosticFactory::isAssociated(const std::string &associationClass,
	framework::Instance *pAntInstance, framework::Instance *pDepInstance)
{
	bool result = true;

	if (associationClass == software::ELEMENTSOFTWAREIDENTITY_CREATIONCLASSNAME &&
			pAntInstance->getClass() == DIAGNOSTICIDENTITY_CREATIONCLASSNAME &&
			pDepInstance->getClass() == NVDIMMDIAGNOSTIC_CREATIONCLASSNAME)
	{
		// ElementSoftwareIdentity between DiagnosticIdentity and NVDIMMDiagnostic
		//	- DiagnosticIdentity.InstanceID (minus host name prefix) should match
		//		NVDIMMDiagnostic.Name
		framework::Attribute depSystemNameAttribute;

		if (pDepInstance->getAttribute(SYSTEMNAME_KEY, depSystemNameAttribute)
				== framework::SUCCESS)
		{
			std::string systemName = depSystemNameAttribute.stringValue();
			result = framework_interface::NvmAssociationFactory::filteredFkMatch(
						pAntInstance, INSTANCEID_KEY, systemName,
						pDepInstance, NAME_KEY, "");
		}
		else
		{
			result = false;
		}
	}


	return result;
}

struct diagnostic wbem::support::NVDIMMDiagnosticFactory::getDiagnosticStructure(
		const std::string& testType,
		const framework::UINT16_LIST &ignoreList)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	struct diagnostic diags;
	memset(&diags, 0, sizeof (diags));

	if	(testType.compare(NVDIMMDIAGNOSTIC_TEST_QUICK) == 0)
	{
		diags.test = DIAG_TYPE_QUICK;

		for (size_t i = 0; i < ignoreList.size(); i++)
		{
			switch (ignoreList[i])
			{
			case HC_IGNORE_HEALTHSTATE:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_HEALTH;
				break;
			case HC_IGNORE_TEMPERATURE:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_MEDIA_TEMP |
					(NVM_UINT64)DIAG_THRESHOLD_QUICK_CONTROLLER_TEMP;
				break;
			case HC_IGNORE_SPARE:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_AVAIL_SPARE;
				break;
			case HC_IGNORE_WEAR:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_PERC_USED;
				break;
			case HC_IGNORE_UNCORRECTABLE:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_UNCORRECT_ERRORS;
				break;
			case HC_IGNORE_CORRECTABLE:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_CORRECTED_ERRORS;
				break;
			case HC_IGNORE_ERASURE_CODE_CORRECTABLE:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_ERASURE_CODED_CORRECTED_ERRORS;
				break;
			case HC_IGNORE_VENDORID:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_VALID_VENDOR_ID;
				break;
			case HC_IGNORE_MANUFACTURER:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_VALID_MANUFACTURER;
				break;
			case HC_IGNORE_MODELNUMBER:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_QUICK_VALID_MODEL_NUMBER;
				break;
			default:
				COMMON_LOG_ERROR_F("Settings Ignore value %d is invalid for test %s",
						ignoreList[i], NVDIMMDIAGNOSTIC_TEST_QUICK.c_str());
			}
		} // walk through ignore list
	} // is quick test
	else if (testType.compare(NVDIMMDIAGNOSTIC_TEST_SECURITY) == 0)
	{
		diags.test = DIAG_TYPE_SECURITY;
		for (size_t i = 0; i < ignoreList.size(); i++)
		{
			switch (ignoreList[i])
			{
			case SEC_IGNORE_SECDISABLED:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_SECURITY_ALL_DISABLED;
				break;
			case SEC_IGNORE_SECCONSISTENT:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_SECURITY_CONSISTENT;
				break;
			default:
				COMMON_LOG_ERROR_F("Settings Ignore value %d is invalid for test %s",
						ignoreList[i], NVDIMMDIAGNOSTIC_TEST_SECURITY.c_str());
			}
		}
	} // is security check test
	else if (testType.compare(NVDIMMDIAGNOSTIC_TEST_SETTING) == 0)
	{
		diags.test = DIAG_TYPE_FW_CONSISTENCY;
		for (size_t i = 0; i < ignoreList.size(); i++)
		{
			switch (ignoreList[i])
			{
			case SET_IGNORE_FWCONSISTENT:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_FW_CONSISTENT;
				break;
			case SET_IGNORE_TEMPMEDIATHRESHOLD:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_FW_MEDIA_TEMP;
				break;
			case SET_IGNORE_TEMPCONTROLLERTHRESHOLD:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_FW_CORE_TEMP;
				break;
			case SET_IGNORE_SPARETHRESHOLD:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_FW_SPARE;
				break;
			case SET_IGNORE_POW_MGMT_POLICIES:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_FW_PEAK_POW_BUDGET_MIN
						| (NVM_UINT64)DIAG_THRESHOLD_FW_PEAK_POW_BUDGET_MAX
						| (NVM_UINT64)DIAG_THRESHOLD_FW_AVG_POW_BUDGET_MIN
						| (NVM_UINT64)DIAG_THRESHOLD_FW_PEAK_POW_BUDGET_MAX;
				break;
			case SET_IGNORE_DIE_SPARING_POLICIES:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_FW_DIE_SPARING_POLICY
						| DIAG_THRESHOLD_FW_DIE_SPARING_LEVEL;
				break;
			case SET_IGNORE_TIME:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_FW_TIME;
				break;
			case SET_IGNORE_DEBUGLOG:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_FW_DEBUGLOG;
				break;
			default:
				COMMON_LOG_ERROR_F("Settings Ignore value %d is invalid for test %s",
						ignoreList[i], NVDIMMDIAGNOSTIC_TEST_SETTING.c_str());
			}
		}
	} // is firmware consistency check and settings test
	else if (testType.compare(NVDIMMDIAGNOSTIC_TEST_PLATFORM) == 0)
	{
		diags.test = DIAG_TYPE_PLATFORM_CONFIG;
		for (size_t i = 0; i < ignoreList.size(); i++)
		{
			switch (ignoreList[i])
			{
			case PF_IGNORE_NFITHEADER:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_NFIT;
				break;
			case PF_IGNORE_CAPABILITYTABLE:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_PCAT;
				break;
			case PF_IGNORE_CONFIGDATA:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_PCD
					| (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_CURRENT_PCD
					| (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_UNCONFIGURED
					| (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_BROKEN_ISET
					| (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_MAPPED_CAPACITY;
				break;
			case PF_IGNORE_CURRENTCONFIG:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_CURRENT_PCD
					| (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_UNCONFIGURED
					| (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_BROKEN_ISET
					| (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_MAPPED_CAPACITY;
				break;
			case PF_IGNORE_DIMMSCONFIGURED:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_UNCONFIGURED;
				break;
			case PF_IGNORE_CONFIGERR:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_BROKEN_ISET;
				break;
			case PF_IGNORE_SPA:
				diags.excludes |= (NVM_UINT64)DIAG_THRESHOLD_PCONFIG_MAPPED_CAPACITY;
				break;
			default:
				COMMON_LOG_ERROR_F("Settings Ignore value %d is invalid for test %s",
						ignoreList[i], NVDIMMDIAGNOSTIC_TEST_PLATFORM.c_str());
			}
		}
	}
	else if	(testType.compare(NVDIMMDIAGNOSTIC_TEST_STORAGE) == 0)
	{
		diags.test = DIAG_TYPE_PM_META;
	}
	else
	{
		throw framework::ExceptionBadParameter(NAME_KEY.c_str());
	}

	return diags;
}

void wbem::support::NVDIMMDiagnosticFactory::validateObjectHostName(
		const wbem::framework::ObjectPath& object)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	const std::string& hostName = wbem::server::getHostName();
	std::string expectedHostName = object.getKeyValue(SYSTEMNAME_KEY).stringValue();

	if (hostName.compare(expectedHostName) != 0)
	{
		// not looking for this host
		throw framework::ExceptionBadParameter(SYSTEMNAME_KEY.c_str());
	}
}

wbem::framework::UINT16_LIST wbem::support::NVDIMMDiagnosticFactory::getDiagnosticIgnoreList(
		wbem::framework::attributes_t& inParms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string settingsString = inParms[NVDIMMDIAGNOSTIC_SETTINGS].stringValue();

	framework::CimXml settingsInstance(settingsString);
	if (settingsInstance.getClass() != NVDIMMDIAGNOSTICINPUT_CREATIONCLASSNAME)
	{
		COMMON_LOG_ERROR_F("Settings XML is the wrong WBEM class: %s, expected: %s",
				settingsInstance.getClass().c_str(),
				NVDIMMDIAGNOSTICINPUT_CREATIONCLASSNAME.c_str());
		throw framework::ExceptionBadParameter(NVDIMMDIAGNOSTIC_SETTINGS.c_str());
	}

	// get the attributes & check test type
	framework::attributes_t settingsAttrs = settingsInstance.getProperties();
	framework::attributes_t::iterator testStrIter = settingsAttrs.find(NVDIMMDIAGNOSTIC_IGNORE_KEY);
	framework::UINT16_LIST ignoreResults;
	if (testStrIter != settingsAttrs.end())
	{
		ignoreResults = testStrIter->second.uint16ListValue();
	}

	return ignoreResults;
}

void wbem::support::NVDIMMDiagnosticFactory::getUidFromManagedElement(
		wbem::framework::attributes_t& inParms, const std::string &testType, NVM_UID uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::string managedElementRef = inParms[wbem::MANAGEDELEMENT_KEY].stringValue();

	// Only HealthCheck requires an NVDIMM
	if (testType == NVDIMMDIAGNOSTIC_TEST_QUICK)
	{
		// check for uid
		framework::ObjectPath managedElementPath = validateManagedElementObjectPath(managedElementRef,
				physical_asset::NVDIMM_CREATIONCLASSNAME);
		uid_copy(managedElementPath.getKeyValue(TAG_KEY).stringValue().c_str(), uid);
	}
	else // other checks require either NULL or BaseServer
	{
		if (!managedElementRef.empty())
		{
			validateManagedElementObjectPath(managedElementRef, server::BASESERVER_CREATIONCLASSNAME);
		}

		memset(uid, 0, NVM_MAX_UID_LEN);
	}
}

wbem::framework::ObjectPath wbem::support::NVDIMMDiagnosticFactory::validateManagedElementObjectPath(
		const std::string& refPath,
		const std::string className)
{
	framework::ObjectPathBuilder pathBuilder(refPath);
	framework::ObjectPath refObjectPath;

	if (!pathBuilder.Build(&refObjectPath))
	{
		COMMON_LOG_ERROR_F("parameter '%s' was not a valid object path: %s",
				wbem::MANAGEDELEMENT_KEY.c_str(),
				refPath.c_str());
		throw framework::ExceptionBadParameter(wbem::MANAGEDELEMENT_KEY.c_str());
	}
	else if (refObjectPath.getClass() != className)
	{
		COMMON_LOG_ERROR_F("parameter '%s' was not a %s ref: %s",
				wbem::MANAGEDELEMENT_KEY.c_str(),
				className.c_str(),
				refPath.c_str());
		throw framework::ExceptionBadParameter(wbem::MANAGEDELEMENT_KEY.c_str());
	}

	return refObjectPath;
}
