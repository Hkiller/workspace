#define _XOPEN_SOURCE
#include "cpe/pal/pal_time.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/time_utils.h"

int64_t cur_time_ms(void) {
    struct timeval ptv;
	if (gettimeofday(&ptv, NULL)) return 0;
	return ptv.tv_usec / 1000 + (int64_t)ptv.tv_sec * 1000;
}

time_t next_time_in_range_vn(
    time_t after,
    uint32_t start_time_vn, uint32_t end_time_vn,
    uint32_t start_date_vn, uint32_t end_date_vn)
{
    struct tm after_time;
    int next_day = 0;
    int after_year;
    int after_mon;
    int after_day;

    localtime_r(&after, &after_time);

    if (end_time_vn) {
        int h = end_time_vn / 10000;
        int m = (end_time_vn - h * 10000) / 100;
        int s = end_time_vn - h * 10000 - m * 100;

        if (after_time.tm_hour > h
            || (after_time.tm_hour == h && after_time.tm_min > m)
            || (after_time.tm_hour == h && after_time.tm_min == m && after_time.tm_sec > s))
        {
            next_day = 1;
        }
    }

    if (start_time_vn) {
        int h = start_time_vn / 10000;
        int m = (start_time_vn - h * 10000) / 100;
        int s = start_time_vn - h * 10000 - m * 100;

        if (next_day
            || after_time.tm_hour < h
            || (after_time.tm_hour == h && after_time.tm_min < m)
            || (after_time.tm_hour == h && after_time.tm_min == m && after_time.tm_sec < s))
        {
            after_time.tm_hour = h;
            after_time.tm_min = m;
            after_time.tm_sec = s;
        }
    }
    else {
        if (next_day) {
            after_time.tm_hour = 0;
            after_time.tm_min = 0;
            after_time.tm_sec = 0;
        }
    }

    if (next_day) {
        time_t tmp;

        after_time.tm_mday = after_time.tm_mday + next_day;
        tmp = mktime(&after_time);

        localtime_r(&tmp, &after_time);
    }

    after_year = after_time.tm_year + 1900;
    after_mon = after_time.tm_mon + 1;
    after_day = after_time.tm_mday;

    if (end_date_vn) {
        int year = end_date_vn / 10000;
        int mon = (end_date_vn - year * 10000) / 100;
        int day = end_date_vn - year * 10000 - mon * 100;

        if (after_year >  year
            || (after_year == year && after_mon > mon)
            || (after_year == year && after_mon == mon && after_day > day))
        {
            return 0;
        }
    }

    if (start_date_vn) {
        int year = start_date_vn / 10000;
        int mon = (start_date_vn - year * 10000) / 100;
        int day = start_date_vn - year * 10000 - mon * 100;

        if (after_year <  year
            || (after_year == year && after_mon < mon)
            || (after_year == year && after_mon == mon && after_day < day))
        {
            after_time.tm_year = year - 1900;
            after_time.tm_mon = mon - 1;
            after_time.tm_mday = day;

            if (start_time_vn) {
                uint32_t h = start_time_vn / 10000;
                uint32_t m = (start_time_vn - h * 10000) / 100;
                uint32_t s = start_time_vn - h * 10000 - m * 100;

                after_time.tm_hour = h;
                after_time.tm_min = m;
                after_time.tm_sec = s;
            }
            else {
                after_time.tm_hour = 0;
                after_time.tm_min = 0;
                after_time.tm_sec = 0;
            }
        }
    }

    return mktime(&after_time);
}

time_t time_from_str(const char * str_time) {
    struct tm tm;

#ifdef _WIN32
    int year, month, day, hour, minute,second;
    int r;

    r = sscanf(str_time, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    if (r != strlen(str_time)) return 0;

    tm.tm_year  = year-1900;
    tm.tm_mon   = month-1;
    tm.tm_mday  = day;
    tm.tm_hour  = hour;
    tm.tm_min   = minute;
    tm.tm_sec   = second;
    tm.tm_isdst = 0;

#else

    strptime(str_time, "%Y-%m-%d %H:%M:%S", &tm);
    tm.tm_isdst = -1;

#endif
    return mktime(&tm);
}

const char * time_to_str(time_t time, void * buf, size_t buf_size) {
    struct tm *tm;
    struct tm tm_buf;
    tm = localtime_r(&time, &tm_buf);

    strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", tm);
    return buf;
}
