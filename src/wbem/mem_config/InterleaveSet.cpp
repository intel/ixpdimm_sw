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
 * This file contains the InterleaveSet class. This class provides functionality
 * for converting library interleave set structures to wbem attributes.
 */


#include "InterleaveSet.h"
#include <LogEnterExit.h>
#include <string.h>
#include <sstream>
#include <nvm_management.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <utility.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>

NVM_UINT16 wbem::mem_config::InterleaveSet::getSocketId() const
{
	return m_socketId;
}

NVM_UINT16 wbem::mem_config::InterleaveSet::getSetIndex()
{
	return m_setIndex;
}

void wbem::mem_config::InterleaveSet::setSize(const NVM_UINT64 size)
{
	m_size = size;
}

NVM_UINT64 wbem::mem_config::InterleaveSet::getSize()
{
	return m_size;
}

NVM_UINT16 wbem::mem_config::InterleaveSet::getChannelInterleaveSize()
{
	return m_channelInterleaveSize;
}

NVM_UINT16 wbem::mem_config::InterleaveSet::getChannelCount()
{
	return m_channelCount;
}

NVM_UINT16 wbem::mem_config::InterleaveSet::getControllerInterleaveSize()
{
	return m_controllerInterleaveSize;
}

NVM_UINT16 wbem::mem_config::InterleaveSet::getReplication()
{
	return m_replication;
}

wbem::mem_config::InterleaveSet::InterleaveSet()
{
	m_socketId = 0;
	m_setIndex = 0;
	m_size = 0;
	m_replication = MEMORYALLOCATIONSETTINGS_REPLICATION_NONE;
	m_channelInterleaveSize = 0;
	m_channelCount = 0;
	m_controllerInterleaveSize = 0;
}

wbem::mem_config::InterleaveSet::InterleaveSet(const struct interleave_set *interleaveSet)
{
	m_socketId = interleaveSet->socket_id;
	m_setIndex = interleaveSet->set_index;
	m_size = interleaveSet->size;
	m_replication = interleaveSet->mirrored ?
			MEMORYALLOCATIONSETTINGS_REPLICATION_LOCAL : MEMORYALLOCATIONSETTINGS_REPLICATION_NONE;
	m_channelCount = interleaveSet->settings.ways;

	// This class is intended to be used to create MemoryAllocationSettings instances and in
	// those instances the interleave sizes are the base 2 exponent of the interleave size.
	// So we do a conversion from the size that comes from the library to the corresponding exponent.
	m_channelInterleaveSize = getExponentFromInterleaveSize(interleaveSet->settings.channel);
	m_controllerInterleaveSize = getExponentFromInterleaveSize(interleaveSet->settings.imc);
}

wbem::mem_config::InterleaveSet::InterleaveSet(const struct config_goal *goal, const NVM_UINT16 setNum)
{
	m_socketId = getSocketIdForGoal(goal);
	if (setNum == 1)
	{
		m_setIndex = goal->app_direct_1_set_id;
		m_size = goal->app_direct_1_size * BYTES_PER_GB;
		m_replication = goal->app_direct_1_settings.mirrored ?
				MEMORYALLOCATIONSETTINGS_REPLICATION_LOCAL : MEMORYALLOCATIONSETTINGS_REPLICATION_NONE;
		m_channelCount = goal->app_direct_1_settings.interleave.ways;

		// This class is intended to be used to create MemoryAllocationSettings instances and in
		// those instances the interleave sizes are the base 2 exponent of the interleave size.
		// So we do a conversion from the size that comes from the library to the corresponding exponent.
		m_channelInterleaveSize = getExponentFromInterleaveSize(goal->app_direct_1_settings.interleave.channel);
		m_controllerInterleaveSize = getExponentFromInterleaveSize(goal->app_direct_1_settings.interleave.imc);
	}
	else
	{
		m_setIndex = goal->app_direct_2_set_id;
		m_size = goal->app_direct_2_size * BYTES_PER_GB;
		m_replication = goal->app_direct_2_settings.mirrored ?
				MEMORYALLOCATIONSETTINGS_REPLICATION_LOCAL : MEMORYALLOCATIONSETTINGS_REPLICATION_NONE;
		m_channelInterleaveSize = getExponentFromInterleaveSize(goal->app_direct_2_settings.interleave.channel);
		m_channelCount = goal->app_direct_2_settings.interleave.ways;
		m_controllerInterleaveSize = getExponentFromInterleaveSize(goal->app_direct_2_settings.interleave.imc);
	}
}

NVM_UINT16 wbem::mem_config::InterleaveSet::getSocketIdForGoal(const struct config_goal *goal)
{
	NVM_UINT16 socket = 0;
	if (goal != NULL)
	{
		if (goal->app_direct_count > 0)
		{
			int rc = NVM_SUCCESS;
			NVM_UID uid;
			memmove(uid, goal->app_direct_1_settings.dimms[0], NVM_MAX_UID_LEN);
			struct device_discovery device;
			memset(&device, 0, sizeof(struct device_discovery));
			if ((rc = nvm_get_device_discovery(uid, &device)) == NVM_SUCCESS)
			{
				socket = device.socket_id;
			}
			else
			{
				COMMON_LOG_ERROR("Could not retrieve device_discovery.");
				throw exception::NvmExceptionLibError(rc);
			}
		}
	}
	return socket;
}

bool wbem::mem_config::InterleaveSet::operator<(InterleaveSet rhs) const
{
	return (this->m_socketId <= rhs.m_socketId && this->m_setIndex < rhs.m_setIndex);
}

bool wbem::mem_config::InterleaveSet::operator==(InterleaveSet rhs) const
{
	return (this->m_socketId == rhs.m_socketId && this->m_setIndex == rhs.m_setIndex);
}

wbem::mem_config::MemoryAllocationSettingsInterleaveSizeExponent
	wbem::mem_config::InterleaveSet::getExponentFromInterleaveSize
		(NVM_UINT16 interleaveSize)
{
	enum MemoryAllocationSettingsInterleaveSizeExponent exponent;

	switch (interleaveSize)
	{
		case INTERLEAVE_SIZE_64B:
			exponent = MEMORYALLOCATIONSETTINGS_EXPONENT_64B;
			break;
		case INTERLEAVE_SIZE_128B:
			exponent = MEMORYALLOCATIONSETTINGS_EXPONENT_128B;
			break;
		case INTERLEAVE_SIZE_256B:
			exponent = MEMORYALLOCATIONSETTINGS_EXPONENT_256B;
			break;
		case INTERLEAVE_SIZE_4KB:
			exponent = MEMORYALLOCATIONSETTINGS_EXPONENT_4KB;
			break;
		case INTERLEAVE_SIZE_1GB:
			exponent = MEMORYALLOCATIONSETTINGS_EXPONENT_1GB;
			break;
		default:
			COMMON_LOG_ERROR_F("%d is not a valid interleave size", interleaveSize);
			std::stringstream sizeStr;
			sizeStr << interleaveSize;
			throw framework::ExceptionBadParameter(sizeStr.str().c_str());
			break;
	}
	return exponent;
}

interleave_size wbem::mem_config::InterleaveSet::getInterleaveSizeFromExponent
	(const NVM_UINT16 exponent)
{
	enum interleave_size interleaveSize;

	switch (exponent)
	{
		case MEMORYALLOCATIONSETTINGS_EXPONENT_64B:
			interleaveSize = INTERLEAVE_SIZE_64B;
			break;
		case MEMORYALLOCATIONSETTINGS_EXPONENT_128B:
			interleaveSize = INTERLEAVE_SIZE_128B;
			break;
		case MEMORYALLOCATIONSETTINGS_EXPONENT_256B:
			interleaveSize = INTERLEAVE_SIZE_256B;
			break;
		case MEMORYALLOCATIONSETTINGS_EXPONENT_4KB:
			interleaveSize = INTERLEAVE_SIZE_4KB;
			break;
		case MEMORYALLOCATIONSETTINGS_EXPONENT_1GB:
			interleaveSize = INTERLEAVE_SIZE_1GB;
			break;
		default:
			COMMON_LOG_ERROR_F
				("%d is not a valid MemoryAllocationSettings interleave size exponent", exponent);
			std::stringstream sizeStr;
			sizeStr << exponent;
			throw framework::ExceptionBadParameter(sizeStr.str().c_str());
			break;
	}
	return interleaveSize;
}

NVM_UINT32 wbem::mem_config::InterleaveSet::getInterleaveSizeValue(
		const enum interleave_size &size)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	NVM_UINT32 result = 0;
	switch (size)
	{
		case INTERLEAVE_SIZE_64B:
			result = 64;
			break;
		case INTERLEAVE_SIZE_128B:
			result = 128;
			break;
		case INTERLEAVE_SIZE_256B:
			result = 256;
			break;
		case INTERLEAVE_SIZE_4KB:
			result = 4 * 1024;
			break;
		case INTERLEAVE_SIZE_1GB:
			result = 1024 * 1024 * 1024;
			break;
	}
	return result;
}

std::string wbem::mem_config::InterleaveSet::getInterleaveSizeString(
		const enum interleave_size &size)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return get_string_for_interleave_size(size);
}

/*
 * Convert an interleave format structure into a string of format:
 * "x[way] - [size IMC x size Channel]"
 */
std::string wbem::mem_config::InterleaveSet::getInterleaveFormatString(
		const struct interleave_format *p_format)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream formatStr;
	bool addSizes = false;

	// set the interleave way
	switch(p_format->ways)
	{
		case INTERLEAVE_WAYS_2:
		case INTERLEAVE_WAYS_3:
		case INTERLEAVE_WAYS_4:
		case INTERLEAVE_WAYS_6:
		case INTERLEAVE_WAYS_8:
		case INTERLEAVE_WAYS_12:
		case INTERLEAVE_WAYS_16:
		case INTERLEAVE_WAYS_24:
			addSizes = true;
			// fall through
		case INTERLEAVE_WAYS_1:
			// x1 doesn't add interleave sizes
			formatStr << "x" << p_format->ways;
			break;
		// default to not applicable
		default:
			formatStr << wbem::NA;
			break;
	}

	if (addSizes)
	{
		// add the imc size
		formatStr << " - " << getInterleaveSizeString(p_format->imc) << " iMC";

		// add the channel size
		formatStr << " x " << getInterleaveSizeString(p_format->channel) << " Channel";
	}
	return formatStr.str();
}

/*
 * Convert an interleave format structure into a CLI create goal setting string
 */
std::string wbem::mem_config::InterleaveSet::getInterleaveFormatInputString(
		const struct interleave_format *p_format, bool mirrorSupported)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	std::stringstream formatStr;
	bool addSizes = false;
	bool addMirror = false;

	// set the interleave way
	switch (p_format->ways)
	{
		case INTERLEAVE_WAYS_1:
			formatStr << wbem::mem_config::APP_DIRECT_SETTING_BYONE;
			break;
		case INTERLEAVE_WAYS_2:
		case INTERLEAVE_WAYS_3:
			// Mirror only possible within a single memory controller
			addMirror = true;
			addSizes = true;
			break;
		case INTERLEAVE_WAYS_4:
		case INTERLEAVE_WAYS_6:
		case INTERLEAVE_WAYS_8:
		case INTERLEAVE_WAYS_12:
		case INTERLEAVE_WAYS_16:
		case INTERLEAVE_WAYS_24:
			addSizes = true;
			break;
		default:
			break;
	}

	if (mirrorSupported && addMirror)
	{
		formatStr << "[" << APP_DIRECT_SETTING_MIRROR << MEMORYPROP_TOKENSEP << "]";
	}

	if (addSizes)
	{
		// add the imc size
		formatStr << getInterleaveSizeString(p_format->imc);

		// add the channel size
		formatStr << MEMORYPROP_TOKENSEP << getInterleaveSizeString(p_format->channel);
	}
	return formatStr.str();
}

/*
 * Convert a numerical representation of an interleave format to a string
 */
std::string wbem::mem_config::InterleaveSet::getInterleaveFormatStringFromInt(const NVM_UINT32 &format)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	struct interleave_format formatStruct;
	memset(&formatStruct, 0, sizeof (formatStruct));
	interleave_format_to_struct(format, &formatStruct);
	return getInterleaveFormatString(&formatStruct);
}
