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
 * Encapsulates functionality that can be utilized throughout the management software
 * stack but does not make sense to expose as part of the public API.
 */

#ifndef	_NVM_UTILITY_H_
#define	_NVM_UTILITY_H_

#include "nvm_management.h"
#include "platform_capabilities.h"
#include "nvm_types.h"
#include <guid/guid.h>
#include <uid/uid.h>
#include <persistence/logging.h>
#include <cr_i18n.h>
#include <os/os_adapter.h>
#include <math.h>
#include <string/s_str.h>
#include <string/x_str.h>
#include <file_ops/file_ops_adapter.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define	APP_DIRECTSETTING_UNKNOWN	"Unknown"
#define	APP_DIRECTSETTING_64B	"64B"
#define	APP_DIRECTSETTING_128B	"128B"
#define	APP_DIRECTSETTING_256B	"256B"
#define	APP_DIRECTSETTING_4KB	"4KB"
#define	APP_DIRECTSETTING_1GB	"1GB"

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
 * Get GiB aligned usable capacity from overall DIMM capacity
 */
#define	RESERVED_CAPACITY_BYTES(cap)	(NVM_UINT64)(cap % BYTES_PER_GIB)
#define	USABLE_CAPACITY_BYTES(cap)	(NVM_UINT64)(cap - RESERVED_CAPACITY_BYTES(cap))

#define	GET_GCD(gcd, a, b)	\
	for (;;) \
	{ \
		if (a == 0) \
		{ \
			gcd = b; \
			break; \
		} \
		b %= a; \
		if (b == 0) \
		{ \
			gcd = a; \
			break; \
		} \
		a %= b; \
	}

/*
 * Line format for a single dimm's configuration in a file.
 * SocketID
 * DeviceHandle
 * Capacity (GiB)
 * MemorySize (GiB)
 * AppDirect1Size (GiB)
 * AppDirect1Format
 * AppDirect1Mirrored
 * AppDirect1Index
 * AppDirect2Size (GiB)
 * AppDirect2Format
 * AppDirect2Mirrored
 * AppDirect2Index
 */
#define	config_line_format	"%hu,%u,%llu,%llu,%llu,%u,%hhu,%hu,%llu,%u,%hhu,%hu\n"

#define	READ_CONFIG_TOK(pLine, delim, trans, pDest, bad) { \
	char *str = x_strtok(&pLine, delim); \
	const char *pEnd; \
	if (str == NULL || trans(str, strlen(str), &pEnd, pDest) != strlen(str)) \
	{ \
		bad = 1; \
	} \
}

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
			serial_number_str[0] = '0';
			serial_number_str[1] = 'x';
			int_to_hex_digits(serial_number[3], &serial_number_str[2]);
			int_to_hex_digits(serial_number[2], &serial_number_str[4]);
			int_to_hex_digits(serial_number[1], &serial_number_str[6]);
			int_to_hex_digits(serial_number[0], &serial_number_str[8]);
			serial_number_str[10] = '\0';
		}
	}
}

/*
 * Compare major and minor version numbers to discover if versions
 * are greater, lesser, or equal.
 * Returns 1 if version1 is greater, -1 if version1 is lesser, 0 if equal
 */
static inline int version_cmp(const unsigned int version1_major,
		const unsigned int version1_minor,
		const unsigned int version2_major,
		const unsigned int version2_minor)
{
	int rc = 0;

	if ((version1_major > version2_major) ||
			(version1_major == version2_major &&
					version1_minor > version2_minor))
	{
		rc = 1;
	}
	else if ((version1_major < version2_major) ||
			(version1_major == version2_major &&
					version1_minor < version2_minor))
	{
		rc = -1;
	}
	// Otherwise must be equal

	return rc;
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
			case INTERLEAVE_SIZE_NONE:
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
			case INTERLEAVE_SIZE_NONE:
				break;
		}
		format |= (imc & PCAT_FORMAT_IMC_MASK) << PCAT_FORMAT_IMC_SHIFT;

		*p_format = format;
	}
}

static inline const char *get_string_for_interleave_size(const enum interleave_size size)
{
	const char *size_str = APP_DIRECTSETTING_UNKNOWN;

	switch (size)
	{
		case INTERLEAVE_SIZE_64B:
			size_str = APP_DIRECTSETTING_64B;
			break;
		case INTERLEAVE_SIZE_128B:
			size_str = APP_DIRECTSETTING_128B;
			break;
		case INTERLEAVE_SIZE_256B:
			size_str = APP_DIRECTSETTING_256B;
			break;
		case INTERLEAVE_SIZE_4KB:
			size_str = APP_DIRECTSETTING_4KB;
			break;
		case INTERLEAVE_SIZE_1GB:
			size_str = APP_DIRECTSETTING_1GB;
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
			health_str = N_TR("Healthy");
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
static inline NVM_UINT32 get_greatest_common_divisor(NVM_UINT32 a, NVM_UINT32 b)
{
	NVM_UINT32 gcd = 0;
	GET_GCD(gcd, a, b);
	return gcd;
}

static inline NVM_UINT32 get_lowest_common_multiple(
		NVM_UINT32 a, NVM_UINT32 b)
{
	int gcd = get_greatest_common_divisor(a, b);
	return a / gcd * b;
}

static inline NVM_REAL32 round_to_decimal_places(NVM_REAL32 number, int decimal_places)
{
	// always rounding down to match UEFI
	return (NVM_REAL32)(floor(number * pow(10, decimal_places)) / pow(10, decimal_places));
}

static inline NVM_UINT64 round_up(const NVM_UINT64 num_to_round, const NVM_UINT64 multiple_of)
{
	NVM_UINT64 rounded = num_to_round;
	if (multiple_of != 0 &&
			num_to_round % multiple_of)
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
	if (multiple_of != 0 &&
			num_to_round % multiple_of)
	{
		NVM_UINT64 adjustment = num_to_round % multiple_of;
		rounded -= adjustment;
	}

	return rounded;
}

/*
 * when a block size of 520, 528, 4104 is requested, the actual block size used by the
 * driver is rounded up to the nearest multiple of cache line size.
 */
static inline NVM_UINT32 get_real_block_size(NVM_UINT32 block_size)
{
	NVM_UINT32 real_block_size = 1;

	if (block_size != 1)
	{
		real_block_size = (NVM_UINT32)round_up(block_size, CACHE_LINE_SIZE);
	}

	return real_block_size;
}

/*
 * returns the actual NS size with the actual block size taking into account the real block size
 */
static inline NVM_UINT64 adjust_namespace_size(NVM_UINT16 block_size, NVM_UINT64 block_count)
{
	block_size = get_real_block_size(block_size);

	return block_size * block_count;
}

static inline NVM_UINT64 calculateBlockCount(NVM_UINT64 namespace_size, NVM_UINT16 block_size)
{
	NVM_UINT64 block_count;

	block_size = get_real_block_size(block_size);
	block_count = namespace_size/ block_size;

	return block_count;
}

static inline int getDigit(int number, int digit)
{
	return (number / (int)pow(10, digit)) % 10;
}

// Unused for now
#if 0
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
#endif

static inline NVM_UINT64 bytesToConfigGoalSize(NVM_UINT64 bytes)
{
	NVM_UINT64 size = bytes / BYTES_PER_GIB;
	return size;
}

static inline NVM_UINT64 configGoalSizeToBytes(NVM_UINT64 size)
{
	NVM_UINT64 bytes = size * BYTES_PER_GIB;
	return bytes;
}

static inline NVM_UINT64 configGoalSizeToGiB(NVM_UINT64 size)
{
	return size;
}

/*!
 * Convert common error code to a Native API library return_code.
 */
static inline int CommonErrorToLibError(int commonError)
{
	int libError = NVM_ERR_UNKNOWN;

	switch (commonError)
	{
		case COMMON_ERR_NOMEMORY:
			libError = NVM_ERR_NOMEMORY;
			break;
		case COMMON_ERR_NOTSUPPORTED:
			libError = NVM_ERR_NOTSUPPORTED;
			break;
		case COMMON_ERR_BADFILE:
			libError = NVM_ERR_BADFILE;
			break;
		case COMMON_ERR_INVALIDPERMISSIONS:
			libError = NVM_ERR_INVALIDPERMISSIONS;
			break;
		case COMMON_ERR_INVALIDPARAMETER:
			libError = NVM_ERR_INVALIDPARAMETER;
			break;
		case COMMON_ERR_NOSIMULATOR:
			libError = NVM_ERR_NOSIMULATOR;
			break;
		case COMMON_ERR_FAILED:
		case COMMON_ERR_BADPATH:
		case COMMON_ERR_NO_SERVICE:
		case COMMON_ERR_SERVICE_RUNNING:
		default:
			break;
	}
	return libError;
}

static inline size_t str_to_interleave_format_struct(const char *const str, size_t str_len,
		const char **pp_end, struct interleave_format *p_interleave)
{
	COMMON_LOG_ENTRY();
	NVM_UINT32 format = 0;
	size_t rc = s_strtoui(str, str_len, pp_end, &format);
	if (rc == strlen(str))
	{
		interleave_format_to_struct(format, p_interleave);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

static inline int read_dimm_config(FILE *p_file, struct config_goal *p_goal,
		NVM_UINT16 *p_socket, NVM_UINT32 *p_dimm_handle,
		NVM_UINT64 *p_dimm_size_gb)
{
	COMMON_LOG_ENTRY();
	int rc = 0; // end of file

	char line[NVM_MAX_CONFIG_LINE_LEN];
	while (fgets(line, NVM_MAX_CONFIG_LINE_LEN, p_file) != NULL)
	{
		// trim any leading white space
		s_strtrim_left(line, NVM_MAX_CONFIG_LINE_LEN);
		if (line[0] != '#')
		{
			NVM_BOOL bad_format = 0;
			const char *delim = ",\n";
			char *pLine = line;

			// removing carriage return to make Windows created
			// file Unix compatible
			for (int i = 0, j = 0; pLine[i] != '\0'; i++)
			{
				if (pLine[i] != '\r')
				{
					pLine[j++] = pLine[i];
				}
			}


			READ_CONFIG_TOK(pLine, delim, s_strtous, p_socket, bad_format);
			READ_CONFIG_TOK(pLine, delim, s_strtoui, p_dimm_handle, bad_format);
			READ_CONFIG_TOK(pLine, delim, s_strtoull, p_dimm_size_gb, bad_format);
			READ_CONFIG_TOK(pLine, delim, s_strtoull, &p_goal->memory_size, bad_format);
			READ_CONFIG_TOK(pLine, delim, s_strtoull, &p_goal->app_direct_1_size, bad_format);
			READ_CONFIG_TOK(pLine, delim, str_to_interleave_format_struct,
				&p_goal->app_direct_1_settings.interleave, bad_format);
			READ_CONFIG_TOK(pLine, delim, s_digitstrtouc,
				&p_goal->app_direct_1_settings.mirrored, bad_format);
			READ_CONFIG_TOK(pLine, delim, s_strtous, &p_goal->app_direct_1_set_id, bad_format);
			READ_CONFIG_TOK(pLine, delim, s_strtoull, &p_goal->app_direct_2_size, bad_format);
			READ_CONFIG_TOK(pLine, delim, str_to_interleave_format_struct,
				&p_goal->app_direct_2_settings.interleave, bad_format);
			READ_CONFIG_TOK(pLine, delim, s_digitstrtouc,
				&p_goal->app_direct_2_settings.mirrored, bad_format);
			READ_CONFIG_TOK(pLine, delim, s_strtous, &p_goal->app_direct_2_set_id, bad_format);

			// make the load command compatible with future file formats that include extra data
			// in the csv list by ignoring extra data at the end of line if encountered
			if (pLine != NULL && strlen(pLine) != 0)
			{
				COMMON_LOG_INFO("Config file has extra data that will be ignored.");
			}

			if (bad_format)
			{
				COMMON_LOG_ERROR_F("Found a badly formatted line in the config file '%s'",
					line);
				rc = NVM_ERR_BADFILE;
			}
			else
			{
				rc = 1;
			}
			break;
		}
	}

	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

/*
 * Write the dimm config to the specified file
 */
static inline int write_dimm_config(const struct device_discovery *p_discovery,
		const struct config_goal *p_goal,
		const NVM_PATH path, const NVM_SIZE path_len, const NVM_BOOL append)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	FILE *p_file;
	// set the mode
	char mode[2];
	if (append)
	{
		s_strcpy(mode, "a", sizeof (mode));
	}
	else
	{
		s_strcpy(mode, "w", sizeof (mode));
	}

	if ((p_file = open_file(path, path_len, mode)) == NULL)
	{
		COMMON_LOG_ERROR_F("Failed to open file %s", path);
		rc = NVM_ERR_BADFILE;
	}
	else
	{
		if (lock_file(p_file, FILE_LOCK_MODE_WRITE) != COMMON_SUCCESS)
		{
			COMMON_LOG_ERROR_F("Failed to lock file %s for writing", path);
			rc = NVM_ERR_BADFILE;
		}
		else
		{
			// write the header
			if (!append)
			{
				fprintf(p_file,
						"#SocketID,"
						"DimmHandle,"
						"Capacity,"
						"MemorySize,"
						"AppDirect1Size,"
						"AppDirect1Format,"
						"AppDirect1Mirrored,"
						"AppDirect1Index,"
						"AppDirect2Size,"
						"AppDirect2Format,"
						"AppDirect2Mirrored,"
						"AppDirect2Index\n");
			}

			// DIMM capacity to GiB
			NVM_UINT64 dimm_size_gb = (p_discovery->capacity / BYTES_PER_GIB);

			// convert interleave format structs to number
			NVM_UINT32 p1_format = 0;
			NVM_UINT32 p2_format = 0;
			interleave_struct_to_format(&p_goal->app_direct_1_settings.interleave,
					&p1_format);
			interleave_struct_to_format(&p_goal->app_direct_2_settings.interleave,
					&p2_format);

			// write to the file
			fprintf(p_file, config_line_format,
					p_discovery->socket_id,
					p_discovery->device_handle.handle,
					dimm_size_gb,
					p_goal->memory_size,
					p_goal->app_direct_1_size,
					p1_format,
					p_goal->app_direct_1_settings.mirrored,
					p_goal->app_direct_1_set_id,
					p_goal->app_direct_2_size,
					p2_format,
					p_goal->app_direct_2_settings.mirrored,
					p_goal->app_direct_2_set_id);

			lock_file(p_file, FILE_LOCK_MODE_UNLOCK);
		}
		fclose(p_file);
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}

#ifdef __cplusplus
}
#endif

#endif /* _NVM_UTILITY_H_ */
