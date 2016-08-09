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

#include <core/exceptions/InvalidArgumentException.h>
#include <core/exceptions/LibraryException.h>
#include <core/exceptions/NoMemoryException.h>
#include "NvmLibrary.h"
#include "NvmLibrary.h"
#include "Helper.h"

namespace core
{

NvmLibrary &NvmLibrary::getNvmLibrary()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
	static NvmLibrary *result = new NvmLibrary();
	return *result;
}

NvmLibrary::NvmLibrary(const LibWrapper &lib) : m_lib(lib)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

NvmLibrary::NvmLibrary(const NvmLibrary &other) : m_lib(other.m_lib)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

NvmLibrary &NvmLibrary::operator=(const NvmLibrary &other)
{
	return *this;
}

NvmLibrary::~NvmLibrary()
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);
}

std::string NvmLibrary::getErrorMessage(const int errorCode)
{
	LogEnterExit(__FUNCTION__, __FILE__, __LINE__);

	NVM_ERROR_DESCRIPTION errorMessage;
	m_lib.getError((const enum return_code)errorCode, errorMessage,
			sizeof (errorMessage));
	return errorMessage;
}

struct host NvmLibrary::getHost()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	struct host result;
	rc = m_lib.getHost(&result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

std::string NvmLibrary::getHostName()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	char result[NVM_COMPUTERNAME_LEN];
	rc = m_lib.getHostName(result, NVM_COMPUTERNAME_LEN);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return std::string(result);

}

struct sw_inventory NvmLibrary::getSwInventory()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	struct sw_inventory result;
	rc = m_lib.getSwInventory(&result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

int NvmLibrary::getSocketCount()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.getSocketCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

std::vector<struct socket> NvmLibrary::getSockets()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	std::vector<struct socket> result;
	rc = m_lib.getSocketCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	int count = rc;
	struct socket fromLib[count];

	rc = m_lib.getSockets(fromLib, count);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	for (int i = 0; i < count; i++)
	{
		result.push_back(fromLib[i]);
	}
	return result;

}

struct socket NvmLibrary::getSocket(const NVM_UINT16 socketId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	struct socket result;
	rc = m_lib.getSocket(socketId, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

struct nvm_capabilities NvmLibrary::getNvmCapabilities()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	struct nvm_capabilities result;
	rc = m_lib.getNvmCapabilities(&result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

struct device_capacities NvmLibrary::getNvmCapacities()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	struct device_capacities result;
	rc = m_lib.getNvmCapacities(&result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

int NvmLibrary::getMemoryTopologyCount()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.getMemoryTopologyCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

std::vector<struct memory_topology> NvmLibrary::getMemoryTopology()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	std::vector<struct memory_topology> result;
	rc = m_lib.getMemoryTopologyCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	int count = rc;
	struct memory_topology fromLib[count];

	rc = m_lib.getMemoryTopology(fromLib, count);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	for (int i = 0; i < count; i++)
	{
		result.push_back(fromLib[i]);
	}
	return result;

}

int NvmLibrary::getDeviceCount()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.getDeviceCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

struct device_discovery NvmLibrary::getDeviceDiscovery(const std::string &uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_uid;
	core::Helper::stringToUid(uid, lib_uid);

	struct device_discovery result;
	rc = m_lib.getDeviceDiscovery(lib_uid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

std::vector<struct device_discovery> NvmLibrary::getDevices()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	std::vector<struct device_discovery> result;
	rc = m_lib.getDeviceCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	int count = rc;
	struct device_discovery fromLib[count];
	memset(fromLib, 0, sizeof (struct device_discovery) * count);

	rc = m_lib.getDevices(fromLib, count);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	for (int i = 0; i < count; i++)
	{
		result.push_back(fromLib[i]);
	}
	return result;

}

struct device_status NvmLibrary::getDeviceStatus(const std::string &deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	struct device_status result;
	rc = m_lib.getDeviceStatus(lib_deviceUid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

struct device_settings NvmLibrary::getDeviceSettings(const std::string &deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	struct device_settings result;
	rc = m_lib.getDeviceSettings(lib_deviceUid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

void NvmLibrary::modifyDeviceSettings(const std::string &deviceUid,
	const struct device_settings &pSettings)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	rc = m_lib.modifyDeviceSettings(lib_deviceUid, &pSettings);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

struct device_details NvmLibrary::getDeviceDetails(const std::string &deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	struct device_details result;
	rc = m_lib.getDeviceDetails(lib_deviceUid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

struct device_performance NvmLibrary::getDevicePerformance(const std::string &deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	struct device_performance result;
	rc = m_lib.getDevicePerformance(lib_deviceUid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

void NvmLibrary::updateDeviceFw(const std::string &deviceUid, const std::string path,
	const bool activate, const bool force)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	char lib_path[path.size()];
	s_strcpy(lib_path, path.c_str(), path.size());

	rc = m_lib.updateDeviceFw(lib_deviceUid, lib_path, path.size(), activate, force);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::examineDeviceFw(const std::string &deviceUid, const std::string path,
	std::string imageVersion)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	char lib_path[path.size()];
	s_strcpy(lib_path, path.c_str(), path.size());

	char lib_imageVersion[imageVersion.size()];
	s_strcpy(lib_imageVersion, imageVersion.c_str(), imageVersion.size());

	rc = m_lib.examineDeviceFw(lib_deviceUid, lib_path, path.size(), lib_imageVersion,
		imageVersion.size());
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::setPassphrase(const std::string &deviceUid, const std::string oldPassphrase,
	const std::string newPassphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	char lib_oldPassphrase[oldPassphrase.size()];
	s_strcpy(lib_oldPassphrase, oldPassphrase.c_str(), oldPassphrase.size());

	char lib_newPassphrase[newPassphrase.size()];
	s_strcpy(lib_newPassphrase, newPassphrase.c_str(), newPassphrase.size());

	rc = m_lib.setPassphrase(lib_deviceUid, lib_oldPassphrase, oldPassphrase.size(),
		lib_newPassphrase, newPassphrase.size());
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::removePassphrase(const std::string &deviceUid, const std::string passphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	char lib_passphrase[passphrase.size()];
	s_strcpy(lib_passphrase, passphrase.c_str(), passphrase.size());

	rc = m_lib.removePassphrase(lib_deviceUid, lib_passphrase, passphrase.size());
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::unlockDevice(const std::string &deviceUid, const std::string passphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	char lib_passphrase[passphrase.size()];
	s_strcpy(lib_passphrase, passphrase.c_str(), passphrase.size());

	rc = m_lib.unlockDevice(lib_deviceUid, lib_passphrase, passphrase.size());
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::eraseDevice(const std::string &deviceUid,
	const std::string passphrase)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	char lib_passphrase[passphrase.size()];
	s_strcpy(lib_passphrase, passphrase.c_str(), passphrase.size());

	rc = m_lib.eraseDevice(lib_deviceUid, lib_passphrase, passphrase.size());
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

int NvmLibrary::getJobCount()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.getJobCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

std::vector<struct job> NvmLibrary::getJobs()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	std::vector<struct job> result;
	rc = m_lib.getJobCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	int count = rc;
	struct job fromLib[count];

	rc = m_lib.getJobs(fromLib, count);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	for (int i = 0; i < count; i++)
	{
		result.push_back(fromLib[i]);
	}
	return result;

}

int NvmLibrary::getPoolCount()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.getPoolCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

std::vector<struct pool> NvmLibrary::getPools()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	std::vector<struct pool> result;
	rc = m_lib.getPoolCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	if (rc > 0)
	{
		int count = rc;
		result.reserve(count);
		struct pool *fromLib = (struct pool *)malloc(sizeof (struct pool) * count);

		rc = m_lib.getPools(fromLib, count);
		if (rc < 0)
		{
			throw core::LibraryException(rc);
		}

		for (int i = 0; i < count; i++)
		{
			result.push_back(fromLib[i]);
		}
		free(fromLib);
	}

	return result;

}

struct pool *NvmLibrary::getPool(const std::string &poolUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc = 0;

	NVM_UID lib_poolUid;
	core::Helper::stringToUid(poolUid, lib_poolUid);

	struct pool *pPool = new struct pool;
	if (pPool == NULL)
	{
		throw core::NoMemoryException();
	}
	rc = m_lib.getPool(lib_poolUid, pPool);
	if (rc < 0)
	{
		delete pPool;
		throw core::LibraryException(rc);
	}

	return pPool;

}

struct possible_namespace_ranges NvmLibrary::getAvailablePersistentSizeRange(
	const std::string &poolUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_poolUid;
	core::Helper::stringToUid(poolUid, lib_poolUid);

	struct possible_namespace_ranges result;
	rc = m_lib.getAvailablePersistentSizeRange(lib_poolUid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

void NvmLibrary::createConfigGoal(const std::string &deviceUid, struct config_goal &pGoal)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	rc = m_lib.createConfigGoal(lib_deviceUid, &pGoal);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

struct config_goal NvmLibrary::getConfigGoal(const std::string &deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	struct config_goal result;
	rc = m_lib.getConfigGoal(lib_deviceUid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

void NvmLibrary::deleteConfigGoal(const std::string &deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	rc = m_lib.deleteConfigGoal(lib_deviceUid);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::dumpConfig(const std::string &deviceUid, const std::string file,
	const bool append)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	char lib_file[file.size()];
	s_strcpy(lib_file, file.c_str(), file.size());

	rc = m_lib.dumpConfig(lib_deviceUid, lib_file, file.size(), append);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::loadConfig(const std::string &deviceUid, const std::string file)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	char lib_file[file.size()];
	s_strcpy(lib_file, file.c_str(), file.size());

	rc = m_lib.loadConfig(lib_deviceUid, lib_file, file.size());
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

int NvmLibrary::getNamespaceCount()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.getNamespaceCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

int NvmLibrary::getDeviceNamespaceCount(const std::string &deviceUid,
	const enum namespace_type type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	rc = m_lib.getDeviceNamespaceCount(lib_deviceUid, type);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

std::vector<struct namespace_discovery> NvmLibrary::getNamespaces()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	std::vector<struct namespace_discovery> result;
	rc = m_lib.getNamespaceCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	int count = rc;
	struct namespace_discovery fromLib[count];

	rc = m_lib.getNamespaces(fromLib, count);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	for (int i = 0; i < count; i++)
	{
		result.push_back(fromLib[i]);
	}
	return result;

}

struct namespace_details NvmLibrary::getNamespaceDetails(const std::string &namespaceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_namespaceUid;
	core::Helper::stringToUid(namespaceUid, lib_namespaceUid);

	struct namespace_details result;
	rc = m_lib.getNamespaceDetails(lib_namespaceUid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

std::string NvmLibrary::createNamespace(const std::string &pool_uid,
	struct namespace_create_settings &p_settings, const struct interleave_format &p_format,
	const bool allow_adjustment)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;
	NVM_UID lib_pool_uid;
	core::Helper::stringToUid(pool_uid, lib_pool_uid);

	NVM_UID fromLibNamespaceUid;
	rc = m_lib.createNamespace(&fromLibNamespaceUid, lib_pool_uid, &p_settings, &p_format,
		allow_adjustment);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return core::Helper::uidToString(fromLibNamespaceUid);
}

void NvmLibrary::modifyNamespaceName(const std::string &namespaceUid,
	const std::string &name)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_namespaceUid;
	core::Helper::stringToUid(namespaceUid, lib_namespaceUid);
	NVM_NAMESPACE_NAME lib_name;
	memmove(lib_name, name.c_str(), NVM_NAMESPACE_NAME_LEN);
	rc = m_lib.modifyNamespaceName(lib_namespaceUid, lib_name);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

int NvmLibrary::modifyNamespaceBlockCount(const std::string &namespaceUid,
	const NVM_UINT64 blockCount, bool allowAdjustment)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_namespaceUid;
	core::Helper::stringToUid(namespaceUid, lib_namespaceUid);

	rc = m_lib.modifyNamespaceBlockCount(lib_namespaceUid, blockCount, allowAdjustment);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

void NvmLibrary::modifyNamespaceEnabled(const std::string &namespaceUid,
	const enum namespace_enable_state enabled)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_namespaceUid;
	core::Helper::stringToUid(namespaceUid, lib_namespaceUid);

	rc = m_lib.modifyNamespaceEnabled(lib_namespaceUid, enabled);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::deleteNamespace(const std::string &namespaceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_namespaceUid;
	core::Helper::stringToUid(namespaceUid, lib_namespaceUid);

	rc = m_lib.deleteNamespace(lib_namespaceUid);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::adjustCreateNamespaceBlockCount(const std::string &poolUid,
	struct namespace_create_settings &pSettings, const struct interleave_format &pFormat)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_poolUid;
	core::Helper::stringToUid(poolUid, lib_poolUid);

	rc = m_lib.adjustCreateNamespaceBlockCount(lib_poolUid, &pSettings, &pFormat);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
}

void NvmLibrary::adjustModifyNamespaceBlockCount(const std::string &namespaceUid,
	NVM_UINT64 &pBlockCount)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_namespaceUid;
	core::Helper::stringToUid(namespaceUid, lib_namespaceUid);

	rc = m_lib.adjustModifyNamespaceBlockCount(lib_namespaceUid, &pBlockCount);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
}

std::vector<struct sensor> NvmLibrary::getSensors(const std::string &deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	std::vector<struct sensor> result;
	struct sensor sensors[NVM_MAX_DEVICE_SENSORS];
	rc = m_lib.getSensors(lib_deviceUid, sensors, NVM_MAX_DEVICE_SENSORS);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	for (int i = 0; i < NVM_MAX_DEVICE_SENSORS; i++)
	{
		result.push_back(sensors[i]);
	}

	return result;

}

struct sensor NvmLibrary::getSensor(const std::string &deviceUid, const enum sensor_type type)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	struct sensor result;
	rc = m_lib.getSensor(lib_deviceUid, type, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

void NvmLibrary::setSensorSettings(const std::string &deviceUid, const enum sensor_type type,
	const struct sensor_settings &pSettings)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	rc = m_lib.setSensorSettings(lib_deviceUid, type, &pSettings);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::addEventNotify(const enum event_type type,
	void (*pEventCallback)(struct event *pEvent))
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.addEventNotify(type, pEventCallback);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::removeEventNotify(const int callbackId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.removeEventNotify(callbackId);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

int NvmLibrary::getEventCount(const struct event_filter &pFilter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.getEventCount(&pFilter);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

std::vector<struct event> NvmLibrary::getEvents(const struct event_filter &pFilter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	std::vector<struct event> result;
	rc = m_lib.getEventCount(&pFilter);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	if (rc > 0)
	{
		int count = rc;
		struct event fromLib[count];
		rc = m_lib.getEvents(&pFilter, fromLib, count);
		if (rc < 0)
		{
			throw core::LibraryException(rc);
		}

		for (int i = 0; i < count; i++)
		{
			result.push_back(fromLib[i]);
		}
	}

	return result;

}

void NvmLibrary::purgeEvents(const struct event_filter &pFilter)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.purgeEvents(&pFilter);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::acknowledgeEvent(NVM_UINT32 eventId)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.acknowledgeEvent(eventId);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::saveState(const std::string name)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	char lib_name[name.size()];
	s_strcpy(lib_name, name.c_str(), name.size());

	rc = m_lib.saveState(lib_name, name.size());
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::purgeStateData()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.purgeStateData();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::gatherSupport(const std::string supportFile)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	char lib_supportFile[supportFile.size()];
	s_strcpy(lib_supportFile, supportFile.c_str(), supportFile.size());

	rc = m_lib.gatherSupport(lib_supportFile, supportFile.size());
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::runDiagnostic(const std::string &deviceUid, const struct diagnostic &pDiagnostic,
	NVM_UINT32 &pResults)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	rc = m_lib.runDiagnostic(lib_deviceUid, &pDiagnostic, &pResults);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

enum fw_log_level NvmLibrary::getFwLogLevel(const std::string &deviceUid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	enum fw_log_level result;
	rc = m_lib.getFwLogLevel(lib_deviceUid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

void NvmLibrary::setFwLogLevel(const std::string &deviceUid, const enum fw_log_level logLevel)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	rc = m_lib.setFwLogLevel(lib_deviceUid, logLevel);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

struct device_fw_info NvmLibrary::getDeviceFwInfo(const std::string &device_uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_device_uid;
	core::Helper::stringToUid(device_uid, lib_device_uid);

	struct device_fw_info result;
	rc = m_lib.getDeviceFwInfo(lib_device_uid, &result);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return result;

}

void NvmLibrary::injectDeviceError(const std::string &deviceUid, const struct device_error &pError)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	rc = m_lib.injectDeviceError(lib_deviceUid, &pError);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::clearInjectedDeviceError(const std::string &deviceUid,
	const struct device_error &pError)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	NVM_UID lib_deviceUid;
	core::Helper::stringToUid(deviceUid, lib_deviceUid);

	rc = m_lib.clearInjectedDeviceError(lib_deviceUid, &pError);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::addSimulator(const std::string simulator)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	char lib_simulator[simulator.size()];
	s_strcpy(lib_simulator, simulator.c_str(), simulator.size());

	rc = m_lib.addSimulator(lib_simulator, simulator.size());
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

void NvmLibrary::removeSimulator()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.removeSimulator();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

bool NvmLibrary::isDebugLoggingEnabled()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.debugLoggingEnabled();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	return rc == 1;

}

void NvmLibrary::toggleDebugLogging(const bool enabled)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.toggleDebugLogging(enabled);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}

int NvmLibrary::getDebugLogCount()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.getDebugLogCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	return rc;

}

std::vector<struct log> NvmLibrary::getDebugLogs()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	std::vector<struct log> result;
	rc = m_lib.getDebugLogCount();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}
	int count = rc;
	struct log fromLib[count];

	rc = m_lib.getDebugLogs(fromLib, count);
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

	for (int i = 0; i < count; i++)
	{
		result.push_back(fromLib[i]);
	}
	return result;

}

void NvmLibrary::purgeDebugLog()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	int rc;

	rc = m_lib.purgeDebugLog();
	if (rc < 0)
	{
		throw core::LibraryException(rc);
	}

}
}

