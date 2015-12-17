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
 * This file contains the definition of a set of utilities
 * to indicate and observe time.
 */


#ifndef _TIME_UTILITIES_H_
#define	_TIME_UTILITIES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <common_types.h>

#define	SECONDSPERMINUTE 60
#define	MINUTESPERHOUR 60
#define	HOURSPERDAY 24
#define DATETIME_DECIMALPOINT_POSITION 14
#define DATETIME_SIGN_POSITION 21
#define DATETIME_NULLTERMINATOR_POSITION 25
#define DATETIME_INTERVAL_DAYS_LEN 8
#define DATETIME_INTERVAL_HOURS_LEN 2
#define DATETIME_INTERVAL_MINUTES_LEN 2
#define DATETIME_INTERVAL_SECONDS_LEN 2
#define DATETIME_YEAR_LEN 4
#define DATETIME_MONTH_LEN 2
#define DATETIME_DAY_LEN 2
#define DATETIME_HOUR_LEN 2
#define DATETIME_MINUTE_LEN 2
#define DATETIME_SECOND_LEN 2
#define DATETIME_OFFSET_LEN 3

/*!
 * Possible datetime types
 */
enum datetime_type
{
	DATETIME_TYPE_UNKNOWN,
	DATETIME_TYPE_DATETIME,
	DATETIME_TYPE_INTERVAL
};

/*!
 * Gets the current time in ms.
 * @param[out] nvm_time
 *		A pointer to the time
 */
extern void get_current_time_msec(unsigned long long *nvm_time);

/*!
 * Get the current time string formatted as @b yyyyMMddHHmmss.mmmmmmsutc
 * @remarks
 * 		@b yyyy - 4 digit year @n
 * 		@b MM - month @n
 * 		@b dd - day of the month @n
 * 		@b HH - the hour (24 hour clock) @n
 * 		@b mm - the minute @n
 * 		@b ss - the second @n
 * 		@b mmmmmm - the number of microseconds @n
 * 		@b s - "+" or "-", indicating a positive or negative offset from UTC @n
 * 		@b utc - the offset from UTC in minutes (using the sign indicated by s)
 * @note
 * 		Caller should allocate a @b char array of 26 bytes to obtain the full datetime string
 * @param[out] time_str
 *		Pointer to buffer where datetime string is output
 * @param[in] buf_size
 *		Number of bytes allocated to @c time_str_buf
 */
extern void get_current_datetime(COMMON_DATETIME_STR time_str, size_t buf_size);

/*!
 * Function used to convert a number of microseconds to a formatted string as defined by CIM
 * @remarks
 * 		format = ddddddddHHMMSS.mmmmmm:000
 * 		dddddddd- Eight digits that represent a number of days (00000000 through 99999999)
 * 		HH      - Two digit number of hours (00 through 23)
 * 		MM      - Two digit number of minutes (00 through 59)
 * 		SS      - Two digit number of seconds (00 through 59)
 * 		mmmmmm  - Six-digits that represent the number of microseconds (000000 through 999999)
 * @note
 * 		Intervals always have a trailing ":000" as the last four characters.
 * 		Further, unlike date and time you cannot use asterisks to indicate
 * 		unused fields. Further, for WMI, all properties of type CIM_DATETIME that
 * 		represent intervals must be marked with the SubType standard qualifier,
 * 		with the qualifier set to "interval".
 * 		CMPI does not require the SubType property.
 * 		Caller should allocate a @b array of 26 bytes to obtain the full datetime
 * 		string.
 * 		The microseconds portion of the string is always 0s
 * 	@param[in] seconds
 * 		The size of the interval in seconds
 * 	@param[out] datetime_buf
 * 		Pointer to buffer where datetime string is output.
 */
extern void convert_seconds_to_datetime_interval(const unsigned long long seconds,
							COMMON_DATETIME_STR datetime_buf);

/*!
 * Function used to convert from the time_t type to a formatted date string as defined by CIM
 * @remarks
 * 		@b yyyy - 4 digit year @n
 * 		@b MM - month @n
 * 		@b dd - day of the month @n
 * 		@b HH - the hour (24 hour clock) @n
 * 		@b mm - the minute @n
 * 		@b ss - the second @n
 * 		@b mmmmmm - the number of microseconds @n
 * 		@b s - "+" or "-", indicating a positive or negative offset from UTC @n
 * 		@b utc - the offset from UTC in minutes (using the sign indicated by s) @n
 * @note
 * 		Caller should allocate a @b char array of 26 bytes to obtain the full datetime string
 * @param[in] raw_time
 *		The time value to convert into a datetime string.
 * @param[out] datetime_buf
 *		Pointer to buffer where datetime string is output
 */
extern void convert_seconds_to_datetime(const time_t raw_time,
						COMMON_DATETIME_STR datetime_buf);
/*!
 * Function used to convert from a formatted date string as defined by CIM to a number of
 * seconds after the epoch
 * @remarks
 * 		@b yyyy - 4 digit year @n
 * 		@b MM - month @n
 * 		@b dd - day of the month @n
 * 		@b HH - the hour (24 hour clock) @n
 * 		@b mm - the minute @n
 * 		@b ss - the second @n
 * 		@b mmmmmm - the number of microseconds @n
 * 		@b s - "+" or "-", indicating a positive or negative offset from UTC @n
 * 		@b utc - the offset from UTC in minutes (using the sign indicated by s) @n
 * @note
 * 		Caller should allocate a @b char array of 26 bytes to obtain the full datetime string
 * @param[in] datetime_buf
 *		Pointer to buffer where datetime string is output
 * @param[out] secs
 *		The number of seconds after the epoch.
 * @return the datetime type found
 */
extern enum datetime_type convert_datetime_string_to_seconds(const COMMON_DATETIME_STR datetime,
														unsigned long long *secs);
/*!
 * Function used to extract the timezone differential from a raw time_t assumed to be local
 * @param[in] raw_time
 * 		The local time.
 * @param[out] timezone_diff_min
 * 		Pointer to the difference (in minutes) between the given time, and UTC.
 */
extern void get_timezone_diff(time_t raw_time, int *timezone_diff_min);

/*!
 * Helper function used to get the time zone differential (tzd), in minutes,
 * between two coordinated time zones.
 * @param time_a
 *
 * @param time_b
 * @return
 */
int get_tm_tzd_mins(struct tm time_a, struct tm time_b);

#ifdef __cplusplus
}
#endif

#endif /* _TIME_UTILITIES_H_ */
