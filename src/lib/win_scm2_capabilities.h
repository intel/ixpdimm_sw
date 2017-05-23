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

#ifndef CR_MGMT_SCM2_CAPABILITIES_H
#define	CR_MGMT_SCM2_CAPABILITIES_H
#ifdef __cplusplus
extern "C"
{
#endif


#define	MAX_NUMBER_OF_BLOCK_SIZES 16


typedef struct _FEATURE_FLAGS
{
	unsigned int GetTopology : 1;
	unsigned int GetInterleave : 1;
	unsigned int GetDimmDetail : 1;
	unsigned int GetNamespaces : 1;
	unsigned int GetNamespaceDetail : 1;
	unsigned int GetPowerData : 1;
	unsigned int CreateNamespace : 1;
	unsigned int RenameNamespace : 1;
	unsigned int DeleteNamespace : 1;
	unsigned int RunDiagnostic : 1;
	unsigned int SendPassthru : 1;
	unsigned int HealthInfoReporting : 1;
	unsigned int AcpiNfitTableUpdateNotification : 1;
	unsigned int AcpiNfitHealthEventNotification : 1;
	unsigned int CommandEffects:1;
	unsigned int PolledSmartDetection: 1;
	unsigned int DeviceStateChanges:1;
	unsigned int BlockNamespace : 1;
	unsigned int PmemNamespace : 1;
	unsigned int EnumerateDimm : 1;
	unsigned int ScmDriverImplementation : 1;
	unsigned int ReservedBits : 11;
	unsigned int Reserved[ 3 ];
} FEATURE_FLAGS;

typedef enum _LOGICAL_BLOCK_SIZE
{
	LOGICAL_BLOCK_SIZE_512 = 512,
	LOGICAL_BLOCK_SIZE_4096 = 4096
} LOGICAL_BLOCK_SIZE;

typedef struct _DRIVER_CAPABILITIES
{
	unsigned long NumBlockSizes;
	LOGICAL_BLOCK_SIZE BlockSizes[MAX_NUMBER_OF_BLOCK_SIZES];
	unsigned long MinNamespaceSize;
	unsigned long long NamespaceAlignment;
	FEATURE_FLAGS SupportedFeatures;
} DRIVER_CAPABILITIES;


int win_scm2_ioctl_get_driver_capabilities(unsigned short nfit_handle,
		DRIVER_CAPABILITIES *p_capabilities);

#ifdef __cplusplus
}
#endif

#endif // CR_MGMT_SCM2_CAPABILITIES_H
