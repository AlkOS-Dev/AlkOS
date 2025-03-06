#ifndef LIBC_INCLUDE_TIME_H_
#define LIBC_INCLUDE_TIME_H_

/* external includes */
#include <errno.h>
#include <stddef.h>
#include <sys/time.h>
#include <types.h>

// ------------------------------
// Defines
// ------------------------------

/* POSIX defines CLOCKS_PER_SEC as one million, regardless of the actual precision of clock. */
#define CLOCKS_PER_SEC ((__clock_t)1000000)

#define TIME_UTC ((int)kTimeUtc)

// ------------------------------
// Types
// ------------------------------

typedef struct tm {
    int tm_sec;  /* seconds after the minute: [0-60] */
    int tm_min;  /* minutes after the hour: [0-59] */
    int tm_hour; /* hours since midnight [0-23] */
    int tm_mday; /* day of the month [1-31] */
    int tm_mon;  /* months since january [0-11] */
    int tm_year; /* years since 1900 */
    int tm_wday; /* days since sunday [0-6] */
    int tm_yday; /* days since january 1 [0-365] */
    int tm_isdst;
    /* Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and
     * negative if no information is available*/
} tm;

typedef unsigned long long time_t;
typedef unsigned long long clock_t;

typedef struct timespec {
    time_t tv_sec; /* seconds */
    long tv_nsec;  /* remainder */
} timespec;

BEGIN_DECL_C
// ------------------------------
// Time manipulation
// ------------------------------

double difftime(time_t time_end, time_t time_beg);

time_t time(time_t *arg);

clock_t clock(void);

/* C11 */
int timespec_get(struct timespec *ts, int base);

/* C23 */
int timespec_getres(struct timespec *ts, int base);

// ------------------------------
// Format conversions
// ------------------------------

/* valid until C23 */
char *asctime(const tm *time_ptr);

/* C11 */
errno_t asctime_s(char *buf, rsize_t bufsz, const tm *time_ptr);

/* valid until C23 */
char *ctime(const time_t *timer);

/* C11 */
errno_t ctime_s(char *buf, rsize_t bufsz, const time_t *timer);

size_t strftime(
    char *__restrict__ s, size_t max_size, const char *__restrict__ format,
    const tm *__restrict__ time_ptr
);

struct tm *gmtime(const time_t *timer);

/* C23 */
struct tm *gmtime_r(const time_t *timer, struct tm *result);

/* C11 */
struct tm *gmtime_s(const time_t *__restrict__ timer, struct tm *__restrict__ result);

struct tm *localtime(const time_t *timer);

/* C23 */
struct tm *localtime_r(const time_t *timer, struct tm *result);

time_t mktime(struct tm *time_ptr);

END_DECL_C

#endif  // LIBC_INCLUDE_TIME_H_
