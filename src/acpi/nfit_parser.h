/*
 * Copyright (c) 2017, Intel Corporation
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
#ifndef _NFIT_INTERFACE_NFIT_PARSER_H_
#define _NFIT_INTERFACE_NFIT_PARSER_H_

#include "nfit.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Allocate space for and copy the spa table
 * to the parsed_nfit structure.
 */
int add_spa_to_parsed_nfit(
	struct parsed_nfit* p_nfit, 
	const struct spa *p_spa_table);
/*
 * Allocate space for and copy the region_mapping table
 * to the parsed_nfit structure.
 */
int add_region_mapping_to_parsed_nfit(
	struct parsed_nfit* p_nfit, 
	const struct region_mapping *p_region_mapping_table);
/*
 * Allocate space for and copy the interleave table
 * to the parsed_nfit structure.
 */
int add_interleave_to_parsed_nfit(
	struct parsed_nfit* p_nfit, 
	const struct interleave *p_interleave_table);
/*
 * Allocate space for and copy the smbios_management_info table
 * to the parsed_nfit structure.
 */
int add_smbios_management_info_to_parsed_nfit(
	struct parsed_nfit* p_nfit, 
	const struct smbios_management_info *p_smbios_management_info_table);
/*
 * Allocate space for and copy the control_region table
 * to the parsed_nfit structure.
 */
int add_control_region_to_parsed_nfit(
	struct parsed_nfit* p_nfit, 
	const struct control_region *p_control_region_table);
/*
 * Allocate space for and copy the block_data_window_region table
 * to the parsed_nfit structure.
 */
int add_block_data_window_region_to_parsed_nfit(
	struct parsed_nfit* p_nfit, 
	const struct block_data_window_region *p_block_data_window_region_table);
/*
 * Allocate space for and copy the flush_hint_address table
 * to the parsed_nfit structure.
 */
int add_flush_hint_address_to_parsed_nfit(
	struct parsed_nfit* p_nfit, 
	const struct flush_hint_address *p_flush_hint_address_table);
#ifdef __cplusplus
}
#endif

#endif //_NFIT_INTERFACE_NFIT_PRINTER_H_