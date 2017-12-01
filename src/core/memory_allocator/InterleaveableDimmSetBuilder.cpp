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


#include "InterleaveableDimmSetBuilder.h"

#include <LogEnterExit.h>

namespace core
{
namespace memory_allocator
{

#define	DIMM_LOCATION(iMC, channel)		(2 * (channel % CHANNELS_PER_IMC) + iMC)
#define	DIMM_POPULATED(map, dimmIndex)	((map >> dimmIndex) & 0b1)
#define	CLEAR_DIMM(map, dimmIndex)		map &= ~(0b1 << dimmIndex)

// 2 memory controllers, 3 channels
// where bit placement represents the
// DIMMs ordered as such:
// ---------
// | 0 | 1 |
// | 2 | 3 |
// | 4 | 5 |
// ---------
static const int END_OF_INTERLEAVE_SETS = 0;
static const int INTERLEAVE_SETS[] =
{
		0b111111, // x6

		0b001111, // x4
		0b111100, // x4
		0b110011, // x4

		0b010101, // x3
		0b101010, // x3

		// favor across memory controller
		0b000011, // x2
		0b001100, // x2
		0b110000, // x2

		// before across channel
		0b000101, // x2
		0b001010, // x2
		0b010100, // x2
		0b101000, // x2
		0b010001, // x2
		0b100010, // x2

		// lastly x1
		0b000001, // x1
		0b000010, // x1
		0b000100, // x1
		0b001000, // x1
		0b010000, // x1
		0b100000, // x1

		END_OF_INTERLEAVE_SETS
};

InterleaveableDimmSetBuilder::InterleaveableDimmSetBuilder()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

InterleaveableDimmSetBuilder::~InterleaveableDimmSetBuilder()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

void InterleaveableDimmSetBuilder::setDimms(const std::vector<Dimm>& dimms)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	m_dimms = dimms;
}

std::vector<Dimm> InterleaveableDimmSetBuilder::getLargestSetOfInterleavableDimms()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	validateDimmList();

	std::vector<Dimm> interleaveDimms;
	for (size_t i = 0;
			interleaveDimms.empty() && (INTERLEAVE_SETS[i] != END_OF_INTERLEAVE_SETS);
			i++)
	{
		interleaveDimms = getDimmsFromListMatchingInterleaveSet(INTERLEAVE_SETS[i]);
	}

	return interleaveDimms;
}

void InterleaveableDimmSetBuilder::validateDimmList()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	if (dimmsAreOnMultipleSockets() || !dimmsHaveValidChannelIds())
	{
		throw InvalidDimmsException();
	}
}

bool InterleaveableDimmSetBuilder::dimmsAreOnMultipleSockets()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	std::map<NVM_UINT16, bool> socketsFound;
	for (std::vector<Dimm>::const_iterator dimm = m_dimms.begin();
			dimm != m_dimms.end(); dimm++)
	{
		socketsFound[dimm->socket] = true;
	}

	return (socketsFound.size() > 1);
}

bool InterleaveableDimmSetBuilder::dimmsHaveValidChannelIds()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool valid = true;

	// Zero-indexed channel IDs iterating over both iMCs
	const size_t maxChannelId = NVM_MAX_IMCS_PER_SOCKET * CHANNELS_PER_IMC - 1;
	std::map<NVM_UINT16, bool> socketsFound;
	for (std::vector<Dimm>::const_iterator dimm = m_dimms.begin();
			dimm != m_dimms.end(); dimm++)
	{
		if (dimm->channel > maxChannelId)
		{
			valid = false;
		}
	}

	return valid;
}

std::vector<Dimm> InterleaveableDimmSetBuilder::getDimmsFromListMatchingInterleaveSet(
		const int interleaveSetMap)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	// Check off DIMM positions as found
	int dimmsNotFound = interleaveSetMap;

	std::vector<Dimm> interleaveSetDimms;
	for (std::vector<Dimm>::const_iterator dimm = m_dimms.begin();
			dimm != m_dimms.end(); dimm++)
	{
		int dimmIndex = DIMM_LOCATION(dimm->memoryController, dimm->channel);
		if (DIMM_POPULATED(interleaveSetMap, dimmIndex))
		{
			interleaveSetDimms.push_back(*dimm);
			CLEAR_DIMM(dimmsNotFound, dimmIndex);
		}
	}

	if (dimmsNotFound)
	{
		interleaveSetDimms.clear();
	}

	return interleaveSetDimms;
}

} /* namespace memory_allocator */
} /* namespace core */
