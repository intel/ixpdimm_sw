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
 * This file contains the implementation of a set of utilities
 * to indicate and observe time.
 */

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include "time_utilities.h"
#include <string/s_str.h>

#define	TEMP_TIMESTR_LEN 15

/*
 * Gets the current time in msecs, since January 1, 1970 @ UTC
 */
void get_current_time_msec(unsigned long long *nvm_time)
{
	time_t seconds_since_epoch = time(NULL);
	*nvm_time = (seconds_since_epoch);
	*nvm_time *= 1000;
}

/*
 * Get the current time as a formatted datetime string
 */
void get_current_datetime(COMMON_DATETIME_STR time_str, size_t buf_size)
{
	// get the current time in seconds since the epoch
	time_t seconds_since_epoch = time(NULL);

	// convert to formatted datetime string, w.r.t. UTC
	convert_seconds_to_datetime(seconds_since_epoch, time_str);
}

/*
 * Return the number of seconds represented by the given datetime interval string
 * This function assumes a well-formed datetime string
 */
unsigned long long return_datetime_interval_seconds(const COMMON_DATETIME_STR datetime)
{
	// largest buffer we'll need, plus one for the null terminator
	char buf[DATETIME_INTERVAL_DAYS_LEN + 1];
	const char *p_datetime_str = datetime;
	unsigned long long secs = 0;
	unsigned int length = COMMON_DATETIME_LEN;

	// The datetime string will have the format ddddddddHHMMSS.mmmmmm:000
	// Where dddddddd - eight digit number of days 00000000-99999999
	//		HH - two digit number of hours 00-23
	//		MM - two digit number of minutes 00-59
	//		SS - two digit number of seconds 00-59
	//		mmmmmm = six digit number of microseconds 000000-999999

	// Plus one for the null terminator
	s_strncpy(buf, DATETIME_INTERVAL_DAYS_LEN + 1, p_datetime_str, length);
	secs += atoi(buf) * HOURSPERDAY * MINUTESPERHOUR * SECONDSPERMINUTE;

	p_datetime_str += DATETIME_INTERVAL_DAYS_LEN;
	length -= DATETIME_INTERVAL_DAYS_LEN;
	s_strncpy(buf, DATETIME_INTERVAL_HOURS_LEN + 1, p_datetime_str, length);
	secs += atoi(buf) * MINUTESPERHOUR * SECONDSPERMINUTE;

	p_datetime_str += DATETIME_INTERVAL_HOURS_LEN;
	length -= DATETIME_INTERVAL_HOURS_LEN;
	s_strncpy(buf, DATETIME_INTERVAL_MINUTES_LEN + 1, p_datetime_str, length);
	secs += atoi(buf) * SECONDSPERMINUTE;

	p_datetime_str += DATETIME_INTERVAL_MINUTES_LEN;
	length -= DATETIME_INTERVAL_MINUTES_LEN;
	s_strncpy(buf, DATETIME_INTERVAL_SECONDS_LEN + 1, p_datetime_str, length);
	secs += atoi(buf);
	return secs;
}

/*
 * Return the number of seconds represented by the given datetime string
 * This function assumes a well-formed datetime string
 */
unsigned long long return_datetime_seconds(const COMMON_DATETIME_STR datetime)
{
	// largest buf we'll need, plus one for the null terminator
	char buf[DATETIME_YEAR_LEN + 1];
	const char *p_datetime_str = datetime;
	unsigned int remaining_length = COMMON_DATETIME_LEN;
	struct tm time_info;
	unsigned int year = 0;
	unsigned int month = 0;
	unsigned int day = 0;
	unsigned int hour = 0;
	unsigned int minute = 0;
	unsigned int second = 0;

	// The datetime string will have the format yyyymmddHHMMSS.mmmmmmsUUU
	// Where yyyy - four digit year 0000-9999
	//		mm - two digit month 01-12
	//		dd - two digit day -1-31
	//		HH - two digit hour 00-23
	//		MM - two digit minute 00-59
	//		SS - two digit second 00-59
	//		mmmmmm = six digit microseconds 000000-999999
	//		s - one character sign either '+' or '-'
	//		UUU - three digit offset indicating number of minutes from UTC

	// Plus one for the null terminator
	s_strncpy(buf, DATETIME_YEAR_LEN + 1, p_datetime_str, remaining_length);
	year = atoi(buf);

	p_datetime_str += DATETIME_YEAR_LEN;
	remaining_length -= DATETIME_YEAR_LEN;
	s_strncpy(buf, DATETIME_MONTH_LEN + 1, p_datetime_str, remaining_length);
	month = atoi(buf);

	p_datetime_str += DATETIME_MONTH_LEN;
	remaining_length -= DATETIME_MONTH_LEN;
	s_strncpy(buf, DATETIME_DAY_LEN + 1, p_datetime_str, remaining_length);
	day = atoi(buf);

	p_datetime_str += DATETIME_DAY_LEN;
	remaining_length -= DATETIME_DAY_LEN;
	s_strncpy(buf, DATETIME_HOUR_LEN + 1, p_datetime_str, remaining_length);
	hour = atoi(buf);

	p_datetime_str += DATETIME_HOUR_LEN;
	remaining_length -= DATETIME_HOUR_LEN;
	s_strncpy(buf, DATETIME_MINUTE_LEN + 1, p_datetime_str, remaining_length);
	minute = atoi(buf);

	p_datetime_str += DATETIME_MINUTE_LEN;
	remaining_length -= DATETIME_MINUTE_LEN;
	s_strncpy(buf, DATETIME_SECOND_LEN + 1, p_datetime_str, remaining_length);
	second = atoi(buf);

	time_info.tm_year = year - 1900;
	time_info.tm_mon = month - 1;
	time_info.tm_mday = day;
	time_info.tm_hour = hour;
	time_info.tm_min = minute;
	time_info.tm_sec = second;
	time_info.tm_isdst = -1;
	unsigned long long secs = (unsigned long long)mktime(&time_info);

	return secs;
}
/*
 * Function used to convert a datetime string, either as an interval or as
 * a datetime to a number of seconds. In the case of an interval, it returns
 * the number of seconds in the interval. In the case of a datetime, it returns
 * the number of seconds since the epoch.
 */
enum datetime_type convert_datetime_string_to_seconds(const COMMON_DATETIME_STR datetime,
														unsigned long long *secs)
{
	enum datetime_type type = DATETIME_TYPE_UNKNOWN;
	unsigned char bad_type = 0;

	*secs = 0;

	// Check to make sure its a properly formed datetime string
	// A properly formatted datetime string may be either a datetime
	// or an interval. In either case, the string must have 14 digits
	// followed by a decimal point. This must be followed by six more
	// digits then one of "+,-,:". That is followed by three more digits
	// and a null terminator.
	// A datetime interval has a colon for the 21st character while a
	// datetime has either a plus or minus sign.

	// The COMMON_DATETIME_LEN includes the null terminator
	if (s_strnlen(datetime, COMMON_DATETIME_LEN) == COMMON_DATETIME_LEN - 1)
	{
		for (int i = 0; i < COMMON_DATETIME_LEN; i++)
		{
			if (i != DATETIME_DECIMALPOINT_POSITION &&
				i != DATETIME_SIGN_POSITION &&
				i != DATETIME_NULLTERMINATOR_POSITION &&
				!isdigit(datetime[i]))
			{
				bad_type = 1;
			}
		}

		if (datetime[DATETIME_DECIMALPOINT_POSITION] != '.')
		{
			bad_type = 1;
		}

		if (datetime[DATETIME_SIGN_POSITION] != ':' &&
			datetime[DATETIME_SIGN_POSITION] != '+' &&
			datetime[DATETIME_SIGN_POSITION] != '-')
		{
			bad_type = 1;
		}

		if (datetime[DATETIME_NULLTERMINATOR_POSITION] != '\0')
		{
			bad_type = 1;
		}
	}
	else
	{
		bad_type = 1;
	}

	if (!bad_type)
	{
		if (datetime[DATETIME_SIGN_POSITION] == ':')
		{
			type = DATETIME_TYPE_INTERVAL;
			*secs = return_datetime_interval_seconds(datetime);
		}
		else
		{
			*secs = return_datetime_seconds(datetime);
			type = DATETIME_TYPE_DATETIME;
		}
	}

	return type;
}

/*
 * Function used to convert a number of seconds to a datetime interval
 */
void convert_seconds_to_datetime_interval(const unsigned long long secs,
							COMMON_DATETIME_STR datetime_buf)
{
	if (datetime_buf != NULL)
	{
		unsigned char seconds = secs % SECONDSPERMINUTE;
		unsigned long long quotient = secs / SECONDSPERMINUTE;
		unsigned char minutes = quotient % MINUTESPERHOUR;
		quotient = quotient / MINUTESPERHOUR;
		unsigned char hours = quotient % HOURSPERDAY;
		unsigned int days = quotient / HOURSPERDAY;

		s_snprintf(datetime_buf, COMMON_DATETIME_LEN,
				"%08u%02d%02d%02d.000000:000", days, hours,
				minutes, seconds);
	}
}

/*
 * Function used to convert from the time_t type to a formatted date string as defined by CIM
 * format = yyyyMMddHHmmss.mmmmmmsutc
 * yyyy - is a 4 digit year
 * MM - is the month
 * dd - is the day of the month
 * HH - is the hour (24 hour clock)
 * mm - is the minute
 * ss - is the second
 * mmmmmm - is the number of microseconds
 * s - is "+" or "-", indicating a positive or negative offset from UTC
 * utc - is the offset from UTC in minutes (using the sign indicated by s).
 * NOTE: Caller allocates char array of 26 chars which is enough space for
 * the datetime string plus a null terminator
 */

void convert_seconds_to_datetime(time_t raw_time, COMMON_DATETIME_STR datetime_buf)
{
	int timezone_diff_min = 0;
	get_timezone_diff((time_t)0, &timezone_diff_min);

	if (datetime_buf != NULL)
	{
		char temp[TEMP_TIMESTR_LEN];
		struct tm *p_localTime;

		// convert to local time
		p_localTime = localtime(&raw_time);

		if (NULL != p_localTime)
		{
			// convert to time string
			strftime(temp, sizeof (temp), "%Y%m%d%H%M%S", p_localTime);

			// make sure we're null terminated
			temp[TEMP_TIMESTR_LEN - 1] = '\0';

			// figure out the sign
			char sign = (timezone_diff_min < 0) ? '-' : '+';

			// now generate the full string
			s_snprintf(datetime_buf, COMMON_DATETIME_LEN, "%s.000000%c%03d", temp, sign,
				abs(timezone_diff_min));
		}
	}
}

/*
 * Function used to determine the timezone differential from a raw time_t (assumed to be local)
 */
void get_timezone_diff(time_t raw_time, int *timezone_diff_min)
{
	struct tm *p_tempTime = gmtime(&raw_time);

	if (NULL != timezone_diff_min)
	{
		*timezone_diff_min = 0;
		if (NULL != p_tempTime)
		{
			struct tm utcTime = *p_tempTime;
			// calculate the local time offset from utc time
			p_tempTime = localtime(&raw_time);

			if (NULL != p_tempTime)
			{
				// make our own copy of the struct because p_localTime
				// points to a shared internal struct
				struct tm localTime = *p_tempTime;
				*timezone_diff_min = get_tm_tzd_mins(utcTime, localTime);
			}
		}
	}
}

/*
 * Helper function used to get the time zone differential (tzd)
 * between two time structs, in minutes
 */
int get_tm_tzd_mins(struct tm time_a, struct tm time_b)
{
	int day_diff = 0;
	if (time_b.tm_wday != time_a.tm_wday)
	{
		// difference can only be +/- a single day...
		// unless Sunday vs. Monday, so detect
		day_diff = time_b.tm_wday - time_a.tm_wday;
		if (abs(day_diff) > 1)
		{
			day_diff = -1;
		}
	}

	return (((day_diff * HOURSPERDAY) +
			(time_b.tm_hour - time_a.tm_hour)) * MINUTESPERHOUR) +
			(time_b.tm_min - time_a.tm_min);
}
