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
 * This file contains the implementation for the InstIndicationFactory class.
 * This factory provides the InstCreation, InstModification, and InstDeletion CIM
 * Indication classes. Because they are so similar this factory is able to provide for
 * all three.
 */

#include <pmem_config/PersistentMemoryNamespaceFactory.h>
#include <server/BaseServerFactory.h>
#include <persistence/event.h>
#include <physical_asset/NVDIMMFactory.h>
#include <LogEnterExit.h>
#include <support/NVDIMMSensorFactory.h>
#include <core/exceptions/NoMemoryException.h>
#include <libinvm-cim/ExceptionNoMemory.h>

#include "InstIndicationFactory.h"

wbem::framework::Instance *wbem::indication::InstIndicationFactory::createIndication(
		struct event *pEvent)
throw(framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::Instance *pResult = NULL;

	if (pEvent == NULL)
	{
		COMMON_LOG_ERROR("pEvent was NULL");
	}
	else
	{
		COMMON_LOG_DEBUG_F("Event Type: %d, Event Code: %d", (int)pEvent->type, (int)pEvent->code);
		try
		{
			if (isNamespaceEvent(pEvent))
			{
				pResult = createNamespaceIndication(pEvent);
			}
			else if (isDeviceEvent(pEvent))
			{
				pResult = createDeviceIndication(pEvent);
			}
			else if (isSensorEvent(pEvent))
			{
				pResult = createSensorIndication(pEvent);
			}
		}
		catch (core::NoMemoryException)
		{
			throw framework::ExceptionNoMemory(__FILE__, __FUNCTION__, "Could not allocate memory");
		}
	}

	return pResult;
}

/*
 * Create an Instance that represents an indication.
 * 		className: name of the indication. (InstCreation, InstDeletion, InstModification)
 * 		pSourcePath: object path to the instance being created, deleted, modified
 * 		pSource: instance being created, deleted, modified
 * 		pPrevious: if is an InstModification, this is the instance before being modified
 * 		pChangedProps: if is an InstModification, this is the list of properties changed
 *
 */
wbem::framework::Instance *wbem::indication::InstIndicationFactory::createIndicationInstance(
		const std::string &className,
		const NVM_UINT64 indicationTime,
		framework::ObjectPath *pSourcePath,
		framework::Instance *pSource,
		framework::Instance *pPrevious,
		framework::STR_LIST * pChangedProps)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::framework::attributes_t keys;
	wbem::framework::ObjectPath path(wbem::server::getHostName(),
			wbem::NVM_NAMESPACE, className, keys);
	framework::Instance *pResult = new wbem::framework::Instance(path);
	if (pSource  == NULL)
	{
		COMMON_LOG_ERROR("pSource is NULL");
	}
	else if (pSourcePath == NULL)
	{
		COMMON_LOG_ERROR("pSourcePath is NULL");
	}
	else
	{
		// SourceInstance
		framework::Attribute targetAttribute = framework::Attribute(pSource->getCimXml(),	false);
		targetAttribute.setIsEmbedded(true);
		pResult->setAttribute(SOURCEINSTANCE, targetAttribute);

#ifdef __WINDOWS__
		// Unique value that indicates the time at which the event was generated. This is a 64-bit value that represents
		// the number of 100-nanosecond intervals after January 1, 1601. The information is in the
		// Coordinated Universal Time (UTC) format.
		pResult->setAttribute(TIMECREATED_KEY, framework::Attribute((NVM_UINT64)indicationTime,
				wbem::framework::DATETIME_SUBTYPE_DATETIME, false));

		if (pPrevious) // must be an __InstancetModificationEvent
		{
			framework::Attribute previousAttribute = framework::Attribute(pPrevious->getCimXml(), false);
			previousAttribute.setIsEmbedded(true);
			pResult->setAttribute(PREVIOUSINSTANCE_KEY, previousAttribute);
		}
#else

		// Indication Time
		pResult->setAttribute(INDICATIONTIME_KEY, framework::Attribute((NVM_UINT64)indicationTime,
				wbem::framework::DATETIME_SUBTYPE_DATETIME, false));

		// SourceInstanceModelPath
		pResult->setAttribute(SOURCEINSTANCEMODELPATH_KEY,
				framework::Attribute(pSourcePath->asString(), false));

		if (pPrevious && pChangedProps) // must be an instModificationEvent
		{
			pResult->setAttribute(CHANGEDPROPERTYNAMES_KEY,
					framework::Attribute(*pChangedProps, false));

			framework::Attribute previousInstance(pPrevious->getCimXml(), false);
			previousInstance.setIsEmbedded(true);
			pResult->setAttribute(PREVIOUSINSTANCE_KEY, previousInstance);
		}
#endif
	}

	return pResult;
}

wbem::framework::Instance *wbem::indication::InstIndicationFactory::createNamespaceIndication(
		event *pEvent)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::Instance *pResult = NULL;
	pmem_config::PersistentMemoryNamespaceFactory nsFactory;

	framework::ObjectPath nsPath;
	nsFactory.createPathFromUid(pEvent->uid, nsPath);

	std::string className;
	framework::Instance *pSourceInstance = NULL;
	if (isNamespaceCreation(pEvent))
	{
		className = INSTCREATION_CLASSNAME;
		framework::attribute_names_t attributes;
		pSourceInstance = nsFactory.getInstance(nsPath, attributes);
	}
	else if (isNamespaceDeletion(pEvent))
	{
		className = INSTDELETION_CLASSNAME;
		pSourceInstance = new framework::Instance(nsPath);
	}

	if (pSourceInstance)
	{
		pResult = createIndicationInstance(className, (COMMON_UINT64) pEvent->time,
				&nsPath, pSourceInstance);
		delete pSourceInstance;
	}
	return pResult;
}

wbem::framework::Instance *wbem::indication::InstIndicationFactory::createDeviceIndication(
		event *pEvent)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	framework::Instance *pResult = NULL;
	framework::ObjectPath path;
	physical_asset::NVDIMMFactory nvdimmFactory;
	nvdimmFactory.createPathFromUid(pEvent->uid, path);
	framework::Instance *pSourceInstance  = NULL;
	std::string className;
	if (isDeviceCreation(pEvent))
	{
		physical_asset::NVDIMMFactory dimmFactory;
		framework::attribute_names_t attributes;
		pSourceInstance = dimmFactory.getInstance(path, attributes);
		className = INSTCREATION_CLASSNAME;
	}
	else if (isDeviceMissing(pEvent))
	{
		className = INSTDELETION_CLASSNAME;
		pSourceInstance = new framework::Instance(path);
	}

	if (pSourceInstance)
	{
		pResult = createIndicationInstance(className, (COMMON_UINT64) pEvent->time, &path, pSourceInstance);
		delete pSourceInstance;
	}

	return pResult;
}

bool wbem::indication::InstIndicationFactory::isNamespaceEvent(event *pEvent)
{
	return isNamespaceCreation(pEvent) || isNamespaceDeletion(pEvent);
}

bool wbem::indication::InstIndicationFactory::isNamespaceCreation(event *pEvent)
{
	return (pEvent->type == EVENT_TYPE_MGMT) &&
			(pEvent->code == EVENT_CODE_MGMT_NAMESPACE_CREATED);
}

bool wbem::indication::InstIndicationFactory::isNamespaceDeletion(event *pEvent)
{
	return (pEvent->type == EVENT_TYPE_MGMT) &&
			(pEvent->code == EVENT_CODE_MGMT_NAMESPACE_DELETED);
}


bool wbem::indication::InstIndicationFactory::isDeviceEvent(event *pEvent)
{
	return isDeviceCreation(pEvent) || isDeviceMissing(pEvent);
}

bool wbem::indication::InstIndicationFactory::isDeviceCreation(event *pEvent)
{
	return (pEvent->type == EVENT_TYPE_CONFIG) &&
			(pEvent->code == EVENT_CODE_CONFIG_TOPOLOGY_ADDED_NEW_DEVICE);
}


bool wbem::indication::InstIndicationFactory::isDeviceMissing(event *pEvent)
{
	return (pEvent->type == EVENT_TYPE_CONFIG) &&
			(pEvent->code == EVENT_CODE_CONFIG_TOPOLOGY_MISSING_DEVICE);
}

bool wbem::indication::InstIndicationFactory::isSensorEvent(event *pEvent)
{
	return (pEvent->type == EVENT_TYPE_HEALTH) && (canGetEventSensorType(pEvent, NULL));
}

wbem::framework::Instance *wbem::indication::InstIndicationFactory::createSensorIndication(
		event *pEvent)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	wbem::support::NVDIMMSensorFactory factory;
	framework::Instance *pResult = NULL;

	std::string dimmUid(pEvent->args[0]);
	enum sensor_type sensorType;
	if (canGetEventSensorType(pEvent, &sensorType))
	{
		// need sensor path to get sensor instances for the indication
		framework::ObjectPath sensorPath = factory.getSensorPath(
				sensorType, server::getHostName(), dimmUid);
		framework::attribute_names_t attributes;
		attributes.push_back(CURRENTSTATE_KEY);
		framework::Instance *pSourceInstance = factory.getInstance(sensorPath, attributes);
		framework::Instance *pPreviousInstance = factory.getInstance(sensorPath, attributes);
		changeCurrentStateToPrevious(pPreviousInstance);

		framework::STR_LIST changedProps;
		changedProps.push_back(CURRENTSTATE_KEY);


		pResult = createIndicationInstance(INSTMODIFICATION_CLASSNAME, (COMMON_UINT64)pEvent->time,
				&sensorPath, pSourceInstance, pPreviousInstance, &changedProps);
	}

	return pResult;
}

bool wbem::indication::InstIndicationFactory::canGetEventSensorType(const struct event *pEvent,
		enum sensor_type *pSensorType)
{
	// These need to match what's in EventMonitor.h
	std::string ERASURE_CODED = "erasure coded";
	std::string CORRECTED = "corrected";
	std::string UNCORRECTABLE = "uncorrectable";

	bool eventCodeIsSensor = true;
	enum sensor_type sensorType = (enum sensor_type)0;
	switch(pEvent->code)
	{
	case EVENT_CODE_HEALTH_LOW_SPARE_CAPACITY:
		sensorType = SENSOR_SPARECAPACITY;
		break;
	case EVENT_CODE_HEALTH_MEDIA_TEMPERATURE_OVER_THRESHOLD:
		sensorType = SENSOR_MEDIA_TEMPERATURE;
		break;
	case EVENT_CODE_HEALTH_MEDIA_TEMPERATURE_UNDER_THRESHOLD:
		sensorType = SENSOR_MEDIA_TEMPERATURE;
		break;
	case EVENT_CODE_HEALTH_HIGH_WEARLEVEL:
		sensorType = SENSOR_WEARLEVEL;
		break;
	case EVENT_CODE_HEALTH_NEW_MEDIAERRORS_FOUND:
		// arg[1] should be media error type
		if (pEvent->args[1] == ERASURE_CODED)
		{
			sensorType = SENSOR_MEDIAERRORS_ERASURECODED;
		}
		else if (pEvent->args[1] == CORRECTED)
		{
			sensorType = SENSOR_MEDIAERRORS_CORRECTED;
		}
		else if (pEvent->args[1] == UNCORRECTABLE)
		{
			sensorType = SENSOR_MEDIAERRORS_UNCORRECTABLE;
		}
		else
		{
			eventCodeIsSensor = false;
		}

		break;
	case EVENT_CODE_HEALTH_UNSAFE_SHUTDOWN:
		sensorType = SENSOR_UNSAFESHUTDOWNS;
		break;
	case EVENT_CODE_HEALTH_CONTROLLER_TEMPERATURE_OVER_THRESHOLD:
		sensorType = SENSOR_CONTROLLER_TEMPERATURE;
		break;
	case EVENT_CODE_HEALTH_CONTROLLER_TEMPERATURE_UNDER_THRESHOLD:
		sensorType = SENSOR_CONTROLLER_TEMPERATURE;
		break;
	default:
		eventCodeIsSensor = false;
		break;
	}

	if (pSensorType != NULL)
	{
		*pSensorType = sensorType;
	}

	return eventCodeIsSensor;
}

void wbem::indication::InstIndicationFactory::changeCurrentStateToPrevious(
		framework::Instance* pPreviousInstance)
{
	std::string normalState = support::NVDIMMSensorFactory::getSensorStateStr(SENSOR_NORMAL);
	std::string criticalState = support::NVDIMMSensorFactory::getSensorStateStr(SENSOR_CRITICAL);

	framework::Attribute currentStateAttribute;
	pPreviousInstance->getAttribute(CURRENTSTATE_KEY, currentStateAttribute);
	std::string currentState = currentStateAttribute.stringValue();

	// don't actually know previous state, but because Sensor can really only be in normal
	// or critical states, going to make an educated guess.
	std::string prevState = currentState == normalState ? criticalState : normalState;

	pPreviousInstance->setAttribute(CURRENTSTATE_KEY,
			framework::Attribute(prevState, false));
}
