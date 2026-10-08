/* SPDX-License-Identifier: BSD-2-Clause */
/* A time's calendar fields (Linux's struct rtc_time), for log stamps. */
#ifndef _VPU_LINUX_RTC_H_
#define	_VPU_LINUX_RTC_H_
#include <sys/types.h>
#include <sys/clock.h>

struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;	/* 0-11 */
	int tm_year;	/* since 1900 */
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

static inline void
rtc_time64_to_tm(int64_t secs, struct rtc_time *tm)
{
	struct clocktime ct;
	struct timespec ts = { .tv_sec = secs, .tv_nsec = 0 };

	clock_ts_to_ct(&ts, &ct);
	tm->tm_sec = ct.sec;
	tm->tm_min = ct.min;
	tm->tm_hour = ct.hour;
	tm->tm_mday = ct.day;
	tm->tm_mon = ct.mon - 1;
	tm->tm_year = ct.year - 1900;
	tm->tm_wday = ct.dow;
	tm->tm_yday = 0;
	tm->tm_isdst = 0;
}
#endif
