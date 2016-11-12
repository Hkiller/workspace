#ifndef CPE_PAL_MSVC_TIME_H
#define CPE_PAL_MSVC_TIME_H
#include <time.h>
#include <winsock.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct timezone {
    __int32  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv/*in*/, struct timezone *tz/*in*/);
struct tm * localtime_r(const time_t *clock, struct tm *result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
