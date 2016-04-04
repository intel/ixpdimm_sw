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


#ifndef _WBEM_PERFORMANCE_METRIC_DEFINITION_FACTORY_H_
#define	_WBEM_PERFORMANCE_METRIC_DEFINITION_FACTORY_H_

#include <string>

#include <framework_interface/NvmInstanceFactory.h>
#include "PerformanceMetricCommon.h"

namespace wbem
{
namespace performance
{

/*!
 * Creation name associated with an instance of a performance metric definition class.
 */
static const std::string PERFORMANCEMETRICDEFINITION_CREATIONCLASSNAME
	= std::string(NVM_WBEM_PREFIX) + "PerformanceMetricDefinition";

/*!
 * Provider Factory for PerformanceDefinition
 */
class NVM_API PerformanceMetricDefinitionFactory : public framework_interface::NvmInstanceFactory
{
public:

	/*!
	 * Initialize a new NVDIMMPerformanceDefinition.
	 */
	PerformanceMetricDefinitionFactory() throw (framework::Exception);

	/*!
	 * Clean up the NVDIMMPerformanceDefinition
	 */
	~PerformanceMetricDefinitionFactory();

	/*!
	 * Implementation of the standard CIM method to retrieve a list of
	 * object paths.
	 * @return The object path.
	 */
	framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	/*!
	 * Implementation of the standard CIM method to retrieve a specific instance
	 * @param[in] path
	 * 		The object path of the instance to retrieve.
	 * @param[in] attributes
	 * 		The attributes to retrieve.
	 * @throw Exception if unable to retrieve the host information.
	 * @todo Should throw an exception if the object path doesn't match
	 * the results of getHostName.
	 * @return The instance.
	 */
	framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);
	/*!
	 * Retrieve the enumerated performance metric type requested in an object path
	 * @param[in] path
	 * 		ObjectPath to be parsed for a performance metric type
	 * @return
	 * 		Upon success returns a 'metric_type'.  Upon failure, throws an Exception.
	 */
	static metric_type metricTypeFromObjectPath
		(const framework::ObjectPath& path) throw (wbem::framework::Exception);
	/*!
	 * Utility function to convert a metric type code back into a text string.
	 * @param[in] metric - the metric_type being requested
	 * @param[in] prefix - optional prefix string to prepend to the result.
	 * @return formatted element name string optionally prefixed with "prefix".
	 */
	static const std::string metricTypeToStr
		(metric_type metric, const std::string& prefix) throw(wbem::framework::Exception);
	/*!
	 * Utility function convert the text representation of a metric type to its enumerated value.
	 * @param[in] str - The string to parse for a metric definition type.
	 * @return enumerated metric type of the metric type definition string.
	 */
	static metric_type metricTypeFromStr(const std::string& str);
	/*!
	 * Create a properly formatted CIM_ManagedElement::ElementName string from a performance metric type.
	 * @param[in] metric - the metric_type being requested
	 * @return formatted element name string.
	 */
	static const std::string getMetricElementName(metric_type metric) throw(wbem::framework::Exception);
	/*!
	 * Create a properly formatted CIM_BaseMetricDefinition::Id string from a performance metric type.
	 * @param[in] metric - the metric_type being requested
	 * @return formatted id string
	 */
	static const std::string getMetricId(metric_type metric) throw(wbem::framework::Exception);
	/*!
	 * Retrieves the enumerated data type associated with a performance metric
	 * @param[in] metric - the type of performance metric
	 * @return CIM_BaseMetricDefinition.DataType value associated with the performance metric
	 */
	static NVM_UINT16 getMetricDataType(metric_type metric) throw(wbem::framework::Exception);
	/*!
	 * Create a properly formatted CIM_BaseMetricDefinition::Name string from a performance metric type.
	 * @param[in] metric - the metric_type being requested
	 * @return formatted Name string
	 */
	static const std::string getMetricName(metric_type metric) throw(wbem::framework::Exception);
	/*!
	 * Create a properly formatted CIM_BaseMetricDefinition::Units string from a performance metric type.
	 * @param[in] metric - the metric_type being requested
	 * @return formatted Units string
	 */
	static const std::string getMetricUnits(metric_type metric) throw(wbem::framework::Exception);
	/*!
	 * Retrieves the enumerated time scope type associated with a performance metric
	 * @param[in] metric - the type of performance metric
	 * @return CIM_BaseMetricDefinition.TimeScope value associated with the performance metric
	 */
	static NVM_UINT16 getMetricTimeScope(metric_type metric) throw(wbem::framework::Exception);
	/*!
	 * Determines if the associated metric performance counter is contious.
	 * @param[in] metric - the type of performance metric
	 * @return True indicating the metric is continous or false indicating that it is not.
	 */
	static bool getMetricIsContinuous(metric_type metric) throw(wbem::framework::Exception);
private:
	void populateAttributeList(framework::attribute_names_t &attributes) throw (framework::Exception);
};

} // performance
} // wbem
#endif  // _WBEM_PERFORMANCE_METRIC_DEFINITION_FACTORY_H_
