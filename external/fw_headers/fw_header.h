/*!
 * Copyright 2015 Intel Corporation All Rights Reserved.
 *
 * INTEL CONFIDENTIAL
 *
 * The source code contained or described herein and all documents related to the
 * source code ("Material") are owned by Intel Corporation or its suppliers or
 * licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material may contain trade secrets and proprietary
 * and confidential information of Intel Corporation and its suppliers and licensors,
 * and is protected by worldwide copyright and trade secret laws and treaty
 * provisions. No part of the Material may be used, copied, reproduced, modified,
 * published, uploaded, posted, transmitted, distributed, or disclosed in any way
 * without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be express
 * and approved by Intel in writing.
 *
 * Unless otherwise agreed by Intel in writing, you may not remove or alter this
 * notice or any other notice embedded in Materials by Intel or Intel's suppliers
 * or licensors in any way.
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
