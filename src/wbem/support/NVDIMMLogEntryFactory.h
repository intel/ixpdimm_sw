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
 * This file contains the provider for the NVDIMMLogEntry instances
 * which represent an individual NVM DIMM event.
 */

#ifndef	_WBEM_SUPPORT_NVDIMMLOGENTRY_FACTORY_H_
#define	_WBEM_SUPPORT_NVDIMMLOGENTRY_FACTORY_H_

#include <string>
#include <vector>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>


namespace wbem
{
namespace support
{
	/*!
	 * Creation Class Name
	 */
	const std::string NVDIMMLOGENTRY_CREATIONCLASSNAME =
			std::string(NVM_WBEM_PREFIX) + "NVDIMMLogEntry";

	/*!
	 * name of parent CIM class
	 */
	const std::string NVDIMMLOGENTRY_PARENTCLASSNAME =
			"CIM_LogEntry";

	/*!
	 * Possible MessageID (type) strings
	 */
	const std::string NVDIMMLOGENTRY_TYPE_ALL = "All";
	const std::string NVDIMMLOGENTRY_TYPE_HEALTH = "HealthEvent";
	const std::string NVDIMMLOGENTRY_TYPE_MGMT = "MgmtEvent";
	const std::string NVDIMMLOGENTRY_TYPE_DIAG = "AllDiagCheck";
	const std::string NVDIMMLOGENTRY_TYPE_QUICKDIAG = "QuickCheck";
	const std::string NVDIMMLOGENTRY_TYPE_PLATFORMCONFIGDIAG = "PlatformCheck";
	const std::string NVDIMMLOGENTRY_TYPE_PMDIAG = "StorageCheck";
	const std::string NVDIMMLOGENTRY_TYPE_SECURITYDIAG = "SecurityCheck";
	const std::string NVDIMMLOGENTRY_TYPE_FWCONSISTENCYDIAG = "SettingCheck";
	const std::string NVDIMMLOGENTRY_TYPE_CONFIG = "Config";
	const std::string NVDIMMLOGENTRY_TYPE_UNKNOWN = "Unknown";

/*!
 * Provider Factory for NVDIMMLogEntryFactory
 * There is a single instance of this class.  It serves as an aggregation point
 * for event results for all NVDIMMs within a system.
 */
class NVM_API NVDIMMLogEntryFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new NVDIMMLogEntryFactory.
		 */
		NVDIMMLogEntryFactory() throw (framework::Exception);

		/*!
		 * Clean up the NVDIMMLogEntryFactory
		 */
		~NVDIMMLogEntryFactory();

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

		/*
		 * Helper method to get instance from a single struct event
		 */
		static void eventToInstance(framework::Instance *pInstance,
				struct event *pEvent,
				framework::attribute_names_t &attributes);

		/*!
		 * Standard CIM method to modify an existing instance.
		 * @param[in] path
		 * 		The object path of the instance to modify.
		 * @param[in] attributes
		 * 		The attributes to modify.
		 * @throw Exception if not implemented.
		 * @return The updated instance.
		 */
		framework::Instance* modifyInstance(framework::ObjectPath &path,
			framework::attributes_t &attributes) throw (wbem::framework::Exception);

		/*!
		 * Provider for nvm_acknowledge_event
		 * @return
		 */
		int (*m_AcknowledgeEvent)(NVM_UINT32 event_id);


		/*!
		 * A static/public version of populateAttributeList that can easily be used by dependent
		 * classes
		 */
		static void populateAttributes(framework::attribute_names_t &attributes);

		/*!
		 * Convert from event type enum to string.
		 */
		static std::string eventTypeToString(const int eventType);

		/*!
		 * Convert from event string to type enum
		 */
		static int eventTypeStringToType(const std::string &typeStr);

	private:
		/*!
		 * Required override
		 */
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

};

} // support
} // wbem
#endif  // _WBEM_SUPPORT_NVDIMMLOGENTRY_FACTORY_H_
