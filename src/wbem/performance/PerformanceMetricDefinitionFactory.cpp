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
 * This file contains the provider for the PerformanceMetricDefinition instances
 * which define an NVM DIMM performance metric.
 */

#include <LogEnterExit.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include "PerformanceMetricDefinitionFactory.h"
#include <server/BaseServerFactory.h>
#include <sstream>

wbem::performance::PerformanceMetricDefinitionFactory
::PerformanceMetricDefinitionFactory()
throw (wbem::framework::Exception)
{ }

wbem::performance::PerformanceMetricDefinitionFactory
::~PerformanceMetricDefinitionFactory()
{ }

wbem::performance::metric_type wbem::performance::PerformanceMetricDefinitionFactory
::metricTypeFromObjectPath(const framework::ObjectPath& path)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// preset return type to unknown
	enum metric_type metricType = METRIC_UNDEFINED;
	const std::string& hostName = wbem::server::getHostName();
	// extract the host name + requested metric type from the nvm path.
	const std::string id_key_value = path.getKeyValue(wbem::ID_KEY).asStr();
	if (std::string::npos == id_key_value.find(hostName))
	{
		throw framework::ExceptionBadParameter(id_key_value.c_str());
	}
	// remove the host name prefix
	std::string metric_type_str = id_key_value.substr(hostName.length());
	metricType = metricTypeFromStr(metric_type_str);
	if (metricType == METRIC_UNDEFINED)
	{
		throw framework::ExceptionBadParameter(id_key_value.c_str());
	}
	return (metricType);
}

void wbem::performance::PerformanceMetricDefinitionFactory
	::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// add key attributes
	attributes.push_back(ID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(NAME_KEY);
	attributes.push_back(DATATYPE_KEY);
	attributes.push_back(UNITS_KEY);
	attributes.push_back(ISCONTINUOUS_KEY);
	attributes.push_back(TIMESCOPE_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::performance::PerformanceMetricDefinitionFactory
::getInstance(framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::Instance *pInstance = NULL;
	try
	{
		// Verify attributes
		checkAttributes(attributes);
		metric_type metric = metricTypeFromObjectPath(path);
		pInstance = new framework::Instance(path);
		if (containsAttribute(wbem::ID_KEY, attributes))
		{
			framework::Attribute a(getMetricId(metric), true);
			pInstance->setAttribute(wbem::ID_KEY, a, attributes);
		}
		if (containsAttribute(wbem::ELEMENTNAME_KEY, attributes))
		{
			framework::Attribute a(getMetricElementName(metric), false);
			pInstance->setAttribute(wbem::ELEMENTNAME_KEY, a, attributes);
		}
		if (containsAttribute(wbem::NAME_KEY, attributes))
		{
			framework::Attribute a(getMetricName(metric), false);
			pInstance->setAttribute(wbem::NAME_KEY, a, attributes);
		}
		if (containsAttribute(wbem::DATATYPE_KEY, attributes))
		{
			framework::Attribute a(getMetricDataType(metric), false);
			pInstance->setAttribute(wbem::DATATYPE_KEY, a, attributes);
		}
		if (containsAttribute(wbem::UNITS_KEY, attributes))
		{
			framework::Attribute a(getMetricUnits(metric), false);
			pInstance->setAttribute(wbem::UNITS_KEY, a, attributes);
		}
		if (containsAttribute(wbem::ISCONTINUOUS_KEY, attributes))
		{
			framework::Attribute a(getMetricIsContinuous(metric), false);
			pInstance->setAttribute(wbem::ISCONTINUOUS_KEY, a, attributes);
		}
		if (containsAttribute(wbem::TIMESCOPE_KEY, attributes))
		{
			framework::Attribute a(getMetricTimeScope(metric), false);
			pInstance->setAttribute(wbem::TIMESCOPE_KEY, a, attributes);
		}
	}
	catch (framework::Exception) // clean up and re-throw
	{
		if (NULL != pInstance)
		{
			delete pInstance;
			pInstance = NULL;
		}
		throw;
	}
	return pInstance;
}

/*
 * Return the object paths for the PerformanceMetricDefinitionFactory class.
 */
wbem::framework::instance_names_t* wbem::performance::PerformanceMetricDefinitionFactory
::getInstanceNames() throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		for(int metric_itr = METRIC_FIRST_TYPE; metric_itr <= METRIC_LAST_TYPE; metric_itr++)
		{
			metric_type metric = static_cast<metric_type>(metric_itr);
			framework::attributes_t keys;
			keys[ID_KEY] = framework::Attribute(getMetricId(metric), true);

			framework::ObjectPath path(wbem::server::getHostName(), NVM_NAMESPACE,
					PERFORMANCEMETRICDEFINITION_CREATIONCLASSNAME, keys);
			if (pNames != NULL)
			{
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


const std::string wbem::performance::PerformanceMetricDefinitionFactory
::metricTypeToStr(metric_type metric, const std::string& prefix)
throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::string ret = prefix.empty() ? "" : prefix;
	switch (metric)
	{
		case METRIC_BYTES_READ :
			ret += METRICDEF_BYTES_READ_STR;
			break;
		case METRIC_BYTES_WRITTEN :
			ret += METRICDEF_BYTES_WRITTEN_STR;
			break;
		case METRIC_HOST_READS :
			ret += METRICDEF_HOST_READS_STR;
			break;
		case METRIC_HOST_WRITES :
			ret += METRICDEF_HOST_WRITES_STR;
			break;
		case METRIC_BLOCK_READS :
			ret += METRICDEF_BLOCK_READS_STR;
			break;
		case METRIC_BLOCK_WRITES :
			ret += METRICDEF_BLOCK_WRITES_STR;
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

wbem::performance::metric_type wbem::performance::PerformanceMetricDefinitionFactory
::metricTypeFromStr(const std::string& str)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	// preset return type to unknown
	wbem::performance::metric_type metricType = wbem::performance::METRIC_UNDEFINED;

	// test for the metric type requested and set the metric type code appropriately.
	if (std::string::npos != str.find(METRICDEF_BYTES_READ_STR))
	{
		metricType = METRIC_BYTES_READ;
	}
	else if (std::string::npos != str.find(METRICDEF_BYTES_WRITTEN_STR))
	{
		metricType = METRIC_BYTES_WRITTEN;
	}
	else if (std::string::npos != str.find(METRICDEF_HOST_READS_STR))
	{
		metricType = METRIC_HOST_READS;
	}
	else if (std::string::npos != str.find(METRICDEF_HOST_WRITES_STR))
	{
		metricType = METRIC_HOST_WRITES;
	}
	else if (std::string::npos != str.find(METRICDEF_BLOCK_READS_STR))
	{
		metricType = METRIC_BLOCK_READS;
	}
	else if (std::string::npos != str.find(METRICDEF_BLOCK_WRITES_STR))
	{
		metricType = METRIC_BLOCK_WRITES;
	}
	else
	{
		metricType = METRIC_UNDEFINED;
	}
	return (metricType);
}

const std::string wbem::performance::PerformanceMetricDefinitionFactory
::getMetricElementName(metric_type metric) throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if ((metric < wbem::performance::METRIC_FIRST_TYPE)
		|| (metric > wbem::performance::METRIC_LAST_TYPE))
	{
		throw framework::ExceptionBadParameter("invalid metric value");
	}
	return (metricTypeToStr(metric, METRICDEF_DEFINITION_STR));
}

const std::string wbem::performance::PerformanceMetricDefinitionFactory
::getMetricId(metric_type metric) throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if ((metric < wbem::performance::METRIC_FIRST_TYPE)
		|| (metric > wbem::performance::METRIC_LAST_TYPE))
	{
		throw framework::ExceptionBadParameter("invalid metric value");
	}
	return (metricTypeToStr(metric, wbem::server::getHostName()));
}

NVM_UINT16 wbem::performance::PerformanceMetricDefinitionFactory
::getMetricDataType(metric_type metric) throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if ((metric < wbem::performance::METRIC_FIRST_TYPE)
		|| (metric > wbem::performance::METRIC_LAST_TYPE))
	{
		throw framework::ExceptionBadParameter("invalid metric value");
	}
	return ((NVM_UINT16)METRICDATATYPE_UINT64);
}

const std::string wbem::performance::PerformanceMetricDefinitionFactory
::getMetricName(metric_type metric) throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if ((metric < wbem::performance::METRIC_FIRST_TYPE)
		|| (metric > wbem::performance::METRIC_LAST_TYPE))
	{
		throw framework::ExceptionBadParameter("invalid metric value");
	}
	return (getMetricId(metric));
}

const std::string wbem::performance::PerformanceMetricDefinitionFactory
::getMetricUnits(metric_type metric) throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if ((metric < wbem::performance::METRIC_FIRST_TYPE)
		|| (metric > wbem::performance::METRIC_LAST_TYPE))
	{
		throw framework::ExceptionBadParameter("invalid metric value");
	}
	std::string ret;
	switch (metric)
	{
		case METRIC_BYTES_READ :
		case METRIC_BYTES_WRITTEN :
			ret = METRICDEF_BYTES_STR;
			break;
		case METRIC_HOST_READS :
		case METRIC_HOST_WRITES :
		case METRIC_BLOCK_READS :
		case METRIC_BLOCK_WRITES :
			ret = METRICDEF_COUNT_STR;
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
	return ret;
}

NVM_UINT16 wbem::performance::PerformanceMetricDefinitionFactory
::getMetricTimeScope(metric_type metric) throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if ((metric < wbem::performance::METRIC_FIRST_TYPE)
		|| (metric > wbem::performance::METRIC_LAST_TYPE))
	{
		throw framework::ExceptionBadParameter("invalid metric value");
	}
	return ((NVM_UINT16) 2); //!< "Point" (2) for all instances
}

bool wbem::performance::PerformanceMetricDefinitionFactory
::getMetricIsContinuous(metric_type metric) throw(wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if ((metric < wbem::performance::METRIC_FIRST_TYPE)
		|| (metric > wbem::performance::METRIC_LAST_TYPE))
	{
		throw framework::ExceptionBadParameter("invalid metric value");
	}
	bool ret = false;
	switch (metric)
	{
		case METRIC_BYTES_READ :
		case METRIC_BYTES_WRITTEN :
		case METRIC_BLOCK_READS:
		case METRIC_BLOCK_WRITES:
			ret = true;
			break;
		case METRIC_HOST_READS :
		case METRIC_HOST_WRITES :
			ret = false;
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
	return ret;
}
