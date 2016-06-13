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

#include "WbemToCli.h"
#include <LogEnterExit.h>
#include <libinvm-cli/Parser.h>
#include "CommandParts.h"
#include <libinvm-cli/SyntaxErrorBadValueResult.h>
#include <libinvm-cli/SyntaxErrorMissingValueResult.h>
#include "WbemToCli_utilities.h"
#include <pmem_config/NamespaceViewFactory.h>
#include <string/s_str.h>
#include <mem_config/PoolViewFactory.h>
#include <uid/uid.h>
#include <string/s_str.h>

cli::nvmcli::WbemToCli::WbemToCli()
{
}

cli::nvmcli::WbemToCli::~WbemToCli()
{
}

/*
 * For commands that support an optional -namespace target,
 * retrieve the namespace UID(s) of the specified target
 * or all namespace UIDs if not specified.
 */
cli::framework::ErrorResult *cli::nvmcli::WbemToCli::getNamespaces(
        const framework::ParsedCommand &parsedCommand,
        std::vector<std::string> &namespaces)
{
    LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
    framework::ErrorResult *pResult = NULL;
    std::vector<std::string> nsList =
            cli::framework::Parser::getTargetValues(parsedCommand,
                                                    cli::nvmcli::TARGET_NAMESPACE.name);
    try
    {
        // a target value was specified
        if (!nsList.empty())
        {
            std::vector<std::string> allNamespaces =
                    wbem::pmem_config::NamespaceViewFactory::getNamespaceUidList();
            // check each namespace uid, validate it's format if it exists
            for (size_t i = 0; i < nsList.size(); i++)
            {
                if (nsList[i].length() + 1 != COMMON_UID_LEN)
                {
                    pResult = new framework::SyntaxErrorBadValueResult(framework::TOKENTYPE_TARGET,
                                                                       TARGET_NAMESPACE.name, nsList[i]);
                    break;
                }
                else if (std::find(allNamespaces.begin(), allNamespaces.end(), nsList[i]) ==
                         allNamespaces.end())
                {
                	std::string errorString = framework::ResultBase::stringFromArgList(
                			TR(INVALID_NAMESPACEID_ERROR_STR.c_str()), nsList[i].c_str());

                    pResult = new framework::ErrorResult(framework::ErrorResult::ERRORCODE_UNKNOWN,
                    		errorString);
                    break;
                }
                else
                {
                    namespaces.push_back(nsList[i]);
                }
            }
        }
            // no target specified, just get all namespace uids
        else
        {
            namespaces = wbem::pmem_config::NamespaceViewFactory::getNamespaceUidList();
        }
    }
    catch (wbem::framework::Exception &e)
    {
        pResult = NvmExceptionToResult(e);
    }
    return pResult;
}

/*
 * For commands that support the -pool target, verify the pool UID specified
 * or retrieve it if not specified
 */
cli::framework::ErrorResult *cli::nvmcli::WbemToCli::checkPoolUid(
		const cli::framework::ParsedCommand &parsedCommand, std::string &poolUid)
{
    LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

    cli::framework::ErrorResult *pResult = NULL;
    std::string poolTarget =
    		cli::framework::Parser::getTargetValue(parsedCommand, cli::nvmcli::TARGET_POOL.name);
    try
    {
    	std::vector<struct pool> allPools = wbem::mem_config::PoolViewFactory::getPoolList(true);
    	// validity of the pool target is verified in the API layer
        if (!poolTarget.empty())
        {
        	poolUid = poolTarget;
        }
        // no pool target was specified.
        else
        {
        	// only 1 pool, then use it
            if (allPools.size() == 1)
            {
            	NVM_UID uidStr;
                uid_copy(allPools[0].pool_uid, uidStr);
            	poolUid = uidStr;
            }
            // more than one pool, the pool target is required
            else
            {
            	pResult = new framework::SyntaxErrorMissingValueResult(
            			framework::TOKENTYPE_TARGET,
						TARGET_POOL.name.c_str());
            }
        }
    }
    catch (wbem::framework::Exception &e)
    {
        pResult = cli::nvmcli::NvmExceptionToResult(e);
    }
    return pResult;
}
