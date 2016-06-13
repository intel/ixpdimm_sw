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
 * This file contains the provider for the PerformanceMetric instances
 * which represent the current value of an NVM DIMM performance metric.
 */


#include <LogEnterExit.h>
#include <nvm_management.h>
#include <string/revision.h>
#include <uid/uid.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <physical_asset/NVDIMMFactory.h>
#include "PerformanceMetricFactory.h"
#include "PerformanceMetricDefinitionFactory.h"
#include <server/BaseServerFactory.h>
#include <sstream>
#include <utility.h>
#include <exception/NvmExceptionLibError.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <NvmStrings.h>
#include <core/device/DeviceHelper.h>

/*
 * splits a instance ID attribute into uid + decoded metric type.
 * sets deviceUid and 'type' and returns non-zero on success.
 */
bool wbem::performance::PerformanceMetricFactory::splitInstanceID(
	const framework::Attribute& instanceId, std::string &deviceUid, metric_type& metric)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool found = false;
	metric = METRIC_UNDEFINED;

	const std::string &instanceIdStr = instanceId.stringValue();
	int index = core::device::findUidStart(instanceIdStr);
	if (index >= 0)
	{
		found = true;
		deviceUid = instanceIdStr.substr(index);
		std::string metricStr = instanceIdStr.substr(0, index);
		metric = stringToMetric(metricStr);
	}
	else
	{
		COMMON_LOG_WARN_F("Could not find a uid in %s", instanceId.stringValue().c_str());
	}

	return found;
}

wbem::performance::PerformanceMetricFactory::PerformanceMetricFactory()
throw (wbem::framework::Exception)
{ }

wbem::performance::PerformanceMetricFactory::~PerformanceMetricFactory()
{ }

void wbem::performance::PerformanceMetricFactory
	::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(METRICDEFINITION_ID_KEY);
	attributes.push_back(MEASUREDELEMENTNAME_KEY);
	attributes.push_back(METRICVALUE_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::performance::PerformanceMetricFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Verify attributes
	checkAttributes(attributes);

	framework::Instance *pInstance = NULL;
	try
	{
		pInstance = new framework::Instance(path);

		framework::Attribute instanceIdAttr = path.getKeyValue(INSTANCEID_KEY);

		std::string deviceUid;
		metric_type metric;
		if (!splitInstanceID(instanceIdAttr, deviceUid, metric))
		{
			throw framework::ExceptionBadParameter(instanceIdAttr.asStr().c_str());
		}

		NVM_UID nvmUid;
		uid_copy(deviceUid.c_str(), nvmUid);

		// serialNumberStr is used in more than 1 attribute so getting here.
		std::string serialNumberStr = getDeviceSerialNumber(nvmUid);

		if (containsAttribute(wbem::ELEMENTNAME_KEY, attributes))
		{
			std::string metricName = getMetricElementNameFromType(metric);
			std::string str = metricName + " " + serialNumberStr;
			framework::Attribute a(str, false);
			(*pInstance).setAttribute(wbem::ELEMENTNAME_KEY, a, attributes);
		}

		if (containsAttribute(wbem::METRICDEFINITION_ID_KEY, attributes))
		{
			const std::string metricDefId
				= PerformanceMetricDefinitionFactory::getMetricId(metric);
			framework::Attribute a(metricDefId, false);
			(*pInstance).setAttribute(wbem::METRICDEFINITION_ID_KEY, a, attributes);
		}

		if (containsAttribute(wbem::MEASUREDELEMENTNAME_KEY, attributes))
		{
			std::string str = METRIC_DIMM_STR + serialNumberStr;
			framework::Attribute a(str, false);
			(*pInstance).setAttribute(wbem::MEASUREDELEMENTNAME_KEY, a, attributes);
		}

		if (containsAttribute(wbem::METRICVALUE_KEY, attributes))
		{
			NVM_UINT64 metricValue = getValueForDeviceMetric(nvmUid, metric);
			std::ostringstream stream;
			stream << metricValue;
			framework::Attribute a(stream.str(), false);
			(*pInstance).setAttribute(wbem::METRICVALUE_KEY, a, attributes);
		}
	}
	catch (framework::Exception) // clean up and re-throw
	{
		if (pInstance != NULL)
		{
			delete pInstance;
			pInstance = NULL;
		}
		throw;
	}
	return pInstance;
}

/*
 * Return the object paths for the PerformanceMetric class.
 */
wbem::framework::instance_names_t*	wbem::performance::PerformanceMetricFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		std::vector<std::string> manageableDevices = wbem::physical_asset::NVDIMMFactory::getManageableDeviceUids();
		std::vector<std::string>::const_iterator lastDevice = manageableDevices.end();
		std::vector<std::string>::const_iterator device = manageableDevices.begin();
		for(; device != lastDevice; ++device)
		{
			for(int metric_itr = METRIC_FIRST_TYPE; metric_itr <= METRIC_LAST_TYPE; metric_itr++)
			{
				metric_type metric = static_cast<metric_type>(metric_itr);
				framework::attributes_t keys;
				keys[INSTANCEID_KEY] =
					framework::Attribute(getInstanceIdNameFromType(metric, *device), true);
				framework::ObjectPath path(wbem::server::getHostName(), NVM_NAMESPACE,
						PERFORMANCE_METRIC_CREATIONCLASSNAME, keys);
				pNames->push_back(path);
			}
		}
	}
	catch (framework::Exception) // clean up and re-throw
	{
		if (pNames != NULL)
		{
			delete pNames;
			pNames = NULL;
		}
		throw;
	}
	return pNames;
}

/*
 * Determine if the metric value is associated based on device uid.
 */
bool wbem::performance::PerformanceMetricFactory::isAssociated(
		const std::string &associationClass,
		framework::Instance* pAntInstance,
		framework::Instance* pDepInstance)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	if (associationClass == wbem::framework_interface::ASSOCIATION_CLASS_METRIC_FOR_ME)
	{
		if (pDepInstance->getClass() == wbem::performance::PERFORMANCE_METRIC_CREATIONCLASSNAME
			&& pAntInstance->getClass() == wbem::physical_asset::NVDIMM_CREATIONCLASSNAME)
		{
			std::vector<std::string> stringsToRemove;
			stringsToRemove.push_back(wbem::performance::METRICVAL_BYTES_READ_STR);
			stringsToRemove.push_back(wbem::performance::METRICVAL_BYTES_WRITTEN_STR);
			stringsToRemove.push_back(wbem::performance::METRICVAL_HOST_READS_STR);
			stringsToRemove.push_back(wbem::performance::METRICVAL_HOST_WRITES_STR);
			stringsToRemove.push_back(wbem::performance::METRICVAL_BLOCK_READS_STR);
			stringsToRemove.push_back(wbem::performance::METRICVAL_BLOCK_WRITES_STR);
			result = framework_interface::NvmAssociationFactory::filteredFkMatch(
					pAntInstance, TAG_KEY, std::vector<std::string>(),
					pDepInstance, INSTANCEID_KEY, stringsToRemove);
		}
		else
		{
			COMMON_LOG_WARN("Incorrect antecedent and dependent class instances.");
		}
	}
	else
	{
		COMMON_LOG_WARN_F("Cannot calculate if instances are an association "
				"based on association class: %s", associationClass.c_str());
	}

	return result;
}

std::string wbem::performance::PerformanceMetricFactory::getInstanceIdNameFromType(
	metric_type metric, const std::string &suffix)
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string ret = suffix.empty() ? "" : suffix;
	switch (metric)
	{
		case METRIC_BYTES_READ :
			ret = METRICVAL_BYTES_READ_STR + suffix;
			break;
		case METRIC_BYTES_WRITTEN :
			ret = METRICVAL_BYTES_WRITTEN_STR + suffix;
			break;
		case METRIC_HOST_READS :
			ret = METRICVAL_HOST_READS_STR + suffix;
			break;
		case METRIC_HOST_WRITES :
			ret = METRICVAL_HOST_WRITES_STR + suffix;
			break;
		case METRIC_BLOCK_READS:
			ret = METRICVAL_BLOCK_READS_STR + suffix;
			break;
		case METRIC_BLOCK_WRITES:
			ret = METRICVAL_BLOCK_WRITES_STR + suffix;
			break;
		default:
		{
			std::ostringstream errormsg;
			errormsg << "LOGIC ERROR: A new metric type has been defined (" << ret;
			errormsg << ") but " << __FUNCTION__ << " " << __FILE__ << ":" << __LINE__;
			errormsg << " has not been updated!";
			throw wbem::framework::Exception(errormsg.str());
		}

	}
	return (ret);
}

wbem::performance::metric_type wbem::performance::PerformanceMetricFactory::stringToMetric(
	const std::string &str)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// preset return type to unknown
	wbem::performance::metric_type metricType = wbem::performance::METRIC_UNDEFINED;
	// test for the metric type requested and set the metric type code appropriately.
	if (str == METRICVAL_BYTES_READ_STR)
	{
		metricType = METRIC_BYTES_READ;
	}
	else if (str == METRICVAL_BYTES_WRITTEN_STR)
	{
		metricType = METRIC_BYTES_WRITTEN;
	}
	else if (str == METRICVAL_HOST_READS_STR)
	{
		metricType = METRIC_HOST_READS;
	}
	else if (str == METRICVAL_HOST_WRITES_STR)
	{
		metricType = METRIC_HOST_WRITES;
	}
	else if (str == METRICVAL_BLOCK_READS_STR)
	{
		metricType = METRIC_BLOCK_READS;
	}
	else if (str == METRICVAL_BLOCK_WRITES_STR)
	{
		metricType = METRIC_BLOCK_WRITES;
	}
	else
	{
		metricType = METRIC_UNDEFINED;
	}
	return (metricType);
}


NVM_UINT64 wbem::performance::PerformanceMetricFactory::getValueForDeviceMetric(
	const NVM_UID deviceUid, const enum metric_type metricType)
throw (framework::Exception)
{
	int rc;
	NVM_UINT64 metricValue = 0;

	struct device_performance nvmPerformance;
	if ((rc = nvm_get_device_performance(deviceUid, &nvmPerformance)) != NVM_SUCCESS)
	{
		throw wbem::exception::NvmExceptionLibError(rc);
	}
	switch (metricType)
	{
		case METRIC_BYTES_READ :
			metricValue = nvmPerformance.bytes_read;
			break;
		case METRIC_BYTES_WRITTEN :
			metricValue = nvmPerformance.bytes_written;
			break;
		case METRIC_HOST_READS :
			metricValue = nvmPerformance.host_reads;
			break;
		case METRIC_HOST_WRITES :
			metricValue = nvmPerformance.host_writes;
			break;
		case METRIC_BLOCK_READS:
			metricValue = nvmPerformance.block_reads;
			break;
		case METRIC_BLOCK_WRITES:
			metricValue = nvmPerformance.block_writes;
			break;
		default:
		{
			std::ostringstream errormsg;
			errormsg << "LOGIC ERROR: A new metric type has been defined (" << metricType;
			errormsg << ") but " << __FUNCTION__ << " " << __FILE__ << ":" << __LINE__;
			errormsg << " has not been updated!";
			throw wbem::framework::Exception(errormsg.str());
		}
	}

	return metricValue;
}

std::string wbem::performance::PerformanceMetricFactory::getMetricElementNameFromType(
	const wbem::performance::metric_type type)
throw (framework::Exception)
{
	std::string metricName = "";
	switch(type)
	{
		case METRIC_BYTES_READ:
			metricName = METRICNAME_BYTES_READ_STR;
			break;
		case METRIC_BYTES_WRITTEN:
			metricName = METRICNAME_BYTES_WRITTEN_STR;
			break;
		case METRIC_HOST_READS:
			metricName = METRICNAME_HOST_READS_STR;
			break;
		case METRIC_HOST_WRITES:
			metricName = METRICNAME_HOST_WRITES_STR;
			break;
		case METRIC_BLOCK_READS:
			metricName = METRICNAME_BLOCK_READS_STR;
			break;
		case METRIC_BLOCK_WRITES:
			metricName = METRICNAME_BLOCK_WRITES_STR;
			break;
		default:
		{
			std::ostringstream errormsg;
			errormsg << "LOGIC ERROR: A new metric type has been defined (" << type;
			errormsg << ") but " << __FUNCTION__ << " " << __FILE__ << ":" << __LINE__;
			errormsg << " has not been updated!";
			throw wbem::framework::Exception(errormsg.str());
		}
	}

	return metricName;
}

std::string wbem::performance::PerformanceMetricFactory::getDeviceSerialNumber(
	const NVM_UID uid)
throw (framework::Exception)
{
	int rc;
	struct device_discovery nvmDiscovery;
	if ((rc = nvm_get_device_discovery(uid, &nvmDiscovery)) != NVM_SUCCESS)
	{
		throw wbem::exception::NvmExceptionLibError(rc);
	}

	char serialNumberStr[NVM_SERIALSTR_LEN];
	SERIAL_NUMBER_TO_STRING(nvmDiscovery.serial_number, serialNumberStr);

	return serialNumberStr;
}
