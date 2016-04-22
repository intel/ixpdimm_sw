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
#include "Topology.h"

const memory_topology &core::device::Topology::getTopology()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_topology;
}
const device_discovery &core::device::Topology::getDiscovery()
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
        return m_device;
}
std::string core::device::Topology::getUid()
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
        return Helper::uidToString(getDiscovery().uid);
}
NVM_UINT32 core::device::Topology::getDeviceHandle()
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
        return getDiscovery().device_handle.handle;
}
NVM_UINT32 core::device::Topology::getChannelPosition()
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
        return getDiscovery().device_handle.parts.mem_channel_dimm_num;
}
NVM_UINT32 core::device::Topology::getChannelId()
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
        return getDiscovery().device_handle.parts.mem_channel_id;
}
NVM_UINT16 core::device::Topology::getSocketId()
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
        return getDiscovery().socket_id;
}
NVM_UINT16 core::device::Topology::getMemoryControllerId()
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
        return getDiscovery().memory_controller_id;
}
NVM_UINT16 core::device::Topology::getNodeControllerId()
{
        LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
        return getDiscovery().node_controller_id;
}
NVM_UINT16 core::device::Topology::getPhysicalID()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().physical_id;
}
enum memory_type core::device::Topology::getMemoryType()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().memory_type;
}
enum device_form_factor core::device::Topology::getFormFactor()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().form_factor;
}
NVM_UINT64 core::device::Topology::getRawCapacity()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().raw_capacity;
}
NVM_UINT64 core::device::Topology::getDataWidth()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().data_width;
}
NVM_UINT64 core::device::Topology::getTotalWidth()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().total_width;
}
NVM_UINT64 core::device::Topology::getSpeed()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().speed;
}
std::string core::device::Topology::getPartNumber()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().part_number;
}
std::string core::device::Topology::getDeviceLocator()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().device_locator;
}
std::string core::device::Topology::getBankLabel()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return getTopology().bank_label;
}
core::device::Topology *core::device::Topology::clone()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return new Topology(*this);
}
