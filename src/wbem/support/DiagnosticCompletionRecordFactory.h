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
 * This file contains the provider for the DiagnosticCompletionRecord instances
 * which model an individual diagnostic test result.
 */

#ifndef	_WBEM_SUPPORT_DIAGNOSTICCOMPLETIONRECORD_FACTORY_H_
#define	_WBEM_SUPPORT_DIAGNOSTICCOMPLETIONRECORD_FACTORY_H_

#include <string>
#include <vector>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>


namespace wbem
{
namespace support
{
	/*!
	* Instance ID static string
	*/
	static const std::string DIAGNOSTICCOMPLETION_INSTANCEID = "NVDIMM Diagnostic Result";
	/*!
	 * Creation Class Name
	 */
	static const std::string DIAGNOSTICCOMPLETION_CREATIONCLASSNAME =
			std::string(NVM_WBEM_PREFIX) + "DiagnosticCompletionRecord";

/*!
 * Provider Factory for DiagnosticLog
 * There is a single instance of this class.  It serves as an aggregation point
 * for diagnostic results for all NVDIMMs within a system.
 */
class NVM_API DiagnosticCompletionRecordFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new DiagnosticCompletionRecordFactory.
		 */
		DiagnosticCompletionRecordFactory() throw (framework::Exception);

		/*!
		 * Clean up the DiagnosticCompletionRecordFactory
		 */
		~DiagnosticCompletionRecordFactory();

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

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		// helper function to add message args to diagnostic results
		std::string buildDiagnosticResultMessage(struct event *p_event);
};

} // support
} // wbem
#endif  // _WBEM_SUPPORT_DIAGNOSTICCOMPLETIONRECORD_FACTORY_H_
