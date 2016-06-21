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
 * This file contains the provider for the NVDIMMSensor instances
 * which represent an individual sensor for an NVM DIMM.
 */

#ifndef	_WBEM_SUPPORT_NVDIMMSENSOR_FACTORY_H_
#define	_WBEM_SUPPORT_NVDIMMSENSOR_FACTORY_H_

#include <string>
#include <nvm_management.h>
#include <common_types.h>
#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace support
{
static const std::string NVDIMMSENSOR_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "NVDIMMSensor"; //!< Creation Class Name static
static const std::string NVDIMMSENSOR_ELEMENTPREFIX = "NVMDIMM Sensor "; //!< prefix to all element attributes

static const framework::UINT16 SENSOR_ENABLEDSTATE_ENABLED = 2; //!< Sensor alarm is enabled
static const framework::UINT16 SENSOR_ENABLEDSTATE_DISABLED = 3; //!< Sensor alarm is disabled
static const framework::UINT16 SENSOR_ENABLEDSTATE_NA = 5; //!< Sensor alarm is not applicable

static const framework::UINT16 SENSOR_LOWER_NONCRITICAL_THRESHOLD = 0; //!< lowest critical threshold
static const framework::UINT16 SENSOR_UPPER_NONCRITICAL_THRESHOLD = 1; //!< lowest critical threshold
static const framework::UINT16 SENSOR_LOWER_CRITICAL_THRESHOLD = 2; //!< lowest critical threshold
static const framework::UINT16 SENSOR_UPPER_CRITICAL_THRESHOLD = 3; //!< highest critical threshold
static const framework::UINT16 SENSOR_UPPER_FATAL_THRESHOLD = 5; //!< lowest critical threshold

#define SENSOR_TYPE_UNKNOWN 0
#define SENSOR_TYPE_OTHER   1
#define SENSOR_TYPE_TEMP    2

/*!
 * Provider Factory for NVDIMMSensor
 */
class NVM_API NVDIMMSensorFactory : public framework_interface::NvmInstanceFactory
{
public:
	/*!
	 * Expose the NVM Sensor Types up to CLI
	 */
	static const int SENSORTYPE_MEDIATEMPERATURE; //!< Device media temperature in degrees Celsius.
	static const int SENSORTYPE_SPARECAPACITY; //!< Amount of spare capacity remaining as a percentage.
	static const int SENSORTYPE_WEARLEVEL; //!< An estimate of the device life used as a percentage.
	static const int SENSORTYPE_POWERCYCLES; //!< Number of power cycles over the lifetime of the device.
	static const int SENSORTYPE_POWERONTIME; //!< Total power-on time over the lifetime of the device.
	static const int SENSORTYPE_UPTIME; //!< Total power-on time since the last power cycle of the device.
	static const int SENSORTYPE_UNSAFESHUTDOWNS; //!< Device shutdowns without notification.
	static const int SENSORTYPE_FWERRORLOGCOUNT; //!< Number of new FW errors
	static const int SENSORTYPE_POWERLIMITED; //!<Whether or not the DIMM is power limited
	static const int SENSORTYPE_MEDIAERRORS_UNCORRECTABLE; //!< Number of ECC uncorrectable errors.
	static const int SENSORTYPE_MEDIAERRORS_CORRECTED; //!< Number of ECC corrected errors.
	static const int SENSORTYPE_MEDIAERRORS_ERASURECODED; //!< Number of erasure code corrected errors.
	static const int SENSORTYPE_WRITECOUNT_MAXIMUM; //!< Largest number of data writes to a single block across the device.
	static const int SENSORTYPE_WRITECOUNT_AVERAGE; //!< Average number of data writes to all blocks across the device.
	static const int SENSORTYPE_MEDIAERRORS_HOST; //!< Number of ECC errors encountered by hosts requests only.
	static const int SENSORTYPE_MEDIAERRORS_NONHOST; //!< Number of ECC errors encountered by non-hosts requests only.
	static const int SENSORTYPE_CONTROLLER_TEMPERATURE; //!< Device controller temperature in degrees Celsius

	static const int SENSOR_TEMP_MODIFIER = 10000;
	static const int SENSOR_TEMP_MODIFIER_POWER = 4;

	NVDIMMSensorFactory() throw (framework::Exception);

	~NVDIMMSensorFactory();

	framework::instances_t *getInstances(framework::attribute_names_t &attributes);

	framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

	framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	wbem::framework::Instance* modifyInstance(wbem::framework::ObjectPath &path, wbem::framework::attributes_t &attributes)
		throw (wbem::framework::Exception);


	/*!
	 * Helper function to scale a 64 bit number to "fit" into two 32 bit return
	 * values such that:
	 * num = pscaled * (10 ^ pscaler)
	 * @param num
	 * @param pscaled
	 * @param pscaler
	 */
	static void scaleNumberBaseTen(COMMON_UINT64 num, COMMON_INT32* pscaled, COMMON_INT32* pscaler);

	/*!
	 * Helper function to split the deviceId Property into its respective device UID and sensor
	 * type
	 * @param[in] deviceIdAttribute
	 * @param[out] deviceUid
	 * @param[out] type
	 * @return
	 * 		true if the type is valid, false if not
	 *
	 */
	static bool splitDeviceIdAttribute(const framework::Attribute& deviceIdAttribute,
			std::string &deviceUid, int& type);

	/*!
	 * Helper function to get the element name given the type
	 * @param nvm_type
	 * @return
	 */
	static const std::string& getCIMSensorElementName(int nvm_type) throw (framework::Exception);

	/*!
	 * Fetch the expected object path for the sensor of a given type on a given DIMM.
	 * @param type - WBEM sensor type
	 * @param hostname - local hostname
	 * @param dimmUid
	 * @return the sensor's object path
	 * @throw Exception if type is invalid
	 */
	static framework::ObjectPath getSensorPath(const int type,
			const std::string &hostname, const std::string &dimmUid)
			throw (framework::Exception);

	/*!
	 * Helper function to update the sensor. Mostly provided as passthrough for the CLI
	 * @param dimmUid
	 * @param type - sensor type
	 * @param attributes - list of attributes to change
	 * @param pInstance - Instance to update
	 */
	void updateSensor(const std::string &dimmUid, const int type, const framework::attributes_t &attributes,
			framework::Instance *pInstance);

	bool isAssociated(const std::string &associationClass, framework::Instance *pAntInstance,
			framework::Instance *pDepInstance);

	/*!
	 * get the string associated with the sensor status
	 * @return the sensor state string
	 */
	static const std::string getSensorStateStr(enum sensor_status state);

	/*!
	 * helper method to return a vector of instance names for this class that can
	 * be shared with the view class
	 * @return the vector of instance names
	 */
	static 	framework::instance_names_t* getNames() throw (framework::Exception);
	static framework::SINT32 nvmTempToCimTemp(const NVM_UINT64 &value);
	static framework::SINT32 realTempToCimTemp(const framework::REAL32 &temp);

private:
	static const std::string& getCIMSensorDeviceName(int nvm_type) throw (framework::Exception);
	static const framework::UINT16& getCIMSensorTypeCode(int nvm_type) throw (framework::Exception);
	static const std::string& getCIMSensorOtherTypeName(int nvm_type) throw (framework::Exception);
	void populateAttributeList(framework::attribute_names_t &attributes) throw (framework::Exception);

	/*
	 * Helper method. Given an attribute name, gets the current (from pInstance) and new
	 * (from attributes) values.
	 * @return true if the named attribute is found in attributes.
	 * @throw Exception if the attribute is found in attributes but not pInstance
	 */
	bool getModifiableAttribute(const std::string &attributeName,
			const framework::attributes_t &attributes,
			const framework::Instance * const pInstance,
			framework::Attribute &currentAttribute,
			framework::Attribute &newAttribute) throw (framework::Exception);

	/*
	 * get the string associated with the enabled state
	 */
	std::string getSensorEnabledString(const NVM_UINT16 enabledState);
	void sensorToInstance(const wbem::framework::attribute_names_t &attributes,
		const struct sensor &sensor, wbem::framework::Instance &instance);

	framework::SINT32 decodeIfTemp(const sensor_type &type, const NVM_UINT64 &value);

	static NVM_UINT32 tempAttributeToNvmValue(const framework::Attribute &attribute);
	static bool isTempSensorType(const sensor_type &type);
};

} // software
} // wbem
#endif  // _WBEM_SUPPORT_NVDIMMSENSOR_FACTORY_H_
