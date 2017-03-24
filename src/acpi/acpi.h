/*
 * Copyright (c) 2016, Intel Corporation
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
 * Common ACPI parsing functions
 */

#ifndef SRC_COMMON_ACPI_ACPI_H_
#define	SRC_COMMON_ACPI_ACPI_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define	ACPI_SIGNATURE_LEN	4
#define	ACPI_OEM_ID_LEN	6
#define	ACPI_OEM_TABLE_ID_LEN	8
#define	ACPI_CHECKSUM_OFFSET	9
#define ACPI_FW_TABLE_SIGNATURE	"ACPI"

/*
 * ACPI errors
 */
enum acpi_error
{
	ACPI_SUCCESS = 0,
	ACPI_ERR_BADINPUT = -1,
	ACPI_ERR_CHECKSUMFAIL = -2,
	ACPI_ERR_BADTABLE = -3,
	ACPI_ERR_BADTABLESIGNATURE = -4,
	ACPI_ERR_TABLENOTFOUND = -5
};
/*
 * ACPI Table Header
 */
struct acpi_table_header
{
	char signature[ACPI_SIGNATURE_LEN];
	unsigned int length; /* Length in bytes for entire table */
	unsigned char revision;
	unsigned char checksum; /* Must sum to zero */
	char oem_id[ACPI_OEM_ID_LEN];
	char oem_table_id[ACPI_OEM_TABLE_ID_LEN];
	unsigned int oem_revision;
	unsigned int creator_id;
	unsigned int creator_revision;
	unsigned char reserved[4];
}__attribute__((packed));

/*
 * ACPI Table
 */
struct acpi_table
{
	struct acpi_table_header header;
	/* Variable extension Tables */
	unsigned char p_ext_tables[0];
}__attribute__((packed));

/*!
 * Retrieve the specified ACPI table.
 * If p_table is NULL, return the size of the table.
 */
int get_acpi_table(
		const char *signature,
		struct acpi_table *p_table,
		const unsigned int size);

/*!
 * Verify the ACPI table size, checksum and signature
 */
int check_acpi_table(const char *signature,
		struct acpi_table *p_table);

/*!
 * Helper function to print an ACPI table header
 */
void print_acpi_table_header(
		struct acpi_table_header *p_table);

#ifdef __cplusplus
}
#endif

#endif /* SRC_COMMON_ACPI_ACPI_H_ */
