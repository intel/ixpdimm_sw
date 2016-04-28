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

#include "DeviceHelper.h"
#include "Device.h"

#define	SHORT_UID_LEN 13
#define LONG_UID_LEN 21

namespace core
{
namespace device
{

bool isUidValid(const std::string &uid)
{
	// "XXXX-XXXXXXXX" or "XXXX-XX-XXXX-XXXXXXXX"
	bool result = false;
	if((uid.length() == SHORT_UID_LEN) && (uid[4] == '-'))
	{
		result = true;
	}
	else if ((uid.length() == LONG_UID_LEN) &&
			 (uid[4] == '-') &&
			 (uid[7] == '-') &&
			 (uid[12] == '-'))
	{
		result = true;
	}

	return result;
}

int findUidEnd(const std::string &stringWithUidAtBeginning)
{
	int index = -1;

	if (stringWithUidAtBeginning.length() >= LONG_UID_LEN)
	{
		std::string potentialLongUid = stringWithUidAtBeginning.substr(0, LONG_UID_LEN);
		std::string potentialShortUid = stringWithUidAtBeginning.substr(0, SHORT_UID_LEN);
		if(isUidValid(potentialLongUid))
		{
			index = LONG_UID_LEN;
		}
		else if(isUidValid(potentialShortUid))
		{
			index = SHORT_UID_LEN;
		}
	}
	else if(stringWithUidAtBeginning.length() >= SHORT_UID_LEN)
	{
		std::string potentialUid = stringWithUidAtBeginning.substr(0, SHORT_UID_LEN);
		if(isUidValid(potentialUid))
		{
			index = SHORT_UID_LEN;
		}
	}

	return index;
}

int findUidStart(const std::string &stringWithUidAtEnd)
{
	int index = -1;
	if ((stringWithUidAtEnd.length() >= LONG_UID_LEN))
	{
		std::string potentialLongUid =
				stringWithUidAtEnd.substr(stringWithUidAtEnd.length() - LONG_UID_LEN);
		std::string potentialShortUid =
				stringWithUidAtEnd.substr(stringWithUidAtEnd.length() - SHORT_UID_LEN);
		if (isUidValid(potentialLongUid))
		{
			index = (int)stringWithUidAtEnd.length() - LONG_UID_LEN;
		}
		else if (isUidValid(potentialShortUid))
		{
			index = (int)stringWithUidAtEnd.length() - SHORT_UID_LEN;
		}
	}
	else if(stringWithUidAtEnd.length() >= SHORT_UID_LEN)
	{
		std::string potentialUid = stringWithUidAtEnd.substr(stringWithUidAtEnd.length() - SHORT_UID_LEN);
		if (isUidValid(potentialUid))
		{
			index = (int)stringWithUidAtEnd.length() - SHORT_UID_LEN;
		}
	}

	return index;
}

}
}