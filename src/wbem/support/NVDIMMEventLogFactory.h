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
 * This file contains the provider for the NVDIMMEventLog instance
 * which contain the NVM DIMM related events for the host system.
 */

#ifndef	_WBEM_SUPPORT_NVDIMMEVENTLOG_FACTORY_H_
#define	_WBEM_SUPPORT_NVDIMMEVENTLOG_FACTORY_H_

#include <string>
#include <vector>
#include <libinvm-cim/WqlConditional.h>
#include <nvm_management.h>

#include <framework_interface/NvmInstanceFactory.h>
#include "EventLogFilter.h"

namespace wbem
{
namespace support
{
	/*!
	* Instance ID & ElementName static string
	*/
	static const std::string NVDIMMEVENTLOG_ELEMENTNAME = "NVDIMM Event Log for: ";
	static const std::string NVDIMMEVENTLOG_INSTANCEID = "NVDIMM Event Log";

	/*!
	 * Creation Class Name
	 */
	const std::string NVDIMMEVENTLOG_CREATIONCLASSNAME =
			std::string(NVM_WBEM_PREFIX) + "NVDIMMEventLog";

	/*!
	 * Overwrite policy
	 */
	const NVM_UINT16 NVDIMMEVENTLOG_OVERWRITEPOLICY_WRAPSWHENFULL = 2;

	/*!
	 * Extrinsic method name
	 */
	const std::string NVDIMMEVENTLOG_CLEARLOG = "ClearLog"; //!< extrinsic method name
	const std::string NVDIMMEVENTLOG_FILTEREVENTS = "GetFilteredEvents"; //!< extrinsic method name

	/*!
	 * Extrinsic method parameter names
	 */
	const std::string NVDIMMEVENTLOG_FILTER = "Filter"; //!< extrinsic method param
	const std::string NVDIMMEVENTLOG_LOGENTRIES = "LogEntries"; //!< extrinsic method param

	static const NVM_UINT32 NVDIMMEVENTLOG_ERR_UNKNOWN = 2;
	static const NVM_UINT32 NVDIMMEVENTLOG_ERR_FAILED = 4;
	static const NVM_UINT32 NVDIMMEVENTLOG_ERR_INVALID_PARAMETER = 5;
	static const NVM_UINT32 NVDIMMEVENTLOG_ERR_INSUFFICIENT_RESOURCES = 4097;
/*!
 * Provider Factory for NVDIMMEventLogFactory
 * There is a single instance of this class.  It serves as an aggregation point
 * for event results for all NVDIMMs within a system.
 */
class NVM_API NVDIMMEventLogFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new NVDIMMEventLogFactory.
		 */
		NVDIMMEventLogFactory() throw (framework::Exception);

		/*!
		 * Clean up the NVDIMMEventLogFactory
		 */
		~NVDIMMEventLogFactory();

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
		 * Implementation of the standard CIM extrinsic method
		 */
		wbem::framework::UINT32 executeMethod(
					wbem::framework::UINT32 &wbem_return,
					const std::string method,
					wbem::framework::ObjectPath &object,
					wbem::framework::attributes_t &inParms,
					wbem::framework::attributes_t &outParms);

		/*!
		 * Implementation of the vendor specific clearEventLog extrinsic method
		 */
		void clearEventLog() throw (framework::Exception);

		/*!
		 * Provider for nvm_purge_events
		 * @return
		 */
		int (*m_PurgeEventLog)(const struct event_filter *p_filter);

		/*
		 * Helper method to convert struct event_filter into EventLogFilter
		 */
		static void convertFactoryToEventFilter(EventLogFilter *pEventLogFilter, struct event_filter *pFilter);

		/*
		 * Method to get filtered events using EventLogFilter.
		 * @param pEventLogFilter Filter on events that have the has.... set
		 * @return Filtered event instances
		 */
		wbem::framework::instances_t * getFilteredEvents(EventLogFilter *pEventLogFilter)
			throw (framework::Exception);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		/*
		 * Filter events using a conditional and return a list of object paths for resulting
		 * events.
		 * @param condStr - conditional string
		 * @return NVM_STR_LIST - list of object paths as strings
		 * @throw Exception - if the query is invalid
		 * @remark A well-formed query may return 0 results if it doesn't apply to the
		 * 		LogEntry class or if it describes a filter we don't support.
		 */
		framework::STR_LIST getFilteredEventRefsFromFilterString(const std::string &condStr)
			throw (framework::Exception);

		/*
		 * Parse a WQL-style conditional into an EventLogFilter.
		 * @param conditional
		 * @return EventLogFilter corresponding to query
		 * @throw Exception if query is invalid or doesn't apply to this class
		 */
		EventLogFilter getEventFilterFromConditional(const framework::WqlConditional &conditional)
			throw (framework::Exception);

};

} // support
} // wbem
#endif  // _WBEM_SUPPORT_NVDIMMEVENTLOG_FACTORY_H_
