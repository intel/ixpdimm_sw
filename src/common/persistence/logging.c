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
 * This file contains the implementation of the common logging functions.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <common_types.h>
#include <os/os_adapter.h>
#include <file_ops/file_ops_adapter.h>

#include "schema.h"
#include "logging.h"
#include "csv_log.h"
#include "config_settings.h"
#include "lib_persistence.h"

#define	SYSLOG_SOURCE	"IntelNVM"

void print_buffer_to_file(const char *filename, char *p_buf, size_t buf_size, char *p_prefix)
{

	if (!p_buf)
	{
		return;
	}

	FILE *p_file = NULL;
	if ((p_file = open_file(filename, COMMON_PATH_LEN, "a+")) != NULL)
	{

		if (p_prefix)
		{
			fprintf(p_file, "\n%s", p_prefix);
		}

		fprintf(p_file, "Buffer Size is %u\n", (unsigned int) (buf_size));

		for (unsigned int i = 0; i < buf_size; ++i)
		{
			if (i > 0 && !(i % 16))
			{
				fprintf(p_file, "\n");
			} else if (i > 0 && !(i % 8))
			{
				fprintf(p_file, "\t");
			}

			fprintf(p_file, "%.2x ", (unsigned char) p_buf[i]);
		}

		fprintf(p_file, "\n");
		fclose(p_file);
	}
}


/*
 * Helper function declarations
 */
void log_to_syslog(int level, const char *file_name, int line_number, const char *message);

/*!
 * Initializes logging
 */
int log_init()
{
	// initialize the csv log
	return csv_log_init();
}

/*!
 * Pushes all remaining log entries into the db and then de-init's the logfile.
 */
void log_close()
{
	// close the csv log
	csv_log_close();
}

/*
 * log something
 */
void do_log(int level, const char *file_name, int line_number, const char *message)
{
	// get config value to determine where to write the log
	int dest = LOG_DEST_DB; // default to DB
	get_config_value_int(SQL_KEY_LOG_DESTINATION, &dest);
	switch (dest)
	{
		case LOG_DEST_DB:
			// csv file is a buffer to the DB for performance
			csv_write_log(level, file_name, line_number, message);
			break;
		case LOG_DEST_SYSLOG:
			log_to_syslog(level, file_name, line_number, message);
			break;
	}
}

/*
 * If logging is turned on, write to the trace table
 */
void log_trace(int level, const char *file_name, int line_number, const char *message)
{
	/* uncomment for quick debugging */
	// printf("---file_name: %s, line_number: %d, message: %s---\n",
	//	file_name, line_number, message);

	if (log_level_check(level))
	{
		do_log(level, file_name, line_number, message);
	}
}

/*
 * If logging is turned on, write to log using format.
 */
void log_trace_f(int level, const char *file_name, int line_number, const char *format, ...)
{
	if (log_level_check(level))
	{
		char *message = NULL;
		int size = 64;

		// if we can't allocate memory, just exit - no logging
		if ((message = (char *)malloc(size)) != NULL)
		{
			va_list args;
			char *temp;
			int need = 0;

			/* uncomment for quick debugging */
//			va_start(args, format);
//			vprintf(format, args);
//			printf("\n");
//			va_end(args);

			while (1)
			{
				va_start(args, format);
				need = vsnprintf(message, size, format, args);
				va_end(args);

				// size was big enough
				if (need > -1 && need < size)
				{
					do_log(level, file_name, line_number, message);
					break;
				}
				else
				{
					// vsnprintf returned exact size
					if (need > -1)
					{
						size = need + 1;
					}
					else // take another guess
					{
						size *= 2;
					}

					// attempt to build a bigger char array
					if ((temp = (char *)realloc(message, size)) == NULL)
					{
						break;
					}
					else
					{
						message = temp;
					}
				}
			}

			// free up the memory for the message now that it has been sent
			// or failed reallocation
			free(message);
			message = NULL;
		}
	}
}

/*
 * check if logging is turned on
 */
int log_level_check(int log_level)
{
	return (log_level <= get_current_log_level());
}

int log_gather()
{
	return flush_csv_log_to_db(get_lib_store());
}

/*
 * Helper function to write a log message to the trace log
 */
void log_to_syslog(int level, const char *file_name, int line_number, const char *message)
{
	// limit tracing to error level logs
	if (level == LOGGING_LEVEL_ERROR)
	{
		char long_message[SYSLOG_MESSAGE_MAX];
		snprintf(long_message, SYSLOG_MESSAGE_MAX, "%s [%d] - %s", file_name, line_number, message);

		log_system_event(SYSTEM_EVENT_TYPE_ERROR, SYSLOG_SOURCE, long_message);
	}
}

/*
 * Retrieve the current log level from the db
 */
int get_current_log_level()
{
	int log_level = -1;
	get_config_value_int(SQL_KEY_LOG_LEVEL, &log_level);
	return log_level;
}

/*
 * Set the current log level in the db
 */
COMMON_BOOL set_current_log_level(int level)
{
	COMMON_BOOL set = 0;
	char level_str[CONFIG_VALUE_LEN];
	snprintf(level_str, CONFIG_VALUE_LEN, "%d", level);
	if (add_config_value(SQL_KEY_LOG_LEVEL, level_str) == COMMON_SUCCESS)
	{
		set = 1;
	}
	return set;
}
