/*
 * Copyright (c) 2016, Intel Corporation
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
#ifndef CR_MGMT_SHOWDEVICECOMMAND_H
#define CR_MGMT_SHOWDEVICECOMMAND_H

#include "framework/PropertyDefinitionBase.h"
#include "framework/PropertyDefinitionList.h"
#include "framework/DisplayOptions.h"
#include <libinvm-cli/CliFrameworkTypes.h>
#include <libinvm-cli/ResultBase.h>
#include <libinvm-cli/PropertyListResult.h>
#include <lib/nvm_types.h>
#include <core/device/DeviceService.h>
#include <core/StringList.h>
#include <cli/features/core/framework/CommandBase.h>

namespace cli
{
namespace nvmcli
{
static const std::string ROOT = "Dimm";
static const std::string DIMMID = "DimmID";
static const std::string CAPACITY = "Capacity";
static const std::string LOCKSTATE = "LockState";
static const std::string HEALTHSTATE = "HealthState";
static const std::string FWVERSION = "FWVersion";
static const std::string ACTIONREQUIRED = "ActionRequired";
static const std::string FWAPIVERSION = "FWAPIVersion";
static const std::string INTERFACEFORMATCODE = "InterfaceFormatCode";
static const std::string MANAGEABILITYSTATE = "ManageabilityState";
static const std::string PHYSICALID = "PhysicalID";
static const std::string DIMMHANDLE = "DimmHandle";
static const std::string DIMMUID = "dimmUid";
static const std::string SOCKETID = "SocketID";
static const std::string MEMCONTROLLERID = "MemControllerID";
static const std::string CHANNELID = "ChannelID";
static const std::string CHANNELPOS = "ChannelPos";
static const std::string MEMORYTYPE = "MemoryType";
static const std::string MANUFACTURER = "Manufacturer";
static const std::string MANUFACTURERID = "ManufacturerID";
static const std::string MODEL = "Model";
static const std::string VENDORID = "VendorID";
static const std::string DEVICEID = "DeviceID";
static const std::string REVISIONID = "RevisionID";
static const std::string SERIALNUMBER = "SerialNumber";
static const std::string SUBSYSTEMVENDORID = "SubsystemVendorID";
static const std::string SUBSYSTEMDEVICEID = "SubsystemDeviceID";
static const std::string SUBSYSTEMREVISIONID = "SubsystemRevisionID";
static const std::string MANUFACTURINGINFOVALID = "ManufacturingInfoValid";
static const std::string MANUFACTURINGLOCATION = "ManufacturingLocation";
static const std::string MANUFACTURINGDATE = "ManufacturingDate";
static const std::string ACTIONREQUIREDEVENTS = "ActionRequiredEvents";
static const std::string ISNEW = "IsNew";
static const std::string FORMFACTOR = "FormFactor";
static const std::string MEMORYCAPACITY = "MemoryCapacity";
static const std::string APPDIRECTCAPACITY = "AppDirectCapacity";
static const std::string UNCONFIGUREDCAPACITY = "UnconfiguredCapacity";
static const std::string INACCESSIBLECAPACITY = "InaccessibleCapacity";
static const std::string RESERVEDCAPACITY = "ReservedCapacity";
static const std::string PARTNUMBER = "PartNumber";
static const std::string DEVICELOCATOR = "DeviceLocator";
static const std::string BANKLABEL = "BankLabel";
static const std::string DATAWIDTH = "DataWidth";
static const std::string TOTALWIDTH = "TotalWidth";
static const std::string SPEED = "Speed";
static const std::string FWLOGLEVEL = "FWLogLevel";
static const std::string POWERMANAGEMENTENABLED = "PowerManagementEnabled";
static const std::string POWERLIMIT = "PowerLimit";
static const std::string PEAKPOWERBUDGET = "PeakPowerBudget";
static const std::string AVGPOWERBUDGET = "AvgPowerBudget";
static const std::string DIESPARINGCAPABLE = "DieSparingCapable";
static const std::string DIESPARINGENABLED = "DieSparingEnabled";
static const std::string DIESPARINGLEVEL = "DieSparingLevel";
static const std::string DIESPARESUSED = "DieSparesUsed";
static const std::string LASTSHUTDOWNSTATUS = "LastShutdownStatus";
static const std::string LASTSHUTDOWNTIME = "LastShutdownTime";
static const std::string FIRSTFASTREFRESH = "FirstFastRefresh";
static const std::string MEMORYMODESSUPPORTED = "ModesSupported";
static const std::string SECURITYCAPABILITIES = "SecurityCapabilities";
static const std::string CONFIGURATIONSTATUS = "ConfigurationStatus";
static const std::string SKUVIOLATION = "SKUViolation";
static const std::string ARSSTATUS = "ARSStatus";
static const std::string VIRALPOLICY = "ViralPolicy";
static const std::string VIRALSTATE = "ViralState";

class NVM_API ShowDeviceCommand : framework::CommandBase
{
public:
	ShowDeviceCommand(
			core::device::DeviceService &service = core::device::DeviceService::getService());

	framework::ResultBase *execute(const framework::ParsedCommand &parsedCommand);

private:
	core::device::DeviceService &m_service;

	framework::ParsedCommand m_parsedCommand;

	bool dimmIdsAreValid();
	void filterDevicesOnDimmIds();
	bool socketIdsAreValid();
	void filterDevicesOnSocketIds();
	void createResults();
	bool displayOptionsAreValid();

	std::string getFirstBadDimmId(core::device::DeviceCollection &devices) const;
	std::string getFirstBadSocketId(core::device::DeviceCollection &devices) const;

	static std::string getDimmId(core::device::Device &);
	static std::string convertLockState(lock_state lockState);
	static std::string convertHealthState(NVM_UINT16 healthState);
	static std::string convertManageabilityState(manageability_state state);
	static std::string convertMemoryType(memory_type type);
	static std::string convertFormFactor(device_form_factor formFactor);
	static std::string convertFwLogLevel(fw_log_level logLevel);
	static std::string convertConfigStatus(config_status status);
	static std::string convertLastShutdownStatus(NVM_UINT16 status);
	static std::string convertToDate(NVM_UINT64 timeValue);
	static std::string convertMemoryModes(NVM_UINT16 mode);
	static std::string convertSecurityCapabilities(NVM_UINT16 capability);
	static std::string convertCapacity(NVM_UINT64 capacity);
	static std::string getManufacturingDate(core::device::Device &device);
	static std::string getManufacturingLoc(core::device::Device &device);
	static std::string toHex(NVM_UINT16 value);
	static std::string convertInterfaceFormatCode(const NVM_UINT16 ifc);
	static std::string getJedecStringForInterfaceFormatCode(const NVM_UINT16 ifc);
	static std::string convertArsStatus(device_ars_status status);

	framework::ResultBase *m_pResult;
	framework::PropertyDefinitionList<core::device::Device> m_props;
	core::StringList m_dimmIds;
	core::StringList m_socketIds;
	framework::DisplayOptions m_displayOptions;
	core::device::DeviceCollection m_devices;
	bool isPropertyDisplayed(framework::IPropertyDefinition<core::device::Device> &p);
};

}
}

#endif //CR_MGMT_SHOWDEVICECOMMAND_H
