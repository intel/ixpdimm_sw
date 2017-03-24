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
#ifndef _NFIT_INTERFACE_NFIT_PRINTER_H_
#define _NFIT_INTERFACE_NFIT_PRINTER_H_

#include "nfit.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Print an NFIT nfit extension table
 */
void print_nfit_table(
	const struct nfit table, 
	const int indent_count);
/*
 * Print an NFIT spa extension table
 */
void print_spa_table(
	const struct spa table, 
	const int indent_count);
/*
 * Print an NFIT region_mapping extension table
 */
void print_region_mapping_table(
	const struct region_mapping table, 
	const int indent_count);
/*
 * Print an NFIT interleave extension table
 */
void print_interleave_table(
	const struct interleave table, 
	const int indent_count);
/*
 * Print an NFIT smbios_management_info extension table
 */
void print_smbios_management_info_table(
	const struct smbios_management_info table, 
	const int indent_count);
/*
 * Print an NFIT control_region extension table
 */
void print_control_region_table(
	const struct control_region table, 
	const int indent_count);
/*
 * Print an NFIT block_data_window_region extension table
 */
void print_block_data_window_region_table(
	const struct block_data_window_region table, 
	const int indent_count);
/*
 * Print an NFIT flush_hint_address extension table
 */
void print_flush_hint_address_table(
	const struct flush_hint_address table, 
	const int indent_count);
#ifdef __cplusplus
}
#endif

#endif //_NFIT_INTERFACE_NFIT_PRINTER_H_