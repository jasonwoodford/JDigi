/*
 * time.c
 * 
 * Copyright (c)2012 Michael Duane Rice. All rights reserved.
 * Modified to adapt with AtomThreads by Jason Woodford, VO1JWW.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions
 * and the following disclaimer in the documentation and/or other materials
 * provided with the distribution. Neither the name of the copyright holders
 * nor the names of contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: time.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

#include "time.h"

/**
 * Private allocation, shared between asctime() and isotime()
 */
static char         __store[26];
static char         *__asc_store = __store;
static long         __utc_offset;
static int          (*__dst_ptr) (const time_t *, int32_t *);
static struct tm    __tm_store;

extern volatile time_t __system_time;

/**
 * Standard time() function. Copying from __system_time must be atomic, since it
 * may be incremented at interrupt time.
 */
time_t time (time_t * timer) {
	time_t  ret;
    CRITICAL_STORE;
    
    CRITICAL_START();
	ret = __system_time;
	CRITICAL_END();
	if (timer) *timer = ret;
	return ret;
}

/**
 * The C90 standard specifies this returns a 'double. Since we do not have a true double,
 * we return a work alike type.
 */
int32_t difftime (time_t t1, time_t t2) {
	return t1 - t2;
}


/**
 * 'Break down' a y2k time stamp into the elements of struct tm.
 *  Unlike mktime(), this function does not 'normalize' the elements of timeptr.
 */
time_t mk_gmtime (const struct tm * timeptr) {
    time_t   ret;
    uint32_t tmp;
    int      n, m, d, leaps;
    
    /* Determine elapsed whole days since the epoch to the beginning of this year.
     * Since our epoch is at a conjunction of the leap cycles, we can do this rather quickly.
     */
    n = timeptr->tm_year - 100;
    leaps = 0;
    if (n) {
        m = n - 1;
        leaps = m / 4;
        leaps -= m / 100;
        leaps++;
    }
    tmp = 365UL * n + leaps;
    /* Derive the day of year from month and day of month. We use the pattern of 31 day months
     * followed by 30 day months to our advantage, but we must 'special case' Jan/Feb, and
     * account for a 'phase change' between July and August (153 days after March 1).
     */
    d = timeptr->tm_mday - 1;   /* tm_mday is one based */

    /* handle Jan/Feb as a special case */
    if (timeptr->tm_mon < 2) {
        if (timeptr->tm_mon) d += 31;

    } else {
        n = 59 + is_leap_year(timeptr->tm_year + 1900);
        d += n;
        n = timeptr->tm_mon - MARCH;

        /* account for phase change */
        if (n > (JULY - MARCH)) d += 153;
        n %= 5;
        /* n is now an index into a group of alternating 31 and 30
         * day months... 61 day pairs.
         */
        m = n / 2;
        m *= 61;
        d += m;
        /*
         * if n is odd, we are in the second half of the
         * month pair
         */
        if (n & 1) d += 31;
    }
    /* Add day of year to elapsed days, and convert to seconds */
    tmp += d;
    tmp *= ONE_DAY;
    ret = tmp;
    /* compute 'fractional' day */
    tmp = timeptr->tm_hour;
    tmp *= ONE_HOUR;
    tmp += timeptr->tm_min * 60UL;
    tmp += timeptr->tm_sec;

    ret += tmp;
    return ret;
}

/**
 * Standard mktime(). The provided broken down Local 'calendar' time is converted into
 * a binary time stamp. The process is then reversed to 'normalize' timeptr.
 */
time_t mktime (struct tm * timeptr) {
	time_t ret;

	ret = mk_gmtime(timeptr);
	if (timeptr->tm_isdst < 0) {
		if (__dst_ptr) timeptr->tm_isdst = __dst_ptr(&ret, &__utc_offset);
	}
	if (timeptr->tm_isdst > 0) ret -= timeptr->tm_isdst;
	ret -= __utc_offset;
	localtime_r(&ret, timeptr);
	return ret;
}

/**
 * Standard gmtime(). We convert binary time into calendar time in our private struct tm object,
 * returning that object.
 */
struct tm * gmtime (const time_t * timeptr) {
	gmtime_r(timeptr, &__tm_store);
	return &__tm_store;
}

/**
 * Re entrant version of gmtime().
 */
void gmtime_r (const time_t * timer, struct tm * timeptr) {
    int32_t     fract;
    ldiv_t      lresult;
    div_t       result;
    uint16_t    days, n, leapyear, years;

    /* break down timer into whole and fractional parts of 1 day */
    days = *timer / 86400UL;
    fract = *timer % 86400UL;
    /* Extract hour, minute, and second from the fractional day */
    lresult = ldiv(fract, 60L);
    timeptr->tm_sec = lresult.rem;
    result = div(lresult.quot, 60);
    timeptr->tm_min = result.rem;
    timeptr->tm_hour = result.quot;
    /* Determine day of week ( the epoch was a Saturday ) */
    n = days + SATURDAY;
    n %= 7;
    timeptr->tm_wday = n;
    /* Our epoch year has the property of being at the conjunction of all three 'leap cycles',
     * 4, 100, and 400 years ( though we can ignore the 400 year cycle in this library).
     * Using this property, we can easily 'map' the time stamp into the leap cycles, quickly
     * deriving the year and day of year, along with the fact of whether it is a leap year.
     */
    /* map into a 100 year cycle */
    lresult = ldiv((long) days, 36525L);
    years = 100 * lresult.quot;
    /* map into a 4 year cycle */
    lresult = ldiv(lresult.rem, 1461L);
    years += 4 * lresult.quot;
    days = lresult.rem;
    if (years > 100) days++;
    /* 'years' is now at the first year of a 4 year leap cycle, which will always be a leap year,
     * unless it is 100. 'days' is now an index into that cycle.
     */
    leapyear = 1;
    if (years == 100) leapyear = 0;
    /* compute length, in days, of first year of this cycle */
    n = 364 + leapyear;
    /* if the number of days remaining is greater than the length of the
     * first year, we make one more division.
     */
    if (days > n) {
        days -= leapyear;
        leapyear = 0;
        result = div(days, 365);
        years += result.quot;
        days = result.rem;
    }
    timeptr->tm_year = 100 + years;
    timeptr->tm_yday = days;
    /* Given the year, day of year, and leap year indicator, we can break down the
     * month and day of month. If the day of year is less than 59 (or 60 if a leap year), then
     * we handle the Jan/Feb month pair as an exception.
     */
    n = 59 + leapyear;
    if (days < n) {
        /* special case: Jan/Feb month pair */
        result = div(days, 31);
        timeptr->tm_mon = result.quot;
        timeptr->tm_mday = result.rem;
    } else {
        /* The remaining 10 months form a regular pattern of 31 day months alternating with 30 day
         * months, with a 'phase change' between July and August (153 days after March 1).
         * We proceed by mapping our position into either March-July or August-December.
         */
        days -= n;
        result = div(days, 153);
        timeptr->tm_mon = 2 + result.quot * 5;
        /* map into a 61 day pair of months */
        result = div(result.rem, 61);
        timeptr->tm_mon += result.quot * 2;
        /* map into a month */
        result = div(result.rem, 31);
        timeptr->tm_mon += result.quot;
        timeptr->tm_mday = result.rem;
    }
    /* Cleanup and return
     */
    timeptr->tm_isdst = 0;  /* gmt is never in DST */
    timeptr->tm_mday++; /* tm_mday is 1 based */
}

/**
 * Standard asctime(), we simply punt to the re-entrant version.
 */
char * asctime (const struct tm * timeptr) {
	asctime_r(timeptr, __asc_store);
	return __asc_store;
}

/**
 * Re-entrant version of asctime().
 */
#ifdef __MEMX
static char const __memx ascmonths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
static char const __memx ascdays[] = "SunMonTueWedThuFriSat";
#else
static char const ascmonths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
static char const ascdays[] = "SunMonTueWedThuFriSat";
#endif

void asctime_r (const struct tm * timeptr, char *buffer) {
	unsigned char   i, m, d;
	div_t result;

	d = timeptr->tm_wday * 3;
	m = timeptr->tm_mon * 3;
	for (i = 0; i < 3; i++) {
	    buffer[i] = ascdays[d++];
	    buffer[i+4] = ascmonths[m++];
	}
	buffer[3]=buffer[7]=' ';
	buffer += 8;

	__print_lz(timeptr->tm_mday,buffer,' ');
	buffer += 3;

	__print_lz(timeptr->tm_hour,buffer,':');
	buffer += 3;

	__print_lz(timeptr->tm_min,buffer,':');
	buffer += 3;

	__print_lz(timeptr->tm_sec,buffer,' ');
	buffer += 3;

	result = div(timeptr->tm_year + 1900 , 100);

	__print_lz(result.quot,buffer,' ');
	buffer += 2;

	__print_lz(result.rem,buffer,0);

}

/**
 * Standard ctime(). We have to break down the time stamp, print it into our
 * private buffer, and return the buffer.
 */
char * ctime (const time_t * timeptr) {
	struct tm       calendar;

	localtime_r(timeptr, &calendar);
	asctime_r(&calendar, __asc_store);
	return __asc_store;
}

/**
 * Re entrant version of ctime().
 */
void ctime_r (const time_t * timeptr, char *buffer) {
	struct tm       calendar;

	localtime_r(timeptr, &calendar);
	asctime_r(&calendar, buffer);
}

/**
 * This function returns ISO8601 formatted time, in our private buffer.
 */
char * isotime (const struct tm * tmptr) {
	isotime_r(tmptr, __asc_store);
	return __asc_store;
}

/**
 * Re-entrant version of isotime(), which prints the date and time in ISO 8601 format.
 */
void isotime_r (const struct tm * tmptr, char *buffer) {
	int i;

	i = tmptr->tm_year + 1900;
	__print_lz(i/100, buffer, '-');
	buffer+=2;
	__print_lz(i%100, buffer,'-');
	buffer+=3;

	i = tmptr->tm_mon + 1;
	__print_lz(i, buffer,'-');
	buffer+=3;

	i = tmptr->tm_mday;
	__print_lz(i, buffer,' ');
	buffer+=3;

	i = tmptr->tm_hour;
	__print_lz(i, buffer,':');
	buffer+=3;

	i = tmptr->tm_min;
	__print_lz(i, buffer,':');
	buffer+=3;

	i = tmptr->tm_sec;
	__print_lz(i, buffer,0);

}

/**
 * Standard localtime() function.
 */
struct tm * localtime (const time_t * timer) {
	localtime_r(timer, &__tm_store);
	return &__tm_store;
}

/**
 * Re entrant version of localtime(). Given a binary UTC time stamp, add the time
 * zone and Daylight savings offset, then break it down into calendar time.
 */
void localtime_r (const time_t * timer, struct tm * timeptr) {
	time_t  lt;
	int16_t dst;

	dst = -1;
	if (__dst_ptr) dst = __dst_ptr(timer, &__utc_offset);
	lt = *timer + __utc_offset;
	if (dst > 0) lt += dst;
	gmtime_r(&lt, timeptr);
	timeptr->tm_isdst = dst;
}

/**
 * Standard strftime(). This is a memory hungry monster.
 */
#ifdef __MEMX
static char const __memx strfwkdays[] = "Sunday Monday Tuesday Wednesday Thursday Friday Saturday ";
static char const __memx strfmonths[] = "January February March April May June July August September October November December ";
#else
static char const strfwkdays[] = "Sunday Monday Tuesday Wednesday Thursday Friday Saturday ";
static char const strfmonths[] = "January February March April May June July August September October November December ";
#endif

#ifdef __MEMX
unsigned char pgm_copystring(const char __memx *p, unsigned char i, char *b, unsigned char l) {
#else
unsigned char pgm_copystring(const char *p, unsigned char i, char *b, unsigned char l) {
#endif
    //
    unsigned char   ret, c;
    //
    ret = 0;
    while (i) {
        c = *p++;
        if (c == ' ') i--;
    }
    c = *p++;
    while (c != ' ' && l--) {
        *b++ = c;
        ret++;
        c = *p++;
    }
    *b = 0;
    return ret;
}

size_t strftime(char *buffer, size_t limit, const char *pattern, const struct tm * timeptr) {
    unsigned int    count, length;
    int             d, w;
    char            c;
    char            _store[26];
    struct week_date wd;

    count = length = 0;
    while (count < limit) {
        c = *pattern++;
        if (c == '%') {
            c = *pattern++;
            if (c == 'E' || c == 'O') c = *pattern++;
            switch (c) {
            case ('%'):
                _store[0] = c;
                length = 1;
                break;
            case ('a'):
                length = pgm_copystring(strfwkdays, timeptr->tm_wday, _store, 3);
                break;
            case ('A'):
                length = pgm_copystring(strfwkdays, timeptr->tm_wday, _store, 255);
                break;
            case ('b'):
            case ('h'):
                length = pgm_copystring(strfmonths, timeptr->tm_mon, _store, 3);
                break;
            case ('B'):
                length = pgm_copystring(strfmonths, timeptr->tm_mon, _store, 255);
                break;
            case ('c'):
                asctime_r(timeptr, _store);
                length = 0;
                while (_store[length]) length++;
                break;
            case ('C'):
                d = timeptr->tm_year + 1900;
                d /= 100;
                length = sprintf_P(_store, PSTR("%.2d"), d);
                break;
            case ('d'):
                length = sprintf_P(_store, PSTR("%.2u"), timeptr->tm_mday);
                break;
            case ('D'):
                length = sprintf_P(_store, PSTR("%.2u/%.2u/%.2u"), timeptr->tm_mon + 1, timeptr->tm_mday, timeptr->tm_year % 100);
                break;
            case ('e'):
                length = sprintf_P(_store, PSTR("%2d"), timeptr->tm_mday);
                break;
            case ('F'):
                length = sprintf_P(_store, PSTR("%d-%.2d-%.2d"), timeptr->tm_year + 1900, timeptr->tm_mon + 1, timeptr->tm_mday);
                break;
            case ('g'):
			case ('G'):
				iso_week_date_r(timeptr->tm_year + 1900, timeptr->tm_yday, &wd);
                if (c == 'g') {
                    length = sprintf_P(_store, PSTR("%.2d"), wd.year % 100);
                } else {
                    length = sprintf_P(_store, PSTR("%.4d"), wd.year);
                }
				break;
            case ('H'):
                length = sprintf_P(_store, PSTR("%.2u"), timeptr->tm_hour);
                break;
            case ('I'):
                d = timeptr->tm_hour % 12;
                if (d == 0) d = 12;
                length = sprintf_P(_store, PSTR("%.2u"), d);
                break;
            case ('j'):
                length = sprintf_P(_store, PSTR("%.3u"), timeptr->tm_yday + 1);
                break;
            case ('m'):
                length = sprintf_P(_store, PSTR("%.2u"), timeptr->tm_mon + 1);
                break;
            case ('M'):
                length = sprintf_P(_store, PSTR("%.2u"), timeptr->tm_min);
                break;
            case ('n'):
                _store[0] = 10;
                length = 1;
                break;
            case ('p'):
                length = 2;
                _store[0] = 'A';
                if (timeptr->tm_hour > 11) _store[0] = 'P';
                _store[1] = 'M';
                _store[2] = 0;
                break;
            case ('r'):
                d = timeptr->tm_hour % 12;
                if (d == 0) d = 12;
                length = sprintf_P(_store, PSTR("%2d:%.2d:%.2d AM"), d, timeptr->tm_min, timeptr->tm_sec);
                if (timeptr->tm_hour > 11) _store[10] = 'P';
                break;
            case ('R'):
                length = sprintf_P(_store, PSTR("%.2d:%.2d"), timeptr->tm_hour, timeptr->tm_min);
                break;
            case ('S'):
                length = sprintf_P(_store, PSTR("%.2u"), timeptr->tm_sec);
                break;
            case ('t'):
                length = sprintf_P(_store, PSTR("\t"));
                break;
            case ('T'):
                length = sprintf_P(_store, PSTR("%.2d:%.2d:%.2d"), timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec);
                break;
            case ('u'):
                w = timeptr->tm_wday;
                if (w == 0) w = 7;
                length = sprintf_P(_store, PSTR("%d"), w);
                break;
            case ('U'):
                length = sprintf_P(_store, PSTR("%.2u"), week_of_year(timeptr, 0));
                break;
			case ('V'):
				iso_week_date_r(timeptr->tm_year + 1900, timeptr->tm_yday, &wd);
                length = sprintf_P(_store, PSTR("%.2u"), wd.week);
				break;
            case ('w'):
                length = sprintf_P(_store, PSTR("%u"), timeptr->tm_wday);
                break;
            case ('W'):
                w = week_of_year(timeptr, 1);
                length = sprintf_P(_store, PSTR("%.2u"), w);
                break;
            case ('x'):
                length = sprintf_P(_store, PSTR("%.2u/%.2u/%.2u"), timeptr->tm_mon + 1, timeptr->tm_mday, timeptr->tm_year % 100);
                break;
            case ('X'):
                length = sprintf_P(_store, PSTR("%.2u:%.2u:%.2u"), timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec);
                break;
            case ('y'):
                length = sprintf_P(_store, PSTR("%.2u"), timeptr->tm_year % 100);
                break;
            case ('Y'):
                length = sprintf_P(_store, PSTR("%u"), timeptr->tm_year + 1900);
                break;
            case ('z'):
                d = __utc_offset / 60;
                w = timeptr->tm_isdst / 60;
                if (w > 0) d += w;
                w = abs(d % 60);
                d = d / 60;
                length = sprintf_P(_store, PSTR("%+.2d%.2d"), d, w);
                break;
            default:
                length = 1;
                _store[0] = '?';
                _store[1] = 0;
                break;
            }

            if ((length + count) < limit) {
                count += length;
                for (d = 0; d < (int) length; d++) {
                    *buffer++ = _store[d];
                }
            } else {
                *buffer = 0;
                return count;
            }

        } else {    /* copy a literal */
            *buffer = c;
            buffer++;
            count++;
            if (c == 0) return count;
        }
    }
    *buffer = 0;
    return count;
}

/**
 * Set the dst function pointer.
 */
void set_dst (int (*d) (const time_t *, int32_t *)) {
	__dst_ptr = d;
}

/**
 * Set the system time. The values passed are assumed to represent local
 * standard time, such as would be obtained from the typical Real Time Clock
 * integrated circuit. It is necessary for this to be atomic, as the value may
 * be incremented at interrupt time.
 */
void set_system_time (time_t timestamp) {
    CRITICAL_STORE;

	CRITICAL_START();
	__system_time = timestamp;
	CRITICAL_END();
}

/**
 * Set the system time zone. The parameter is seconds offset from UTC.
 */
void set_zone(long z) {
	__utc_offset = z;
}

/**
 * Return 1 if 'year' is a leap year, else 0.
 */
unsigned char is_leap_year (int year) {
    div_t           d;

    /* year must be divisible by 4 to be a leap year */
    if (year & 3) return 0;
    /* If theres a remainder after division by 100, year is not divisible by 100 or 400 */
    d = div(year, 100);
    if (d.rem) return 1;
    /* If the quotient is divisible by 4, then year is divisible by 400 */
    if ((d.quot & 3) == 0) return 1;
    return 0;
}

/**
 * Return the length of a month in days, given the year and month in question.
 * The month parameter must be '1 based', ranging from 1 to 12.
 */
uint8_t month_length (int year, uint8_t month) {
    if (month == 2) return 28 + is_leap_year(year);
    /* 'knuckles' algorithm */
    if (month > 7) month++;
    return 30 + (month & 1);
}

/**
 * Return the week of month, where 'base' represents the starting day.
 * In the USA, the week is generally considered to start on Sunday (base = 0),
 * while in Europe it is generally considered to be Monday (base = 1).
 *
 * Return value ranges from 0 to 5.
 */
uint8_t week_of_month(const struct tm * timestruct, uint8_t base) {
	int first, n;

	/* zero base the day of month */
	n = timestruct->tm_mday - 1;
	/* find the first base day of the month (start of week 1) */
	first = 7 + n - timestruct->tm_wday + base;
	first %= 7;
	/* find days since the first week began */
	n = n - first;
	/* if negative, we are in week 0 */
	if (n < 0) return 0;
	return n / 7 + 1;
}

/**
 * Return the week of year, where 'base' represents the first day of the week.
 * In the USA, the week is generally considered to start on Sunday (base = 0),
 * while in Europe it is generally considered to be Monday (base = 1).
 * 
 * Return value ranges from 0 to 52.
 */
uint8_t week_of_year(const struct tm * timestruct, uint8_t base) {
	int first, n;

	/* find the first base day of the year (start of week 1) */
	first = 7 + timestruct->tm_yday - timestruct->tm_wday + base;
    first %= 7;
	/* find days since that first base day*/
	n = timestruct->tm_yday - first;
	/* if negative, we are in week 0 */
	if (n < 0) return 0;
	return n / 7 + 1;
}

/**
 * Compute the ISO 8601 week date corresponding to the given year and day of year.
 * See http://en.wikipedia.org/wiki/ISO_week_date for a full description.
 * 
 * See iso_week_date_r.c for implementation details.
 */
struct week_date * iso_week_date(int y, int yday) {
    struct week_date *iso;

    iso = (struct week_date *) __asc_store;
    iso_week_date_r(y, yday, iso);
    return iso;
}

/**
 * Compute the ISO 8601 week date corresponding to the given year and day of year.
 * See http://en.wikipedia.org/wiki/ISO_week_date for a full description. To summarize:
 * 
 * Weeks are numbered from 1 to 53.
 * Week days are numbered 1 to 7, beginning with Monday as day 1.
 * The first week of the year contains the first Thursday in that year.
 * Dates prior to week 1 belong to the final week of the previous year.
 * The final week of the year contains the last Thursday in that year.
 * Dates after the final week belong to week 1 of the following year.
 */
void iso_week_date_r(int y, int yday, struct week_date * iso) {
    uint16_t years, n, wday;
    int      weeknum, isLeap;

    iso->year = y;
    isLeap = is_leap_year(y);
    /* compute days elapsed since epoch */
    years = y - 2000;
    n = 365 * years + yday;
    if (years) {
        n++;        /* epoch was a leap year */
        n += years / 4;
        n -= isLeap;
        if (years > 100)
            n--;
    }
    /* compute ISO8601 day of week (1 ... 7, Monday = 1) */
    wday = n + 6; /* epoch was a Saturday */
    wday %= 7;
    if (wday == 0) wday = 7;
    iso->day = wday;
    /* compute tentative week number */
    weeknum = yday + 11 - wday;
    weeknum /= 7;
    /* if 53, it could be week 1 of the following year */
    if (weeknum == 53) {
        /* The final week must include its Thursday in the year. We determine the yday of this
         * weeks Thursday, and test whether it exceeds this years length.
         */
        /* determine final yday of this year, 364 or 365 */
        n = 364 + isLeap;
        /* compute yday of this weeks Thursday */
        wday--;       /* convert to zero based week, Monday = 0 */
        yday -= wday; /* yday of this weeks Monday */
        yday += 3;    /* yday of this weeks Thursday */
        /* Is this weeks Thursday included in the year? */
        if (yday > (int) n) {
            iso->year++;
            weeknum = 1;
        }
    }
    iso->week = weeknum;
    /* If zero, it is the final week of the previous year.
     * We determine that by asking for the week number of Dec 31.
     */
    if (weeknum == 0) {
        y = y - 1;
        iso_week_date_r(y, 364 + is_leap_year(y), iso);
        iso->day = wday;
    }
}

/**
 * Print two digit integer with leading zero.
 * Auxillary function for isotime and asctime.
 */
void __print_lz (int i, char *buffer, char s) {
    div_t result;

    result = div(i, 10);
	*buffer++ = result.quot + '0';
	*buffer++ = result.rem + '0';
	*buffer = s;
}

/**
 * Daylight Saving function for the USA. To utilize this function, you must
 * \code #include <util/usa_dst.h> \endcode
 * and
 * \code set_dst(usa_dst); \endcode
 * Given the time stamp and time zone parameters provided, the Daylight Saving function must
 * return a value appropriate for the tm structures' tm_isdst element. That is...
 *  0 : If Daylight Saving is not in effect.
 * -1 : If it cannot be determined if Daylight Saving is in effect.
 * A positive integer : Represents the number of seconds a clock is advanced for Daylight Saving.
 * This will typically be ONE_HOUR.
 * Daylight Saving 'rules' are subject to frequent change. For production applications it is
 * recommended to write your own DST function, which uses 'rules' obtained from, and modifiable by,
 * the end user (perhaps stored in EEPROM).
 */
int usa_dst (const time_t * timer, int32_t * z) {
    time_t          t;
    struct tm       tmptr;
    uint8_t         month, week, hour, day_of_week;
    /* obtain the variables */
    t = *timer + *z;
    gmtime_r(&t, &tmptr);
    month = tmptr.tm_mon;
    day_of_week = tmptr.tm_wday;
    week = week_of_month(&tmptr, 0);
    hour = tmptr.tm_hour;

    if ((month > DST_START_MONTH) && (month < DST_END_MONTH)) return ONE_HOUR;
    if (month < DST_START_MONTH) return 0;
    if (month > DST_END_MONTH) return 0;
    if (month == DST_START_MONTH) {
        if (week < DST_START_WEEK) return 0;
        if (week > DST_START_WEEK) return ONE_HOUR;
        if (day_of_week > SUNDAY) return ONE_HOUR;
        if (hour >= 2) return ONE_HOUR;
        return 0;
    }
    if (week > DST_END_WEEK) return 0;
    if (week < DST_END_WEEK) return ONE_HOUR;
    if (day_of_week > SUNDAY) return 0;
    if (hour >= 1) return 0;
    return ONE_HOUR;
}
