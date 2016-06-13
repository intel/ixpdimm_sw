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
 * This file contains the definition for the InstIndicationFactory class.
 * This factory provides the InstCreation, InstModification, and InstDeletion CIM
 * Indication classes. Because they are so similar this factory is able to provide for
 * all three.
 */


#ifndef _WBEM_INDICATION_INSTINDICATION_FACTORY_H_
#define _WBEM_INDICATION_INSTINDICATION_FACTORY_H_

#include "NvmIndicationFactory.h"
#include <libinvm-cim/ObjectPath.h>
#include <libinvm-cim/Types.h>
#include <NvmStrings.h>

namespace  wbem
{
namespace indication
{

// Indication class names
// Note: Inst Indication classes are pretty much the same between WMI and CMPI. The class names and
// attribute names are different, but the intentions are the same. On WMI they are "Intrinsic"
// classes so they cannot be changed.
#ifdef __WINDOWS__
static const std::string INSTCREATION_CLASSNAME = "__InstanceCreationEvent";
static const std::string INSTDELETION_CLASSNAME = "__InstanceDeletionEvent";
static const std::string INSTMODIFICATION_CLASSNAME = "__InstanceModificationEvent";
static const std::string SOURCEINSTANCE = TARGETINSTANCE_KEY;
#else
static const std::string INSTCREATION_CLASSNAME = std::string(NVM_WBEM_PREFIX) + "InstCreation";
static const std::string INSTDELETION_CLASSNAME = std::string(NVM_WBEM_PREFIX) + "InstDeletion";
static const std::string INSTMODIFICATION_CLASSNAME = std::string(NVM_WBEM_PREFIX) + "InstModification";

static const std::string SOURCEINSTANCE = SOURCEINSTANCE_KEY;

#endif


class NVM_API InstIndicationFactory : public NvmIndicationFactory
{
public:
	/*!
	 * Override the createIndication method. Converts an event to an appropriate indication
	 * instance.
	 */
	framework::Instance *createIndication(struct event *pEvent)
	throw (framework::Exception);

private:
	/*!
	 * Is the event related to a namespace
	 */
	bool isNamespaceEvent(event *pEvent);

	/*!
	 * Is the event a namespace creation event
	 */
	bool isNamespaceCreation(event *pEvent);

	/*!
	 * Is the event a namespace deletion event
	 */
	bool isNamespaceDeletion(event *pEvent);

	/*!
	 * Is the event related to a device
	 */
	bool isDeviceEvent(event *pEvent);

	/*!
	 * Is the event a device creation event
	 */
	bool isDeviceCreation(event *pEvent);

	/*!
	 * Is the event a device missing event
	 */
	bool isDeviceMissing(event *pEvent);

	/*!
	 * Is the event related to sensors
	 */

	bool isSensorEvent(event *pEvent);

	/*!
	 * Get the sensor_type from the event. It will be used to construct the sensor indication.
	 * Returns true if the event code is not related to a sensor.
	 */
	bool canGetEventSensorType(const struct event *pEvent, enum sensor_type *pSensorType);
	/*!
	 * Construct an Instance of an InstModification/InstCreation/InstDeletion indication
	 */
	wbem::framework::Instance * createIndicationInstance(
			const std::string &className,
			const NVM_UINT64 indicationTime,
			framework::ObjectPath *pSourcePath,
			framework::Instance *pSource,
			framework::Instance *pPrevious = NULL,
			framework::STR_LIST * pChangedProps = NULL);

	/*!
	 * Create a namespace indication from the event
	 */
	framework::Instance *createNamespaceIndication(event *pEvent);

	/*!
	 * Create a device indication from the event
	 */
	framework::Instance *createDeviceIndication(event *pEvent);

	/*!
	 * Create a sensor indication from the event
	 */
	framework::Instance *createSensorIndication(event *pEvent);

	void changeCurrentStateToPrevious(framework::Instance *pPreviousInstance);

};

};

};

#endif /* _WBEM_INDICATION_INSTINDICATION_FACTORY_H_ */
