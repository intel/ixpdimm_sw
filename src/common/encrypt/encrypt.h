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
 * This file contains the definition of the common methods to compress ad
 * encrypt files.
 */

#ifndef	_NVMD_ENCRYPT_H_
#define	_NVMD_ENCRYPT_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <common_types.h>

#include <sys/stat.h>
#include <sys/types.h>

/*
 * ************************************************************************************
 * Compression
 * ************************************************************************************
 */

/*!
 * 128KB; must be < sizeof (uInt)
 */
#define	COMPRESSION_PROCESS_BYTES			(128 * 1024)

/*!
 * Good average for speed and compression factor
 */
#define	DFLT_COMPRESSION_LEVEL				6

/*!
 * The file extension to be used for a compressed file
 */
#define	COMPRESS_FILE_EXT					".compress"

/*
 * ************************************************************************************
 * Encryption
 * ************************************************************************************
 */

/*!
 * The file extension to be used for an encrypted file
 */
#define	CRYPTO_FILE_EXT						".crypto"

/*!
 * The padding mode to be used for encryption.
 * @see
 * 		http://www.openssl.org/docs/crypto/RSA_public_encrypt.html
 */
#define	RSA_PKCS1_OAEP_PADDING_OFFSET		42

#ifdef __WINDOWS__
/*!
 * The set of file permissions allowed for newly created files
 * @remarks
 * 		Windows has no concept of group and other, thus no definitions
 * 		exist for those constants.
 */
#define	GENERIC_NEW_FILE_PERMISSION			S_IRUSR|S_IWUSR
#else
/*!
 * The set of file permissions allowed for newly created files
 */
#define	GENERIC_NEW_FILE_PERMISSION			S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
#endif

/*!
 * Generate a compressed file from the contents of an input file.
 * @param[in] src_file
 * 		The input filepath
 * @param[in] out_file
 * 		The (compressed) output filepath
 * @return
 * 		@c COMMON_SUCCESS @n
 * 		@c COMMON_ERR_BADFILE @n
 * 		@c COMMON_ERR_UNKNOWN @n
 * 		@c COMMON_ERR_FAILED
 */
extern int compress_file(const COMMON_PATH src_file, COMMON_PATH out_file);

/*!
 * Decompress a compressed file.
 * @param[in] srcFile
 * 		The (compressed) input filepath
 * @param[in] outFile
 * 		The (decompressed) output filepath
 * @return
 * 		1 if success, 0 if failed
 */
extern int decompress_file(const COMMON_PATH srcFile, COMMON_PATH outFile);

/*!
 * Generate an encrypted file from the contents of an input file, using an RSA public key.
 * @param[in] src_file
 * 		The input filepath
 * @param[in] out_file
 * 		The (encrypted) output filepath
 * @return
 * 		@c COMMON_SUCCESS @n
 * 		@c COMMON_ERR_BADFILE @n
 * 		@c COMMON_ERR_UNKNOWN
 */
extern int rsa_encrypt(const COMMON_PATH src_file, COMMON_PATH out_file);

/*!
 * Decrypt an encrypted file
 * @param[in] rsaKeyFile
 * 		The filepath of a RSA private key file used to decrypt @c encryptedFile
 * @param[in] encryptedFile
 *		The (encrypted) input filepath
 * @param[in] decryptedFile
 * 		The (decrypted) output filepath
 * @return
 * 		1 if success, 0 if failed
 */
extern int rsa_decrypt(const COMMON_PATH rsaKeyFile, const COMMON_PATH encryptedFile,
		const COMMON_PATH decryptedFile);


#ifdef __cplusplus
}
#endif

#endif  /* _NVMD_ENCRYPT_H_ */
