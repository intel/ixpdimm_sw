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
 * This file contains the implementation of helper functions
 * for GUID creation and generation.
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <openssl/sha.h>

#include "guid.h"

int randomSeed = 1;

/*
 * used in hexdec to convert hex string to number
 */
static const char hextable[] =
		{
				[0 ... 255] = -1,
				['0'] = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
				['A'] = 10, 11, 12, 13, 14, 15,
				['a'] = 10, 11, 12, 13, 14, 15
		};

/*
 * Helper function for guid_to_str to convert an integer
 * to a 2 char hex string
 */
void int_to_hex_str(unsigned char value, char *destination)
{
	if (destination != NULL)
	{
		// if value is 0, destination will be null, make it zero instead
		destination[0] = '0';
		destination[1] = '0';
		if (value != 0)
		{
			// snprintf includes null terminator, so using buf[3] as intermediate step
			char buf[3];
			snprintf(buf, 3, "%02x", value);
			destination[0] = buf[0];
			destination[1] = buf[1];
		}
	}
}

/*
 * Convert binary guid to a string.
 * Note requirement that caller supply buffer of appropriate size.
 */
void guid_to_str(const COMMON_GUID guid, COMMON_GUID_STR guid_str)
{
	// check input parameters
	if (guid_str != NULL)
	{
		guid_str[0] = '\0'; // make empty string first
		if (guid != NULL)
		{
			int_to_hex_str(guid[0], &guid_str[0]);
			int_to_hex_str(guid[1], &guid_str[2]);
			int_to_hex_str(guid[2], &guid_str[4]);
			int_to_hex_str(guid[3], &guid_str[6]);
			guid_str[8] = '-';
			int_to_hex_str(guid[4], &guid_str[9]);
			int_to_hex_str(guid[5], &guid_str[11]);
			guid_str[13] = '-';
			int_to_hex_str(guid[6], &guid_str[14]);
			int_to_hex_str(guid[7], &guid_str[16]);
			guid_str[18] = '-';
			int_to_hex_str(guid[8], &guid_str[19]);
			int_to_hex_str(guid[9], &guid_str[21]);
			guid_str[23] = '-';
			int_to_hex_str(guid[10], &guid_str[24]);
			int_to_hex_str(guid[11], &guid_str[26]);
			int_to_hex_str(guid[12], &guid_str[28]);
			int_to_hex_str(guid[13], &guid_str[30]);
			int_to_hex_str(guid[14], &guid_str[32]);
			int_to_hex_str(guid[15], &guid_str[34]);
			guid_str[36] = '\0';
		}
	}
}

/*
 * convert a hexidecimal string to a unsigned int
 * will not produce or process negative numbers except
 * to signal error.
 *
 */
unsigned int hexdec(const unsigned char *hex)
{
	unsigned int result = 0;
	while (*hex != '\0')
	{
		result = (result << 4) | hextable[*hex];
		hex++;
	}
	return result;
}

/*
 * Helper function for str_to_guid to convert a 2 char hex string
 * to an integer
 */
unsigned int hexstr_to_int(const char *hex_str)
{
	unsigned char sub_str[3] = {hex_str[0], hex_str[1], '\0'};
	return hexdec(sub_str);
}

/*
 * Convert string to a binary guid.
 * Note requirement that caller supply buffers of appropriate size.
 */
void str_to_guid(const COMMON_GUID_STR guid_str, COMMON_GUID guid)
{
	if (guid != NULL)
	{
		if (guid_str != NULL)
		{
			guid[0] = hexstr_to_int(&guid_str[0]);
			guid[1] = hexstr_to_int(&guid_str[2]);
			guid[2] = hexstr_to_int(&guid_str[4]);
			guid[3] = hexstr_to_int(&guid_str[6]);
			guid[4] = hexstr_to_int(&guid_str[9]);
			guid[5] = hexstr_to_int(&guid_str[11]);
			guid[6] = hexstr_to_int(&guid_str[14]);
			guid[7] = hexstr_to_int(&guid_str[16]);
			guid[8] = hexstr_to_int(&guid_str[19]);
			guid[9] = hexstr_to_int(&guid_str[21]);
			guid[10] = hexstr_to_int(&guid_str[24]);
			guid[11] = hexstr_to_int(&guid_str[26]);
			guid[12] = hexstr_to_int(&guid_str[28]);
			guid[13] = hexstr_to_int(&guid_str[30]);
			guid[14] = hexstr_to_int(&guid_str[32]);
			guid[15] = hexstr_to_int(&guid_str[34]);
		}
		else
		{
			memset(guid, 0, COMMON_GUID_LEN);
		}
	}
}

/*
 * Check if two guids are equal
 */
int guid_cmp(const COMMON_GUID guid1, const COMMON_GUID guid2)
{
	return cmp_bytes(guid1, guid2, COMMON_GUID_LEN);
}

/*
 * generate a new GUID as a GUID
 */
void generate_guid(COMMON_GUID guid)
{
	if (guid != NULL)
	{
		int count;
		int randomNum;

		// seed random with the current time + number
		// (to account for multiple calls to this method within 1 sec)
		srand(time(NULL) + randomSeed);
		randomSeed++;
		for (count = 0; count < COMMON_GUID_LEN; count = count + 2)
		{
			// rand generates an int (16-bits)
			randomNum = rand();
			// least significant byte
			guid[count] = (unsigned char) randomNum;
			// most significant byte
			guid[count + 1] = (unsigned char) (randomNum >> 8);
		}
	}
}

/*
 * generate a new GUID as a string
 */
void generate_guid_str(COMMON_GUID_STR guid_str)
{
	if (guid_str != NULL)
	{
		COMMON_GUID guid;
		// generate a new guid
		generate_guid(guid);
		// convert it to a string
		guid_to_str(guid, guid_str);
	}
}

/*
 * Create hash guid from specified input string
 */
int guid_hash(const unsigned char *source,
		const size_t source_len, unsigned char *p_guid)
{
	unsigned char outHash[SHA_DIGEST_LENGTH];

	if (!SHA1(source, source_len, outHash))
	{
		return 0;
	}

	// now follow instructions in RFC 4122 to create GUID from hash
	// sort of ridiculous way to do it but wanted the mapping to the RFC to be explicit

	// set ocets zero through 3 of the time_low field to ocets zero through 3 of the hash
	p_guid[0] = outHash[0];
	p_guid[1] = outHash[1];
	p_guid[2] = outHash[2];
	p_guid[3] = outHash[3];

	// set octets zero and one of the time_mid field to octets 4 and 5 of the hash
	p_guid[4] = outHash[4];
	p_guid[5] = outHash[5];

	// set octets zero and one of the time_hi_and_version field to octets 6 and 7 of the hash
	p_guid[6] = outHash[6];
	p_guid[7] = outHash[7];

	// set the four most significant bits of the time_hi_and_version field to the
	// appropriate 4-bit version number from Section 4.1.3.  In our case the
	// version is the "name-based version that utilizes SHA-1 hashing" or 0101b.
	p_guid[7] = p_guid[7] & 0x0F;
	p_guid[7] = p_guid[7] ^ 0x50;

	// set the clock_seq_hi_and_reserved field to octet 8 of the hash
	p_guid[8] = outHash[8];

	// set the two most significant bits of the clock_seq_hi_and_reserved
	// to zero and 1 respectively
	p_guid[8] = p_guid[8] & 0x3F;
	p_guid[8] = p_guid[8] ^ 0x80;

	// Set the clock_seq_low field to octet 9 of the hash
	p_guid[9] = outHash[9];

	// set octets zero through five of the node field to octets 10 - 15 of the hash
	p_guid[10] = outHash[10];
	p_guid[11] = outHash[11];
	p_guid[12] = outHash[12];
	p_guid[13] = outHash[13];
	p_guid[14] = outHash[14];
	p_guid[15] = outHash[15];

	return 1;
}

/*
 * Create hash guid from specified input string
 */
int guid_hash_str(const unsigned char *source,
	const size_t source_len, char *p_guid)
{
	COMMON_GUID guid_old;
	int rc = guid_hash(source, source_len, guid_old);
	if (rc)
	{
		guid_to_str(guid_old, p_guid);
	}

	return rc;
}