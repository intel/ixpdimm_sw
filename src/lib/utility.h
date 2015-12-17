/*
 * Copyright (c) 2015, Intel Corporation
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
 * Encapsulates functionality that can be utilized throughout the management software
 * stack but does not make sense to expose as part of the public API.
 */

#ifndef	_NVM_UTILITY_H_
#define	_NVM_UTILITY_H_

#include "nvm_management.h"
#include "platform_capabilities.h"
#include "nvm_types.h"
#include <guid/guid.h>
#include <persistence/logging.h>
#include <cr_i18n.h>
#include <os/os_adapter.h>
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define	PERSISTENTSETTING_UNKNOWN	"Unknown"
#define	PERSISTENTSETTING_64B	"64B"
#define	PERSISTENTSETTING_128B	"128B"
#define	PERSISTENTSETTING_256B	"256B"
#define	PERSISTENTSETTING_4KB	"4KB"
#define	PERSISTENTSETTING_1GB	"1GB"

/*
 * Convert manufacturer ID array to number for DB storage and display
 */
// assuming little endian
#define	MANUFACTURER_TO_UINT(manufacturer)	\
	(manufacturer[1] << 8) | \
	manufacturer[0]

/*
 * Convert serial number array to number for DB storage
 */
// assuming little endian
#define	SERIAL_NUMBER_TO_UINT(serial_number)	\
	(serial_number[3] << 24) | \
	(serial_number[2] << 16) | \
	(serial_number[1] << 8) | \
	serial_number[0]

/*
 * Get GB aligned usable capacity from overall DIMM capacity
 */
#define	RESERVED_CAPACITY_BYTES(cap)	(NVM_UINT64)(cap % BYTES_PER_GB)
#define	USABLE_CAPACITY_BYTES(cap)	(NVM_UINT64)(cap - RESERVED_CAPACITY_BYTES(cap))

/*
 * Convert number to manufacturer ID array
 */
// assuming little endian
static inline void UINT_TO_MANUFACTURER(const COMMON_UINT16 value, unsigned char *manufacturer)
{
	manufacturer[1] = (value >> 8) & 0xFF;
	manufacturer[0] = value & 0xFF;
}

/*
 * Convert number to serial number array
 */
// assuming little endian
static inline void UINT_TO_SERIAL_NUMBER(const COMMON_UINT32 value, unsigned char *serial_number)
{
	serial_number[3] = (value >> 24) & 0xFF;
	serial_number[2] = (value >> 16) & 0xFF;
	serial_number[1] = (value >> 8) & 0xFF;
	serial_number[0] = value & 0xFF;
}

/*
 * Convert serial number to string
 * TODO TA-15497: use proper conversion method when defined by manufacturing
 */
// assuming little endian
static inline void SERIAL_NUMBER_TO_STRING(const unsigned char *serial_number,
		char *serial_number_str)
{
	if (serial_number_str != NULL)
	{
		serial_number_str[0] = '\0'; // make empty string first
		if (serial_number != NULL)
		{
			int_to_hex_str(serial_number[3], &serial_number_str[0]);
			int_to_hex_str(serial_number[2], &serial_number_str[2]);
			int_to_hex_str(serial_number[1], &serial_number_str[4]);
			int_to_hex_str(serial_number[0], &serial_number_str[6]);
			serial_number_str[8] = '\0';
		}
	}
}

static inline enum interleave_ways interleave_way_to_enum(int way)
{
	enum interleave_ways enum_way;

	switch (way)
	{
		case BITMAP_INTERLEAVE_WAYS_1:
			enum_way = INTERLEAVE_WAYS_1;
			break;
		case BITMAP_INTERLEAVE_WAYS_2:
			enum_way = INTERLEAVE_WAYS_2;
			break;
		case BITMAP_INTERLEAVE_WAYS_3:
			enum_way = INTERLEAVE_WAYS_3;
			break;
		case BITMAP_INTERLEAVE_WAYS_4:
			enum_way = INTERLEAVE_WAYS_4;
			break;
		case BITMAP_INTERLEAVE_WAYS_6:
			enum_way = INTERLEAVE_WAYS_6;
			break;
		case BITMAP_INTERLEAVE_WAYS_8:
			enum_way = INTERLEAVE_WAYS_8;
			break;
		case BITMAP_INTERLEAVE_WAYS_12:
			enum_way = INTERLEAVE_WAYS_12;
			break;
		case BITMAP_INTERLEAVE_WAYS_16:
			enum_way = INTERLEAVE_WAYS_16;
			break;
		case BITMAP_INTERLEAVE_WAYS_24:
			enum_way = INTERLEAVE_WAYS_24;
			break;
		default:
			enum_way = INTERLEAVE_WAYS_0;
			break;
	}

	return enum_way;
}

/*
 * Convert a BIOS interleave format value into a structure
 */
static inline void interleave_format_to_struct(NVM_UINT32 format,
		struct interleave_format *p_format)
{
	if (p_format)
	{
		p_format->recommended = ((format >> PCAT_FORMAT_RECOMMENDED_SHIFT) & 1);

		int cr_channel_ways = ((format >> PCAT_FORMAT_WAYS_SHIFT) & PCAT_FORMAT_WAYS_MASK);
		p_format->ways = interleave_way_to_enum(cr_channel_ways);

		int cr_channel_size = (format & PCAT_FORMAT_CHANNEL_MASK);
		switch (cr_channel_size)
		{
			case BITMAP_INTERLEAVE_SIZE_64B: // 64B
				p_format->channel = INTERLEAVE_SIZE_64B;
				break;
			case BITMAP_INTERLEAVE_SIZE_128B: // 128B
				p_format->channel = INTERLEAVE_SIZE_128B;
				break;
			case BITMAP_INTERLEAVE_SIZE_256B: // 256B
				p_format->channel = INTERLEAVE_SIZE_256B;
				break;
			case BITMAP_INTERLEAVE_SIZE_4KB: // 4KB
				p_format->channel = INTERLEAVE_SIZE_4KB;
				break;
			case BITMAP_INTERLEAVE_SIZE_1GB: // 1GB
				p_format->channel = INTERLEAVE_SIZE_1GB;
				break;
		}
		int cr_imc_size = ((format >> PCAT_FORMAT_IMC_SHIFT) & PCAT_FORMAT_IMC_MASK);
		switch (cr_imc_size)
		{
			case BITMAP_INTERLEAVE_SIZE_64B: // 64B
				p_format->imc = INTERLEAVE_SIZE_64B;
				break;
			case BITMAP_INTERLEAVE_SIZE_128B: // 128B
				p_format->imc = INTERLEAVE_SIZE_128B;
				break;
			case BITMAP_INTERLEAVE_SIZE_256B: // 256B
				p_format->imc = INTERLEAVE_SIZE_256B;
				break;
			case BITMAP_INTERLEAVE_SIZE_4KB: // 4KB
				p_format->imc = INTERLEAVE_SIZE_4KB;
				break;
			case BITMAP_INTERLEAVE_SIZE_1GB: // 1GB
				p_format->imc = INTERLEAVE_SIZE_1GB;
				break;
		}
	}
}

/*
 * Convert an interleave format structure to a BIOS supported value.
 */
static inline void interleave_struct_to_format(const struct interleave_format *p_struct,
		NVM_UINT32 *p_format)
{
	if (p_format && p_struct)
	{
		NVM_UINT32 format = 0;

		// ways
		int ways = 0;
		switch (p_struct->ways)
		{
			case INTERLEAVE_WAYS_1:
				ways = BITMAP_INTERLEAVE_WAYS_1;
				break;
			case INTERLEAVE_WAYS_2:
				ways = BITMAP_INTERLEAVE_WAYS_2;
				break;
			case INTERLEAVE_WAYS_3:
				ways = BITMAP_INTERLEAVE_WAYS_3;
				break;
			case INTERLEAVE_WAYS_4:
				ways = BITMAP_INTERLEAVE_WAYS_4;
				break;
			case INTERLEAVE_WAYS_6:
				ways = BITMAP_INTERLEAVE_WAYS_6;
				break;
			case INTERLEAVE_WAYS_8:
				ways = BITMAP_INTERLEAVE_WAYS_8;
				break;
			case INTERLEAVE_WAYS_12:
				ways = BITMAP_INTERLEAVE_WAYS_12;
				break;
			case INTERLEAVE_WAYS_16:
				ways = BITMAP_INTERLEAVE_WAYS_16;
				break;
			case INTERLEAVE_WAYS_24:
				ways = BITMAP_INTERLEAVE_WAYS_24;
				break;
			case INTERLEAVE_WAYS_0:
				break;
		}
		format |= (ways & PCAT_FORMAT_WAYS_MASK) << PCAT_FORMAT_WAYS_SHIFT;

		// recommended
		if (p_struct->recommended)
		{
			format |= (1 << PCAT_FORMAT_RECOMMENDED_SHIFT);
		}

		// channel
		int channel = 0;
		switch (p_struct->channel)
		{
			case INTERLEAVE_SIZE_64B:
				channel = BITMAP_INTERLEAVE_SIZE_64B;
				break;

			case INTERLEAVE_SIZE_128B:
				channel = BITMAP_INTERLEAVE_SIZE_128B;
				break;

			case INTERLEAVE_SIZE_256B:
				channel = BITMAP_INTERLEAVE_SIZE_256B;
				break;

			case INTERLEAVE_SIZE_4KB:
				channel = BITMAP_INTERLEAVE_SIZE_4KB;
				break;

			case INTERLEAVE_SIZE_1GB:
				channel = BITMAP_INTERLEAVE_SIZE_1GB;
				break;
		}
		format |= (channel & PCAT_FORMAT_CHANNEL_MASK);

		// IMC
		int imc = 0;
		switch (p_struct->imc)
		{
			case INTERLEAVE_SIZE_64B:
				imc = BITMAP_INTERLEAVE_SIZE_64B;
				break;
			case INTERLEAVE_SIZE_128B:
				imc = BITMAP_INTERLEAVE_SIZE_128B;
				break;
			case INTERLEAVE_SIZE_256B:
				imc = BITMAP_INTERLEAVE_SIZE_256B;
				break;
			case INTERLEAVE_SIZE_4KB:
				imc = BITMAP_INTERLEAVE_SIZE_4KB;
				break;
			case INTERLEAVE_SIZE_1GB:
				imc = BITMAP_INTERLEAVE_SIZE_1GB;
				break;
		}
		format |= (imc & PCAT_FORMAT_IMC_MASK) << PCAT_FORMAT_IMC_SHIFT;

		*p_format = format;
	}
}

static inline const char *get_string_for_interleave_size(const enum interleave_size size)
{
	const char *size_str = PERSISTENTSETTING_UNKNOWN;

	switch (size)
	{
		case INTERLEAVE_SIZE_64B:
			size_str = PERSISTENTSETTING_64B;
			break;
		case INTERLEAVE_SIZE_128B:
			size_str = PERSISTENTSETTING_128B;
			break;
		case INTERLEAVE_SIZE_256B:
			size_str = PERSISTENTSETTING_256B;
			break;
		case INTERLEAVE_SIZE_4KB:
			size_str = PERSISTENTSETTING_4KB;
			break;
		case INTERLEAVE_SIZE_1GB:
			size_str = PERSISTENTSETTING_1GB;
			break;
		default:
			break;
	}

	return size_str;
}

static inline const char *get_string_for_device_health_status(enum device_health health)
{
	const char *health_str = N_TR("Unknown");

	switch (health)
	{
		case DEVICE_HEALTH_NORMAL:
			health_str = N_TR("OK");
			break;
		case DEVICE_HEALTH_NONCRITICAL:
			health_str = N_TR("Minor Failure");
			break;
		case DEVICE_HEALTH_CRITICAL:
			health_str = N_TR("Critical Failure");
			break;
		case DEVICE_HEALTH_FATAL:
			health_str = N_TR("Non-recoverable error");
			break;
		default:
			break;
	}

	return health_str;
}

/*
 * this function finds the greatest common divisor using the Euclidean algorithm
 */
static inline NVM_UINT32 get_greatest_common_divisor(NVM_UINT32 block_size, NVM_UINT32 page_size)
{
	NVM_UINT32 gcd = 0;
	for (;;)
	{
		if (block_size == 0)
		{
			gcd = page_size;
			break;
		}
		page_size %= block_size;

		if (page_size == 0)
		{
			gcd = block_size;
			break;
		}
		block_size %= page_size;
	}

	return gcd;
}

static inline NVM_UINT32 get_lowest_common_multiple(
		NVM_UINT32 block_size, NVM_UINT32 page_size_alignment)
{
	int gcd = get_greatest_common_divisor(block_size, page_size_alignment);
	return block_size / gcd * page_size_alignment;
}

static inline NVM_UINT64 round_up(const NVM_UINT64 num_to_round, const NVM_UINT64 multiple_of)
{
	NVM_UINT64 rounded = num_to_round;
	if (num_to_round % multiple_of)
	{
		NVM_UINT64 remainder = num_to_round % multiple_of;
		NVM_UINT64 adjustment = multiple_of - remainder;
		rounded += adjustment;
	}

	return rounded;
}

static inline NVM_UINT64 round_down(const NVM_UINT64 num_to_round, const NVM_UINT64 multiple_of)
{
	NVM_UINT64 rounded = num_to_round;
	if (num_to_round % multiple_of)
	{
		NVM_UINT64 adjustment = num_to_round % multiple_of;
		rounded -= adjustment;
	}

	return rounded;
}

static inline NVM_UINT32 get_real_block_size(NVM_UINT32 block_size)
{
	NVM_UINT32 real_block_size = 1;

	if (block_size != 1)
	{
		real_block_size = (NVM_UINT32)round_up(block_size, CACHE_LINE_SIZE);
	}

	return real_block_size;
}

enum small_block_sizes
{
	BLOCK_SIZE_520 = 520,
	BLOCK_SIZE_528 = 528,
	BLOCK_SIZE_576 = 576
};

static inline NVM_BOOL smallerBlockSizeIsSelected(NVM_UINT16 block_size)
{
	NVM_BOOL result = 0;
	if (block_size == BLOCK_SIZE_520 || block_size == BLOCK_SIZE_528)
	{
		result = 1;
	}
	return result;
}

/*
 * When a smaller block size with protection information (PI) i.e. 520 and 528 is selected the
 * actual size used is 576 bytes. The extra space is not used by the block driver wasting a
 * substantial fraction of the allocated space.
 * This function calculates the size of the namespace to accomodate this wastage.
 */
static inline NVM_UINT64 adjust_namespace_size(NVM_UINT16 block_size, NVM_UINT64 block_count)
{
	if (smallerBlockSizeIsSelected(block_size))
	{
		block_size = BLOCK_SIZE_576;
	}
	return block_size * block_count;
}

static inline NVM_UINT64 calculateBlockCount(NVM_UINT64 namespace_size, NVM_UINT16 block_size)
{
	NVM_UINT64 block_count;
	if (smallerBlockSizeIsSelected(block_size))
	{
		block_count = namespace_size/ BLOCK_SIZE_576;
	}
	else
	{
		block_count = namespace_size/ block_size;
	}
	return block_count;
}

static inline int getDigit(int number, int digit)
{
	return (number / (int)pow(10, digit)) % 10;
}

// helper function that checks if block size is a 4k variant (4096, 4160, etc)
static inline NVM_BOOL blockSizeIs4KVariant(COMMON_UINT64 blockSize)
{
	NVM_BOOL result = 0;
	if (getDigit(blockSize, 3) == 4)
	{
		result = 1;
	}
	return result;
}

// helper function that checks if block size is 5** ( 512, 520, 528, etc)
static inline NVM_BOOL blockSizeIsPI(COMMON_UINT64 blockSize)
{
	NVM_BOOL result = 0;
	if (getDigit(blockSize, 2) == 5)
	{
		result = 1;
	}
	return result;
}

static inline NVM_UINT64 bytesToConfigGoalSize(NVM_UINT64 bytes)
{
	NVM_UINT64 size = bytes / BYTES_PER_GB;
	return size;
}

static inline NVM_UINT64 configGoalSizeToBytes(NVM_UINT64 size)
{
	NVM_UINT64 bytes = size * BYTES_PER_GB;
	return bytes;
}

#ifdef __cplusplus
}
#endif

#endif /* _NVM_UTILITY_H_ */
