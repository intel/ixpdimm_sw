/*
 * Copyright (c) 2015, Intel Corporation
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
 * This file contains the FW image header structures. It comes from the FW team and should
 * stay aligned with their version.
 */

#ifndef	_FW_HEADER_H_
#define	_FW_HEADER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define	FW_HEADER_MODULEVENDOR 0x8086
#define FW_HEADER_MODULETYPE 6

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

/* Version Struct Definition */
typedef union
{
	struct _nibbles1
	{
		u8 digit2:4;
		u8 digit1:4;
	} nibble;
	u8 version;
} versionNibbles;

typedef union
{
	struct _nibbles2
	{
		u16 digit4:4;
		u16 digit3:4;
		u16 digit2:4;
		u16 digit1:4;
	} nibble;
	u16 build;
} buildNibbles;

typedef struct
{
	buildNibbles   buildVer;
	versionNibbles hotfixVer;
	versionNibbles minorVer;
	versionNibbles majorVer;
}  __attribute__((packed)) versionStruct;

/* FW Image header: Intel CSS Header (128 bytes) */

		typedef struct
		{
			u32 moduleType; // Required CSS field
			u32 headerLen; // Required CSS field
			u32 headerVersion; // bits [31:16] are major version, bits [15:0] are minor version
			u32 moduleID; // Required CSS field
			u32 moduleVendor; // moduleVendor = 0x00008086
			u32 date; // BCD format: yyyymmdd
			u32 size; // Size of entire module (header, crypto, data) in DWORDs

			u8 reserved0[12];

			u8 ImageType;
			versionStruct imageVersion;

			u8 reserved1[12];

			u8 fwApiVersion; // Same API as in ID DIMM

			u8 reserved[69];
		} __attribute__((packed)) fwImageHeader;


#ifdef __cplusplus
}
#endif

#endif /* _FW_HEADER_H_ */
