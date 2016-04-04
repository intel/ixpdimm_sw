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
 * This file contains the implementation of the common methods to compress ad
 * encrypt files.
 */

#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <zlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// file I/O
#include <sys/stat.h>
#include <fcntl.h>

#include "encrypt.h"
#include <string/s_str.h>
#include <file_ops/file_ops_adapter.h>
#include <os/os_adapter.h>

#define	PUBLIC_KEY_FILE	"public.rev0.pem"

/*
 * Iterative, core guts of compression contained herein
 */
int raw_compression(int ofd, z_stream *zvar)
{
	ssize_t num_done;
	int rc = COMMON_SUCCESS;
	COMMON_UINT8 output[COMPRESSION_PROCESS_BYTES];
	int flush = (zvar->avail_in == 0) ? Z_FINISH : Z_NO_FLUSH;

	do
	{
		zvar->avail_out = COMPRESSION_PROCESS_BYTES;
		zvar->next_out = output;
		if (deflate(zvar, flush) == Z_STREAM_ERROR)
		{
			rc = COMMON_ERR_BADFILE;
			break;
		}

		num_done = (COMPRESSION_PROCESS_BYTES - zvar->avail_out);
		if (write(ofd, output, num_done) != num_done)
		{
			rc = COMMON_ERR_BADFILE;
			break;
		}
	}
	while (zvar->avail_out == 0);

	return rc;
}

/*
 * Compress the entire contents of 'src_file'(INPUT) into a new file called 'out_file'(OUTPUT)
 */
int compress_file(const COMMON_PATH src_file, COMMON_PATH out_file)
{
	int sfd = -1;		// src_file
	int ofd = -1;		// out file
	int rc = COMMON_SUCCESS;
	char temp_file[COMMON_PATH_LEN];
	z_stream zvar;
#ifdef __WINDOWS__
	int OS_flags = O_BINARY;
#else
	int OS_flags = 0;
#endif

	// Create a new file, verify the resulting name is within our max allowed length
	s_strncpy(temp_file, COMMON_PATH_LEN, src_file, COMMON_PATH_LEN);
	s_strncat(temp_file, COMMON_PATH_LEN, COMPRESS_FILE_EXT, sizeof (COMPRESS_FILE_EXT));
	if (s_strnlen(temp_file, COMMON_PATH_LEN) > COMMON_PATH_LEN)
	{
		rc = COMMON_ERR_BADFILE;
	}
	else
	{
		s_strncpy(out_file, COMMON_PATH_LEN, temp_file, COMMON_PATH_LEN);

		struct stat statbuf;
		if (stat(out_file, &statbuf) != -1)
		{
			unlink(out_file);
		}

		// Prepare compression variable
		zvar.zalloc = Z_NULL;
		zvar.zfree = Z_NULL;
		zvar.opaque = Z_NULL;
		if ((rc = deflateInit(&zvar, DFLT_COMPRESSION_LEVEL)) != Z_OK)
		{
			rc = COMMON_ERR_UNKNOWN;
		}
		else if ((sfd = open(src_file, O_RDWR | OS_flags, 0)) == -1)
		{
			rc = COMMON_ERR_BADFILE;
		}
		else if ((ofd = open(out_file, O_RDWR | O_TRUNC | O_CREAT | O_EXCL | OS_flags,
				GENERIC_NEW_FILE_PERMISSION)) == -1)
		{
			rc = COMMON_ERR_BADFILE;
		}
		else
		{
			ssize_t num_read;
			COMMON_UINT8 input[COMPRESSION_PROCESS_BYTES];

			while ((num_read = read(sfd, input, COMPRESSION_PROCESS_BYTES)) != 0)
			{
				// Assert checks in case someone accidentally and incorrectly decided to
				// change the size of COMPRESSION_PROCESS_BYTES to something larger than uInt
				assert(num_read <= (1ull << (sizeof (uInt) * 8)));
				zvar.avail_in = (uInt)num_read;
				zvar.next_in = input;
				if ((rc = raw_compression(ofd, &zvar)) != COMMON_SUCCESS)
				{
					break;
				}
			}

			// One more time to finalize and flush to output file
			if (rc == COMMON_SUCCESS)
			{
				rc = raw_compression(ofd, &zvar);
				if (zvar.avail_in != 0)
				{
					rc = COMMON_ERR_FAILED;
				}
			}
		}
	}

	if (ofd != -1)
	{
		close(ofd);

		// Delete the corrupted output if we detect a failure
		if (rc != COMMON_SUCCESS)
		{
			delete_file(out_file, COMMON_PATH_LEN);
		}
	}
	if (sfd != -1)
	{
		// src file is being replaced with an compressed version
		close(sfd);
		sfd = -1;
		delete_file(src_file, COMMON_PATH_LEN);
	}

	// release resources back to OS obtained from deflateInit()
	deflateEnd(&zvar);

	return rc;
}

/*
 * Iterative, core guts of compression contained herein
 */
int raw_decompression(int ofd, z_stream *zvar)
{
	int zrc;
	ssize_t num_done;
	int retval = 1;
	COMMON_UINT8 output[COMPRESSION_PROCESS_BYTES];

	do
	{
		zvar->avail_out = COMPRESSION_PROCESS_BYTES;
		zvar->next_out = output;
		zrc = inflate(zvar, Z_NO_FLUSH);

		switch (zrc)
		{
		case Z_STREAM_ERROR:
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			retval = 0;
			break;
		}

		num_done = (COMPRESSION_PROCESS_BYTES - zvar->avail_out);
		if (write(ofd, output, num_done) != num_done)
		{
			// Unable to write decrypted SQL DB file
			retval = 0;
			break;
		}
	}
	while ((zvar->avail_out == 0) && (retval == 1));

	return retval;
}

/*
 * Uncompress the entire contents of 'srcFile'(INPUT) into a new file called 'outFile'(OUTPUT)
 */
int decompress_file(const COMMON_PATH srcFile, COMMON_PATH outFile)
{
	int sfd = -1;		// srcFile
	int ofd = -1;		// outFile
	int retval = 1;
	z_stream zvar;
#ifdef __WINDOWS__
	int OS_flags = O_BINARY;
#else
	int OS_flags = 0;
#endif

	// trim of ".zip" from the output file name
	s_strncpy(outFile, COMMON_PATH_LEN,
			srcFile, strlen(srcFile) - strlen(COMPRESS_FILE_EXT));

	// Prepare compression variable
	zvar.zalloc = Z_NULL;
	zvar.zfree = Z_NULL;
	zvar.opaque = Z_NULL;
	zvar.avail_in = 0;
	zvar.next_in = Z_NULL;
	if (inflateInit(&zvar) != Z_OK)
	{
		retval = 0;
	}
	else if ((sfd = open(srcFile, O_RDWR|OS_flags, 0)) == -1)
	{
		// Unable to open encrypted file
		retval = 0;
	}
	else if ((ofd = open(outFile, O_RDWR|O_TRUNC|O_CREAT|O_EXCL|OS_flags,
			GENERIC_NEW_FILE_PERMISSION)) == -1)
	{
		// Unable to create decrypted file
		retval = 0;
	}
	else
	{
		ssize_t num_read;
		COMMON_UINT8 input[COMPRESSION_PROCESS_BYTES];

		while ((num_read = read(sfd, input, COMPRESSION_PROCESS_BYTES)) != 0)
		{
			// Assert checks in case someone accidentally and incorrectly decided to
			// change the size of COMPRESSION_PROCESS_BYTES to something larger than uInt
			assert(num_read <= (1ull << (sizeof (uInt) * 8)));
			zvar.avail_in = (uInt)num_read;
			zvar.next_in = input;

			if (raw_decompression(ofd, &zvar) != 1)
			{
				break;
			}
		}
	}


	if (ofd != -1)
	{
		close(ofd);
	}
	if (sfd != -1)
	{
		close(sfd);
	}
	inflateEnd(&zvar);	// releases resources back to OS obtained from inflateInit()

	return retval;
}

int get_key_file_path(COMMON_PATH key_file)
{
	// try the local directory first
	s_strcpy(key_file, "./", COMMON_PATH_LEN);
	s_strcat(key_file, COMMON_PATH_LEN, PUBLIC_KEY_FILE);
	if (!file_exists(key_file, COMMON_PATH_LEN))
	{
		// try to find the config file in the install directory
		get_install_dir(key_file);
		s_strcat(key_file, COMMON_PATH_LEN, PUBLIC_KEY_FILE);
	}

	return file_exists(key_file, COMMON_PATH_LEN) ? COMMON_SUCCESS : COMMON_ERR_BADFILE;
}
/*
 * Encrypt 'src_file'(INPUT) using an RSA public key, adds a CRYPTO_FILE_EXT file extension
 */
int rsa_encrypt(const COMMON_PATH src_file, COMMON_PATH out_file)
{
	int sfd = -1;		// src_file
	int efd = -1;		// encrypted file
	int rc = COMMON_SUCCESS;
	RSA *rsa = NULL;
	BIO *bio = NULL;
	char temp_file[COMMON_PATH_LEN];
#ifdef __WINDOWS__
	int OS_flags = O_BINARY;
#else
	int OS_flags = 0;
#endif

	// Create a new file, verify the resulting name is within our max allowed length
	s_strncpy(temp_file, COMMON_PATH_LEN, src_file, COMMON_PATH_LEN);
	s_strncat(temp_file, COMMON_PATH_LEN, CRYPTO_FILE_EXT, sizeof (CRYPTO_FILE_EXT));
	if (s_strnlen(temp_file, COMMON_PATH_LEN) > COMMON_PATH_LEN)
	{
		rc = COMMON_ERR_BADFILE;
	}
	else
	{
		s_strncpy(out_file, COMMON_PATH_LEN, temp_file, COMMON_PATH_LEN);

		// Read the public RSA key into a openssl data structure
		COMMON_PATH key_file;
		if ((rc = get_key_file_path(key_file)) == COMMON_SUCCESS)
		{
			bio = BIO_new_file(key_file, "r");

			struct stat statbuf;
			if (stat(out_file, &statbuf) != -1)
			{
				unlink(out_file);
			}

			// Convert the memory copy PEM format into an openssl data structure
			if (PEM_read_bio_RSA_PUBKEY(bio, &rsa, NULL, NULL) == NULL)
			{
				rc = COMMON_ERR_UNKNOWN;
			}
			else if ((sfd = open(src_file, O_RDWR | OS_flags, 0)) == -1)
			{
				rc = COMMON_ERR_BADFILE;
			}
			else if ((efd = open(out_file, O_RDWR | O_TRUNC | O_CREAT | O_EXCL | OS_flags,
					GENERIC_NEW_FILE_PERMISSION)) == -1)
			{
				rc = COMMON_ERR_BADFILE;
			}
			else
			{
				// Encryption using an RSA key can't encrypt data larger than
				// the key itself by definition. Thus the input file must be
				// chunked up into small pieces and each block of file must be
				// encrypted sequentially to get around this restriction. Further
				// qualification comes from
				// "http://www.openssl.org/docs/crypto/RSA_public_encrypt.html"
				// where it indicates that the limit must be
				// modified by RSA_PKCS1_OAEP_PADDING_OFFSET.
				const int MAX_ENCRYPTION_LENGTH = (RSA_size(rsa) - RSA_PKCS1_OAEP_PADDING_OFFSET);
				COMMON_UINT8 input[MAX_ENCRYPTION_LENGTH];
				COMMON_UINT8 output[RSA_size(rsa)]; // openssl lib API requires larger output buf
				ssize_t num_read;
				int cnvt_bytes;

				while ((num_read = read(sfd, input, MAX_ENCRYPTION_LENGTH)) != 0)
				{
					cnvt_bytes = RSA_public_encrypt(num_read, input, output, rsa,
							RSA_PKCS1_OAEP_PADDING);
					if (cnvt_bytes == -1)
					{
						char err_buf[120];
						ERR_load_crypto_strings();

						ERR_error_string_n(ERR_get_error(), err_buf, sizeof (err_buf));
						rc = COMMON_ERR_UNKNOWN;
						break;
					}
					else if (write(efd, output, cnvt_bytes) != cnvt_bytes)
					{
						rc = COMMON_ERR_BADFILE;
						break;
					}
				}
			}
		}
	}

	if (rsa != NULL)
	{
		RSA_free(rsa);
	}
	if (bio != NULL)
	{
		BIO_free(bio);
	}
	if (efd != -1)
	{
		close(efd);

		// Delete the corrupted output if we detect a failure
		if (rc != COMMON_SUCCESS)
		{
			delete_file(out_file, COMMON_PATH_LEN);
		}
	}
	if (sfd != -1)
	{
		// src file is being replaced with an encrypted version
		close(sfd);
		sfd = -1;
		delete_file(src_file, COMMON_PATH_LEN);
	}

	return rc;
}

/*
 * Decrypt the 'encryptedFile' into the output file 'decryptedFile'
 */
int rsa_decrypt(const COMMON_PATH rsaKeyFile, const COMMON_PATH encryptedFile,
		const COMMON_PATH decryptedFile)
{
	int efd = -1;		// encrypted file
	int dfd = -1;		// decrypted file
	RSA *rsa = NULL;
	BIO *bio = NULL;
	int retval = 1;
#ifdef __WINDOWS__
	int OS_flags = O_BINARY;
#else
	int OS_flags = 0;
#endif

	// Read the private RSA key into a openssl data structure
	bio = BIO_new_file(rsaKeyFile, "r");
	if (bio == NULL)
	{
		// Unable to read private certificate file contents
		retval = 0;
	}
	// Convert the memory copy PEM format into an openssl data structure
	else if (PEM_read_bio_RSAPrivateKey(bio, &rsa, NULL, NULL) == NULL)
	{
		// Unable to convert security data
		retval = 0;
	}
	else if ((efd = open(encryptedFile, O_RDWR|OS_flags, 0)) == -1)
	{
		// Unable to open encrypted file
		retval = 0;
	}
	else if ((dfd = open(decryptedFile, O_RDWR|O_TRUNC|O_CREAT|O_EXCL|OS_flags,
		GENERIC_NEW_FILE_PERMISSION)) == -1)
	{
		// Unable to create decrypted file
		retval = 0;
	}
	else
	{
		// Encryption using an RSA key can't encrypt data larger than the key itself by definition.
		// Thus the input file must be chunked up into small pieces and each block of file must be
		// encrypted sequentially to get around this restriction. Further qualification comes from
		// "http://www.openssl.org/docs/crypto/RSA_public_encrypt.html" where it indicates that the
		// limit must be modified by RSA_PKCS1_OAEP_PADDING_OFFSET.
		const int MAX_ENCRYPTION_LENGTH = (RSA_size(rsa) - RSA_PKCS1_OAEP_PADDING_OFFSET);
		COMMON_UINT8 output[MAX_ENCRYPTION_LENGTH];
		COMMON_UINT8 input[RSA_size(rsa)];
		ssize_t num_read;
		int cnvt_bytes;

		while ((num_read = read(efd, input, sizeof (input))) != 0)
		{
			cnvt_bytes = RSA_private_decrypt(num_read, input, output, rsa,
			RSA_PKCS1_OAEP_PADDING);
			if (cnvt_bytes == -1)
			{
				// Unable to decrypt SQL DB file
				retval = 0;
				break;
			}
			else if (write(dfd, output, cnvt_bytes) != cnvt_bytes)
			{
				// Unable to write decrypted SQL DB file
				retval = 0;
				break;
			}
		}
	}

	if (rsa != NULL)
	{
		RSA_free(rsa);
	}
	if (bio != NULL)
	{
		BIO_free(bio);
	}
	if (efd != -1)
	{
		close(efd);
	}
	if (dfd != -1)
	{
		close(dfd);
	}

	return retval;
}
