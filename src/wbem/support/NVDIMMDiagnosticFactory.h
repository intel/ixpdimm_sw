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
 * This file contains the provider for the NVDIMMDiagnostic instances
 * which represent an diagnostic test.
 */


#ifndef	_WBEM_SUPPORT_NVDIMMDIAGNOSTIC_FACTORY_H_
#define	_WBEM_SUPPORT_NVDIMMDIAGNOSTIC_FACTORY_H_

#include <string>
#include <server/BaseServerFactory.h>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>

namespace wbem
{
namespace support
{
	static const std::string NVDIMMDIAGNOSTIC_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "NVDIMMDiagnostic"; //!< Creation ClassName static

	// diagnostic test types
	static const std::string NVDIMMDIAGNOSTIC_TEST_QUICK = "HealthCheck"; //!< method parameter for quick health check
	static const std::string NVDIMMDIAGNOSTIC_TEST_PLATFORM = "PlatformCheck"; //!< method parameter for platform config check
	static const std::string NVDIMMDIAGNOSTIC_TEST_STORAGE = "StorageCheck"; //!< method parameter for PM metadata check
	static const std::string NVDIMMDIAGNOSTIC_TEST_SECURITY = "SecurityCheck"; //!< method parameter for security check
	static const std::string NVDIMMDIAGNOSTIC_TEST_SETTING = "SettingCheck"; //!< method parameter for settings check

	// methods
	static const std::string NVDIMMDIAGNOSTIC_RUNDIAGNOSTICSERVICE = "RunDiagnosticService"; //!< method name

	static const std::string NVDIMMDIAGNOSTIC_SETTINGS = "DiagnosticSettings"; //!< method parameter for RunDiagnosticService
	static const std::string NVDIMMDIAGNOSTIC_RUNTYPE_KEY = "RunType"; //!< settings key for test type
	static const std::string NVDIMMDIAGNOSTIC_IGNORE_KEY = "Ignore"; //!< settings key for test statuses to ignore

	static const std::string NVDIMMDIAGNOSTICINPUT_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "NVDIMMDiagnosticInput"; //!< Creation ClassName static

	static const int NVDIMMDIAGNOSTIC_NUMTESTTYPES = 5;
	static const std::string validTestTypes[NVDIMMDIAGNOSTIC_NUMTESTTYPES] =
			{NVDIMMDIAGNOSTIC_TEST_QUICK,
			NVDIMMDIAGNOSTIC_TEST_PLATFORM,
			NVDIMMDIAGNOSTIC_TEST_STORAGE,
			NVDIMMDIAGNOSTIC_TEST_SECURITY,
			NVDIMMDIAGNOSTIC_TEST_SETTING};

	// test names for ignoring health check diag results
	enum healthcheck_ignore
	{
		HC_IGNORE_HEALTHSTATE = 0,
		HC_IGNORE_TEMPERATURE,
		HC_IGNORE_SPARE,
		HC_IGNORE_WEAR,
		HC_IGNORE_POWERLOSSPROTECTION,
		HC_IGNORE_INTERFACE,
		HC_IGNORE_THERMALTHROTTLE,
		HC_IGNORE_UNCORRECTABLE,
		HC_IGNORE_CORRECTABLE,
		HC_IGNORE_ERASURE_CODE_CORRECTABLE,
		HC_IGNORE_VENDORID,
		HC_IGNORE_MANUFACTURER,
		HC_IGNORE_MODELNUMBER
	};

	// test names for ignoring platform diag results
	enum platform_ignore
	{
		PF_IGNORE_NFITHEADER = 100,
		PF_IGNORE_CAPABILITYTABLE,
		PF_IGNORE_CONFIGDATA,
		PF_IGNORE_CURRENTCONFIG,
		PF_IGNORE_DIMMSCONFIGURED,
		PF_IGNORE_CONFIGERR,
		PF_IGNORE_SPA,
#if 0 // TODO: Rally DE4333 Don't have enough information to implement these yet.
		PF_IGNORE_PERMIT,
		PF_IGNORE_POPULATIONRULES,
#endif

	};

	// test names for ignoring storage diag results
	enum storage_ignore
	{
		ST_IGNORE_DIMMSPRESENT = 200,
		ST_IGNORE_LABELSCOMPLETE = 201,
		ST_IGNORE_VOLUMECONSISTENCY = 203
	};

	// test names for ignoring security diag results
	enum security_ignore
	{
		SEC_IGNORE_SECDISABLED = 300,
		SEC_IGNORE_SECCONSISTENT
	};

	// test names for ignoring settings diag results
	enum settings_ignore
	{
		SET_IGNORE_FWCONSISTENT = 400,
		SET_IGNORE_TEMPMEDIATHRESHOLD,
		SET_IGNORE_TEMPCONTROLLERTHRESHOLD,
		SET_IGNORE_SPARETHRESHOLD,
		SET_IGNORE_POW_MGMT_POLICIES,
		SET_IGNORE_DIE_SPARING_POLICIES,
		SET_IGNORE_TIME,
		SET_IGNORE_DEBUGLOG
	};

	static const NVM_UINT32 NVDIMMDIAGNOSTIC_ERR_NOT_SUPPORTED = 1;
	static const NVM_UINT32 NVDIMMDIAGNOSTIC_ERR_FAILED = 4;
	static const NVM_UINT32 NVDIMMDIAGNOSTIC_ERR_INVALID_PARAMETER = 5;

/*!
 * Models the physical aspects of an Intel NVDIMM
 */
class NVM_API NVDIMMDiagnosticFactory : public framework_interface::NvmInstanceFactory
{
	public:

		/*!
		 * Initialize a new SupportDataServiceFactory.
		 */
		NVDIMMDiagnosticFactory() throw (framework::Exception);

		/*!
		 * Clean up the SupportDataServiceFactory
		 */
		~NVDIMMDiagnosticFactory();

		/*!
		 * Implementation of the standard CIM method to retrieve a specific instance
		 * @param[in] path
		 * 		The object path of the instance to retrieve.
		 * @param[in] attributes
		 * 		The attributes to retrieve.
		 * @throw Exception if unable to retrieve the DIMM info
		 * @return The NVDIMMDiagnostic instance.
		 */
		framework::Instance* getInstance(framework::ObjectPath &path,
			framework::attribute_names_t &attributes) throw (framework::Exception);

		/*!
		 * Implementation of the standard CIM method to retrieve a list of
		 * NVDIMMDiagnostic object paths.
		 * @throw Exception if unable to retrieve the DIMM list
		 * @throw Exception if unable to retrieve the server name
		 * @return The object paths for each NVDIMMDiagnostic.
		 */
		framework::instance_names_t* getInstanceNames() throw (framework::Exception);

		/*!
		 * Provider for extern int nvm_run_diagnostic
		 * @param[in] device_uid
		 * 		The device identifier.
		 * @param[in] p_diagnostic
		 * 		A pointer to a #diagnostic structure containing the
		 * 		diagnostic to run allocated by the caller.
		 * @param[in,out] p_results
		 * 		The number of diagnostic failures. To see full results use #nvm_get_diagnostic_result
		 */
		int (*m_RunDiagProvider)(const NVM_UID device_uid,
				const struct diagnostic *p_diagnostic,
				NVM_UINT32 *p_results);

		/*!
		 * Helper method to verify test type is valid.
		 * @param testType test type in question
		 * @return whether or not testType is a valid type
		 */
		static bool testTypeValid(std::string testType);

		// Extrinsic method
		/*!
		 * Implementation of the standard CIM method to capture system support information
		 * @param ManagedElement The NVMDIMM against which to run the diagnostic
		 * @param SettingsInstance Embedded DiagnosticSetting instance
		 * @param testType test type as defined for NAME_KEY
		 * @throw Exception if unable to run the diagnostics
		 */
		void RunDiagnosticService(NVM_UID device_uid,
				framework::UINT16_LIST ignoreList,
				std::string testType)
			throw (framework::Exception);

		wbem::framework::UINT32 executeMethod(
				wbem::framework::UINT32 &wbem_return,
				const std::string method,
				wbem::framework::ObjectPath &object,
				wbem::framework::attributes_t &inParms,
				wbem::framework::attributes_t &outParms);

	bool  isAssociated(const std::string &associationClass, framework::Instance *pAntInstance,
			framework::Instance *pDepInstance);

	private:
		void populateAttributeList(framework::attribute_names_t &attributes)
			throw (framework::Exception);

		struct diagnostic getDiagnosticStructure(const std::string &testType,
				const framework::UINT16_LIST &ignoreList);
		void validateObjectHostName(const wbem::framework::ObjectPath &object);
		framework::UINT16_LIST getDiagnosticIgnoreList(wbem::framework::attributes_t &inParms);
		void getUidFromManagedElement(wbem::framework::attributes_t &inParms,
				const std::string &testType,
				NVM_UID uid);
		framework::ObjectPath validateManagedElementObjectPath(
				const std::string &refPath, const std::string className);
};

} // support
} // wbem
#endif  // #ifndef _WBEM_SUPPORT_NVDIMMDIAGNOSTIC_FACTORY_H_
