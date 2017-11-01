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

/*
 * This file defines an interface to retrieve parsed information from the
 * NFIT tables
 */

#ifndef _NFIT_INTERFACE_NFIT_H_
#define	_NFIT_INTERFACE_NFIT_H_

#include "nfit_tables.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NFIT_MAX_DIMMS	12
#define	MAX_REGIONS_PER_ISET	64

/*
 * NFIT parsing errors
 */
enum nfit_error
{
	NFIT_SUCCESS = 0,
	NFIT_ERR_TABLENOTFOUND = -1,
	NFIT_ERR_CHECKSUMFAIL = -2,
	NFIT_ERR_BADNFIT = -3,
	NFIT_ERR_NOMEMORY = -4,
	NFIT_ERR_BADINPUT = -5
};

/*
 * NFIT NVDIMM
 */
#define NFIT_MAX_IFC_COUNT	9
struct nfit_dimm
{
	unsigned int handle;
	unsigned short physical_id;
	unsigned int serial_number;
	unsigned short vendor_id;
	unsigned short device_id;
	unsigned short revision_id;
	unsigned short subsystem_vendor_id;
	unsigned short subsystem_device_id;
	unsigned short subsystem_revision_id;
	unsigned char valid_fields;
	unsigned char manufacturing_location;
	unsigned short manufacturing_date;
	unsigned short state_flags;
	unsigned short ifc[NFIT_MAX_IFC_COUNT];
};

/*
 * NFIT Interleave set
 */
struct nfit_interleave_set
{
	unsigned short id; // SPA range index
	unsigned int proximity_domain;
	unsigned long long address; // spa range base address
	unsigned long long size; // total interleave set size
	unsigned long long attributes; // memory mapping attributes
	int dimm_count; // number of dimms in the interleave set
	unsigned int dimms[NFIT_MAX_DIMMS]; // array of DIMM handles
	unsigned long long dimm_region_pdas[NFIT_MAX_DIMMS]; // address base for each DIMM
	unsigned long long dimm_region_offsets[NFIT_MAX_DIMMS]; // address offset for each DIMM
	unsigned long long dimm_sizes[NFIT_MAX_DIMMS]; // region size for each DIMM
};

/*
 * Print a descriptive version of an NFIT error
 */
void nfit_print_error(const int err);


/*
 * Retrieve the NFIT table from ACPI and parse it.
 * The caller is responsible for freeing the parsed_nfit structure.
 */
int nfit_get_parsed_nfit(struct parsed_nfit **pp_parsed_nfit);

/*
 * Clean up a parsed nfit structure
 */
void free_parsed_nfit(struct parsed_nfit *p_parsed_nfit);

/*
 * Parse raw NFIT data into a parsed_nfit structure.
 * The caller is responsible for freeing the parsed_nfit structure.
 */
int nfit_parse_raw_nfit(unsigned char *buffer, size_t buffer_size,
	struct parsed_nfit** pp_parsed_nfit);

/*
 * Print a parsed_nfit structure
 */
void nfit_print_parsed_nfit(const struct parsed_nfit *p_nfit);


/*
 * Retrieves a list of DIMMs from the NFIT and returns the count.
 * If input count is 0, simply retrieve the count from the NFIT.
 */
int nfit_get_dimms(const int count, struct nfit_dimm *p_nfit_dimms);

/*
 * Retrieves a list of DIMMs from a pre-parsed NFIT and returns the count.
 * If input count is 0, simply retrieve the count from the NFIT.
 */
int nfit_get_dimms_from_parsed_nfit(const int count,
		struct nfit_dimm *p_nfit_dimms,
		const struct parsed_nfit *p_parsed_nfit);

/*
 * Print an NFIT NVDIMM
 */
void nfit_print_dimm(const struct nfit_dimm *p_dimm);


/*
 * Retrieves a list of interleave sets from the NFIT and returns the count.
 * If input count is 0, simply retrieve the count from the NFIT.
 */
int nfit_get_interleave_sets(const int count, struct nfit_interleave_set *p_interleave_sets);

/*
 * Retrieves a list of interleave sets from a pre-parsed NFIT and returns the count.
 * If input count is 0, simply retrieve the count from the NFIT.
 */
int nfit_get_interleave_sets_from_parsed_nfit(const int count,
		struct nfit_interleave_set *p_interleave_sets,
		const struct parsed_nfit *p_parsed_nfit);

/*
 * Print an NFIT interleave set
 */
void nfit_print_interleave_set(const struct nfit_interleave_set *p_interleave);

#ifdef __cplusplus
}
#endif

#endif /* _NFIT_INTERFACE_NFIT_H_ */
