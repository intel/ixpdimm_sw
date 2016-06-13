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
 * This file contains the provider for the NVDIMMHealthSensorView instances
 * which aggregates the sensors for an NVM DIMM in one view.
 */


#ifndef	_WBEM_SUPPORT_NVDIMMHEALTHSENSORVIEW_FACTORY_H_
#define	_WBEM_SUPPORT_NVDIMMHEALTHSENSORVIEW_FACTORY_H_

#include <string>
#include <uid/uid.h>
#include <nvm_management.h>
#include <LogEnterExit.h>
#include <libinvm-cim/Attribute.h>
#include <libinvm-cim/ObjectPath.h>
#include <exception/NvmExceptionLibError.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <NvmStrings.h>
#include "NVDIMMSensorFactory.h"

namespace wbem
{
namespace support 
{
	static const std::string NVDIMMHEALTHSENSORVIEW_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "NVDIMMHealthSensorView"; //!< Creation Class Name static
	
	static const NVM_UINT16 NVDIMMHEALTHSENSORVIEW_UNITS_TEMPERATURE = 4;
	static const NVM_UINT16 NVDIMMHEALTHSENSORVIEW_UNITS_PERCENTAGE = 65;
	static const NVM_UINT16 NVDIMMHEALTHSENSORVIEW_UNITS_SECONDS = 21;
	static const NVM_UINT16 NVDIMMHEALTHSENSORVIEW_UNITS_CYCLES = 39;
	static const NVM_UINT16 NVDIMMHEALTHSENSORVIEW_UNITS_OTHER = 1;

	static const std::string NVDIMMHEALTHSENSORVIEW_UNITS_TEMPERATURE_STR = "Degrees C"; 
	static const std::string NVDIMMHEALTHSENSORVIEW_UNITS_PERCENTAGE_STR = "Percentage"; // SpareUnits and WearUnits
	static const std::string NVDIMMHEALTHSENSORVIEW_UNITS_SECONDS_STR = "Seconds"; // PowerOnUnits, UpTimeUnits
	static const std::string NVDIMMHEALTHSENSORVIEW_UNITS_CYCLES_STR = "Cycles"; // PowerCycleUnits
	static const std::string NVDIMMHEALTHSENSORVIEW_UNITS_OTHER_STR = "Other"; // UnsafeShutdownUnits, MediaErrorsUncorrectableUnits, PowerLimitedUnits, MediaErrorsCorrectedUnits
	
	static const std::string NVDIMMHEALTHSENSORVIEW_CURRENTSTATE_CRITICAL = "Critical"; 
	static const std::string NVDIMMHEALTHSENSORVIEW_CURRENTSTATE_NORMAL = "Normal"; 
	static const std::string NVDIMMHEALTHSENSORVIEW_CURRENTSTATE_UNKNOWN = "Unknown"; 

/*!
 * Provider Factory for NVDIMMHealthSensorView
 */
class NVM_API NVDIMMHealthSensorViewFactory : public framework_interface::NvmInstanceFactory
{
	public:

		NVDIMMHealthSensorViewFactory() throw (framework::Exception);

		~NVDIMMHealthSensorViewFactory();

		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		std::string translateSensorStatus(const enum sensor_status current_state);

		// Helper functions that adds the non-key attributes
		void addMediaTemperatureAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addSpareAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addWearLevelAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addPowerCyclesAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addPowerOnAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addUptimeAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addUnsafeShutdownsAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addPowerLimitedAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addUncorrectableMediaErrorsAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addCorrectedMediaErrorsAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
		void addControllerTemperatureAttributesToInstance(framework::Instance &instance,
			framework::attribute_names_t &attrNames, struct sensor &sensor);
};

} // support 
} // wbem
#endif  // _WBEM_SUPPORT_NVDIMMHEALTHSENSORVIEW_FACTORY_H_
