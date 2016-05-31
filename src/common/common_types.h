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
 * This file defines the common defines and typedefs for the common code library
 */

#ifndef _COMMON_TYPES_H
#define	_COMMON_TYPES_H

#include <wchar.h>

#include <limits.h>

#ifdef __cplusplus
extern "C"
{
#endif


/*
 * *******************************************************************************************
 * Common defines and typedefs
 * *******************************************************************************************
 */

/*!
 * Copyright, Null-terminated string.
 */
#define	INTEL_COPYRIGHT_STRING	"Copyright© 2015 Intel Corporation. All rights reserved\0"

/*!
 * Company Name, Null-terminated string.
 */
#define	INTEL_COMPANY_NAME	"Intel Corporation\0"

/*!
 * Name of the SQLite database file that contains the configuration settings.
 */
#define	CONFIG_FILE	"apss.dat"

/*!
 * Bytes per kilobyte for size conversions.
 */
#define	BYTES_PER_KB	1024

/*!
 * Bytes per mebibyte for size conversions.
 */
#define	BYTES_PER_MB	(COMMON_UINT64)(1 << 20) // 1024^2

/*!
 * Bytes per gibibyte for size conversions.
 */
#define	BYTES_PER_GB	(1024 * BYTES_PER_MB)

/*!
 * Convert MiB to GiB
 */
#define MB_TO_GB(x)		(x / 1024)

/*!
 * Convert GiB to MiB
 */
#define GB_TO_MB(x)		(x * 1024)

/*!
 * Bytes per 4K chunk, for size conversions.
 */
#define	BYTES_PER_4K_CHUNK	4096llu

/*
 * Maximum number of MiB that can be converted to bytes in a UINT64
 */
#define	MAX_UINT64_MB	((0xFFFFFFFFFFFFFFFFllu - 1) >> 20)

/*
 * Maximum number of GiB that can be converted to bytes in a UINT64
 */
#define	MAX_UINT64_GB	((0xFFFFFFFFFFFFFFFFllu - 1) >> 30)

/*!
 * Helper macro.
 * Use STRINGIZE(s) instead of this macro
 */
#define	STRINGIZE2(s) #s

/*!
 * Turns the argument @c s into a string.
 * @param s
 * 		Version number.  Ideally formatted as A.B.C.D, where A,B,C,D are integers.
 */
#define	STRINGIZE(s) STRINGIZE2(s)

/*!
 * Turns the defined preprocessor variable into the globally accessible @c VERSION_STR
 */
#define	VERSION_STR STRINGIZE(__VERSION_NUMBER__)

/*!
 * Number of characters needed to fully support a datetime string
 */
#define	COMMON_DATETIME_LEN	26

/*!
 * @a ???
 */
#define	COMMON_VALUE_LEN	1024

/*!
 * @a ???
 */
#define	COMMON_STRINGREPLACE_SIZE 2048

/*!
 * Number of characters allowed allocated for revision string.
 */
#define	COMMON_REVISION_LEN	25

/*!
 * Number of characters allowed for Major revision portion of the revision string
 */
#define	COMMON_MAJOR_REVISION_LEN	2

/*!
 * Number of characters allowed for Minor revision portion of the revision string
 */
#define	COMMON_MINOR_REVISION_LEN	2

/*!
 * Number of characters allowed for Hotfix revision portion of the revision string
 */
#define	COMMON_HOTFIX_REVISION_LEN	2

/*!
 * Number of characters allowed for Build revision portion of the revision string
 */
#define	COMMON_BUILD_REVISION_LEN 	4

/*!
 * Key name for unit tests to simulate unsupported DIMMs
 */
#define	COMMON_TEST_NOTSUPPORTED	"TEST_NOTSUPPORTED"



/*!
 * 8-bit Signed Integer.
 */
typedef signed char COMMON_INT8;

/*!
 * 16-bit Signed Integer.
 */
typedef signed short COMMON_INT16;

/*!
 * 32-bit Signed Integer.
 */
typedef signed int COMMON_INT32;

/*!
 * 64-bit Signed Integer.
 */
typedef signed long long COMMON_INT64;

/*!
 * 8-bit Unsigned Integer.
 */
typedef unsigned char COMMON_UINT8;

/*!
 * 16-bit Unsigned Integer.
 */
typedef unsigned short COMMON_UINT16;

/*!
 * 32-bit Unsigned Integer.
 */
typedef unsigned int COMMON_UINT32;

/*!
 * 64-bit Unsigned Integer.
 */
typedef unsigned long long COMMON_UINT64;

/*!
 * 8 bit unsigned integer as a boolean
 */
typedef unsigned char COMMON_BOOL;

/*!
 * The maximum number of characters a file or directory path can contain
 */
#define	COMMON_PATH_LEN	PATH_MAX

/*!
 * Type to use when defining the allocated size of a COMMON_* type.
 */
typedef size_t COMMON_SIZE;

#ifdef	__WINDOWS__
/*!
 * The path (& file) separator for Windows (as a string)
 */
#define	COMMON_PATH_SEP	"\\"

#else
/*!
 * The path (& file) separator for Linux (as a string)
 */
#define	COMMON_PATH_SEP	"/"

#endif

/*!
 * Type to use for file or directory paths.
 */
typedef char COMMON_PATH[COMMON_PATH_LEN];

/*!
 * Type to use when defining/converting a UTF-16 encoded file or directory path.
 * This type should be used when passing unicode strings to Windows API functions.
 */
typedef wchar_t COMMON_WPATH[COMMON_PATH_LEN];

/*!
 * Type to use when getting a datetime string
 */
typedef char COMMON_DATETIME_STR[COMMON_DATETIME_LEN];

#define	COMMON_GUID_LEN	16
#define	COMMON_UID_LEN	(16 * 2) + 5
#define	COMMON_GUID_STR_LEN	(16 * 2) + 5

typedef char COMMON_UID[COMMON_UID_LEN];
typedef char COMMON_GUID_STR[COMMON_GUID_STR_LEN];
typedef unsigned char COMMON_GUID[COMMON_GUID_LEN];

/*
 * *******************************************************************************************
 * 	Enums
 * ******************************************************************************************
 */

/*!
 * Return values for the common library.
 */
enum common_return_code
{
	COMMON_SUCCESS = 0, 					//!< the method succeeded
	COMMON_ERR_UNKNOWN = -1, 				//!< an unknown error occurred
	COMMON_ERR_NOMEMORY = -2, 				//!< not enough memory to complete requested operation
	COMMON_ERR_NOTSUPPORTED = -5, 			//!< this method is not supported in the current context
	COMMON_ERR_FAILED = -6, 				//!< the method failed
	COMMON_ERR_BADPATH = -20, 				//!< the path does not exist
	COMMON_ERR_BADFILE = -32, 				//!< the file is not valid
	COMMON_ERR_INVALIDPERMISSIONS = -37,	//!< the caller doesn't have appropriate permissions
	COMMON_ERR_INVALIDPARAMETER = -39, 		//!< invalid input parameter
	COMMON_ERR_NOSIMULATOR = -63, 			//!< no simulator loaded
	COMMON_ERR_NO_SERVICE = -68, 			//!< The service is not running or cannot be located.
	COMMON_ERR_SERVICE_RUNNING = -69 		//!< The service is already running
};

/*!
 * Logging level used with the library logging functions.
 */
enum logging_level_t
{
	// Levels could become accretive if levels are numerically preserved
	LOGGING_LEVEL_ERROR = 0, //!< Errors only
	LOGGING_LEVEL_WARN = 1, //!< Warnings + Errors
	LOGGING_LEVEL_INFO = 2, //!< Information + Warnings + Errors
	LOGGING_LEVEL_DEBUG = 3 //!< Debug + Information + Warnings + Errors
};

/*
 * *******************************************************************************************
 * 	Macros
 * ******************************************************************************************
 */

/*!
 * Macro that determines if the @c status argument indicates an error.
 * @param[in] status
 * 		Status code (@b enum @b common_return_code)
 */
#define	CHECK_ERROR(status) (status < COMMON_SUCCESS)

/*!
 * Macro that will persist the first error encountered.
 * @param[in,out] rc
 * 		Return code to be returned if indicating an error
 * @param[in] rc_new
 * 		Return code to be returned if @c rc does not indicate error
 */
#define	KEEP_ERROR(rc, rc_expr) \
{ \
	int rc_new = rc_expr; \
	rc = (rc < COMMON_SUCCESS) ? rc : rc_new; \
}


/*!
 * Macro that will check the current return code, then execute an expression
 * if it is in a success state.
 * If the expression is not successful, then the current return code is set to the expression
 * result.
 *
 * Useful for chaining together various expressions where error handling is needed.
 */
#define	IF_SUCCESS_EXEC(current_rc, expression) \
				{\
					if (current_rc == NVM_SUCCESS)\
					{\
						int expression_result = (expression);\
						if (expression_result != NVM_SUCCESS) \
						{\
							current_rc = expression_result;\
						}\
					}\
				}

/*!
 * Macro that will check the current return code for a success state, then execute an expression.
 * If the expression is successful, then the result is added to the sum. If not, then the current_rc
 * will be set to the expression result.
 *
 * Useful for chaining together various expressions where the results are summed and error handling
 * is needed.
 */
#define	IF_SUCCESS_EXEC_AND_SUM(expression, current_rc, sum) \
				{\
					if (current_rc == NVM_SUCCESS)\
					{\
						int expression_result = (expression);\
						if (expression_result >= 0) \
						{\
							sum += expression_result;\
						}\
						else \
						{\
							current_rc = expression_result;\
						}\
					}\
				}

/*!
 * Macro that will persist the first success encountered.
 * @param[in,out] rc
 * 		Return code to be returned if indicating success
 * @param[in] rc_new
 * 		Return code to be returned if @c rc does not indicate success
 */
#define	KEEP_SUCCESS(rc, rc_new)	rc = (rc >= COMMON_SUCCESS) ? rc : rc_new;


/*
 * *******************************************************************************************
 * 	Utility functions
 * ******************************************************************************************
 */
static inline int cmp_bytes(const unsigned char *bytes1,
		const unsigned char *bytes2, const COMMON_SIZE len)
{
	if ((bytes1 == NULL) || (bytes2 == NULL))
	{
		return -1;
	}

	for (COMMON_SIZE count = 0; count < len; count++)
	{
		if (bytes1[count] != bytes2[count])
		{
			return 0;
		}
	}

	return 1;
};


static inline unsigned char bcd_byte_to_dec(unsigned char num)
{
	unsigned char ret_val;

	if (((num & 0xF0) >> 4) >= 10)
	{
		ret_val = 0xFF;
	}
	else if ((num & 0x0F) >= 10)
	{
		ret_val = 0xFF;
	}
	else
	{
		ret_val = (((num & 0xF0) >> 4) * 10 + (num & 0x0F));
	}

	return ret_val;
}

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_TYPES_H */
