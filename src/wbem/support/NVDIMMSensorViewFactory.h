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
 * This file contains the provider for the NVDIMMSensorView instances
 * which is an internal only sensor view used by the CLI show -sensor command.
 */

#ifndef	_WBEM_SUPPORT_NVDIMMSENSORVIEW_FACTORY_H_
#define	_WBEM_SUPPORT_NVDIMMSENSORVIEW_FACTORY_H_

#include <support/NVDIMMSensorFactory.h>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace support
{
// Sensor properties unique to thi CLI command
static std::string CURRENTVALUE_KEY = "CurrentValue"; //!< current value key

// Sensor strings
static std::string SENSORENABLEDSTATESTR_ENABLED = N_TR("Enabled");
static std::string SENSORENABLEDSTATESTR_DISABLED = N_TR("Disabled");
static std::string SENSORENABLEDSTATESTR_UNKNOWN = N_TR("Unknown");
static std::string SENSORTHRESHOLDTYPE_LOWERNONCRITICAL = N_TR("LowerThresholdNonCritical");
static std::string SENSORTHRESHOLDTYPE_UPPERNONCRITICAL = N_TR("UpperThresholdNonCritical");
static std::string SENSORTHRESHOLDTYPE_LOWERCRITICAL = N_TR("LowerThresholdCritical");
static std::string SENSORTHRESHOLDTYPE_UPPERCRITICAL = N_TR("UpperThresholdCritical");
static std::string SENSORTHRESHOLDTYPE_UPPERFATAL = N_TR("UpperThresholdFatal");
static std::string SENSORTHRESHOLDTYPE_UNKNOWN = N_TR("Unknown");

// Sensor properties used by the CLI

static std::string PROPERTY_SENSOR_TYPE_MEDIATEMP = N_TR("MediaTemperature"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_SPARE = N_TR("SpareCapacity"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_WEAR = N_TR("WearLevel"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_POWERCYCLES = N_TR("PowerCycles"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_POWERON = N_TR("PowerOnTime"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_UPTIME = N_TR("UpTime"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_UNSAFESHUTDOWNS = N_TR("UnsafeShutDowns"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_FWERRORLOGCOUNT = N_TR("FWErrorCount"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_POWERLIMITED = N_TR("PowerLimited"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_MEDIAERRORS_UNCORRECTABLE = N_TR("MediaErrorsUncorrectable"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_MEDIAERRORS_CORRECTED = N_TR("MediaErrorsCorrected"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_MEDIAERRORS_ERASURECODED = N_TR("MediaErrorsErasureCoded"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_WRITECOUNT_MAXIMUM = N_TR("WriteCountMax"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_WRITECOUNT_AVERAGE = N_TR("WriteCountAvg"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_MEDIAERRORS_HOST = N_TR("MediaErrorsHost"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_MEDIAERRORS_NONHOST = N_TR("MediaErrorsNonHost"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_CONTROLLERTEMP = N_TR("ControllerTemperature"); //!< type value
static std::string PROPERTY_SENSOR_TYPE_UNKNOWN = N_TR("Unknown"); //!< unknown type
/*!
 * Provider Factory for NVDIMMSensorView
 */
class NVM_API NVDIMMSensorViewFactory : public framework_interface::NvmInstanceFactory
{
public:
	/*!
	 * Initialize a new NVDIMMSensorViewFactory.
	 */
	NVDIMMSensorViewFactory() throw (framework::Exception);

	/*!
	 * Clean up the NVDIMMSensorViewFactory.
	 */
	~NVDIMMSensorViewFactory();

	/*!
	 * Implementation of the standard CIM method to retrieve a specific instance
	 * @param[in] path
	 * 		The object path of the instance to retrieve.
	 * @param[in] attributes
	 * 		The attributes to retrieve.
	 * @throw Exception if unable to retrieve the NVDIMMSensor instance.
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
	 *  Return a string value associated with the type of threshold.
	 *  @return the threshold type string value.
	 */
	static std::string getThresholdTypeStr(int threshold);

	/*
	 * return string associated with the type of unit
	 */
	static std::string baseUnitToString(int baseUnit);

	/*
	 * return the string associated with the sensor name
	 */
	static std::string getSensorNameStr(int type);

	/*
	 * return the string associated with the enabled state
	 */
	static std::string getEnabledStateStr(int state);

private:

	/*
	 * populate the attribute list for this class
	 */
	void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

	void sensorToInstance(NVM_UID uidStr, const sensor &sensor, framework::Instance *pInstance, framework::attribute_names_t attributes);

	void addThresholdForType(framework::Instance *pInstance, framework::attribute_names_t vector,
		std::string key, sensor_type type, NVM_UINT64 threshold);
};
}  // support
}  // wbem
#endif  // _WBEM_SUPPORT_NVDIMMSENSOR_FACTORYVIEW_H_
