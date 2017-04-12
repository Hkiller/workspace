#ifndef CPE_DR_HTTP_ARGS_H
#define CPE_DR_HTTP_ARGS_H
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int dr_http_args_read_args(
    void * result,
    size_t capacity,
    cpe_http_arg_t args,
    uint16_t arg_count,
    LPDRMETA meta,
    error_monitor_t em);

int dr_http_args_write(
    write_stream_t s,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
