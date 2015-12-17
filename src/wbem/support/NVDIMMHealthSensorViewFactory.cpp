/*
 * Copyright (c) 2015, Intel Corporation
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
 * This file contains the provider for the NVDIMMHealthSensorView instances
 * which aggregates the sensors for an NVM DIMM in one view.
 */

#include "NVDIMMHealthSensorViewFactory.h"
#include <physical_asset/NVDIMMFactory.h>
#include <server/BaseServerFactory.h>

wbem::support::NVDIMMHealthSensorViewFactory::NVDIMMHealthSensorViewFactory()
throw (wbem::framework::Exception)
{ }

wbem::support::NVDIMMHealthSensorViewFactory::~NVDIMMHealthSensorViewFactory()
{ }

void wbem::support::NVDIMMHealthSensorViewFactory::populateAttributeList(framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	attributes.push_back(INSTANCEID_KEY);

	attributes.push_back(MEDIACURRENTTEMPERATURE_KEY);
	attributes.push_back(MEDIATEMPERATUREUNITS_KEY);
	attributes.push_back(MEDIATEMPERATUREUNITMODIFIER_KEY);
	attributes.push_back(MEDIATEMPERATURETHRESHOLD_KEY);
	attributes.push_back(MEDIATEMPERATURESTATE_KEY);
	attributes.push_back(CORECURRENTTEMPERATURE_KEY);
	attributes.push_back(CORETEMPERATUREUNITS_KEY);
	attributes.push_back(CORETEMPERATUREUNITMODIFIER_KEY);
	attributes.push_back(CORETEMPERATURETHRESHOLD_KEY);
	attributes.push_back(CORETEMPERATURESTATE_KEY);
	attributes.push_back(CURRENTSPARELEVEL_KEY);
	attributes.push_back(SPAREUNITS_KEY);
	attributes.push_back(SPAREUNITMODIFIER_KEY);
	attributes.push_back(SPARETHRESHOLD_KEY);
	attributes.push_back(SPARESTATE_KEY);
	attributes.push_back(CURRENTWEARLEVEL_KEY);
	attributes.push_back(WEARUNITS_KEY);
	attributes.push_back(WEARUNITMODIFIER_KEY);
	attributes.push_back(WEARSTATE_KEY);
	attributes.push_back(POWERONTIME_KEY);
	attributes.push_back(POWERONUNITS_KEY);
	attributes.push_back(POWERONMODIFIER_KEY);
	attributes.push_back(POWERONSTATE_KEY);
	attributes.push_back(UPTIME_KEY);
	attributes.push_back(UPTIMEUNITS_KEY);
	attributes.push_back(UPTIMEMODIFIER_KEY);
	attributes.push_back(UPTIMESTATE_KEY);
	attributes.push_back(POWERCYCLECOUNT_KEY);
	attributes.push_back(POWERCYCLEUNITS_KEY);
	attributes.push_back(POWERCYCLEMODIFIER_KEY);
	attributes.push_back(POWERCYCLESTATE_KEY);
	attributes.push_back(UNSAFESHUTDOWNS_KEY);
	attributes.push_back(UNSAFESHUTDOWNUNITS_KEY);
	attributes.push_back(UNSAFESHUTDOWNMODIFIER_KEY);
	attributes.push_back(UNSAFESHUTDOWNSTATE_KEY);
	attributes.push_back(MEDIAERRORSUNCORRECTABLE_KEY);
	attributes.push_back(MEDIAERRORSUNCORRECTABLEUNITS_KEY);
	attributes.push_back(MEDIAERRORSUNCORRECTABLEMODIFIER_KEY);
	attributes.push_back(MEDIAERRORSUNCORRECTABLESTATE_KEY);
	attributes.push_back(POWERLIMITED_KEY);
	attributes.push_back(POWERLIMITEDUNITS_KEY);
	attributes.push_back(POWERLIMITEDMODIFIER_KEY);
	attributes.push_back(POWERLIMITEDSTATE_KEY);
	attributes.push_back(MEDIAERRORSCORRECTED_KEY);
	attributes.push_back(MEDIAERRORSCORRECTEDUNITS_KEY);
	attributes.push_back(MEDIAERRORSCORRECTEDMODIFIER_KEY);
	attributes.push_back(MEDIAERRORSCORRECTEDSTATE_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::support::NVDIMMHealthSensorViewFactory::getInstance(
	framework::ObjectPath &path, framework::attribute_names_t &attributes)
	throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::Instance *pInstance = new framework::Instance(path);

	try
	{
		checkAttributes(attributes);

		std::string guidStr = path.getKeyValue(INSTANCEID_KEY).stringValue();
		NVM_GUID guid;
		str_to_guid(guidStr.c_str(), guid);

		struct sensor sensors[NVM_MAX_DEVICE_SENSORS];
		int rc = NVM_SUCCESS;
		if ((rc = nvm_get_sensors(guid, sensors, NVM_MAX_DEVICE_SENSORS)) != NVM_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to get sensor information for DIMM %s", guidStr.c_str());
			throw exception::NvmExceptionLibError(rc);
		}

		for (int i = 0; i < NVM_MAX_DEVICE_SENSORS; i++)
		{
			switch (sensors[i].type)
			{
				case SENSOR_MEDIA_TEMPERATURE:
					addMediaTemperatureAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_SPARECAPACITY:
					addSpareAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_WEARLEVEL:
					addWearLevelAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_POWERCYCLES:
					addPowerCyclesAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_POWERONTIME:
					addPowerOnAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_UPTIME:
					addUptimeAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_UNSAFESHUTDOWNS:
					addUnsafeShutdownsAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_POWERLIMITED:
					addPowerLimitedAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_MEDIAERRORS_UNCORRECTABLE:
					addUncorrectableMediaErrorsAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_MEDIAERRORS_CORRECTED:
					addCorrectedMediaErrorsAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				case SENSOR_CORE_TEMPERATURE:
					addCoreTemperatureAttributesToInstance(
						*pInstance, attributes, sensors[i]);
					break;
				default:
					break;
			}
		}
	}
	catch (framework::Exception &)
	{
		if (pInstance != NULL)
		{
			delete pInstance;
		}
		throw;
	}

	return pInstance;
}

wbem::framework::instance_names_t* wbem::support::NVDIMMHealthSensorViewFactory::getInstanceNames()
	throw (framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		framework::attributes_t keys;
		std::vector<std::string> guids = physical_asset::NVDIMMFactory::getManageableDeviceGuids();
		for (size_t i = 0; i < guids.size(); i ++)
		{
			keys[INSTANCEID_KEY] = framework::Attribute(std::string(guids[i]), true);
			framework::ObjectPath path(server::getHostName(), NVM_NAMESPACE,
				NVDIMMHEALTHSENSORVIEW_CREATIONCLASSNAME, keys);
			pNames->push_back(path);
		}

	}
	catch (framework::Exception &)
	{
		delete pNames;
		throw;
	}

	return pNames;
}

std::string wbem::support::NVDIMMHealthSensorViewFactory::translateSensorStatus(
	const enum sensor_status current_state)
{
	std::string tempState;
	switch (current_state)
	{
		case SENSOR_CRITICAL:
			tempState = NVDIMMHEALTHSENSORVIEW_CURRENTSTATE_CRITICAL;
			break;
		case SENSOR_NORMAL:
			tempState = NVDIMMHEALTHSENSORVIEW_CURRENTSTATE_NORMAL;
			break;
		default:
			tempState = NVDIMMHEALTHSENSORVIEW_CURRENTSTATE_UNKNOWN;
			break;
	}

	return tempState;
}

void wbem::support::NVDIMMHealthSensorViewFactory::addMediaTemperatureAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(MEDIACURRENTTEMPERATURE_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(MEDIACURRENTTEMPERATURE_KEY, a,  attrNames);
	}
	if (containsAttribute(MEDIATEMPERATUREUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_TEMPERATURE,
			NVDIMMHEALTHSENSORVIEW_UNITS_TEMPERATURE_STR, false);
		instance.setAttribute(MEDIATEMPERATUREUNITS_KEY, a,  attrNames);
	}
	if (containsAttribute(MEDIATEMPERATUREUNITMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(MEDIATEMPERATUREUNITMODIFIER_KEY, a,  attrNames);
	}
	if (containsAttribute(MEDIATEMPERATURETHRESHOLD_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.settings.upper_critical_threshold, false);
		instance.setAttribute(MEDIATEMPERATURETHRESHOLD_KEY, a,  attrNames);
	}
	if (containsAttribute(MEDIATEMPERATURESTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(MEDIATEMPERATURESTATE_KEY, a,  attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addCoreTemperatureAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(CORECURRENTTEMPERATURE_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(CORECURRENTTEMPERATURE_KEY, a,  attrNames);
	}
	if (containsAttribute(CORETEMPERATUREUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_TEMPERATURE,
			NVDIMMHEALTHSENSORVIEW_UNITS_TEMPERATURE_STR, false);
		instance.setAttribute(CORETEMPERATUREUNITS_KEY, a,  attrNames);
	}
	if (containsAttribute(CORETEMPERATUREUNITMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(CORETEMPERATUREUNITMODIFIER_KEY, a,  attrNames);
	}
	if (containsAttribute(CORETEMPERATURETHRESHOLD_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.settings.upper_critical_threshold, false);
		instance.setAttribute(CORETEMPERATURETHRESHOLD_KEY, a,  attrNames);
	}
	if (containsAttribute(CORETEMPERATURESTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(MEDIATEMPERATURESTATE_KEY, a,  attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addSpareAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(CURRENTSPARELEVEL_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(CURRENTSPARELEVEL_KEY, a, attrNames);
	}
	if (containsAttribute(SPAREUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_PERCENTAGE,
			NVDIMMHEALTHSENSORVIEW_UNITS_PERCENTAGE_STR, false);
		instance.setAttribute(SPAREUNITS_KEY, a, attrNames);
	}
	if (containsAttribute(SPAREUNITMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(SPAREUNITMODIFIER_KEY, a, attrNames);
	}
	if (containsAttribute(SPARETHRESHOLD_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.settings.lower_critical_threshold, false);
		instance.setAttribute(SPARETHRESHOLD_KEY, a, attrNames);
	}
	if (containsAttribute(SPARESTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(SPARESTATE_KEY, a, attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addWearLevelAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(CURRENTWEARLEVEL_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(CURRENTWEARLEVEL_KEY, a, attrNames);
	}
	if (containsAttribute(WEARUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_PERCENTAGE,
			NVDIMMHEALTHSENSORVIEW_UNITS_PERCENTAGE_STR, false);
		instance.setAttribute(WEARUNITS_KEY, a, attrNames);
	}
	if (containsAttribute(WEARUNITMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(WEARUNITMODIFIER_KEY, a, attrNames);
	}
	if (containsAttribute(WEARSTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(WEARSTATE_KEY, a, attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addPowerCyclesAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(POWERCYCLECOUNT_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(POWERCYCLECOUNT_KEY, a, attrNames);
	}
	if (containsAttribute(POWERCYCLEUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_CYCLES,
			NVDIMMHEALTHSENSORVIEW_UNITS_CYCLES_STR, false);
		instance.setAttribute(POWERCYCLEUNITS_KEY, a, attrNames);
	}
	if (containsAttribute(POWERCYCLEMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(POWERCYCLEMODIFIER_KEY, a, attrNames);
	}
	if (containsAttribute(POWERCYCLESTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(POWERCYCLESTATE_KEY, a, attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addPowerOnAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(POWERONTIME_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(POWERONTIME_KEY, a, attrNames);
	}
	if (containsAttribute(POWERONUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_SECONDS,
			NVDIMMHEALTHSENSORVIEW_UNITS_SECONDS_STR, false);
		instance.setAttribute(POWERONUNITS_KEY, a, attrNames);
	}
	if (containsAttribute(POWERONMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(POWERONMODIFIER_KEY, a, attrNames);
	}
	if (containsAttribute(POWERONSTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(POWERONSTATE_KEY, a, attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addUptimeAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(UPTIME_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(UPTIME_KEY, a, attrNames);
	}
	if (containsAttribute(UPTIMEUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_SECONDS,
			NVDIMMHEALTHSENSORVIEW_UNITS_SECONDS_STR, false);
		instance.setAttribute(UPTIMEUNITS_KEY, a, attrNames);
	}
	if (containsAttribute(UPTIMEMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(UPTIMEMODIFIER_KEY, a, attrNames);
	}
	if (containsAttribute(UPTIMESTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(UPTIMESTATE_KEY, a, attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addUnsafeShutdownsAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(UNSAFESHUTDOWNS_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(UNSAFESHUTDOWNS_KEY, a, attrNames);
	}
	if (containsAttribute(UNSAFESHUTDOWNUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_OTHER,
			NVDIMMHEALTHSENSORVIEW_UNITS_OTHER_STR, false);
		instance.setAttribute(UNSAFESHUTDOWNUNITS_KEY, a, attrNames);
	}
	if (containsAttribute(UNSAFESHUTDOWNMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(UNSAFESHUTDOWNMODIFIER_KEY, a, attrNames);
	}
	if (containsAttribute(UNSAFESHUTDOWNSTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(UNSAFESHUTDOWNSTATE_KEY, a, attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addPowerLimitedAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(POWERLIMITED_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(POWERLIMITED_KEY, a, attrNames);
	}
	if (containsAttribute(POWERLIMITEDUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_OTHER,
			NVDIMMHEALTHSENSORVIEW_UNITS_OTHER_STR, false);
		instance.setAttribute(POWERLIMITEDUNITS_KEY, a, attrNames);
	}
	if (containsAttribute(POWERLIMITEDMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(POWERLIMITEDMODIFIER_KEY, a, attrNames);
	}
	if (containsAttribute(POWERLIMITEDSTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(POWERLIMITEDSTATE_KEY, a, attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addUncorrectableMediaErrorsAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(MEDIAERRORSUNCORRECTABLE_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(MEDIAERRORSUNCORRECTABLE_KEY, a, attrNames);
	}
	if (containsAttribute(MEDIAERRORSUNCORRECTABLEUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_OTHER,
			NVDIMMHEALTHSENSORVIEW_UNITS_OTHER_STR, false);
		instance.setAttribute(MEDIAERRORSUNCORRECTABLEUNITS_KEY, a, attrNames);
	}
	if (containsAttribute(MEDIAERRORSUNCORRECTABLEMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(MEDIAERRORSUNCORRECTABLEMODIFIER_KEY, a, attrNames);
	}
	if (containsAttribute(MEDIAERRORSUNCORRECTABLESTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(MEDIAERRORSUNCORRECTABLESTATE_KEY, a, attrNames);
	}
}

void wbem::support::NVDIMMHealthSensorViewFactory::addCorrectedMediaErrorsAttributesToInstance(
	framework::Instance &instance,
	framework::attribute_names_t &attrNames,
	struct sensor &sensor)
{
	if (containsAttribute(MEDIAERRORSCORRECTED_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT32)sensor.reading, false);
		instance.setAttribute(MEDIAERRORSCORRECTED_KEY, a, attrNames);
	}
	if (containsAttribute(MEDIAERRORSCORRECTEDUNITS_KEY, attrNames))
	{
		framework::Attribute a(NVDIMMHEALTHSENSORVIEW_UNITS_OTHER,
			NVDIMMHEALTHSENSORVIEW_UNITS_OTHER_STR, false);
		instance.setAttribute(MEDIAERRORSCORRECTEDUNITS_KEY, a, attrNames);
	}
	if (containsAttribute(MEDIAERRORSCORRECTEDMODIFIER_KEY, attrNames))
	{
		framework::Attribute a((NVM_UINT16)0, false);
		instance.setAttribute(MEDIAERRORSCORRECTEDMODIFIER_KEY, a, attrNames);
	}
	if (containsAttribute(MEDIAERRORSCORRECTEDSTATE_KEY, attrNames))
	{
		framework::Attribute a(translateSensorStatus(sensor.current_state), false);
		instance.setAttribute(MEDIAERRORSCORRECTEDSTATE_KEY, a, attrNames);
	}
}
