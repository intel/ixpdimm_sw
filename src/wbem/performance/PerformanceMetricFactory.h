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

#ifndef	_WBEM_PERFORMANCE_METRIC_FACTORY_H_
#define	_WBEM_PERFORMANCE_METRIC_FACTORY_H_

#include <string>

#include <framework_interface/NvmInstanceFactory.h>
#include "PerformanceMetricCommon.h"

namespace wbem
{
namespace performance
{

/*!
 * Creation name associated with an instance of a performance metric class.
 */
static const std::string PERFORMANCE_METRIC_CREATIONCLASSNAME
	= std::string(NVM_WBEM_PREFIX) + "PerformanceMetric";

/*!
 * Provider Factory for NVDIMMSensor
 */
class NVM_API PerformanceMetricFactory : public framework_interface::NvmInstanceFactory
{
public:

	/*!
	 * Initialize a new PerformanceMetric
	 */
	PerformanceMetricFactory() throw (framework::Exception);

	/*!
	 * Clean up the PerformanceMetric
	 */
	~PerformanceMetricFactory();

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
	 * Implementation of the standard CIM method to retrieve a list of
	 * object paths.
	 * @return The object path.
	 */
	framework::instance_names_t* getInstanceNames() throw (framework::Exception);
	/*!
	 * Utility function to find a substring that matches a defined metric type string
	 * @param[in] str
	 * 		the string to parse.
	 * @return metric value code for the string found (METRIC_UNDEFINED, if not found).
	 */
	static metric_type stringToMetric(const std::string &str);
	/*!
	 * Utility function to convert a metric type code back into a text string.
	 * @param[in] metric
	 * 		the metric_type being requested
	 * @param[in] suffix
	 * 		optional suffix string to append to the result.
	 * @return formatted element name string optionally suffixed with "suffix".
	 */
	static std::string getInstanceIdNameFromType
		(metric_type metric, const std::string &suffix = "") throw(wbem::framework::Exception);

	/*!
	 * Test to see if to instances are associated.
	 * @param[in] associationClass
	 * 		Pointer to the association class
	 * @param[in] pAntInstance
	 * 		Pointer to the antecedent instance
	 * @param pDepInstance
	 * 		Pointer to the dependent instance
	 * @return true if there is a antecedent/dependent relationship.
	 */
	bool isAssociated(const std::string &associationClass, framework::Instance* pAntInstance,
		 framework::Instance* pDepInstance);
	/*!
	 * Splits Performance Metrics InstanceID into the metric type and device uid.
	 * @param[in] instanceId
	 * 		The Attribute containing the instance id to parse.
	 * @param[out] deviceUid
	 * 		If successfully parsed, the uid for the device.
	 * @param[out] metric
	 * 		If successful, the enumerated type code for the metric type requested.
	 * @return false on fail, true on success.
	 */
	static bool splitInstanceID(const framework::Attribute& instanceId,
		std::string &deviceUid,
		metric_type& metric);

private:
	void populateAttributeList(framework::attribute_names_t &attributes)
		throw (framework::Exception);

	static NVM_UINT64 getValueForDeviceMetric(const NVM_UID deviceUid, const enum metric_type metricType)
		throw (framework::Exception);

	static std::string getMetricElementNameFromType(const metric_type type)
		throw (framework::Exception);

	static std::string getDeviceSerialNumber(const NVM_UID uid)
		throw (framework::Exception);
};

} // performance
} // wbem
#endif //_WBEM_PERFORMANCE_METRIC_FACTORY_H_
