/*
 * Copyright (c) 2018, Intel Corporation
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <common_types.h>
#include "ixp.h"
#include "ixp_prv.h"


int ixp_create_ctx_nfit_handle(struct ixp_context **ctx, const NVM_NFIT_DEVICE_HANDLE handle, void *user_data)
{
	if (NULL == ctx)
		return IXP_NULL_INPUT_PARAM;

	if (NULL == (*ctx = (struct ixp_context *)malloc(sizeof(struct ixp_context))))
		return IXP_NO_MEM_RESOURCES;

	struct ixp_context *pctx = (struct ixp_context*)*ctx;
	pctx->handle = handle;
	pctx->user_data = user_data;
	return IXP_SUCCESS;
}

int ixp_create_ctx_uid(struct ixp_context **ctx, const NVM_UID uid, void *user_data)
{
	if (!ctx || !uid || !user_data)
	{
		return IXP_NULL_INPUT_PARAM;
	}
	return IXP_NOT_YET_IMPLEMENTED;
}

int ixp_get_ctx_user_data(struct ixp_context *ctx, void **user_data)
{
	if (ctx && user_data)
	{
		*user_data = ctx->user_data;
		return IXP_SUCCESS;
	}
	else return IXP_NULL_INPUT_PARAM;
}

int ixp_free_ctx(struct ixp_context *ctx)
{
	if (!ctx)
	{
		return IXP_NULL_INPUT_PARAM;
	}
	else
	{
		free(ctx);
	}
	return IXP_SUCCESS;
}

