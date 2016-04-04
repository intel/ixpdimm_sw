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
 * This file contains the definition of common functions to retrieve system
 * processor information.
 */

#ifndef	_SYSTEM_H_
#define	_SYSTEM_H_

#include <stdio.h>
#include <common_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * ****************************************************************************
 * STRUCTS
 * ****************************************************************************
 */

/*!
 * A structure used to contain information about a given physical processor
 */
typedef struct
{
	COMMON_UINT8 type; //!< CPUID processor type enumeration
	COMMON_UINT8 family; //!< CPUID processor family enumeration
	COMMON_UINT8 model; //!< CPUID processor model enumeration
	COMMON_UINT8 stepping; //!< CPUID processor stepping enumeration
	COMMON_UINT8 brand_index; //!< CPUID processor brand_index
} processor_info;

/*
 * ***************************************************************
 * FUNCTIONS - specific to the CPU running the process
 * ***************************************************************
 */

/*!
 * Retrieves the processor brand string.
 * @param brand
 * 		A caller supplied buffer to hold the processor brand string
 * @param brand_len
 * 		The length of the buffer.  Should be at least 48 characters.
 * @return
 * 		COMMON_SUCCESS
 * 		COMMON_ERR_INVALIDPARAMETER
 * 		COMMON_ERR_UNKNOWN
 */
extern int get_processor_brand(char *brand, COMMON_SIZE brand_len);

/*!
 * Retrieves the processor manufacturer string.
 * @param manufacturer
 * 		A caller supplied buffer to hold the processor manufacturer string
 * @param manufacturer_len
 * 		The length of the buffer.  Should be at least 12 characters.
 * @return
 * 		COMMON_SUCCESS
 * 		COMMON_ERR_INVALIDPARAMETER
 * 		COMMON_ERR_UNKNOWN
 */
extern int get_processor_manufacturer(char *manufacturer, COMMON_SIZE manufacturer_len);

/*!
 * Retrieve the processor manufacturer information.
 * @param p_info
 *		A caller supplied buffer of the type @c processor_info.
 * @return
 * 		COMMON_SUCCESS
 * 		COMMON_ERR_INVALIDPARAMETER
 * 		COMMON_ERR_UNKNOWN
 */
extern int get_processor_info(processor_info *p_info);

#ifdef __cplusplus
}
#endif

#endif  /* _OS_ADAPTER_H_ */
