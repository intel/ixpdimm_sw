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
#include "DeviceFirmwareInfoCollection.h"
#include <iostream>

core::device::DeviceFirmwareInfoCollection::DeviceFirmwareInfoCollection(
	const DeviceFirmwareInfoCollection &other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	*this = other;
}

core::device::DeviceFirmwareInfoCollection& core::device::DeviceFirmwareInfoCollection::operator=(
		const DeviceFirmwareInfoCollection& other)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	clear();
	for (std::map<std::string, DeviceFirmwareInfo *>::const_iterator iter = other.m_collection.begin();
			iter != other.m_collection.end(); iter++)
	{
		DeviceFirmwareInfo *pOther = iter->second;
		if (pOther)
		{
			m_collection[iter->first] = pOther->clone();
		}
	}

	return *this;
}

core::device::DeviceFirmwareInfoCollection::~DeviceFirmwareInfoCollection()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	clear();
}

const core::device::DeviceFirmwareInfo &core::device::DeviceFirmwareInfoCollection::operator[](
	const std::string &uid)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::map<std::string, DeviceFirmwareInfo *>::iterator existingItem = m_collection.find(uid);
	if (existingItem == m_collection.end())
	{
		// Create a new empty DeviceFirmwareInfo if none found
		struct device_fw_info emptyInfo;
		memset(&emptyInfo, 0, sizeof (emptyInfo));
		m_collection[uid] = new DeviceFirmwareInfo(uid, emptyInfo);
	}

	return *(m_collection[uid]);
}

void core::device::DeviceFirmwareInfoCollection::push_back(DeviceFirmwareInfo &info)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	DeviceFirmwareInfo *d = info.clone();

	// Need to delete existing item, if any
	std::map<std::string, DeviceFirmwareInfo *>::iterator existingItem =
			m_collection.find(d->getUid());
	if (existingItem != m_collection.end())
	{
		delete existingItem->second;
		existingItem->second = NULL;
	}

	m_collection[d->getUid()] = d;
}

size_t core::device::DeviceFirmwareInfoCollection::size() const
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return m_collection.size();
}

void core::device::DeviceFirmwareInfoCollection::clear()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	for (std::map<std::string, DeviceFirmwareInfo *>::iterator iter = m_collection.begin();
			iter != m_collection.end(); iter++)
	{
		DeviceFirmwareInfo *d = iter->second;
		delete d;

		iter->second = NULL;
	}
	m_collection.clear();
}
