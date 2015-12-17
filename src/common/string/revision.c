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
 * This file contains the implementation of common revision helper functions.
 */

#include <stdlib.h>
#include <string/s_str.h>

#include "revision.h"


/*
 * Format a revision into the correct format
 */
int format_revision(const char *rev, size_t rev_len, char *fmt_rev, size_t fmt_rev_len)
{
	int valid_rev = 0;
	if (rev && (rev_len != 0) && fmt_rev && (fmt_rev_len != 0))
	{
		// get the parts
		unsigned short int major, minor, hotfix, build;
		valid_rev = parse_main_revision(&major, &minor, &hotfix, &build, rev, rev_len);

		// now format it
		build_revision(fmt_rev, fmt_rev_len, major, minor, hotfix, build);
	}

	return valid_rev;
}

/*
 * Format a revision into the correct format
 */
int format_fw_revision(const char *p_firmware_rev, size_t fw_rev_len, char *fmt_rev,
		size_t fmt_rev_len)
{
	int valid_rev = 0;
	if (p_firmware_rev && (fw_rev_len != 0) && fmt_rev && (fmt_rev_len != 0))
	{
		// get the parts
		unsigned short int major, minor;
		valid_rev = parse_fw_revision(&major, &minor, p_firmware_rev, fw_rev_len);

		// now format it
		build_fw_revision(fmt_rev, fmt_rev_len, major, minor);
	}

	return valid_rev;
}

/*
 * Utility method to create version string from parts
 */
void build_revision(char *revision, size_t revision_len,
		unsigned short int major, unsigned short int minor,
		unsigned short int hotfix, unsigned short int build)
{
	if (revision && (revision_len != 0))
	{
		// dynamically build revision string format
		char rev_format_str[64];
		rev_format_str[0] = '\0';
		char num_digit_buf[4];


		// Major Revision
		s_strcat(rev_format_str, 64, "%0");
		snprintf(num_digit_buf, 4, "%d", COMMON_MAJOR_REVISION_LEN);
		s_strcat(rev_format_str, 64, num_digit_buf);
		s_strcat(rev_format_str, 64, "hd");

		// Minor Revision
		s_strcat(rev_format_str, 64, ".%0");
		snprintf(num_digit_buf, 4, "%d", COMMON_MINOR_REVISION_LEN);
		s_strcat(rev_format_str, 64, num_digit_buf);
		s_strcat(rev_format_str, 64, "hd");

		// Hotfix Revision
		s_strcat(rev_format_str, 64, ".%0");
		snprintf(num_digit_buf, 4, "%d", COMMON_HOTFIX_REVISION_LEN);
		s_strcat(rev_format_str, 64, num_digit_buf);
		s_strcat(rev_format_str, 64, "hd");

		// Build Revision
		s_strcat(rev_format_str, 64, ".%0");
		snprintf(num_digit_buf, 4, "%d", COMMON_BUILD_REVISION_LEN);
		s_strcat(rev_format_str, 64, num_digit_buf);
		s_strcat(rev_format_str, 64, "hd");
		s_snprintf(revision, revision_len, rev_format_str, major, minor, hotfix, build);
	}
}

/*
 * Utility method to create a fw revision from parts
 */
void build_fw_revision(char *fw_revision, size_t fw_revision_len,
		unsigned short int major, unsigned short int minor)
{
	if (fw_revision && (fw_revision_len != 0))
	{
		snprintf(fw_revision, fw_revision_len, "%hd.%hd", major, minor);
	}
}


int parse_revision(unsigned short int **pp_parts, int parts_count,
		const char * const revision, size_t revision_len)
{
	// no checking of input parameters for this helper function

	// initialize end pointer to the first char in revision
	const char *p_end = &(revision[0]);

	// iterate through the revision string, grabbing the number of parts specified,
	// or until the defined end of the revision string is reached
	int i = 0;
	size_t revision_len_left = revision_len;
	for (; (i < parts_count) && (revision_len_left > 0) && (p_end != NULL) && (*p_end != '\0'); i++)
	{
		revision_len_left -= s_strtous(p_end, revision_len_left, &p_end, pp_parts[i]);
	}

	// returns true only if we grabbed the expected number of parts
	return (i == parts_count);
}

/*
 * truncate an unsigned short int to be a certain length
 */
void truncate_us(unsigned short int *p_value, unsigned short int count)
{
	// no checking of input parameters for this helper function

	while (*p_value >= pow(10, count))
	{
		*p_value = *p_value / 10;
	}
}

/*
 * Parse a version string into it's appropriate parts (major, minor, and build numbers)
 */
int parse_main_revision(unsigned short int *p_major, unsigned short int *p_minor,
	unsigned short int *p_hotfix, unsigned short int *p_build,
	const char * const revision, size_t revision_len)
{
	int success = 0;
	if (p_major && p_minor && p_hotfix && p_build && revision && (revision_len != 0))
	{
		*p_major = *p_minor = *p_hotfix = *p_build = 0;
		unsigned short int *parts[4] = { p_major, p_minor, p_hotfix, p_build };
		success = parse_revision(parts, 4, revision, revision_len);

		truncate_us(p_major, COMMON_MAJOR_REVISION_LEN);
		truncate_us(p_minor, COMMON_MINOR_REVISION_LEN);
		truncate_us(p_hotfix, COMMON_HOTFIX_REVISION_LEN);
		truncate_us(p_build, COMMON_BUILD_REVISION_LEN);
	}

	return success;
}

/*
 * parse a firmware version string into it's appropriate parts (major & minor numbers)
 */
int parse_fw_revision(unsigned short int *p_major, unsigned short int *p_minor,
		const char *fw_revision, size_t revision_len)
{
	int success = 0;
	if (p_major && p_minor && fw_revision && (revision_len != 0))
	{
		// make sure no garbage in case no match
		*p_major = *p_minor = 0;
		unsigned short int *parts[2] = { p_major, p_minor };
		success = parse_revision(parts, 2, fw_revision, revision_len);
	}

	return success;
}
