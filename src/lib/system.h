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
 * This file defines the internal interface for the system level functionality.
 */

#ifndef SYSTEM_H_
#define	SYSTEM_H_

#include "nvm_management.h"

/*
 * Retrieve basic information about the host server the native API library is running on.
 * @param[in,out] p_host
 * 		A pointer to a #host structure allocated by the caller.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_UNKNOWN @n
 */
int get_host(struct host *p_host);

/*
 * Load a simulator database file.  A simulator may be used to emulate
 * a server with 0 or more devices.
 * @version 0.4 Ready for peer review.
 * @param[in] simulator
 * 		Absolute path to a simulator file.
 * @param[in] simulator_len
 * 		String length of the simulator file path, should be <= #NVM_PATH_LEN.
 * @pre Only available for simulated builds of the Native API library, all other
 * builds will return #NVM_ERR_NOTSUPPORTED.
 * @remarks If another file is currently open, it will remove it before
 * loading the current file.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_BADFILE @n
 * 		#NVM_ERR_UNKNOWN @n
 */
int add_simulator(const NVM_PATH simulator, const NVM_SIZE simulator_len);

/*
 * Remove a simulator database file previously added using #nvm_add_simulator.
 * @version 0.4 Ready for peer review.
 * @pre Only available for simulated builds of the Native API library, all other
 * builds will return #NVM_ERR_NOTSUPPORTED.
 * @pre A simulator file was previously loaded using #nvm_add_simulator.
 * @return Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_NOTSUPPORTED @n
 * 		#NVM_ERR_NOMEMORY @n
 * 		#NVM_ERR_NOSIMULATOR @n
 * 		#NVM_ERR_UNKNOWN @n
 */
int remove_simulator();

/*!
 * Determine if the caller has permission to make changes to the system
 * @return
 * 		#COMMON_SUCCESS @n
 * 		#COMMON_ERR_INVALIDPERMISSIONS
 */
int check_caller_permissions();

/*
 * ***************************************************
 * NUMA
 * ***************************************************
 */

/*
 * Retrieves the number of processor sockets in the system.
 * @return
 * 		Returns the number of sockets on success or one of the following @link #return_code
 * 		return_codes: @endlink @n
 * 		#NVM_ERR_UNKNOWN @n
 */
int get_socket_count();

/*
 * Retrieves s information about each processor socket in the system.
 * @param[in,out] p_sockets
 * 		An array of #socket structures allocated by the caller.
 * @param[in] count
 * 		The size of the array
 * @return
 * 		Returns the number of sockets on success or one of the following @link #return_code
 * 		return_codes: @endlink @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_ARRAYTOOSMALL @n
 * 		#NVM_ERR_UNKNOWN @n
 */
int get_sockets(struct socket *p_sockets, NVM_UINT16 count);

/*
 * Retrieves NUMA information about a given processor socket.
 * @param[in] socket_id
 * 		The NUMA node identifier
 * @param[in,out] p_socket
 * 		A pointer to a #socket structure allocated by the caller.
 * @return
 * 		Returns one of the following @link #return_code return_codes: @endlink @n
 * 		#NVM_SUCCESS @n
 * 		#NVM_ERR_INVALIDPARAMETER @n
 * 		#NVM_ERR_ARRAYTOOSMALL @n
 * 		#NVM_ERR_UNKNOWN @n
 */
int get_socket(NVM_UINT16 socket_id, struct socket *p_socket);

/*
 * Fetches the SMBIOS table as raw data.
 * Returns a pointer to a dynamically-allocated buffer and the size allocated.
 * Caller is responsible for freeing the memory at *pp_smbios_table.
 */
int get_smbios_table_alloc(NVM_UINT8 **pp_smbios_table, size_t *p_allocated_size);

#endif /* SYSTEM_H_ */
