#ifndef CPE_DR_YAML_H
#define CPE_DR_YAML_H
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int dr_yaml_read(
    void * result,
    size_t capacity,
    const char * input,
    LPDRMETA meta,
    error_monitor_t em);

int dr_yaml_read_to_buffer(
    struct mem_buffer * result, 
    const char * input,
    LPDRMETA meta,
    error_monitor_t em);

int dr_yaml_print(
    write_stream_t output,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    error_monitor_t em);

int dr_yaml_print_array(
    write_stream_t output,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    error_monitor_t em);

const char * dr_yaml_dump(mem_buffer_t buffer, const void * input, size_t capacity, LPDRMETA meta);

#ifdef __cplusplus
}
#endif

#endif
