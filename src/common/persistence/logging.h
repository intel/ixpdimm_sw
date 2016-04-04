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
 * This file contains the definition of the common logging functions.
 */

#ifndef	LOGGING_H_
#define	LOGGING_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <common_types.h>
#include "schema.h"

/*!
 * Where the logs get written
 */
enum log_dest
{
	LOG_DEST_DB = 0,   //!< LOG_DEST_DB - write to the config DB
	LOG_DEST_SYSLOG = 1//!< LOG_DEST_SYSLOG - write to the syslog
};

/*
 * Macros to facilitate logging
 */

//! Log Macro: Log Level = Debug
#define	COMMON_LOG_DEBUG(statement)  \
	log_trace(LOGGING_LEVEL_DEBUG, __FILE__, __LINE__, statement)

//! Log Macro: Log Level = Info
#define	COMMON_LOG_INFO(statement)  \
	log_trace(LOGGING_LEVEL_INFO, __FILE__, __LINE__, statement)

//! Log Macro: Log Level = Warning
#define	COMMON_LOG_WARN(statement)  \
	log_trace(LOGGING_LEVEL_WARN,  __FILE__, __LINE__, statement)

//! Log Macro: Log Level = Error
#define	COMMON_LOG_ERROR(statement)  \
	log_trace(LOGGING_LEVEL_ERROR, __FILE__, __LINE__, statement)

//! Function Entry Log Macro: Log Level = Info
#define	COMMON_LOG_ENTRY()  log_trace_f \
	(LOGGING_LEVEL_INFO, __FILE__, __LINE__, \
	"Entering %s()", ((char *)__func__))

//! Function Exit Log Macro: Log Level = Info
#define	COMMON_LOG_EXIT()  \
	log_trace_f(LOGGING_LEVEL_INFO, __FILE__, __LINE__, \
	"Exiting %s", ((char *)__func__))

/*
 * Macros to facilitate logging w/ formatted messages
 */

//! Formatted Log Macro: Log Level = Debug
#define	COMMON_LOG_DEBUG_F(format, ...)  \
	log_trace_f(LOGGING_LEVEL_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)

//! Formatted Log Macro: Log Level = Info
#define	COMMON_LOG_INFO_F(format, ...)  \
	log_trace_f(LOGGING_LEVEL_INFO, __FILE__, __LINE__, format, __VA_ARGS__)

//! Formatted Log Macro: Log Level = Warning
#define	COMMON_LOG_WARN_F(format, ...)  \
	log_trace_f(LOGGING_LEVEL_WARN, __FILE__, __LINE__, format, __VA_ARGS__)

//! Formatted Log Macro: Log Level = Error
#define	COMMON_LOG_ERROR_F(format, ...)  \
	log_trace_f(LOGGING_LEVEL_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)

//! Error message for when get_* != get_*_count - in most cases they should return the same count
#define	COMMON_LOG_ERROR_BAD_COUNT(base, count, base_rc) \
	COMMON_LOG_ERROR_F(base " doesn't match " base "_count. %d != %d", base_rc, count);\
	base_rc = COMMON_ERR_UNKNOWN;

//! Formatted Function Entry Log Macro: Log Level = Info
#define	COMMON_LOG_ENTRY_PARAMS(param_format, ...)  \
	log_trace_f(LOGGING_LEVEL_INFO, __FILE__, __LINE__, \
	"Entering %s(" param_format ")", ((char *)__func__), __VA_ARGS__)

//! Formatted Function Exit Log Macro: Log Level = Info
#define	COMMON_LOG_EXIT_RETURN(return_format, ...)  \
	log_trace_f(LOGGING_LEVEL_INFO, __FILE__, __LINE__, \
	"Exiting %s(): " return_format, ((char *)__func__), __VA_ARGS__)

//! Formatted Function Exit w/ Return Value Log Macro: Log Level = Info
#define	COMMON_LOG_EXIT_RETURN_I(return_value) \
	COMMON_LOG_EXIT_RETURN("%d", return_value)

void print_buffer_to_file(const char *filename, char *p_buf, size_t buf_size, char *p_prefix);

/*!
 * The global to contain the number of trace rows
 */
extern int trace_rows;

/*!
 * If logging is turned on, write to the trace table
 * @param[in] level
 * 		How severe is the log.  Error, Warning, Info, or Debug
 * @param[in] file_name
 * 		File name where the log entry initiated from
 * @param[in] line_number
 * 		Line number within the file the log entry initiated from
 * @param[in] message
 * 		The message to be logged
 */
void log_trace(int level, const char *file_name, int line_number, const char *message);

/*!
 * If logging is turned on, write to log using format.
 * @param[in] level
 * 		How severe is the log.  Error, Warning, Info, or Debug
 * @param[in] file_name
 * 		File name where the log entry initiated from
 * @param[in] line_number
 * 		Line number within the file the log entry initiated from
 * @param[in] format
 * 		The string format used to create the log message
 * @param[in] ...
 * 		Parameters need to match the format passed
 */
void log_trace_f(int level, const char *file_name, int line_number, const char *format, ...);

/*!
 * Checks if the given log level is enabled
 * @param[in] log_level
 * 		log_level to check
 * @return
 * 		1 if True, 0 if False
 */
int log_level_check(int log_level);

/*!
 * Initializes logging
 */
int log_init();

/*!
 * Pushes all remaining log entries into the db and then de-init's the logfile.
 */
void log_close();

/*!
 * Performs appropriate gather functions to ensure logs are in the config database
 * @return
 * 		COMMON_SUCCESS @n
 * 		COMMON_ERR_BADFILE @n
 * 		COMMON_ERR_UNKNOWN
 */
int log_gather();

/*
 * Retrieve the current log level
 */
int get_current_log_level();

/*
 * Set the current log level
 */
COMMON_BOOL set_current_log_level(int level);

#ifdef __cplusplus
}
#endif

#endif /* LOGGING_H_ */
