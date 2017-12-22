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
 * This file contains the helper functions for interfacing with the NVM Wbem Library.
 */


#ifndef _CLI_FRAMEWORK_WBEMTOCLI_UTILITIES_H
#define _CLI_FRAMEWORK_WBEMTOCLI_UTILITIES_H

#include <nvm_management.h>

#include <libinvm-cli/PropertyListResult.h>
#include <libinvm-cli/ObjectListResult.h>
#include <libinvm-cim/Instance.h>
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <cli/features/core/StringList.h>
#include <cli/features/ExportCli.h>

namespace cli
{
namespace nvmcli
{

static const std::string CAPACITY_UNITS_GB = "GB";

// constants used in IDEMA formula for LBA count to Advertised Capacity conversion
#define IDEMA_CONVERSION_CONSTANT1 (NVM_UINT64) 97696368
#define IDEMA_CONVERSION_CONSTANT2 (NVM_UINT64) 1953504
#define IDEMA_CONVERSION_CONSTANT3 50

struct instanceFilter
{
	std::string attributeName;
	std::vector<std::string> attributeValues;
};

typedef std::vector<struct instanceFilter> filters_t;

/*!
 *
 * @param instance
 * 		Instance to convert
 * @return
 * 		pointer to a new PropertyListResult.  It's up to the caller to free this.
 */
NVM_CLI_API cli::framework::PropertyListResult *NvmInstanceToPropertyListResult(
		const wbem::framework::Instance &instance,
		const wbem::framework::attribute_names_t &attributes = wbem::framework::attribute_names_t(),
		const std::string name = "");

NVM_CLI_API int filterInstances(const wbem::framework::instances_t &instances,
			const std::string &name,
			const cli::nvmcli::filters_t &filters,
			wbem::framework::instances_t &matchedInstances,
			bool checkMatches = true);
/*!
 * Convert a list of NvmInstances to an ObjectListResult
 * @param instances
 * 		instances to convert
 * @param headerPrefix
 * 		a prefix to the header for each section (or if headerProperty is empty the entire header)
 * @param headerProperty
 * 		which property value to use as the section header
 * @param filters
 * 		one or more result filters
 * @return
 */
NVM_CLI_API cli::framework::ObjectListResult *NvmInstanceToObjectListResult(
		const wbem::framework::instances_t &instances,
		const std::string &headerPrefix,
		const std::string &headerProperty,
		const wbem::framework::attribute_names_t &attributes = wbem::framework::attribute_names_t(),
		const filters_t &filters = filters_t());

/*!
 * Get display appropriate options and convert the list of attribute names that should be displayed
 * @param options
 * 		The complete list of options the user entered
 * @return
 * 		The result will be passed down to the WBEM instances
 */
NVM_CLI_API wbem::framework::attribute_names_t GetAttributeNames(const cli::framework::StringMap &options);

/*!
 * Get display appropriate options and convert the list of attribute names that should be displayed
 * @param options
 * 		The complete list of options the user entered
 * @param defaultNames
 * 		Some Instances don't list all attributes by default and only show a subset of default.
 * @return
 * 		The result will be passed down to the WBEM instances
 */
NVM_CLI_API wbem::framework::attribute_names_t GetAttributeNames(
		const cli::framework::StringMap &options,
		const wbem::framework::attribute_names_t defaultNames);

/*!
 * Get display appropriate options and convert the list of attribute names that should be displayed
 * @param options
 * 		The complete list of options the user entered
 * @param defaultNames
 * 		Some Instances don't list all attributes by default and only show a subset of default.
 * @param allNames
 * 		An optional list of a subset of attributes that should be returned/displayed
 * @return
 * 		The result will be passed down to the WBEM instances
 */
NVM_CLI_API wbem::framework::attribute_names_t GetAttributeNames(
		const cli::framework::StringMap &options,
		const wbem::framework::attribute_names_t defaultNames,
		const wbem::framework::attribute_names_t allNames);

NVM_CLI_API cli::framework::ErrorResult *GetRequestedCapacityUnits(const cli::framework::ParsedCommand& parsedCommand, std::string &units);

NVM_CLI_API bool handleToUid(const NVM_UINT32 &handle, std::string &dimmUid);

NVM_CLI_API std::string getUidFromTarget(std::string dimmUid);

/*!
 * Generate a dimm filter based on the parsed target
 */
NVM_CLI_API void generateDimmFilter(
		const cli::framework::ParsedCommand& parsedCommand,
		wbem::framework::attribute_names_t &attributes,
		cli::nvmcli::filters_t &filters,
		std::string dimmAttributeKey = wbem::DIMMUID_KEY);

/*!
 * Generate a socket filter based on the parsed target
 */
NVM_CLI_API void generateSocketFilter(
		const cli::framework::ParsedCommand& parsedCommand,
		wbem::framework::attribute_names_t &attributes,
		cli::nvmcli::filters_t &filters);

/*
 * Generically create a filter for some target's values.
 */
NVM_CLI_API void generateFilterForAttributeWithTargetValues(const cli::framework::ParsedCommand& parsedCommand,
                                                const std::string &target,
                                                const std::string &attributeName, cli::nvmcli::filters_t &filters);

/*!
 * For commands that support an optional -dimm target, retrieve the dimm UID of the specified target
 * or all manageable dimm UIDs if not specified. Return a syntax error if the dimm target value
 * is invalid.
 */
NVM_CLI_API cli::framework::ErrorResult *getDimms(
		const framework::ParsedCommand &parsedCommand,
		std::vector<std::string> &dimms);

/*!
 * For commands that support an optional -socket target, retrieve the dimm UID for all dimms on the
 * specified socket(s). Return a syntax error if the socket target value is invalid.
 */
NVM_CLI_API cli::framework::ErrorResult *getDimmsFromSockets(
		const framework::ParsedCommand &parsedCommand,
		std::vector<std::string> &dimms);


/*!
 * RenameAttributeKey will replace the key (in an instance) or the value (in an
 * attribute names list) with the string provided. This is helpful when a CLI "property" is
 * different than the attribute from a CIM instance.
 *
 */
NVM_CLI_API void RenameAttributeKey(wbem::framework::instances_t &instances,
		std::string from, std::string to);
NVM_CLI_API void RenameAttributeKey(wbem::framework::attribute_names_t &attributes,
		std::string from, std::string to);
NVM_CLI_API void RenameAttributeKey(wbem::framework::instances_t &instances,
		wbem::framework::attribute_names_t &attributes,
		std::string from, std::string to);

NVM_CLI_API void RemoveAttributeName(wbem::framework::attribute_names_t &attributes, std::string nameToRemove);

/*!
 * Given a list of attribute names, remove any that don't match the filter.  If the filter is
 * empty, then attributes is not modified
 *
 */
NVM_CLI_API void FilterAttributeNames(wbem::framework::attribute_names_t &attributes, std::string filter);

/*!
 * Helper function to convert an Exception to an appropriate Result
 */
NVM_CLI_API cli::framework::ErrorResult *NvmExceptionToResult(wbem::framework::Exception &e,
		std::string prefix = "");
NVM_CLI_API cli::framework::ErrorResult *CoreExceptionToResult(std::exception &e,
		std::string prefix = "");

/*
 * Helper function to set an error code on a result
 */
NVM_CLI_API void SetResultErrorCodeFromException(cli::framework::ResultBase &result,
		wbem::framework::Exception &e, std::string prefix = "");

/*!
 * Helper function to convert a string to a UINT64
 */
NVM_CLI_API NVM_UINT64 stringToUInt64(const std::string& value);

/*!
 * Helper function to convert a string to float
 */
NVM_CLI_API bool stringToReal32(const std::string& str, NVM_REAL32 *p_value);

/*!
 * Helper function to convert a string to int
 */
NVM_CLI_API bool stringToInt(const std::string& str, int* p_value);

/*!
 * Detect whether a string represents a valid number.
 * Input may be in hex or decimal format.
 */
NVM_CLI_API bool isStringValidNumber(const std::string &value);

NVM_CLI_API bool isStringHex(const std::string &value);

NVM_CLI_API NVM_UINT64 calculateBlockCountForNamespace(const NVM_UINT64 capacityInGB,
	const NVM_UINT64 blockSize);

NVM_CLI_API std::string convertCapacityFormat(NVM_UINT64 capacityInBytes, std::string capacityUnits = "");

NVM_CLI_API std::string translateCapacityToRequestedUnits(NVM_UINT64 capacityInBytes, std::string units);

NVM_CLI_API void convertCapacityAttribute(wbem::framework::Instance &wbemInstance,
	const std::string attributeName, const std::string capacityUnits = "");

NVM_CLI_API NVM_UINT64 convertCapacityToBytes(std::string capacityUnits, const NVM_REAL32 capacity);

NVM_CLI_API void findBestCapacityFormatInBinaryMultiples(const NVM_UINT64 capacityInBytes, std::string &units);

NVM_CLI_API void findBestCapacityFormatInDecimalMultiples(const NVM_UINT64 capacityInBytes, std::string &units);

NVM_CLI_API std::string AttributeToString(const wbem::framework::Attribute &attr);

NVM_CLI_API std::string AttributeToHexString(const wbem::framework::Attribute &attr);

NVM_CLI_API std::string getInvalidDimmIdErrorString(const std::string &invalidDimmId);

NVM_CLI_API std::string getInvalidPoolIdErrorString(const std::string &invalidPoolId);

NVM_CLI_API std::string uint64ToString(const unsigned long long &value);
NVM_CLI_API std::string uint64ToHexString(const unsigned long long &value);

#define SET_PROVIDER(currentProvider, newProvider) \
	if (newProvider != NULL) \
	{ \
		delete currentProvider; \
		currentProvider = newProvider; \
	}

// Not using IDEMA capacity conversions for now, but we might revert to this in the future
#if 0
std::string calculateAdvertisedCapacity(NVM_UINT64 capacityInBytes,
	const NVM_UINT64 blockCount = 0, const NVM_UINT64 blockSize = 0);

void convertCapacityAttributeToGB(wbem::framework::Instance &wbemInstance,
		const std::string attributeName);
#endif

}
}
#endif //_CLI_FRAMEWORK_WBEMTOCLI_UTILITIES_H
