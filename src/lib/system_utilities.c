/*
 * Copyright (c) 2016, Intel Corporation
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

#include <system_utilities.h>
#include <persistence/logging.h>
#include <system.h>
#include <nvm_management.h>

/*
 * Find the core_id number of a given socket_id
 */
int get_first_core_id(NVM_UINT16 socket_id)
{
	COMMON_LOG_ENTRY();
	int rc = NVM_SUCCESS;

	unsigned int socket_count;

	if ((socket_count = get_socket_count()) > 0)
	{
		struct socket *sockets =  malloc(socket_count * sizeof(struct socket));
		if (socket_count != get_sockets(sockets, socket_count))
		{
			rc = NVM_ERR_UNKNOWN;
		}
		else
		{
			int core_id = 0;
			int found = 0;

			for (int i = 0; i < socket_count && !found; i++)
			{
				if (sockets[i].id == socket_id)
				{
					found = 1;
				}
				else
				{
					core_id += sockets[i].logical_processor_count;
				}
			}

			if (!found)
			{
				rc = NVM_ERR_BADSOCKET;
			}
			else
			{
				rc = core_id;
			}
		}

        free(sockets);
	}
	else
	{
		rc = NVM_ERR_UNKNOWN;
	}
	COMMON_LOG_EXIT_RETURN_I(rc);
	return rc;
}
