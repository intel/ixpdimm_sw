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

#include <LogEnterExit.h>
#include <nvm_management.h>
#include <string/revision.h>
#include <uid/uid.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <physical_asset/NVDIMMFactory.h>
#include "NVDIMMSensorFactory.h"
#include <server/BaseServerFactory.h>
#include <framework_interface/NvmAssociationFactory.h>
#include <limits>
#include <stdint.h>

#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>
#include <framework_interface/FrameworkExtensions.h>
#include <core/device/DeviceHelper.h>

namespace wbem
{
namespace support
{

/*
 * Sensor Types are the same as the library
 */
const int NVDIMMSensorFactory::SENSORTYPE_MEDIATEMPERATURE = SENSOR_MEDIA_TEMPERATURE;
const int NVDIMMSensorFactory::SENSORTYPE_SPARECAPACITY = SENSOR_SPARECAPACITY;
const int NVDIMMSensorFactory::SENSORTYPE_WEARLEVEL = SENSOR_WEARLEVEL;
const int NVDIMMSensorFactory::SENSORTYPE_POWERCYCLES = SENSOR_POWERCYCLES;
const int NVDIMMSensorFactory::SENSORTYPE_POWERONTIME = SENSOR_POWERONTIME;
const int NVDIMMSensorFactory::SENSORTYPE_UPTIME = SENSOR_UPTIME;
const int NVDIMMSensorFactory::SENSORTYPE_UNSAFESHUTDOWNS = SENSOR_UNSAFESHUTDOWNS;
const int NVDIMMSensorFactory::SENSORTYPE_FWERRORLOGCOUNT = SENSOR_FWERRORLOGCOUNT;
const int NVDIMMSensorFactory::SENSORTYPE_POWERLIMITED = SENSOR_POWERLIMITED;
const int NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_UNCORRECTABLE = SENSOR_MEDIAERRORS_UNCORRECTABLE;
const int NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_CORRECTED = SENSOR_MEDIAERRORS_CORRECTED;
const int NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_ERASURECODED = SENSOR_MEDIAERRORS_ERASURECODED;
const int NVDIMMSensorFactory::SENSORTYPE_WRITECOUNT_MAXIMUM = SENSOR_WRITECOUNT_MAXIMUM;
const int NVDIMMSensorFactory::SENSORTYPE_WRITECOUNT_AVERAGE = SENSOR_WRITECOUNT_AVERAGE;
const int NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_HOST = SENSOR_MEDIAERRORS_HOST;
const int NVDIMMSensorFactory::SENSORTYPE_MEDIAERRORS_NONHOST = SENSOR_MEDIAERRORS_NONHOST;
const int NVDIMMSensorFactory::SENSORTYPE_CONTROLLER_TEMPERATURE = SENSOR_CONTROLLER_TEMPERATURE;

/*
 * CIM list of possible sensor states.
 */

const std::string NVDIMMSensorFactory::getSensorStateStr(enum sensor_status state)
{
	std::string result;
	switch (state)
	{
		case SENSOR_NORMAL:
			result = N_TR("Normal");
			break;
		case SENSOR_CRITICAL:
			result = N_TR("Critical");
			break;
		case SENSOR_FATAL:
			result = N_TR("Fatal");
			break;
		case SENSOR_NONCRITICAL:
			result = N_TR("NonCritical");
			break;
		case SENSOR_UNKNOWN:
		default:
			result = N_TR("Unknown");
			break;
	}
	return result;
}

/*!
 * CIM compatible sensor attributes / descriptions
 */
typedef struct
{
	std::string deviceName;            //!< device path name as used in Device ID.
	std::string elementName;            //!< Human readable name as used in ElementName
	NVM_UINT16 sensorTypeCode;        //!< CIM Schema compatible type code.
	std::string otherSensorTypeName;    //!< 'other' sensor type ID strings/descriptions for non-standard sensors.
} cimSensorDescription;

/*
 * static map of nvm sensor type enums to wbem compatible descriptors and attributes.
 */
typedef std::map<sensor_type, cimSensorDescription> cimSensorDescriptionsMap;

const cimSensorDescriptionsMap &getSensorDescriptionMap()
{
	static cimSensorDescriptionsMap result;
	if (result.empty())
	{
		result[SENSOR_MEDIA_TEMPERATURE] = (cimSensorDescription) {"mediatemp", NVDIMMSENSOR_ELEMENTPREFIX + "Media Temp", SENSOR_TYPE_TEMP, ""};
		result[SENSOR_SPARECAPACITY] = (cimSensorDescription) {"spare", NVDIMMSENSOR_ELEMENTPREFIX + "Spare", SENSOR_TYPE_OTHER, "SpareCapacity"};
		result[SENSOR_WEARLEVEL] = (cimSensorDescription) {"wear", NVDIMMSENSOR_ELEMENTPREFIX + "Wear", SENSOR_TYPE_OTHER, "WearLevel"};
		result[SENSOR_POWERCYCLES] = (cimSensorDescription) {"pc", NVDIMMSENSOR_ELEMENTPREFIX + "Power Cycles", SENSOR_TYPE_OTHER, "PowerCycles"};
		result[SENSOR_POWERONTIME] = (cimSensorDescription) {"poh", NVDIMMSENSOR_ELEMENTPREFIX + "Power-on", SENSOR_TYPE_OTHER, "PowerOnTime"};
		result[SENSOR_UPTIME] = (cimSensorDescription) {"ut", NVDIMMSENSOR_ELEMENTPREFIX + "Up", SENSOR_TYPE_OTHER, "UpTime"};
		result[SENSOR_UNSAFESHUTDOWNS] =(cimSensorDescription) {"us", NVDIMMSENSOR_ELEMENTPREFIX + "Unsafe Shutdowns", SENSOR_TYPE_OTHER, "UnsafeShutdowns"};
		result[SENSOR_FWERRORLOGCOUNT] = (cimSensorDescription) {"error", NVDIMMSENSOR_ELEMENTPREFIX + "FW Error Log Count", SENSOR_TYPE_OTHER, "FWError"};
		result[SENSOR_POWERLIMITED] = (cimSensorDescription) {"pl", NVDIMMSENSOR_ELEMENTPREFIX + "Power Limited", SENSOR_TYPE_OTHER, "PowerLimited"};
		result[SENSOR_MEDIAERRORS_UNCORRECTABLE] = (cimSensorDescription) {"meuc", NVDIMMSENSOR_ELEMENTPREFIX + "Media Errors Uncorrectable", SENSOR_TYPE_OTHER, "MediaErrorsUncorrectable"};
		result[SENSOR_MEDIAERRORS_CORRECTED] = (cimSensorDescription) {"mece", NVDIMMSENSOR_ELEMENTPREFIX + "Media Errors Corrected", SENSOR_TYPE_OTHER, "MediaErrorsCorrected"};
		result[SENSOR_MEDIAERRORS_ERASURECODED] = (cimSensorDescription) {"meecc", NVDIMMSENSOR_ELEMENTPREFIX + "Media Errors Erasure Coded", SENSOR_TYPE_OTHER, "MediaErrorsErasureCoded"};
		result[SENSOR_WRITECOUNT_MAXIMUM] = (cimSensorDescription) {"wcmax", NVDIMMSENSOR_ELEMENTPREFIX + "Write Count Maximum", SENSOR_TYPE_OTHER, "WriteCountMaximum"};
		result[SENSOR_WRITECOUNT_AVERAGE] = (cimSensorDescription) {"wcavg", NVDIMMSENSOR_ELEMENTPREFIX + "Write Count Average", SENSOR_TYPE_OTHER, "WriteCountAverage"};
		result[SENSOR_MEDIAERRORS_HOST] = (cimSensorDescription) {"meh", NVDIMMSENSOR_ELEMENTPREFIX + "Media Errors Host", SENSOR_TYPE_OTHER, "MediaErrorsHost"};
		result[SENSOR_MEDIAERRORS_NONHOST] = (cimSensorDescription) {"menh", NVDIMMSENSOR_ELEMENTPREFIX + "Media Errors Non-host", SENSOR_TYPE_OTHER, "MediaErrorsNonHost"};
		result[SENSOR_CONTROLLER_TEMPERATURE] = (cimSensorDescription) {"controllertemp", NVDIMMSENSOR_ELEMENTPREFIX + "Controller Temp", SENSOR_TYPE_TEMP, ""};
	}

	return result;
}

static const cimSensorDescriptionsMap cimSensorDescriptions = getSensorDescriptionMap();

/*
 * Iterator typedefs for cimSensorDescriptionsMap
 */
typedef std::map<sensor_type, cimSensorDescription>::iterator cimSensorDescriptionsIter;
typedef std::map<sensor_type, cimSensorDescription>::const_iterator cimSensorConstDescriptionsIter;

/*
 * Helper function to convert an nvm sensor type code into a pointer to sensor description.
 */
static const cimSensorConstDescriptionsIter getSensorDescriptionIter(int nvm_type)
throw(wbem::framework::Exception)
{
	cimSensorConstDescriptionsIter it = cimSensorDescriptions.find((enum sensor_type) nvm_type);
	if (it == cimSensorDescriptions.end())
	{
		throw wbem::framework::ExceptionBadParameter("Invalid NVM Sensor Type");
	}
	return it;
}

/*
 * convert an nvm sensor type code into a CIM sensor device name
 */
const std::string &NVDIMMSensorFactory::getCIMSensorDeviceName(
	int nvm_type) throw(wbem::framework::Exception)
{
	return (getSensorDescriptionIter(nvm_type)->second.deviceName);
}

/*
 * convert an nvm sensor type code into a CIM sensor element name
 */
const std::string &NVDIMMSensorFactory::getCIMSensorElementName(
	int nvm_type) throw(wbem::framework::Exception)
{
	return (getSensorDescriptionIter(nvm_type)->second.elementName);
}

/*
 * convert an nvm sensor type code into a CIM sensor type code
 */
const wbem::framework::UINT16 &NVDIMMSensorFactory::getCIMSensorTypeCode(
	int nvm_type) throw(wbem::framework::Exception)
{
	return (getSensorDescriptionIter(nvm_type)->second.sensorTypeCode);
}

/*
 * convert an nvm sensor type code into a CIM sensor 'other' type name
 */
const std::string &NVDIMMSensorFactory::getCIMSensorOtherTypeName(
	int nvm_type) throw(wbem::framework::Exception)
{
	return (getSensorDescriptionIter(nvm_type)->second.otherSensorTypeName);
}

/*
 * splits a DeviceID attribute and splits it into uid + decoded nvm sensor type.
 * sets deviceUid and 'type' and returns non-zero on success.
 */
bool NVDIMMSensorFactory::splitDeviceIdAttribute(const framework::Attribute &deviceIdAttribute,
	std::string &deviceUid, int &type)
{
	bool found = false;
	const std::string devIdAttrStr = deviceIdAttribute.stringValue();

	int uidIndex = core::device::findUidEnd(devIdAttrStr);
	if (uidIndex >= 0)
	{
		const std::string sensorTypeName = devIdAttrStr.substr(uidIndex);
		cimSensorConstDescriptionsIter it = cimSensorDescriptions.begin();
		for (; it != cimSensorDescriptions.end() && !found; it++)
		{
			if (it->second.deviceName == sensorTypeName)
			{
				type = it->first;
				found = true;
			}

		}
		deviceUid = deviceIdAttribute.stringValue().substr(0, uidIndex);
	}
	return found;
}

/*
 * Helper function to scale a 64 bit number to "fit" into two 32 bit return
 * values such that:
 * num = pscaled * (10 ^ pscaler)
 */
void NVDIMMSensorFactory::scaleNumberBaseTen(COMMON_UINT64 num, COMMON_INT32 *pscaled,
	COMMON_INT32 *pscaler)
{
	*pscaler = 0;
	while (num > ((COMMON_UINT64) std::numeric_limits<int32_t>::max()))
	{
		num /= 10;
		(*pscaler)++;
	}
	*pscaled = (int32_t) num;
}

NVDIMMSensorFactory::NVDIMMSensorFactory()
throw(wbem::framework::Exception) { }

NVDIMMSensorFactory::~NVDIMMSensorFactory() { }

void NVDIMMSensorFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw(wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(SYSTEMCREATIONCLASSNAME_KEY);
	attributes.push_back(SYSTEMNAME_KEY);
	attributes.push_back(CREATIONCLASSNAME_KEY);
	attributes.push_back(DEVICEID_KEY);

	// add non-key attributes
	attributes.push_back(ELEMENTNAME_KEY);
	attributes.push_back(SENSORTYPE_KEY);
	attributes.push_back(BASEUNITS_KEY);
	attributes.push_back(CURRENTREADING_KEY);
	attributes.push_back(UNITMODIFIER_KEY);
	attributes.push_back(OTHERSENSORTYPEDESCRIPTION_KEY);
	attributes.push_back(LOWERTHRESHOLDNONCRITICAL_KEY);
	attributes.push_back(UPPERTHRESHOLDNONCRITICAL_KEY);
	attributes.push_back(LOWERTHRESHOLDCRITICAL_KEY);
	attributes.push_back(UPPERTHRESHOLDCRITICAL_KEY);
	attributes.push_back(LOWERTHRESHOLDFATAL_KEY);
	attributes.push_back(UPPERTHRESHOLDFATAL_KEY);
	attributes.push_back(SETTABLETHRESHOLDS_KEY);
	attributes.push_back(SUPPORTEDTHRESHOLDS_KEY);
	attributes.push_back(POSSIBLESTATES_KEY);
	attributes.push_back(CURRENTSTATE_KEY);
	attributes.push_back(ENABLEDSTATE_KEY);
}

framework::instances_t *NVDIMMSensorFactory::getInstances(framework::attribute_names_t &attributes)
{
	framework::instances_t *pResult = new framework::instances_t();

	sensor sensors[NVM_MAX_DEVICE_SENSORS];

	std::vector<std::string> manageableDevices = physical_asset::NVDIMMFactory::getManageableDeviceUids();
	std::string hostName = server::getHostName();

	for (size_t dimmIdx = 0; dimmIdx < manageableDevices.size(); dimmIdx++)
	{
		std::string uidStr = manageableDevices[dimmIdx];
		NVM_UID uid;
		uid_copy(uidStr.c_str(), uid);

		int rc = nvm_get_sensors(uid, sensors, NVM_MAX_DEVICE_SENSORS);
		if (rc != NVM_SUCCESS)
		{
			delete pResult;
			throw exception::NvmExceptionLibError(rc);
		}
		for (int i = 0; i < NVM_MAX_DEVICE_SENSORS; i++)
		{
			framework::ObjectPath path = getSensorPath(sensors[i].type, hostName,
				manageableDevices[dimmIdx]);
			framework::Instance sensorInstance(path);
			sensorToInstance(attributes, sensors[i], sensorInstance);
			pResult->push_back(sensorInstance);
		}
	}

	return pResult;

}

/*
 * Retrieve a specific instance given an object path
 */
framework::Instance *NVDIMMSensorFactory::getInstance(framework::ObjectPath &path,
	framework::attribute_names_t &attributes)
throw(framework::Exception)
{
	// Verify attributes
	checkAttributes(attributes);

	// Verify ObjectPath ...
	std::string hostName = server::getHostName();

	//	Verify SystemCreatonClassName
	framework::Attribute attribute = path.getKeyValue(SYSTEMCREATIONCLASSNAME_KEY);
	if (attribute.stringValue() != server::BASESERVER_CREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(SYSTEMCREATIONCLASSNAME_KEY.c_str());
	}

	//	Verify SystemName
	attribute = path.getKeyValue(SYSTEMNAME_KEY);
	if (attribute.stringValue() != hostName)
	{
		throw framework::ExceptionBadParameter(SYSTEMNAME_KEY.c_str());
	}
	//	Verify CreationClassName
	attribute = path.getKeyValue(CREATIONCLASSNAME_KEY);
	if (attribute.stringValue() != NVDIMMSENSOR_CREATIONCLASSNAME)
	{
		throw framework::ExceptionBadParameter(CREATIONCLASSNAME_KEY.c_str());
	}

	attribute = path.getKeyValue(DEVICEID_KEY);

	// Verify DeviceID and get the device UID && NVM_SENSOR_TYPE based on sensor type name from the device ID
	// Device ID format: DIMM UUID + elementof("temp", "spare", "wear", "poh", "pc", "us", "me")
	std::string str_uid;
	enum sensor_type type;
	if (!splitDeviceIdAttribute(attribute, str_uid, (int &) type))
	{
		throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
	}

	NVM_UID uid;
	uid_copy(str_uid.c_str(), uid);

	int rc;
	struct sensor sensor;
	if ((rc = nvm_get_sensor(uid, type, &sensor)) != NVM_SUCCESS)
	{
		throw exception::NvmExceptionLibError(rc);
	}

	framework::Instance *pInstance = new framework::Instance(path);

	sensorToInstance(attributes, sensor, *pInstance);
	return pInstance;
}

void NVDIMMSensorFactory::sensorToInstance(const framework::attribute_names_t &a,
	const struct sensor &sensor, framework::Instance &i)
{
	// convert the sensor current reading to CIM appropriate value and modifier
	framework::SINT32 unit_modifier;
	framework::SINT32 current_reading;

	if (isTempSensorType(sensor.type))
	{
		unit_modifier = SENSOR_TEMP_MODIFIER_POWER;
		current_reading = nvmTempToCimTemp(sensor.reading);
	}
	else
	{
		scaleNumberBaseTen((framework::SINT32) sensor.reading, &current_reading, &unit_modifier);

	}
	enum sensor_type sensor_type = sensor.type;

	ADD_ATTRIBUTE(i, a, ELEMENTNAME_KEY, framework::STR, getCIMSensorElementName(sensor_type));
	ADD_ATTRIBUTE(i, a, SENSORTYPE_KEY, framework::UINT16, getCIMSensorTypeCode(sensor_type));
	ADD_ATTRIBUTE(i, a, BASEUNITS_KEY, framework::UINT16, sensor.units);// assume that the library sensor_units values are same as what CIM expects
	ADD_ATTRIBUTE(i, a, CURRENTREADING_KEY, framework::SINT32, current_reading);
	ADD_ATTRIBUTE(i, a, UNITMODIFIER_KEY, framework::SINT32 , unit_modifier);
	ADD_ATTRIBUTE(i, a, OTHERSENSORTYPEDESCRIPTION_KEY, framework::STR, getCIMSensorOtherTypeName(sensor_type));
	ADD_ATTRIBUTE(i, a, LOWERTHRESHOLDNONCRITICAL_KEY, framework::SINT32 ,
		decodeIfTemp(sensor.type, sensor.settings.lower_noncritical_threshold));
	ADD_ATTRIBUTE(i, a, UPPERTHRESHOLDNONCRITICAL_KEY, framework::SINT32 ,
		decodeIfTemp(sensor.type, sensor.settings.upper_noncritical_threshold));
	ADD_ATTRIBUTE(i, a, LOWERTHRESHOLDCRITICAL_KEY, framework::SINT32 ,
		decodeIfTemp(sensor.type, sensor.settings.lower_critical_threshold));
	ADD_ATTRIBUTE(i, a, UPPERTHRESHOLDCRITICAL_KEY, framework::SINT32 ,
		decodeIfTemp(sensor.type, sensor.settings.upper_critical_threshold));
	ADD_ATTRIBUTE(i, a, LOWERTHRESHOLDFATAL_KEY, framework::SINT32 ,
		decodeIfTemp(sensor.type, sensor.settings.lower_fatal_threshold));
	ADD_ATTRIBUTE(i, a, UPPERTHRESHOLDFATAL_KEY, framework::SINT32 ,
		decodeIfTemp(sensor.type, sensor.settings.upper_fatal_threshold));

	framework::UINT16_LIST settableThresholds;
	if (sensor.lower_noncritical_settable)
	{
		settableThresholds.push_back(SENSOR_LOWER_NONCRITICAL_THRESHOLD);
	}
	if (sensor.upper_noncritical_settable)
	{
		settableThresholds.push_back(SENSOR_UPPER_NONCRITICAL_THRESHOLD);
	}
	ADD_ATTRIBUTE(i, a, SETTABLETHRESHOLDS_KEY, framework::UINT16_LIST, settableThresholds);

	framework::UINT16_LIST supportedThresholds;
	if (sensor.lower_noncritical_support)
	{
		supportedThresholds.push_back(SENSOR_LOWER_NONCRITICAL_THRESHOLD);
	}
	if (sensor.upper_noncritical_support)
	{
		supportedThresholds.push_back(SENSOR_UPPER_NONCRITICAL_THRESHOLD);
	}
	if (sensor.lower_critical_support)
	{
		supportedThresholds.push_back(SENSOR_LOWER_CRITICAL_THRESHOLD);
	}
	if (sensor.upper_critical_support)
	{
		supportedThresholds.push_back(SENSOR_UPPER_CRITICAL_THRESHOLD);
	}
	if (sensor.upper_fatal_support)
	{
		supportedThresholds.push_back(SENSOR_UPPER_FATAL_THRESHOLD);
	}
	ADD_ATTRIBUTE(i, a, SUPPORTEDTHRESHOLDS_KEY, framework::UINT16_LIST, supportedThresholds);
	framework::STR_LIST sensorStates;
	sensorStates.push_back(getSensorStateStr(SENSOR_UNKNOWN));
	sensorStates.push_back(getSensorStateStr(SENSOR_NORMAL));
	if (sensor.lower_noncritical_support || sensor.upper_noncritical_support)
	{
		sensorStates.push_back(getSensorStateStr(SENSOR_NONCRITICAL));
	}
	if (sensor.lower_critical_support || sensor.upper_critical_support)
	{
		sensorStates.push_back(getSensorStateStr(SENSOR_CRITICAL));
	}
	if (sensor.lower_fatal_support || sensor.upper_fatal_support)
	{
		sensorStates.push_back(getSensorStateStr(SENSOR_FATAL));
	}

	ADD_ATTRIBUTE(i, a, POSSIBLESTATES_KEY, framework::STR_LIST, sensorStates);
	ADD_ATTRIBUTE(i, a, CURRENTSTATE_KEY, framework::STR, getSensorStateStr(sensor.current_state));
	framework::UINT16 enabledState = 0;
	// EnabledState only applies to temp and spare capacity sensors
	if (isTempSensorType (sensor_type) || sensor_type == SENSOR_SPARECAPACITY)
	{
		enabledState = (sensor.settings.enabled) ?
					   SENSOR_ENABLEDSTATE_ENABLED : SENSOR_ENABLEDSTATE_DISABLED;
	}
	else
	{
		enabledState = SENSOR_ENABLEDSTATE_NA;
	}
	framework::Attribute enumAttribute(enabledState, getSensorEnabledString(enabledState), false);
	i.setAttribute(ENABLEDSTATE_KEY, enumAttribute, a);
}

/*
 * Return the object paths for the NVDIMMSensor class.
 */
framework::instance_names_t *NVDIMMSensorFactory::getNames()
throw(framework::Exception)
{

	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	int rc = NVM_ERR_UNKNOWN;
	struct sensor sensors[NVM_MAX_DEVICE_SENSORS];
	framework::instance_names_t *pNames = new framework::instance_names_t();

	try
	{
		std::string hostName = server::getHostName();

		std::vector<std::string> manageableDevices = physical_asset::NVDIMMFactory::getManageableDeviceUids();
		for (size_t dimmIdx = 0; dimmIdx < manageableDevices.size(); dimmIdx++)
		{
			std::string uidStr = manageableDevices[dimmIdx];
			NVM_UID uid;
			uid_copy(uidStr.c_str(), uid);

			rc = nvm_get_sensors(uid, sensors, NVM_MAX_DEVICE_SENSORS);
			if (rc != NVM_SUCCESS)
			{
				throw exception::NvmExceptionLibError(rc);
			}
			for (int sensorIdx = 0; sensorIdx < NVM_MAX_DEVICE_SENSORS; sensorIdx++)
			{
				framework::ObjectPath path = getSensorPath(sensors[sensorIdx].type, hostName,
					uidStr);

				pNames->push_back(path);
			}
		}
	}
	catch (framework::Exception &) // clean up and re-throw
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
 * Return the object paths for the NVDIMMSensor class.
 */
framework::instance_names_t *NVDIMMSensorFactory::getInstanceNames()
throw(framework::Exception)
{
	return getNames();
}

framework::ObjectPath NVDIMMSensorFactory::getSensorPath(const int type,
	const std::string &hostname, const std::string &dimmUid)
throw(framework::Exception)
{
	std::string sensorTypeName;
	sensorTypeName = getCIMSensorDeviceName(type);

	framework::attributes_t keys;
	keys[SYSTEMCREATIONCLASSNAME_KEY] =
		framework::Attribute(server::BASESERVER_CREATIONCLASSNAME, true);

	keys[SYSTEMNAME_KEY] =
		framework::Attribute(std::string(hostname), true);

	keys[CREATIONCLASSNAME_KEY] =
		framework::Attribute(NVDIMMSENSOR_CREATIONCLASSNAME, true);

	keys[DEVICEID_KEY] =
		framework::Attribute(dimmUid + sensorTypeName, true);

	return framework::ObjectPath(hostname, NVM_NAMESPACE,
		NVDIMMSENSOR_CREATIONCLASSNAME, keys);
}

/*
 * Modify an NVMDIMMSensor instance.  Only the temp and spare capacities can change.  And on those
 * only the upper and lower thresholds respectively can change.
 */
framework::Instance *NVDIMMSensorFactory::modifyInstance(framework::ObjectPath &path,
	framework::attributes_t &attributes)
throw(framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = NULL; // return value

	// get the device UID and sensor type from the device ID
	framework::Attribute deviceIdAttribute = path.getKeyValue(DEVICEID_KEY);
	std::string uidStr;
	int type = 0;
	if (!splitDeviceIdAttribute(deviceIdAttribute, uidStr, type))
	{
		throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
	}

	try
	{
		if (type == SENSORTYPE_MEDIATEMPERATURE || type == SENSORTYPE_SPARECAPACITY
			|| type == SENSORTYPE_CONTROLLER_TEMPERATURE)
		{
			framework::attribute_names_t modifyableAttributes;
			modifyableAttributes.push_back(ENABLEDSTATE_KEY);
			if (type == SENSORTYPE_MEDIATEMPERATURE || type == SENSORTYPE_CONTROLLER_TEMPERATURE)
			{
				modifyableAttributes.push_back(UPPERTHRESHOLDNONCRITICAL_KEY);
			}
			else
			{
				modifyableAttributes.push_back(LOWERTHRESHOLDNONCRITICAL_KEY);
			}

			framework::attribute_names_t attributeNames;
			pInstance = getInstance(path, attributeNames);

			checkAttributesAreModifiable(pInstance, attributes, modifyableAttributes);

			if (pInstance)
			{
				updateSensor(uidStr, type, attributes, pInstance);
			}
		}
		else
		{
			COMMON_LOG_ERROR("Only temperature and spare capacity sensors can be changed");
			throw framework::ExceptionBadParameter(DEVICEID_KEY.c_str());
		}
	}
	catch (framework::Exception &)
	{
		if (pInstance) // clean up
		{
			delete pInstance;
		}
		throw;
	}
	return pInstance;
}

bool NVDIMMSensorFactory::getModifiableAttribute(const std::string &attributeName,
	const framework::attributes_t &attributes,
	const framework::Instance *const pInstance,
	framework::Attribute &currentAttribute,
	framework::Attribute &newAttribute)
throw(framework::Exception)
{
	if (pInstance && pInstance->getAttribute(attributeName, currentAttribute)
					 != framework::SUCCESS)
	{
		COMMON_LOG_ERROR_F("Failed to get attribute '%s' from NVDIMMSensor",
			ENABLEDSTATE_KEY.c_str());
		throw framework::Exception(TR("An internal error occurred."));
	}

	bool found = false;
	if (attributes.find(attributeName) != attributes.end())
	{
		found = true;
		newAttribute = attributes.at(attributeName);
	}

	return found;
}

void NVDIMMSensorFactory::updateSensor(const std::string &dimmUid,
	const int type,
	const framework::attributes_t &attributes,
	framework::Instance *pInstance)
{
	if (!pInstance)
	{
		throw framework::ExceptionBadParameter("pInstance");
	}

	// Create/initialize the settings structure
	struct sensor_settings settings;
	memset(&settings, 0, sizeof(settings));

	// Utility variables
	bool changed = false;
	bool modifiablePropFound = false; // expect to find at least one
	framework::Attribute newAttribute;
	framework::Attribute currentAttribute;

	// Get current settings for the sensor
	struct sensor sensor;
	memset(&sensor, 0, sizeof(sensor));

	// EnabledState attribute may have changed
	if (getModifiableAttribute(ENABLEDSTATE_KEY, attributes, pInstance, currentAttribute,
		newAttribute))
	{
		modifiablePropFound = true;

		// Changed the value
		NVM_UINT16 oldEnabledState = (NVM_UINT16) currentAttribute.uintValue();
		NVM_UINT16 enabledState = (NVM_UINT16) newAttribute.uintValue();

		switch (enabledState)
		{
			case SENSOR_ENABLEDSTATE_ENABLED:
				settings.enabled = 1;
				break;
			case SENSOR_ENABLEDSTATE_DISABLED:
				settings.enabled = 0;
				break;
			default:
				COMMON_LOG_ERROR_F("'%hu' is not a valid value for '%s'", enabledState,
					ENABLEDSTATE_KEY.c_str());
		}

		if (oldEnabledState != enabledState)
		{
			COMMON_LOG_INFO_F("DeviceId %s - %s changing to %hu",
				dimmUid.c_str(),
				ENABLEDSTATE_KEY.c_str(),
				enabledState);

			changed = true;
			pInstance->setAttribute(ENABLEDSTATE_KEY, newAttribute);
		}
	}
	else // no new value - use the old one
	{
		settings.enabled = (NVM_BOOL) (currentAttribute.boolValue() ? 1 : 0);
	}

	// Threshold may have changed
	// set which threshold attribute is modifiable based on the sensor type
	std::string thresholdAttribute =
		(type == SENSORTYPE_MEDIATEMPERATURE || type == SENSORTYPE_CONTROLLER_TEMPERATURE) ?
		UPPERTHRESHOLDNONCRITICAL_KEY : // temp
		LOWERTHRESHOLDNONCRITICAL_KEY; // spare
	if (getModifiableAttribute(thresholdAttribute, attributes, pInstance, currentAttribute,
		newAttribute))
	{
		modifiablePropFound = true;
		// only change if no old value or new value is different than old value
		if (currentAttribute.intValue() != newAttribute.intValue())
		{
			COMMON_LOG_INFO_F("DeviceId %s - %s changing to %d",
				dimmUid.c_str(),
				thresholdAttribute.c_str(),
				newAttribute.intValue());

			// Threshold values come in as signed ints but must be converted to uint64
			if (thresholdAttribute == UPPERTHRESHOLDNONCRITICAL_KEY)
			{
				if (type == SENSOR_MEDIA_TEMPERATURE || type == SENSORTYPE_CONTROLLER_TEMPERATURE)
				{
					settings.upper_noncritical_threshold = tempAttributeToNvmValue(newAttribute);
				}
			}
			else // it's the lower threshold
			{
				settings.lower_noncritical_threshold = newAttribute.uint64Value();
			}

			changed = true;
			pInstance->setAttribute(thresholdAttribute, newAttribute);
		}
	}
	else // no new value - use the old one
	{
		if (thresholdAttribute == UPPERTHRESHOLDNONCRITICAL_KEY)
		{
			if (type == SENSOR_MEDIA_TEMPERATURE || type == SENSORTYPE_CONTROLLER_TEMPERATURE)
			{
				settings.upper_noncritical_threshold = tempAttributeToNvmValue(currentAttribute);
			}
		}
		else // it's the lower threshold
		{
			settings.lower_noncritical_threshold = currentAttribute.uint64Value();
		}
	}

	if (!modifiablePropFound)
	{
		COMMON_LOG_WARN_F("Tried to modify '%s' without a modifiable property",
			dimmUid.c_str());
	}

	// If any of the values were modified, update the settings
	if (changed)
	{
		NVM_UID uid;
		uid_copy(dimmUid.c_str(), uid);
		int rc = nvm_set_sensor_settings(uid, (enum sensor_type) type, &settings);
		if (rc != NVM_SUCCESS)
		{
			throw exception::NvmExceptionLibError(rc);
		}
	}
}

NVM_UINT32 NVDIMMSensorFactory::tempAttributeToNvmValue(const framework::Attribute &attribute)
{
	return nvm_encode_temperature(((framework::REAL32)attribute.intValue() / SENSOR_TEMP_MODIFIER));
}

std::string NVDIMMSensorFactory::getSensorEnabledString(const NVM_UINT16 enabledState)
{
	std::string result;

	switch (enabledState)
	{
		case SENSOR_ENABLEDSTATE_DISABLED:
			result = TR("Disabled");
			break;
		case SENSOR_ENABLEDSTATE_ENABLED:
			result = TR("Enabled");
			break;
		case SENSOR_ENABLEDSTATE_NA:
			result = TR(NA.c_str());
			break;
		default:
			result = TR("Unknown");
	}

	return result;
}

/*
 * Determine if the instances are associated based on the AssociatedSensor association class.
 * Because the Sensor deviceID Attribute has the sensor type appended to the Device UID,
 * this is a little more complex matching.
 */
bool NVDIMMSensorFactory::isAssociated(const std::string &associationClass,
	framework::Instance *pAntInstance,
	framework::Instance *pDepInstance)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	bool result = false;

	if (associationClass == framework_interface::ASSOCIATION_CLASS_ASSOCIATEDSENSOR)
	{
		std::vector<std::string> stringsToRemove;
		for (cimSensorDescriptionsMap::const_iterator iter = cimSensorDescriptions.begin();
			 iter != cimSensorDescriptions.end(); iter++)
		{
			stringsToRemove.push_back(iter->second.deviceName);
		}
		result = framework_interface::NvmAssociationFactory::filteredFkMatch(
			pAntInstance, DEVICEID_KEY, stringsToRemove,
			pDepInstance, TAG_KEY, std::vector<std::string>());
	}
	else
	{
		COMMON_LOG_WARN_F("Cannot calculate if instances are an association "
			"based on association class: %s", associationClass.c_str());
	}

	return result;
}

framework::SINT32 NVDIMMSensorFactory::decodeIfTemp(const sensor_type &type, const NVM_UINT64 &value)
{
	framework::SINT32 result = (framework::SINT32)value;
	if (isTempSensorType(type))
	{
		result = nvmTempToCimTemp (value);
	}

	return result;
}

bool NVDIMMSensorFactory::isTempSensorType(const sensor_type &type)
{
	return type == SENSOR_CONTROLLER_TEMPERATURE || type == SENSOR_MEDIA_TEMPERATURE;
}

framework::SINT32 NVDIMMSensorFactory::nvmTempToCimTemp(const NVM_UINT64 &value)
{
	return realTempToCimTemp(nvm_decode_temperature((NVM_UINT32)value));
}

framework::SINT32 NVDIMMSensorFactory::realTempToCimTemp(const framework::REAL32 &temp)
{
	return (framework::SINT32)(temp * SENSOR_TEMP_MODIFIER);
}

}
}
