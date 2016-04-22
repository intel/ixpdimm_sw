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
 * This file contains diagnostic helper functions for the native API.
 */

#ifndef DIAGNOSTIC_H_
#define	DIAGNOSTIC_H_

#include "nvm_management.h"

/*
 * Used in diagnostic helper functions to indicate relationship between
 * diagnostic value and threshold
 */
enum equality
{
	EQUALITY_LESSTHAN,
	EQUALITY_LESSTHANEQUAL,
	EQUALITY_EQUAL,
	EQUALITY_GREATERTHANEQUAL,
	EQUALITY_GREATHERTHAN,
	EQUALITY_NOTEQUAL
};

/*
 * Compare unsigned integer values as part of a diagnostic test.
 */
NVM_BOOL diag_check(const struct diagnostic *p_diagnostic,
		const NVM_UINT32 indicator, const NVM_UINT64 actual,
		NVM_UINT64 *p_expected, enum equality e);

/*
 * Compare signed integer values as part of a diagnostic test.
 */
NVM_BOOL diag_check_real(const struct diagnostic *p_diagnostic,
		const NVM_UINT32 indicator, const NVM_REAL32 actual,
		NVM_REAL32 *p_expected, enum equality e);

/*
 * Compare string values as part of a diagnostic test
 */
NVM_BOOL diag_check_str(const struct diagnostic *p_diagnostic,
		const NVM_UINT32 indicator, const char *actual,
		char *expected);

/*
 * Clear existing diagnostic results for the specified diagnostic
 * and optionally for the specified device.
 */
void diag_clear_results(const enum diagnostic_test type,
		const NVM_BOOL clear_specific_device, const NVM_UID device_uid);

/*
 * Available diagnostics
 */
int diag_quick_health_check(const NVM_UID device_uid,
		const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results);
int diag_security_check(const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results);
int diag_firmware_check(const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results);
int diag_platform_config_check(const struct diagnostic *p_diagnostic, NVM_UINT32 *p_results);
int diag_pm_metadata_check(NVM_UINT32 *p_results);

/*
 * Sub-diagnostics for Platform Config Diagnostic
 */
int verify_nfit(int *p_dev_count, NVM_UINT32 *p_results);
int check_platform_config_best_practices(NVM_UINT32 *p_results);

#endif /* DIAGNOSTIC_H_ */
