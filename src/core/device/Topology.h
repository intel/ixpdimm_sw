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
#ifndef CR_MGMT_TOPOLOGY_H
#define CR_MGMT_TOPOLOGY_H


#include <string>
#include <nvm_management.h>
#include <core/Helper.h>
#include <common_types.h>
#include <system/jedec_manufacturer.h>
#include <utility.h>
#include <nvm_types.h>

#include <string>
#include <vector>
#include <string/s_str.h>
#include <sstream>
#include <core/exceptions/LibraryException.h>
#include <core/Collection.h>
#include "Device.h"

namespace core
{
namespace device
{
class NVM_API Topology
{
public:
	Topology(const memory_topology &topology, const device_discovery &device) :
		m_pDetails(NULL)
	{
		memmove(&m_topology, &topology, sizeof(m_topology));
		memmove(&m_device, &device, sizeof(m_device));
	}

	Topology(const Topology &other) :
		m_pDetails(NULL)
	{
		this->m_topology = other.m_topology;
		this->m_device = other.m_device;
	}

	Topology &operator=(const Topology &other)
	{
		if (&other == this)
			return *this;
		this->m_topology = other.m_topology;
		this->m_device = other.m_device;
		return *this;
	}

	virtual ~Topology() { }

	virtual Topology *clone();
	virtual std::string getUid();
	virtual NVM_UINT32 getDeviceHandle();
	virtual NVM_UINT32 getChannelPosition();
        virtual NVM_UINT32 getChannelId();
        virtual NVM_UINT16 getSocketId();
        virtual NVM_UINT16 getMemoryControllerId();
        virtual NVM_UINT16 getNodeControllerId();
        virtual NVM_UINT16 getPhysicalID();
	virtual enum memory_type getMemoryType();
	virtual enum device_form_factor getFormFactor();
	virtual NVM_UINT64 getRawCapacity();
	virtual NVM_UINT64 getDataWidth();
	virtual NVM_UINT64 getTotalWidth();
	virtual NVM_UINT64 getSpeed();
	virtual std::string getPartNumber();
	virtual std::string getDeviceLocator();
	virtual std::string getBankLabel();

private:
	const memory_topology &getTopology();
	const device_discovery &getDiscovery();
	device_details *m_pDetails;
	memory_topology m_topology;
	device_discovery m_device;
};

class NVM_API TopologyCollection : public Collection<Topology>
{
};
}
}
#endif //CR_MGMT_TOPOLOGY_H
