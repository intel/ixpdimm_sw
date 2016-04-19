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
 * This file contains the implementation for the csv_log interface.
 *
 * The csv log is used for caching log entries to a csv file
 * for better performance over the SQLite DB.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <common_types.h>
#include <string/s_str.h>
#include <os/os_adapter.h>
#include <file_ops/file_ops_adapter.h>

#include "csv_log.h"
#include "config_settings.h"
#include <persistence/lib_persistence.h>
#include <sys/stat.h>

// thread id, time, level, filename, linenumber, message
#define	MAX_LOG_LINE_LEN	20 + 20 + 10 + 1024 + 10 + 2048 + 1
#define	ADD_LOG_SQL	"INSERT INTO log \
	(thread_id, time, level, file_name, line_number, message) VALUES (%s)"
#define	TRIM_LOG_SQL	"DELETE FROM log where id NOT IN \
	(SELECT id FROM log ORDER BY time DESC LIMIT %d)"
#define	TRIM_LOG_SQL_LEN	256
#define	MAX_LOGS	10000
#define	MAX_CACHE_FILE_SIZE	BYTES_PER_MB // 1 MB Max

#define	MUTEX_NAME	"8086_NVM_CSV_LOG_DB_MUTEX"
#ifdef __WINDOWS__
#include <windows.h>
	HANDLE g_db_mutex;
#else
	pthread_mutex_t g_db_mutex;
#endif

/*
 * Initialize the lock
 */
int csv_log_init()
{
	int rc = COMMON_ERR_UNKNOWN;
	if (mutex_init((OS_MUTEX*)&g_db_mutex, MUTEX_NAME))
	{
		rc = COMMON_SUCCESS;
	}
	return rc;
}

/*
 * Flush the cache and clean up the lock
 */
void csv_log_close()
{
	flush_csv_log_to_db(get_lib_store());
	mutex_delete((OS_MUTEX*)&g_db_mutex, MUTEX_NAME);
}

/*
 * Retrieve the path to the cache file - it sits next to the database
 */
void get_log_file_path(COMMON_PATH path)
{
	if (path)
	{
		COMMON_PATH lib_path;
		get_lib_store_path(lib_path);
		s_snprintf(path, COMMON_PATH_LEN, "%s%s", lib_path, ".log");
	}
}

/*
 * Roll the log table in the database to keep a configurable max number of logs
 */
int roll_db_log(PersistentStore *p_db)
{
	int rc = COMMON_SUCCESS;
	int max_logs = MAX_LOGS;
	get_bounded_config_value_int(SQL_KEY_LOG_MAX, &max_logs);
	if (max_logs)
	{
		char sql[TRIM_LOG_SQL_LEN];
		s_snprintf(sql, TRIM_LOG_SQL_LEN, TRIM_LOG_SQL, max_logs);
		if (db_run_custom_sql(p_db, sql) != DB_SUCCESS)
		{
			rc = COMMON_ERR_UNKNOWN;
		}
	}
	return rc;
}

/*
 * Flush the CSV log cache to the database
 */
int flush_csv_log_to_db(PersistentStore *p_db)
{
	int rc = COMMON_ERR_UNKNOWN;
	if (p_db)
	{
		if (mutex_lock(&g_db_mutex))
		{
			// get the log file path
			COMMON_PATH logfile_path;
			get_log_file_path(logfile_path);
			FILE *p_file = NULL;
			if ((p_file = open_file(logfile_path, COMMON_PATH_LEN, "r")) != NULL)
			{
				rc = COMMON_SUCCESS;
				db_begin_transaction(p_db);

				// read in next log entry in file
				char line[MAX_LOG_LINE_LEN];
				while (fgets(line, MAX_LOG_LINE_LEN, p_file) != NULL)
				{
					size_t line_len = s_strnlen(line, MAX_LOG_LINE_LEN);
					// remove the endline
					if (line[line_len-1] == '\n')
					{
						line[line_len-1] = '\0';
					}
					// add it to the db
					char add_log_stmt[line_len + sizeof (ADD_LOG_SQL)];
					s_snprintf(add_log_stmt, MAX_LOG_LINE_LEN, ADD_LOG_SQL, line);

					if (db_run_custom_sql(p_db, add_log_stmt) != DB_SUCCESS)
					{
						KEEP_ERROR(rc, COMMON_ERR_UNKNOWN);
					}
				}

				fclose(p_file);
				delete_file(logfile_path, COMMON_PATH_LEN);

				// roll the log
				KEEP_ERROR(rc, roll_db_log(p_db));
				db_end_transaction(p_db);
			}
			mutex_unlock(&g_db_mutex);
		}
	}
	return rc;
}

/*
 * Write a log to the csv log cache
 */
int csv_write_log(int level, const char *file_name,
		const int line_number, const char *message)
{
	int rc = COMMON_ERR_UNKNOWN;
	COMMON_BOOL flush_log = 0;
	if (mutex_lock(&g_db_mutex))
	{
		COMMON_PATH logfile_path;
		get_log_file_path(logfile_path);
		FILE *p_file = NULL;
		if ((p_file = open_file(logfile_path, COMMON_PATH_LEN, "a+")) != NULL)
		{
			fprintf(p_file, "%llu,%llu,%d,\'%s\',%d,\'%s\'\n",
					(COMMON_UINT64)get_thread_id(), (COMMON_UINT64)time(NULL), level,
					file_name, line_number, message);
			fclose(p_file);
			rc = COMMON_SUCCESS;
		}

		// time to flush the cache to the db?
		struct stat file_stats;
		if ((stat(logfile_path, &file_stats) == 0) && file_stats.st_size)
		{
			if (file_stats.st_size > MAX_CACHE_FILE_SIZE)
			{
				flush_log = 1;
			}
		}
		mutex_unlock(&g_db_mutex);
	}

	if (flush_log)
	{
		rc = flush_csv_log_to_db(get_lib_store());
	}
	return rc;
}
