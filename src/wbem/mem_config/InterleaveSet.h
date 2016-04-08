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



#ifndef	_WBEM_MEMCONFIG_INTERLEAVESET_H_
#define	_WBEM_MEMCONFIG_INTERLEAVESET_H_

#include <nvm_management.h>
#include <string>

namespace wbem {
namespace mem_config {

static const std::string SIZE_REMAINING = "Remaining";
static const std::string APP_DIRECT_SETTING_MIRROR = "Mirror";
static const std::string APP_DIRECT_SETTING_BYONE = "ByOne";
static const std::string MEMORYPROP_TOKENSEP = "_";

enum MemoryAllocationSettingsInterleaveSizeExponent {
	MEMORYALLOCATIONSETTINGS_EXPONENT_UNKNOWN  = 0,
	MEMORYALLOCATIONSETTINGS_EXPONENT_64B  = 6,
	MEMORYALLOCATIONSETTINGS_EXPONENT_128B = 7,
	MEMORYALLOCATIONSETTINGS_EXPONENT_256B = 8,
	MEMORYALLOCATIONSETTINGS_EXPONENT_4KB  = 12,
	MEMORYALLOCATIONSETTINGS_EXPONENT_1GB  = 30,
};

enum MemoryAllocationSettingsResourceType {
	MEMORYALLOCATIONSETTINGS_RESOURCETYPE_UNKNOWN     = 0,   // resource type unknown
	MEMORYALLOCATIONSETTINGS_RESOURCETYPE_OTHER       = 1,   // resource type not volatile or app direct
	MEMORYALLOCATIONSETTINGS_RESOURCETYPE_MEMORY      = 4,   // resource type volatile
	MEMORYALLOCATIONSETTINGS_RESOURCETYPE_NONVOLATILE = 35   // resource type app direct
};

enum MemoryAllocationSettingsReplication {
	MEMORYALLOCATIONSETTINGS_REPLICATION_UNKNOWN = 0,    // the replication of this region is unknown
	MEMORYALLOCATIONSETTINGS_REPLICATION_NONE    = 2,    // this region is not replicated
	MEMORYALLOCATIONSETTINGS_REPLICATION_LOCAL   = 3,    // this region is replicated locally
	MEMORYALLOCATIONSETTINGS_REPLICATION_REMOTE  = 4	    // this region is replicated remotely
};


// This class is intended to be used to create MemoryAllocationSettings
class NVM_API InterleaveSet {
public:
	InterleaveSet();
	InterleaveSet(const struct interleave_set *interleaveSet);
	InterleaveSet(const struct config_goal *goal, NVM_UINT16 setNum);
	bool operator<(InterleaveSet rhs) const;
	bool operator==(InterleaveSet rhs) const;

	static interleave_size getInterleaveSizeFromExponent(const NVM_UINT16 exponent);
	static enum MemoryAllocationSettingsInterleaveSizeExponent getExponentFromInterleaveSize(const NVM_UINT16 size);
	static NVM_UINT32 getInterleaveSizeValue(const enum interleave_size &size);
	static std::string getInterleaveSizeString(const enum interleave_size &size);
	static std::string getInterleaveFormatString(const struct interleave_format *p_format);
	static std::string getInterleaveFormatStringFromInt(const NVM_UINT32 &format);
	static std::string getInterleaveFormatInputString(const struct interleave_format *p_format, bool mirrorSupported);

	NVM_UINT16 getSocketId() const;
	NVM_UINT16 getSetIndex();
	NVM_UINT64 getSize();
	void setSize(const NVM_UINT64 size);
	NVM_UINT16 getChannelInterleaveSize();
	NVM_UINT16 getChannelCount();
	NVM_UINT16 getControllerInterleaveSize();
	NVM_UINT16 getReplication();


private:
	NVM_UINT16 m_socketId;
	NVM_UINT16 m_setIndex;
	NVM_UINT64 m_size;
	NVM_UINT16 m_channelInterleaveSize;
	NVM_UINT16 m_channelCount;
	NVM_UINT16 m_controllerInterleaveSize;
	NVM_UINT16 m_replication;

	NVM_UINT16 getSocketIdForGoal(const struct config_goal *goal);
};

} // mem_config
} // wbem

#endif // _WBEM_MEMCONFIG_INTERLEAVESET_H_
