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

#include <LogEnterExit.h>
#include "SystemMemoryResources.h"

namespace core
{
namespace system
{

SystemMemoryResources::SystemMemoryResources() :
		m_capacities()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

SystemMemoryResources::SystemMemoryResources(struct device_capacities device_capacities) :
		m_capacities(device_capacities)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

SystemMemoryResources::SystemMemoryResources(const SystemMemoryResources &other) :
		m_capacities(other.m_capacities)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	copy(other);
}

SystemMemoryResources &SystemMemoryResources::operator=(const SystemMemoryResources &other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	if (&other == this)
		return *this;

	copy(other);

	return *this;
}

void SystemMemoryResources::copy(const SystemMemoryResources &other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	this->m_capacities = other.m_capacities;
}

SystemMemoryResources::~SystemMemoryResources()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

SystemMemoryResources *SystemMemoryResources::clone()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return new SystemMemoryResources(*this);
}

NVM_UINT64 SystemMemoryResources::getTotalCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_capacities.capacity;
}

NVM_UINT64 SystemMemoryResources::getTotalMemoryCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_capacities.memory_capacity;
}

NVM_UINT64 SystemMemoryResources::getTotalAppDirectCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_capacities.app_direct_capacity;
}

NVM_UINT64 SystemMemoryResources::getTotalStorageCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_capacities.storage_capacity;
}

NVM_UINT64 SystemMemoryResources::getTotalUnconfiguredCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_capacities.unconfigured_capacity;
}

NVM_UINT64 core::system::SystemMemoryResources::getTotalInaccessibleCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_capacities.inaccessible_capacity;
}

NVM_UINT64 SystemMemoryResources::getTotalReservedCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_capacities.reserved_capacity;
}

}
}
