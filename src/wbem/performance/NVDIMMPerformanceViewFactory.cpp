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
 * This file contains the provider for the NVDIMMPerformanceView instances
 * which aggregate all the performance metrics for an NVM DIMM.
 */

#include <LogEnterExit.h>
#include <nvm_management.h>
#include <uid/uid.h>
#include <server/BaseServerFactory.h>
#include <physical_asset/NVDIMMFactory.h>
#include "NVDIMMPerformanceViewFactory.h"

#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>

wbem::performance::NVDIMMPerformanceViewFactory::NVDIMMPerformanceViewFactory()
throw (wbem::framework::Exception)
{ }

wbem::performance::NVDIMMPerformanceViewFactory::~NVDIMMPerformanceViewFactory()
{ }

void wbem::performance::NVDIMMPerformanceViewFactory::populateAttributeList(framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	// add key attributes
	attributes.push_back(INSTANCEID_KEY);

	// add non-key attributes
	attributes.push_back(DIMMID_KEY);
	attributes.push_back(DIMMHANDLE_KEY);
	attributes.push_back(BYTESREAD_KEY);
	attributes.push_back(BYTESWRITTEN_KEY);
	attributes.push_back(HOSTWRITECOMMANDS_KEY);
	attributes.push_back(HOSTREADREQUESTS_KEY);
	attributes.push_back(BLOCKWRITECOMMANDS_KEY);
	attributes.push_back(BLOCKREADREQUESTS_KEY);
}

/*
 * Retrieve a specific instance given an object path
 */
wbem::framework::Instance* wbem::performance::NVDIMMPerformanceViewFactory::getInstance(
		framework::ObjectPath &path, framework::attribute_names_t &attributes)
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// create the instance, initialize with attributes from the path
	framework::Instance *pInstance = new framework::Instance(path);
	try
	{

		std::string uidStr = path.getKeyValue(INSTANCEID_KEY).stringValue();
		NVM_UID uid;
		uid_copy(uidStr.c_str(), uid);
		int rc;
		struct device_performance performance;
		if ((rc = nvm_get_device_performance(uid, &performance)) != NVM_SUCCESS)
		{
			throw wbem::exception::NvmExceptionLibError(rc);
		}

		checkAttributes(attributes);

		// DimmID = handle or uid depending on user selection
		if (containsAttribute(DIMMID_KEY, attributes))
		{
			framework::Attribute attrDimmId = physical_asset::NVDIMMFactory::uidToDimmIdAttribute(uidStr);
			pInstance->setAttribute(DIMMID_KEY, attrDimmId, attributes);
		}
		// DimmHandle
		if (containsAttribute(DIMMHANDLE_KEY, attributes))
		{
			NVM_UINT32 dimmHandle;
			physical_asset::NVDIMMFactory::uidToHandle(uidStr, dimmHandle);
			framework::Attribute attrDimmHandle(dimmHandle, false);
			pInstance->setAttribute(DIMMHANDLE_KEY, attrDimmHandle, attributes);
		}
		// BytesRead - Identical to the PerformanceMetric.MetricValue for the bytesread PerformanceMetric instance (but reported here as an unsigned int).
		if (containsAttribute(BYTESREAD_KEY, attributes))
		{
			framework::Attribute a(performance.bytes_read, false);
			pInstance->setAttribute(BYTESREAD_KEY, a, attributes);
		}

		// BytesWritten - Identical to the PerformanceMetric.MetricValue for the byteswritten PerformanceMetric instance (but reported here as an unsigned int).
		if (containsAttribute(BYTESWRITTEN_KEY, attributes))
		{
			framework::Attribute a(performance.bytes_written, false);
			pInstance->setAttribute(BYTESWRITTEN_KEY, a, attributes);
		}

		// HostWriteCommands - Identical to the PerformanceMetric.MetricValue for the by host writes PerformanceMetric instance (but reported here as an unsigned int).
		if (containsAttribute(HOSTWRITECOMMANDS_KEY, attributes))
		{
			framework::Attribute a(performance.host_writes, false);
			pInstance->setAttribute(HOSTWRITECOMMANDS_KEY, a, attributes);
		}

		// HostReadRequests - Identical to the PerformanceMetric.MetricValue for the by host reads PerformanceMetric instance (but reported here as an unsigned int).
		if (containsAttribute(HOSTREADREQUESTS_KEY, attributes))
		{
			framework::Attribute a(performance.host_reads, false);
			pInstance->setAttribute(HOSTREADREQUESTS_KEY, a, attributes);
		}

		// BlockWriteCommands - Identical to the PerformanceMetric.MetricValue for the by host writes PerformanceMetric instance (but reported here as an unsigned int).
		if (containsAttribute(BLOCKWRITECOMMANDS_KEY, attributes))
		{
			framework::Attribute a(performance.block_writes, false);
			pInstance->setAttribute(BLOCKWRITECOMMANDS_KEY, a, attributes);
		}

		// BlockReadRequests - Identical to the PerformanceMetric.MetricValue for the by BLOCK reads PerformanceMetric instance (but reported here as an unsigned int).
		if (containsAttribute(BLOCKREADREQUESTS_KEY, attributes))
		{
			framework::Attribute a(performance.block_reads, false);
			pInstance->setAttribute(BLOCKREADREQUESTS_KEY, a, attributes);
		}
	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pInstance;
		throw;
	}

	return pInstance;
}

/*
 * Return the object paths for the NVDIMMPerformanceView class.
 */
wbem::framework::instance_names_t* wbem::performance::NVDIMMPerformanceViewFactory::getInstanceNames()
throw (wbem::framework::Exception)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	framework::instance_names_t *pNames = new framework::instance_names_t();
	try
	{
		framework::attributes_t keys;
		std::vector<std::string> uids = physical_asset::NVDIMMFactory::getManageableDeviceUids();
		for (size_t i = 0; i < uids.size(); i ++)
		{
			// DIMM UID
			keys[INSTANCEID_KEY] = framework::Attribute(std::string(uids[i]), true);
			framework::ObjectPath path(server::getHostName(), NVM_NAMESPACE,
					NVDIMMPERFORMANCEVIEW_CREATIONCLASSNAME, keys);
			pNames->push_back(path);
		}

	}
	catch (framework::Exception &) // clean up and re-throw
	{
		delete pNames;
		throw;
	}
	return pNames;
}
