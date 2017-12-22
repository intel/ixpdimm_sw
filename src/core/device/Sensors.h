/*
 * Copyright (c) 2017, Intel Corporation
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
#ifndef CR_MGMT_SENSORS_H
#define CR_MGMT_SENSORS_H


#include <nvm_management.h>
#include <core/Helper.h>
#include <common_types.h>
#include <system/jedec_manufacturer.h>
#include <utility.h>
#include <nvm_types.h>

#include <string>
#include <vector>
#include <string/s_str.h>
#include <sstream>
#include <core/exceptions/LibraryException.h>
#include <core/Collection.h>
#include <core/NvmLibrary.h>
#include <core/ExportCore.h>
#include "SensorCelsius.h"
#include "SensorTime.h"
namespace core
{
	namespace device
	{
		namespace sensor
		{
			class NVM_CORE_API SensorFactory
			{
			public:
				static Sensor* CreateSensor(const struct sensor sensor);
			};

			class NVM_CORE_API SensorMediaTemp: public SensorCelsius
			{
			public:
				SensorMediaTemp();
				SensorMediaTemp(const struct sensor sensor);
				virtual ~SensorMediaTemp();
				virtual std::string GetName();
			};

			class NVM_CORE_API SensorSpareCapacity : public Sensor
			{
			public:
				SensorSpareCapacity();
				SensorSpareCapacity(const struct sensor sensor);
				virtual ~SensorSpareCapacity();
				virtual std::string GetName();
				virtual std::string GetEnabledState();
			protected:
				virtual std::string baseUnit();
			};

			class NVM_CORE_API SensorWearLevel : public Sensor
			{
			public:
				SensorWearLevel();
				SensorWearLevel(const struct sensor sensor);
				virtual ~SensorWearLevel();
				virtual std::string GetName();
			protected:
				virtual std::string baseUnit();
			};

			class NVM_CORE_API SensorPwrCycles : public Sensor
			{
			public:
				SensorPwrCycles();
				SensorPwrCycles(const struct sensor sensor);
				virtual ~SensorPwrCycles();
				virtual std::string GetName();
			};

			class NVM_CORE_API SensorPwrOnTime : public SensorTime
			{
			public:
				SensorPwrOnTime();
				SensorPwrOnTime(const struct sensor sensor);
				virtual ~SensorPwrOnTime();
				virtual std::string GetName();
			};

			class NVM_CORE_API SensorUpTime : public SensorTime
			{
			public:
				SensorUpTime();
				SensorUpTime(const struct sensor sensor);
				virtual ~SensorUpTime();
				virtual std::string GetName();
			};

			class NVM_CORE_API SensorUnsafeShutdowns : public Sensor
			{
			public:
				SensorUnsafeShutdowns();
				SensorUnsafeShutdowns(const struct sensor sensor);
				virtual ~SensorUnsafeShutdowns();
				virtual std::string GetName();
			};

			class NVM_CORE_API SensorFwErrorLogCnt : public Sensor
			{
			public:
				SensorFwErrorLogCnt();
				SensorFwErrorLogCnt(const struct sensor sensor);
				virtual ~SensorFwErrorLogCnt();
				virtual std::string GetName();
			};

			class NVM_CORE_API SensorPwrLimited : public Sensor
			{
			public:
				SensorPwrLimited();
				SensorPwrLimited(const struct sensor sensor);
				virtual ~SensorPwrLimited();
				virtual std::string GetName();
			};

			class NVM_CORE_API SensorControllerTemp : public SensorCelsius
			{
			public:
				SensorControllerTemp();
				SensorControllerTemp(const struct sensor sensor);
				virtual ~SensorControllerTemp();
				virtual std::string GetName();
			};

			class NVM_CORE_API SensorHealth : public Sensor
			{
			public:
				SensorHealth();
				SensorHealth(const struct sensor sensor);
				virtual ~SensorHealth();
				virtual std::string GetName();
			};
		}
	}
}
#endif //CR_MGMT_SENSORS_H
