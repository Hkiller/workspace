#ifndef CPE_UTILS_TIME_H
#define CPE_UTILS_TIME_H
#include "cpe/pal/pal_time.h"
#include "stream.h"
#include "error.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

int64_t cur_time_ms(void);

time_t next_time_in_range_vn(
    time_t after,
    uint32_t start_time_vn, uint32_t end_time_vn,
    uint32_t start_date_vn, uint32_t end_date_vn);

/*2014-02-13 23:23:23*/
time_t time_from_str(const char * str_time);
const char * time_to_str(time_t time, void * buf, size_t buf_size);

time_t time_from_str_tz(const char * str_time);
const char * time_to_str_tz(time_t time, void * buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif
