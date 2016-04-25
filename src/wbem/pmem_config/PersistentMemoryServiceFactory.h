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
 * This file contains the provider for the PersistentMemoryService instances.
 */

#include <server/BaseServerFactory.h>
#include <nvm_types.h>
#include <nvm_management.h>
#include <mem_config/InterleaveSet.h>
#include <framework_interface/NvmInstanceFactory.h>
#include "NamespaceSettingsFactory.h"
#include <exception/NvmExceptionLibError.h>

#ifndef	_WBEM_PMEMCONFIG_PERSISTENTMEMORYSERVICEFACTORY_H_
#define	_WBEM_PMEMCONFIG_PERSISTENTMEMORYSERVICEFACTORY_H_
namespace wbem
{
namespace pmem_config
{

static const std::string PM_SERVICE_CLASSNAME = std::string(NVM_WBEM_PREFIX) + "PersistentMemoryService";
static const std::string PM_SERVICE_SYSTEMCREATIONCLASSNAME = wbem::server::BASESERVER_CREATIONCLASSNAME;
static const std::string PM_SERVICE_NAME = "Persistent Memory Service";

// supported extrinsic methods
static const std::string PM_SERVICE_RETURNTOPOOL = "ReturnToPool";
static const std::string PM_SERVICE_ALLOCATEFROMPOOL = "AllocateFromPool";
static const std::string PM_SERVICE_MODIFYNAMESPACE = "ModifyNamespace";

// parameters of extrinsic methods
static const std::string PM_SERVICE_NAMESPACE = "Namespace";
static const std::string PM_SERVICE_GOAL = "Goal";
static const std::string PM_SERVICE_RESOURCE_POOL = "Pool";
static const std::string PM_SERVICE_STATE = "State";

// return values of extrinsic methods
static const NVM_UINT32 PM_SERVICE_ERR_UNKNOWN = 2;
static const NVM_UINT32 PM_SERVICE_ERR_TIMEOUT = 3;
static const NVM_UINT32 PM_SERVICE_ERR_FAILED = 4;
static const NVM_UINT32 PM_SERVICE_ERR_INVALIDPARAMETER = 5;
static const NVM_UINT32 PM_SERVICE_ERR_INSUFFICIENT_RESOURCES = 4097;
static const NVM_UINT32 PM_SERVICE_ERR_INCONSISTENT_PARAMETERS = 4098;
static const NVM_UINT32 PM_SERVICE_ERR_IN_USE = 32768;

// constants used in AllocateFromPool
static const NVM_UINT32 PM_SERVICE_STORAGE_TYPE = NS_RESOURCETYPE_BLOCK_ADDRESSABLE; // Block addressable persistent memory
static const NVM_UINT32 PM_SERVICE_APP_DIRECT_TYPE = NS_RESOURCETYPE_BYTE_ADDRESSABLE; // Byte addressable persistent memory
static const NVM_UINT32 PM_SERVICE_OPTIMIZE_NONE = 2; // best performance
static const NVM_UINT32 PM_SERVICE_OPTIMIZE_BEST_PERFORMANCE = 3; // best performance
static const NVM_UINT32 PM_SERVICE_OPTIMIZE_SMALLEST_SIZE = 4; // smallest size
static const NVM_UINT32 PM_SERVICE_OPTIMIZE_COPYONWRITE = 5; // best performance
static const NVM_UINT16 PM_SERVICE_SECURITY_ENCRYPTION_OFF = 0; 
static const NVM_UINT16 PM_SERVICE_SECURITY_ENCRYPTION_ON = 1; 
static const NVM_UINT16 PM_SERVICE_SECURITY_ENCRYPTION_IGNORE = 2; 
static const NVM_UINT16 PM_SERVICE_SECURITY_ERASE_CAPABLE_FALSE = 0; 
static const NVM_UINT16 PM_SERVICE_SECURITY_ERASE_CAPABLE_TRUE = 1; 
static const NVM_UINT16 PM_SERVICE_SECURITY_ERASE_CAPABLE_IGNORE = 2; 
static const NVM_UINT32 PM_SERVICE_OPTIMIZE_CRYPTO_ERASE = 3; // crypto erase
static const NVM_UINT32 PM_SERVICE_NAMESPACE_ENABLE_STATE_ENABLED = 2; // enabled
static const NVM_UINT32 PM_SERVICE_NAMESPACE_ENABLE_STATE_DISABLED = 3; // disabled
static const NVM_UINT16 PM_SERVICE_MEMORYPAGEALLOCATION_NONE = 1;
static const NVM_UINT16 PM_SERVICE_MEMORYPAGEALLOCATION_DRAM = 2;
static const NVM_UINT16 PM_SERVICE_MEMORYPAGEALLOCATION_APP_DIRECT = 3;

class NVM_API PersistentMemoryServiceFactory : public framework_interface::NvmInstanceFactory
{
public:
	/*!
		 * Initialize a new PersistentMemoryServiceFactory.
		 */
	PersistentMemoryServiceFactory() throw (framework::Exception);

	/*!
	 * Clean up the PersistentMemoryServiceFactory
	 */
	~PersistentMemoryServiceFactory();

	/*!
	 * Implementation of the standard CIM method to retrieve a specific instance
	 * @param[in] path
	 * 		The object path of the instance to retrieve.
	 * @param[in] attributes
	 * 		The attributes to retrieve.
	 * @throw Exception if unable to retrieve the host information.
	 * @todo Should throw an exception if the object path doesn't match
	 * the results of getHostName.
	 * @return The instance.
	 */
	framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

	/*!
	 * Implementation of the standard CIM method to retrieve a list of
	 * object paths.
	 * @return The object path.
	 */
	framework::instance_names_t* getInstanceNames() throw (framework::Exception);

	/*!
	 * Entry point for CIM extrinsic methods
	 */
	wbem::framework::UINT32 executeMethod(
			wbem::framework::UINT32 &wbem_return,
			const std::string method,
			wbem::framework::ObjectPath &object,
			wbem::framework::attributes_t &inParms,
			wbem::framework::attributes_t &outParms);

	/*!
	 * Interface to the library API nvm_delete_namespace function.
	 * This pointer allows for dependency injection and decouples the dependency on the API
	 */
	int (*m_deleteNamespace)(const NVM_UID namespace_uid);

	/*!
	 * delete a namespace
	 */
	void deleteNamespace(const std::string &namespaceUid);

	/*!
	 * Interface to the library API nvm_create_namespace function.
	 * This pointer allows for dependency injection and decouples the dependency on the API
	 */
	int (*m_createNamespace)(NVM_UID *p_namespace_uid,
			const NVM_UID pool_uid,
			struct namespace_create_settings *p_create_settings,
			const struct interleave_format *p_format, NVM_BOOL allow_adjustment);

	// structure to hold the information for creating a namespace
	typedef struct
	{
		NVM_UINT32 type;
		std::string poolId;
		std::string friendlyName;
		NVM_UINT64 blockSize;
		NVM_UINT64 blockCount;
		NVM_UINT16 enabled;
		NVM_UINT16 optimize;
		NVM_UINT16 encryption;
		NVM_UINT16 eraseCapable;
		enum mem_config::MemoryAllocationSettingsInterleaveSizeExponent interleaveChannelSize;
		enum mem_config::MemoryAllocationSettingsInterleaveSizeExponent interleaveControllerSize;
		bool byOne;
		NVM_UINT16 memoryPageAllocation;
	} createNamespaceParams;

	/*!
 	* Create a namespace
 	*/
	virtual void createNamespace(const createNamespaceParams &settings, std::string &namespaceUid);
	virtual void createNamespace(std::string &namespaceUidStr,
			const std::string poolUidStr, const NVM_UINT16 stateValue,
			const std::string friendlyNameStr, const NVM_UINT64 blockSize,
			const NVM_UINT64 blockCount, const NVM_UINT16 type,
			const NVM_UINT16 optimize,
			const NVM_UINT16 encryption, const NVM_UINT16 eraseCapable,
			const mem_config::MemoryAllocationSettingsInterleaveSizeExponent channelSize,
			const mem_config::MemoryAllocationSettingsInterleaveSizeExponent controllerSize,
			const bool byOne, const NVM_UINT16 memoryPageAllocation);

	/*!
	 * Interface to the library API nvm_modify_namespace_name function.
	 * This pointer allows for dependency injection and decouples the dependency on the API
	 */
	int (*m_modifyNamespaceName)(const NVM_UID namespace_uid, const NVM_NAMESPACE_NAME name);

	/*!
	 * Interface to the library API nvm_modify_namespace_name_block_count function.
	 * This pointer allows for dependency injection and decouples the dependency on the API
	 */
	int (*m_modifyNamespaceBlockCount)(
			const NVM_UID namespace_uid, NVM_UINT64 block_count, NVM_BOOL allow_adjustment);

	/*!
	 *  Check if the modifyNamespaceName operation is supported
	 */
	virtual bool isModifyNamespaceNameSupported();

	/*!
	 *  Check if the modifyNamespaceBlockCount operation is supported
	 */
	virtual bool isModifyNamespaceBlockCountSupported(const namespace_details &details, const NVM_UINT64 blockCount);

	/*!
	 * modify a namespace name
	 */
	virtual void modifyNamespaceName(const std::string namespaceUidStr, const std::string friendlyNameStr);

	/*!
	 * modify a namespace block count
	 */
	virtual void modifyNamespaceBlockCount(const std::string namespaceUidStr, const NVM_UINT64 blockCount);

	/*
	 * get the details for a given namespace
	 */
	virtual void getNamespaceDetails(const std::string namespaceUidStr, struct namespace_details &details);

	/*
	 * Verifies the ParentPool reference refers to a real MemoryResources instance.
	 * Throws an exception if not.
	 */
	void validatePool(std::string poolRef) throw (framework::Exception);

	static enum namespace_enable_state namespaceEnabledToEnum(const unsigned int enabled);
	static enum namespace_type namespaceTypeToEnum(const NVM_UINT32 type);
	static enum encryption_status encryptionTypeToEnum(const NVM_UINT16 type);
	static enum erase_capable_status eraseCapableToEnum(const NVM_UINT16 type);
	static enum namespace_memory_page_allocation memoryPageAllocationTypeToEnum(const NVM_UINT16 memoryPageAllocation);

	/*
	 * Retrieve the namespace uid from a persistent memory namespace path
	 */
	static framework::UINT32 getNamespaceFromPath(
			const wbem::framework::ObjectPath &path, std::string &namespaceUid);

	/*
	 * Retrieve the adjusted block count for a namespace creation request
	 */
	virtual NVM_UINT64 getAdjustedCreateNamespaceBlockCount(std::string poolUidStr,
			const NVM_UINT16 type, const NVM_UINT32 blockSize, const NVM_UINT64 blockCount,
			const NVM_UINT16 eraseCapable, const NVM_UINT16 encryption,
			const NVM_UINT16 enableState);

	/*
 	* Retrieve the adjusted block count for a namespace modification request
 	*/
	virtual NVM_UINT64 getAdjustedModifyNamespaceBlockCount(std::string namespaceUidStr, const NVM_UINT64 blockCount);
private:
	void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

	/*
	 * Return the number of blocks in the reservation for a namespace
	 */
	NVM_UINT64 getBlockCountFromReservation(const namespace_details &details, NVM_UINT64 reservation);

	/*
	 * Perform the modification atomically, either it is all successful or none of it is
	 */
	void performAtomicModification(std::string namespaceUidStr, NVM_UINT64 reservation, std::string friendlyNameStr);
	void allocateFromPool(framework::attributes_t &inParms, framework::attributes_t &outParms,
			NVM_UINT32 &httpRc);
	void modifyNamespace(framework::attributes_t &inParms, framework::attributes_t &outParms,
			NVM_UINT32 &httpRc);
	void generateNamespaceRefAttribute(std::string namespaceUidStr, wbem::framework::Attribute& value);

	/*
	 * convert NvmExceptionLibError to extrinsic return code
	 */
	wbem::framework::UINT32 getReturnCodeFromLibException(const exception::NvmExceptionLibError &e);
};
}

}

#endif /* _WBEM_PMEMCONFIG_PERSISTENTMEMORYSERVICEFACTORY_H_ */
