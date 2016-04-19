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
 * This file contains the definition of common utility methods for
 * interfacing with the configuration database.
 */

#ifndef	_LIB_PERSISTENCE_H_
#define	_LIB_PERSISTENCE_H_

#include <common_types.h>
#include <nvm_types.h>
#include "schema.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Get the path to the configuration database.
 * @param path
 * @return
 */
extern int get_lib_store_path(COMMON_PATH path);

/*!
 * create a database with default configuration
 * @param path
 * @return
 */
extern int create_default_config(const char *path);

/*!
 * Open the configuration database and initialize logging.
 * @param[in] path
 * 		The absolute path to the database
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_UNKNOWN
 */
extern int open_lib_store(const char *path);

/*!
 * Close the configuration database and flush the log to the database.
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_UNKNOWN
 */
extern int close_lib_store();

/*!
 * Return a pointer to the configuration database.
 * @return
 * 		A pointer to the configuration database or NULL if not open.
 */
extern PersistentStore *get_lib_store();

/*!
 * Open the default configuration database and return the pointer to it.
 */
PersistentStore *open_default_lib_store();

/*!
 * Retrieve a configuration value given a key
 * @param[in] key
 * 		The key to retrieve
 * @param[out] value
 * 		A buffer to hold the value
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_UNKNOWN
 */
extern int get_config_value(const char *key, char *value);

/*!
 * Retrieve a bounded configuration value given a key, whose value is expected to be an integer.
 * @param[in] key
 * 		The key to retrieve
 * @param[out] value
 * 		A buffer to hold the value
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_UNKNOWN
 */
extern int get_bounded_config_value_int(const char *key, int *value);

/*!
 * Retrieve a bounded configuration value given a key
 * @param[in] key
 * 		The key to retrieve
 * @param[out] value
 * 		A buffer to hold the value
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_UNKNOWN
 */
extern int get_bounded_config_value(const char *key, char *value);

/*!
 * Retrieve a bounded configuration value given a key
 * @param[in] key
 * 		The key to retrieve
 * @param[out] value
 * 		A buffer to hold the value
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_UNKNOWN
 */

/*!
 * Return true if the given value is in the valid range for the given key
 * @param[in] key
 * 		The key to retrieve
 * @param[out] value
 * 		The given value
 * @return
 * 		0 False
 * 		1 True
 */
extern NVM_BOOL is_valid_value(const char *key, int value);

/*!
 * Retrieve a configuration value given a key, whose value is expected to be an integer.
 * @param[in] key
 * 		The key to retrieve
 * @param[out] value
 * 		A pointer to an integer to hold the value
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_UNKNOWN
 */
extern int get_config_value_int(const char *key, int *value);

/*!
 * Add a new configuration setting.
 * @param[in] key
 * 		The key to add.
 * @param[in] value
 * 		The value.
 * @remarks If the key already exists, it updates the value to the new value.
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_UNKNOWN
 */
extern int add_config_value(const char *key, const char *value);

/*!
 * Remove a configuration value.
 * @param[in] key
 * 		The key to remove.
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_UNKNOWN
 */
extern int rm_config_value(const char *key);

/*
 * Set the configuration settings to their default values
 */
extern int set_default_config_settings(PersistentStore *p_ps);

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_PERSISTENCE_H_ */
