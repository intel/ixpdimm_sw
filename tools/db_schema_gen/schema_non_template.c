/*
 * This file contains helper functions and MACROS for schema db generation.
 * It is combined with the schema.c.template code when generating schema.c
 * and therefore it does not contain the copyright header.
 */

#define	SQLITE_PREPARE(db, sql, p_stmt) \
		(sqlite3_prepare_v2((db), (sql), strlen(sql) + 1, (&p_stmt), NULL) == SQLITE_OK)

/*!
 * copy a sqlite int64 column into the destination
 */
#define	INTEGER_COLUMN(p_stmt, col, dest) \
	{ \
		dest = sqlite3_column_int64(p_stmt, col); \
	}

/*
 * Replace strncpy with own version. This string copy does not
 * guarantee an ending '\0' which is more appropriate for loading some
 * DB data into some structs.
 */
char *db_strcpy(char *dst, const char *src, size_t dst_size)
{
	if (dst && src && (dst_size != 0))
	{
		memset(dst, 0, dst_size);
		char * dst_i = dst;
		const char * src_i = src;
		memset(dst, 0, dst_size);
		char *end = &dst[dst_size];
		while ((dst_i < end) && (*src_i != '\0'))
		{
			*dst_i++ = *src_i++;
		}
		if (*src_i == '\0' && dst_i < end)
		{
			*dst_i = '\0';
		}
	}

	return dst;
}

/*!
 * copy a sqlite text column into the destination
 */
#define	TEXT_COLUMN(p_stmt, col, dest, len) \
	{\
		if (sqlite3_column_bytes(p_stmt, col) > 0) \
		{ \
			db_strcpy((char *)dest, (char *)sqlite3_column_text(p_stmt, col), len); \
		}\
	}

/*!
 * bind a string value to a specific parameter in the query
 */
#define	BIND_TEXT(p_stmt, parameter, text) \
		sqlite3_bind_text(p_stmt, \
			sqlite3_bind_parameter_index(p_stmt, parameter), \
			(char *)(text), -1, SQLITE_STATIC)

/*!
 * bind an int value to a specific parameter in the query
 */
#define	BIND_INTEGER(p_stmt, parameter, value) \
		sqlite3_bind_int64(p_stmt, \
			sqlite3_bind_parameter_index(p_stmt, parameter), \
			value)

/*!
 * Macro that will persist the first database error encountered.
 * @param[in,out] rc
 * 		Return code to be returned if indicating an error
 * @param[in] rc_new
 * 		Return code to be returned if @c rc does not indicate error
 */
#define	KEEP_DB_ERROR(rc, rc_new)	rc = (rc < DB_SUCCESS) ? rc : rc_new;

/*!
 * Macro that will persist the first database success encountered.
 * @param[in,out] rc
 * 		Return code to be returned if indicating success
 * @param[in] rc_new
 * 		Return code to be returned if @c rc does not indicate success
 */
#define	KEEP_DB_SUCCESS(rc, rc_new)	rc = (rc >= DB_SUCCESS) ? rc : rc_new;


/*
 *	SQL API
 */
struct persistentStore
{
	sqlite3 *db;
};

/*!
 * Returns the number of rows in the table name provided.  If there is an issue with the
 * query (or the table doesn't exist) will return 0.
 * @param[in] p_ps
 * 		Pointer to the persistent store connection
 * @param[in] table_name
 * 		Table name
 * @param[out] p_count
 *		Count of the table is put in p_count
 * @return
 * 		Whether it was successful or not
 */
enum db_return_codes table_row_count(const PersistentStore *p_ps, const char *table_name, int *p_count)
{
	enum db_return_codes rc = DB_ERR_FAILURE;
	*p_count = 0;
	sqlite3_stmt *p_stmt;
	char buffer[1024];
	snprintf(buffer, 1024, "select count(*) from %s", table_name);
	if (SQLITE_PREPARE(p_ps->db, buffer, p_stmt))
	{
		if (sqlite3_step(p_stmt) == SQLITE_ROW)
		{
			*p_count = sqlite3_column_int(p_stmt, 0);
			rc = DB_SUCCESS;
		}
		// cleanup
		sqlite3_finalize(p_stmt);
	}
	return rc;
}

/*
 *	Execute some sql with no expected results (INSERT, UPDATE)
 */
enum db_return_codes  run_sql_no_results(sqlite3 *p_db, const char *sql)
{
	sqlite3_stmt *p_stmt;
	enum db_return_codes rc = DB_ERR_FAILURE;
	if (SQLITE_PREPARE(p_db, sql, p_stmt))
	{
		if (sqlite3_step(p_stmt) == SQLITE_DONE)
		{
			rc = DB_SUCCESS;
		}
		else
		{
			// uncomment for debugging
			// printf("Error running SQL: \n%s\n", sql);
		}
		sqlite3_finalize(p_stmt);
	}
	else
	{
		// uncomment for debugging
		// printf("Error preparing SQL: \n%s\n", sql);
	}
	return rc;
}

/*
 * Execute some SQL on a sqlite db and expect a single int value as result
 */
enum db_return_codes run_scalar_sql(const PersistentStore *p_ps, const char *sql, int *p_scalar)
{
	enum db_return_codes rc = DB_ERR_FAILURE;
	sqlite3_stmt *p_stmt;
	if (SQLITE_PREPARE(p_ps->db, sql, p_stmt))
	{
		if (sqlite3_step(p_stmt) == SQLITE_ROW)
		{
			*p_scalar = sqlite3_column_int(p_stmt, 0);
			rc = DB_SUCCESS;
		}
		sqlite3_finalize(p_stmt);
	}
	return rc;
}

/*
 * Execute some SQL on a sqlite db and expect a single char* value as result
 */
enum db_return_codes run_text_scalar_sql(const PersistentStore *p_ps, const char *sql, char *p_value, int len)
{
	enum db_return_codes rc = DB_ERR_FAILURE;
	sqlite3_stmt *p_stmt;
	if (SQLITE_PREPARE(p_ps->db, sql, p_stmt))
	{
		if (sqlite3_step(p_stmt) == SQLITE_ROW)
		{
			TEXT_COLUMN(p_stmt, 0, p_value, len);
			rc = DB_SUCCESS;
		}
		sqlite3_finalize(p_stmt);
	}
	return rc;
}

/*
 * Returns if the table exists or not
 */
int table_exists(sqlite3 *p_db, const char *table)
{
	int exists = 0;  // table does not exist by default (false)
	sqlite3_stmt *p_stmt;
	char buf[1024];

	snprintf(buf, 1024,
			"SELECT name FROM sqlite_master WHERE name = '%s'", table);

	if (SQLITE_PREPARE(p_db, buf, p_stmt))
	{
		if (sqlite3_step(p_stmt) == SQLITE_ROW)
		{
			exists = 1; // table exists (true)
		}
		sqlite3_finalize(p_stmt);
	}
	return exists;
}

PersistentStore *open_PersistentStore(const char *path)
{
	PersistentStore *result = (PersistentStore *)malloc(sizeof (PersistentStore));
	if (result != NULL)
	{
		if (sqlite3_open_v2(path, &(result->db),
			SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX, NULL) != SQLITE_OK)
		{
			free_PersistentStore(&result);
		}
		else
		{
			// set a busy timeout to avoid file locking issues
			sqlite3_busy_timeout(result->db, 30000);
		}
	}

	return result;
}

/*
 * Close the DB and release the memory for a PersistentStore object
 */
int free_PersistentStore(PersistentStore **pp_persistentStore)
{
	int rc = DB_SUCCESS;
	if (*pp_persistentStore != NULL && (*pp_persistentStore)->db != NULL)
	{
		if (sqlite3_close((*pp_persistentStore)->db) != SQLITE_OK)
		{
			rc = DB_ERR_FAILURE;
		}
	}
	if (*pp_persistentStore != NULL)
	{
		free(*pp_persistentStore);
	}
	*pp_persistentStore = NULL;
	return rc;
}

/*
 * Add a new history instance.  Return the new instance ID
 */
enum db_return_codes db_add_history(PersistentStore *p_ps,
	const char *history_name,
	int *p_history_id)
{
	enum db_return_codes rc = DB_ERR_FAILURE;
	*p_history_id = 0;
	sqlite3_stmt *p_stmt;
	if (run_scalar_sql(p_ps, "SELECT MAX(history_id) FROM history", p_history_id) == DB_SUCCESS)
	{
		(*p_history_id)++;
		char *sql = "INSERT INTO history \
			( history_id,  timestamp,  history_name) VALUES \
			($history_id, datetime('now'), $history_name);";
		if (SQLITE_PREPARE(p_ps->db, sql, p_stmt))
		{
			BIND_INTEGER(p_stmt, "$history_id", *p_history_id);
			BIND_TEXT(p_stmt, "$history_name", history_name);
			if (sqlite3_step(p_stmt) == SQLITE_DONE)
			{
				rc = DB_SUCCESS;
			}
			sqlite3_finalize(p_stmt);
		}
	}
	return rc;
}

enum db_return_codes  db_begin_transaction(PersistentStore *p_ps)
{
	return run_sql_no_results(p_ps->db, "BEGIN TRANSACTION");
}

enum db_return_codes  db_end_transaction(PersistentStore *p_ps)
{
	return run_sql_no_results(p_ps->db, "END TRANSACTION");
}

enum db_return_codes  db_rollback_transaction(PersistentStore *p_ps)
{
	return run_sql_no_results(p_ps->db, "ROLLBACK TRANSACTION");
}

enum db_return_codes db_run_custom_sql(PersistentStore *p_ps, const char *sql)
{
	return run_sql_no_results(p_ps->db, sql);
}

void update_sqlite3_hook(PersistentStore *p_ps, void (*xCallback)(void*,int,char const *,char const *, long long))
{
	sqlite3_update_hook(p_ps->db, xCallback, NULL);
}

/*
 * Get history table entries
 */
int db_get_history_ids(const PersistentStore *p_ps,
	int *p_history_ids,
	int count)
{
	int rc = DB_ERR_FAILURE;
	memset(p_history_ids, 0, sizeof (int) * count);
	char *sql = "SELECT \
		history_id \
		FROM history \
		";
	sqlite3_stmt *p_stmt;
	if (SQLITE_PREPARE(p_ps->db, sql, p_stmt))
	{
		int index = 0;
		while (sqlite3_step(p_stmt) == SQLITE_ROW && index < count)
		{
			INTEGER_COLUMN(p_stmt,
					0,
					p_history_ids[index]);
			index++;
		}
		sqlite3_finalize(p_stmt);
		rc = index;
	}
	return rc;
}
