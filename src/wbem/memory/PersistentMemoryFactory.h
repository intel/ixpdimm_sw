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
 * This file contains the provider for the PersistentMemory instances
 * which represent individual persistent memory extents in the system.
 */

#ifndef _WBEM_MEMORY_PERSISTENTMEMORY_FACTORY_H_
#define _WBEM_MEMORY_PERSISTENTMEMORY_FACTORY_H_

#include <libinvm-cim/Types.h>
#include <server/BaseServerFactory.h>
#include <nvm_management.h>
#include <string>
#include <vector>
#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace memory
{

// Constants
const std::string PERSISTENTMEMORY_CREATIONCLASSNAME =
		std::string(NVM_WBEM_PREFIX) + "PersistentMemory";
const std::string PERSISTENTMEMORY_SYSTEMCREATIONCLASSNAME =
		server::BASESERVER_CREATIONCLASSNAME;

// HealthState values
const NVM_UINT16 PERSISTENTMEMORY_HEALTHSTATE_UNKNOWN = 0;
const NVM_UINT16 PERSISTENTMEMORY_HEALTHSTATE_OK = 5;
const NVM_UINT16 PERSISTENTMEMORY_HEALTHSTATE_DEGRADED = 10;
const NVM_UINT16 PERSISTENTMEMORY_HEALTHSTATE_NONRECOVERABLE = 30;

// OperationalStatus values
const NVM_UINT16 PERSISTENTMEMORY_OPSTATUS_UNKNOWN = 0;
const NVM_UINT16 PERSISTENTMEMORY_OPSTATUS_OK = 2;
const NVM_UINT16 PERSISTENTMEMORY_OPSTATUS_LOSTCOMM = 13;
const NVM_UINT16 PERSISTENTMEMORY_OPSTATUS_SUPPORTINGENTITYERROR = 16;

// AccessGranularity values
const NVM_UINT16 PERSISTENTMEMORY_ACCESSGRANULARITY_UNKNOWN = 0;
const NVM_UINT16 PERSISTENTMEMORY_ACCESSGRANULARITY_BLOCK = 1;
const NVM_UINT16 PERSISTENTMEMORY_ACCESSGRANULARITY_BYTE = 2;

// Replication values
const NVM_UINT16 PERSISTENTMEMORY_REPLICATION_UNKNOWN = 0;
const NVM_UINT16 PERSISTENTMEMORY_REPLICATION_NONE = 1;
const NVM_UINT16 PERSISTENTMEMORY_REPLICATION_LOCAL = 2;

// EnabledState values
const NVM_UINT16 PERSISTENTMEMORY_ENABLEDSTATE_NA = 6;



class NVM_API PersistentMemoryFactory : public framework_interface::NvmInstanceFactory
{
	public:
		/*!
		 * Initialize a new PersistentMemoryFactory
		 */
		PersistentMemoryFactory();

		/*!
		 * Clean up the PersistentMemoryFactory
		 */
		virtual ~PersistentMemoryFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the persistent memory information.
		 * @return The instance.
		 */
		virtual framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * persistent memory object paths.
		 * @return The object path of the instances.
		 */
		virtual framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*!
		 * Determines if the two instances should be associated by the Association Class. Usually only
		 * used if the association is more complex than simple FK relationships
		 * @param associationClass
		 * @param pAntInstance
		 * @param pDepInstance
		 * @return true if the instances are associated, false otherwise
		 */
		virtual bool isAssociated(const std::string &associationClass,
				framework::Instance *pAntInstance, framework::Instance *pDepInstance);

		/*!
		 * Generate a UUID string for an interleave set.
		 * @param setIndex - the unique interleave set index
		 * @param socketId - the socket the interleave set belongs to
		 * @return unique ID string
		 */
		static std::string getInterleaveSetUuid(const NVM_UINT32 setIndex,
				const NVM_UINT32 socketId);

		/*!
		 * Generate a UUID string for a storage-only region.
		 * @param the DIMM whose storage region we should fetch
		 * @return unique ID string
		 */
		static std::string getStorageRegionUuid(const std::string &dimmUidStr);

	protected:
		/*
		 * Get default attributes.
		 */
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*
		 * Validate the key attributes for the PersistentMemory object path
		 */
		void validatePath(const framework::ObjectPath &path) throw (framework::Exception);

		/*
		 * Generate an instance name for an instance with a given DeviceID.
		 */
		framework::ObjectPath getInstanceName(const std::string &deviceId);

		/*
		 * Look up interleave set instance names and add them to instanceNames
		 */
		void getInterleaveSetInstanceNames(framework::instance_names_t &instanceNames,
				const struct pool &pool);

		/*
		 * Look up storage region instance names and add them to instanceNames
		 */
		void getStorageRegionInstanceNames(framework::instance_names_t &instanceNames,
				const struct pool &pool);

		/*
		 * Search the pool for an interleave set matching the UUID.
		 * @param uuid
		 * @param pool
		 * @param interleave - if found, the interleave set is returned in this param
		 * @return true if found, false otherwise
		 */
		bool findInterleaveSetForUuid(const std::string &uuid, const struct pool &pool,
				struct interleave_set &interleave);

		/*
		 * Search the pool for a storage region with a given UUID.
		 * @param uuid
		 * @param pool
		 * @param index - if found, the DIMM index is returned in this param
		 * @return true if found, false otherwise
		 */
		bool findStorageDimmIndexForUuid(const std::string &uuid, const struct pool &pool, size_t &index);

		/*
		 * Set up the attributes peculiar to interleave sets.
		 */
		void setInterleaveSetInstanceAttributes(framework::Instance &instance,
				const framework::attribute_names_t &attributes,
				const struct interleave_set &interleave) throw (framework::Exception);

		/*
		 * Set up the attributes peculiar to storage regions.
		 */
		void setStorageCapacityInstanceAttributes(framework::Instance &instance,
				const framework::attribute_names_t &attributes,
				const struct pool &pool, const size_t &dimmIdx) throw (framework::Exception);

		/*
		 * Set up the instance attributes that are uniform regardless of the type of PM extent.
		 */
		void setGenericInstanceAttributes(framework::Instance &instance,
				const framework::attribute_names_t &attributes,
				const NVM_UINT16 socketId) throw (framework::Exception);

		/*
		 * Determine if the DIMM is in use by the PersistentMemory instance
		 */
		bool isPersistentMemoryUsingDimm(const std::string &pmUuid, const std::string &dimmUid)
			throw (framework::Exception);

		/*
		 * Fetch the PM alignment from capabilities in bytes.
		 */
		NVM_UINT64 getAppDirectAlignment() throw (framework::Exception);

		/*
		 * Calculate the number of blocks based on the extent capacity.
		 */
		NVM_UINT64 getNumBlocks(const NVM_UINT64 capacity) throw (framework::Exception);

		/*
		 * Get the HealthState for an interleave set.
		 */
		NVM_UINT16 getInterleaveSetHealthState(const struct interleave_set &interleave);

		/*
		 * Get the HealthState for a storage region on a given DIMM.
		 */
		NVM_UINT16 getStorageRegionHealthState(const NVM_UID dimmUid) throw (framework::Exception);

		/*
		 * Get the OperationalStatus for an interleave set.
		 */
		NVM_UINT16 getInterleaveSetOperationalStatus(const struct interleave_set &interleave)
			throw (framework::Exception);

		/*
		 * Get the OperationalSTatus for a storage region on a given DIMM.
		 */
		NVM_UINT16 getStorageRegionOperationalStatus(const NVM_UID dimmUid) throw (framework::Exception);

		/*
		 * Translate HealthState value to string
		 */
		std::string getHealthStateString(const NVM_UINT16 value);

		/*
		 * Translate OperationalStatus value to string
		 */
		std::string getOperationalStatusString(const NVM_UINT16 value);

		/*
		 * Translate AccessGranularity value to string
		 */
		std::string getAccessGranularityString(const NVM_UINT16 value);

		/*
		 * Translate Replication value to string
		 */
		std::string getReplicationString(const NVM_UINT16 value);

		/*
		 * Translate EnabledState value to string
		 */
		std::string getEnabledStateString(const NVM_UINT16 value);

		/*
		 * Compare pool struct with PersistentMemory object to see if the PM object
		 * belongs to the pool
		 */
		bool poolMatchesPmObject(struct pool *pool, framework::Instance* pPMObject);

		/*
		 * Return true if the socket_id of the pool matches the processorAffinity of the PM object
		 */
		bool socketsMatch(struct pool *pool, framework::Instance* pPMObject);

		/*
		 * Return if the type of PM object matches the type of pool
		 */
		bool pmTypesMatch(struct pool *pool, framework::Instance* pPMObject);

		/*
		 * Return true if both the pool and the PM object are mirrored or both not mirrored
		 */
		bool mirroringMatches(struct pool *pool, framework::Instance* pPMObject);

		bool isPersistentMemoryAssociatedToPersistentMemoryNamespace(
				framework::Instance &pmInstance, framework::Instance &pmnsInstance);
};

} /* namespace memory */
} /* namespace wbem */

#endif /* _WBEM_MEMORY_PERSISTENTMEMORY_FACTORY_H_ */
