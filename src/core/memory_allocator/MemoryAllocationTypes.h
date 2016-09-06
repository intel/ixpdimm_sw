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
 * Data structures used by the memory allocation memory_allocator.
 *
 * Note: These structures have constructors because it is dangerous to memset
 * structures using STL objects as members.
 */

#ifndef _core_LOGIC_MEMORYALLOCATIONTYPES_H_
#define _core_LOGIC_MEMORYALLOCATIONTYPES_H_

#include <nvm_types.h>
#include <nvm_management.h>
#include <vector>
#include <map>
#include <string>

namespace core
{
namespace memory_allocator
{

static const int REQUEST_DEFAULT_INTERLEAVE_FORMAT = -1;
static const size_t MAX_APP_DIRECT_EXTENTS = 1;
static const NVM_UINT16 DIMMS_PER_SOCKET = 6;
static const NVM_UINT16 IMCS_PER_SOCKET = 2;
static const NVM_UINT16 CHANNELS_PER_IMC = 3;
static const NVM_UINT64 PM_ALIGNMENT_GIB = 32;

struct Dimm
{
	Dimm() : uid(""), capacityBytes(0), socket(0), memoryController(0), channel(0) {}

	std::string uid;
	NVM_UINT64 capacityBytes; // bytes
	NVM_UINT16 socket;
	NVM_UINT16 memoryController;
	NVM_UINT32 channel;
};

enum LayoutWarningCode
{
	LAYOUT_WARNING_APP_DIRECT_NOT_SUPPORTED_BY_DRIVER,
	LAYOUT_WARNING_STORAGE_NOT_SUPPORTED_BY_DRIVER,
	LAYOUT_WARNING_APP_DIRECT_SETTINGS_NOT_RECOMMENDED,
	LAYOUT_WARNING_NONOPTIMAL_POPULATION,
	LAYOUT_WARNING_REQUESTED_MEMORY_MODE_NOT_USABLE
};

struct MemoryAllocationLayout
{
	MemoryAllocationLayout() :
		memoryCapacity(0), appDirectCapacities(), storageCapacity(0), goals() {}

	NVM_UINT64 memoryCapacity; // total in GiB
	std::vector<NVM_UINT64> appDirectCapacities; // in GiB
	NVM_UINT64 storageCapacity; // in GiB

	// the string is a DIMM UID
	std::map<std::string, struct config_goal> goals;
	std::string reservedimmUid;
	std::vector<enum LayoutWarningCode> warnings;
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* _core_LOGIC_MEMORYALLOCATIONTYPES_H_ */
