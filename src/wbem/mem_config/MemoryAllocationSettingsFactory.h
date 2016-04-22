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
 * This file contains the provider for the MemoryAllocationSettings instances.
 */


#ifndef	_WBEM_MEMCONFIG_MEMORYALLOCATIONSETTINGS_FACTORY_H_
#define	_WBEM_MEMCONFIG_MEMORYALLOCATIONSETTINGS_FACTORY_H_

#include <string>
#include <physical_asset/NVDIMMFactory.h>
#include <mem_config/InterleaveSet.h>
#include <nvm_management.h>
#include <list>
#include <vector>
#include <framework_interface/NvmInstanceFactory.h>

namespace wbem {
namespace mem_config {

typedef std::list<std::string> StringListType;

// object path strings
static const std::string MEMORYALLOCATIONSETTINGS_ELEMENTNAME =
		" NVM allocation setting"; //!< Element Name static
static const std::string MEMORYALLOCATIONSETTINGS_CREATIONCLASSNAME =
		std::string(NVM_WBEM_PREFIX) + "MemoryAllocationSettings"; //!< CreationClassName
static const std::string MEMORYALLOCATIONSETTINGS_ALLOCATIONUNITS = "bytes";

// A MemoryAllocationSettings instanceId has the form AA.B.CCCC.D
// Where:
// AA is the socket number
// B is the type of memory region (V - memory, P - app direct, U - unmapped)
// CCCC is the regionID - all digits
// D is the config type - either C (current config) or G (goal config)

// The constants below are used to verify the format of the instanceId
#define MEMORYALLOCATIONSETTINGS_INSTANCEID_LEN 11
#define MEMORYALLOCATIONSETTINGS_SOCKETNUMBERFIRSTDIGIT_POSITION 0
#define MEMORYALLOCATIONSETTINGS_SOCKETNUMBERSECONDDIGIT_POSITION 1
#define MEMORYALLOCATIONSETTINGS_PERIOD1_POSITION 2
#define MEMORYALLOCATIONSETTINGS_MEMORYCONTROLLERID_LENGTH 2
#define MEMORYALLOCATIONSETTINGS_CHANNELID_LENGTH 2
#define MEMORYALLOCATIONSETTINGS_MEMORYTYPE_POSITION 3
#define MEMORYALLOCATIONSETTINGS_PERIOD2_POSITION 4
#define MEMORYALLOCATIONSETTINGS_REGIONID_LENGTH 4
#define MEMORYALLOCATIONSETTINGS_REGIONIDFIRSTDIGIT_POSITION 5
#define MEMORYALLOCATIONSETTINGS_MEMORYCONTROLLERIDFIRSTDIGIT_POSITION 5
#define MEMORYALLOCATIONSETTINGS_REGIONIDSECONDDIGIT_POSITION 6
#define MEMORYALLOCATIONSETTINGS_REGIONIDTHIRDDIGIT_POSITION 7
#define MEMORYALLOCATIONSETTINGS_CHANNELIDFIRSTDIGIT_POSITION 7
#define MEMORYALLOCATIONSETTINGS_REGIONIDFOURTHDIGIT_POSITION 8
#define MEMORYALLOCATIONSETTINGS_PERIOD3_POSITION 9
#define MEMORYALLOCATIONSETTINGS_CONFIGTYPE_POSITION 10

/*!
 * Represents the support data maintained by the management software
 */
class NVM_API MemoryAllocationSettingsFactory: public framework_interface::NvmInstanceFactory {
public:
	// constructor
	MemoryAllocationSettingsFactory();
	MemoryAllocationSettingsFactory(wbem::framework::STR instance);
	~MemoryAllocationSettingsFactory();

	// methods required for CIM provider support, note that if
	// you don't see an expected method here the inherited version is used and
	// it throws an unsupported exception
	framework::Instance* getInstance(framework::ObjectPath &path,
		framework::attribute_names_t &attributes)
			throw (framework::Exception);
	framework::instance_names_t* getInstanceNames()
		throw (framework::Exception);
	bool isAssociated(const std::string &associationClass,
		framework::Instance* pAntInstance,
		framework::Instance* pDepInstance);

	/*!
	 * Get the regionType from the instanceIdStr ('P', 'U' or 'V')
	 */
	static char getRegionType(const std::string instanceIdStr);

	/*!
	 * Get the socketId from the instanceIdStr
	 */
	static NVM_UINT32 getSocketId(const std::string instanceIdStr);

	/*
	 * Return true if the given uid is a device uid
	 */
	static bool isADeviceUid(const NVM_UID uid);

	/*
	 * Return true if the instance is a memory mode, goal config instance
	 */
	bool isMemoryModeGoalConfigInstance(const framework::Instance* pInstance);

	/*
	 * Return true if the instance is a app direct, goal config instance
	 */
	bool isAppDirectGoalConfigInstance(const framework::Instance* pInstance);

private:
	/*
	 * Return the uid generated from the settings instance
	 */
	bool deviceUidMatchesSetting(const framework::Instance* pSettingInstance,
		const NVM_UID uid);

	/*
	 * Return the uid that makes up the DEVICEID of the AppDirectMemory instance
	 */
	void getAppDirectMemoryUid(const framework::Instance* pMemoryInstance,
		NVM_UID uid);

	/*
	 * Return true if the app direct region is associated with the AppDirectMemory instance
	 */
	bool isAppDirectSettingAssociatedWithMemoryInstance(
		const framework::Instance* pSettingInstance,
		const framework::Instance* pMemoryInstance);

	/*
	 * Return true if the unmapped region is associated with the AppDirectMemory instance
	 */
	bool isUnmappedSettingAssociatedWithMemoryInstance(
		const framework::Instance* pSettingInstance,
		const framework::Instance* pMemoryInstance);

	/*
	 * Return true if the setting instance describes an app direct memory region
	 */
	bool isAppDirectInstance(const framework::Instance* pSettingInstance);

	/*
	 * Return true if the setting instance describes an unmapped memory region
	 */
	bool isUnmappedInstance(const framework::Instance* pSettingInstance);

	/*
	 * Return true if the the app direct memory setting described by the
	 * setting instance is associated with the AppDirectMemory
	 * class instance.
	 */
	bool isSettingAssociatedWithMemoryInstance(
		const framework::Instance* pSettingInstance,
		const framework::Instance* pMemoryInstance);

	/*
	 * Get the interleave set uid described by the setting instance
	 */
	void getIlsetUidFromSettingInstance(
		const framework::Instance* pSettingInstance, NVM_UID ilsetUid);

	/*
	 * Return true if the instance is a goal instance
	 */
	bool isGoalInstance(const framework::Instance* pInstance);

	/*
	 * Return true if the instance is a memory mode, current config instance
	 */
	bool isMemoryModeCurrentConfigInstance(const framework::Instance* pInstance);

	/*
	 * return the handle that corresponds to the dimm uid
	 */
	NVM_NFIT_DEVICE_HANDLE getHandleForDimmUid(
			const wbem::physical_asset::devices_t &devices, const NVM_UID uid);

	/*
	 * Add the attributes that go with this class
	 */
	void populateAttributeList(framework::attribute_names_t &attributes)
		throw (framework::Exception);

	/*
	 * Add the attributes to the instance that are unique to the current config instance
	 */
	void finishCurrentConfigInstance(framework::Instance *pInstance,
		const std::string instanceIdStr, const framework::attribute_names_t attributes);

	/*
	 * Add the attributes to the instance that are unique to a goal instance
	 */
	void finishGoalInstance(framework::Instance *pInstance,
		const std::string instanceIdStr, const framework::attribute_names_t attributes);

	/*
	 * Finish a memory or storage instance given a reservation
	 */
	void finishMemoryOrStorageInstance(framework::Instance *pInstance, NVM_UINT64 reservation,
		framework::attribute_names_t attributes);

	/*
	 * Finish a app direct instance given a reservation
	 */
	void finishAppDirectInstance(framework::Instance *pInstance, InterleaveSet &ilset,
		framework::attribute_names_t attributes);
	/*
	 * Get the instanceNames derived from the current config
	 */
	StringListType getCurrentConfigNames(const physical_asset::devices_t &devices);

	/*
	 * Get the instanceNames derived from the goals
	 */
	StringListType getGoalNames(const physical_asset::devices_t &devices);

	/*
	 * Get the instanceNames derived from the goal on a socket
	 */
	wbem::mem_config::StringListType getGoalNamesForSocket
		(const physical_asset::devices_t &devices, NVM_UINT16 socketId);

	/*
	 * Validate the instanceIdStr to make sure it is in the correct format
	 */
	void validateNameFormat(const std::string name);

	/*
	 * throw if the pool in the instanceIdStr is not on the system
	 */
	NVM_UINT16 validateAndReturnSocketId(const std::string instanceIdStr);

	/*
	 * From the instanceIdStr return the instance type ('C' of 'G')
	 */
	char getInstanceType(const std::string instanceIdStr);

	/*
	 * Get the Memory Mode capacity for the given socket from the pools structs
	 */
	NVM_UINT64 getMemoryReservationFromPools(const std::vector<struct pool> &pools,
		std::string instanceIdStr);

	/*
	 * Get the storage capacity of the dimm described by the instanceID
	 * from the pool structs
	 */
	NVM_UINT64 getUnmappedReservationFromPools(const std::vector<struct pool> &pools,
		const std::string instanceIdStr);

	/*
	 * From the pool struct get the Memory Mode capacity on a socket
	 */
	NVM_UINT64 getMemoryCapacityForSocket(const struct pool *pool,
		const NVM_UINT16 socketId);

	/*
	 * Gather all the ilsets from the pools into a vector
	 */
	InterleaveSet getInterleaveSetFromPools
		(const std::vector<struct pool> &pools, std::string instanceIdStr);

	/*
	 * From the handle info get the dimm uid
	 */
	void getUidFromHandleInfo(NVM_UINT16 socketId,
		const NVM_UINT32 memoryControllerId, const NVM_UINT32 channelId, NVM_UID uid);

	/*
	 *  From the pool struct return the storage capacity of a dimm
	 */
	NVM_UINT64 getStorageCapacityForDimm(const std::vector<struct pool> &pools,
		const NVM_UID uid);

	/*
	 * Find the index of the given dimm in the array of dimms that participate in the pool
	 */
	int getIndexOfDimmInPoolOrReturnNotFound(const struct pool *pPool, const NVM_UID uid);

	/*
	 * Find the amount of memory mode capacity for the goals on the given socket
	 */
	NVM_UINT64 getMemoryReservationFromGoals(const physical_asset::devices_t &devices, std::string instanceIdStr);

	/*
	 * Derive the vector of ilsets from the array of goal structs
	 */
	InterleaveSet getInterleaveSetFromGoals
		(const physical_asset::devices_t &devices, std::string instanceIdStr);

	/*
	 * Add the ilsets from the goal struct to the vector of ilsets
	 */
	void addIlsetsFromGoal(std::vector<InterleaveSet> &ilsets, const struct config_goal *pGoal);

	/*
	 * Add an ilsets to the vector of ilsets
	 */
	void addIlset (std::vector<InterleaveSet> &ilsets, const struct config_goal *pGoal, const int setNum);

	/*
	 * Return the socketId from the setting instance
	 */
	NVM_UINT32 getSocketIdFromSettingInstance(
		const framework::Instance* pSettingInstance);

	/*
	 * Return the instanceId from the setting instance
	 */
	std::string getInstanceIdStrFromSettingInstance(const framework::Instance* pSettingInstance);

	/*
	 * Return the memoryControllerId from the setting instance
	 */
	NVM_UINT32 getMemoryControllerIdFromSettingInstance(
		const framework::Instance* pSettingInstance);

	/*
	 * Return the memControllerId from the instanceId of a setting instance
	 */
	NVM_UINT32 getMemoryControllerId(const std::string instanceIdStr);


	/*
	 * Return the channelId from the setting instance
	 */
	NVM_UINT32 getChannelIdFromSettingInstance(
		const framework::Instance* pSettingInstance);

	/*
	 * Return the chlId from the instanceId of a setting instance
	 */
	NVM_UINT32 getChannelId(const std::string instanceIdStr);

	/*
	 * Return the chlId from the instanceId of a setting instance
	 */
	NVM_UINT32 getInterleaveSetIndex(const std::string instanceIdStr);

	/*
	 *  Return true is instanceId describes a goal instance
	 */
	bool isGoal (const std::string instanceId);

	/*
	 * Return true if the instanceId describes a app direct memory instance
	 */
	bool isAppDirectMemory(const std::string instanceIdStr);

	/*
	 * Return true if the instanceId describes a memory mode instance
	 */
	bool isMemory(const std::string instanceIdStr);
};

} // mem_config
} // wbem
#endif  // #ifndef _WBEM_MEMCONFIG_MEMORYALLOCATIONSETTINGS_FACTORY_H_
