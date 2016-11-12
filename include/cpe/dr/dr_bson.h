#ifndef CPE_DR_BSON_H
#define CPE_DR_BSON_H
#include "cpe/utils/stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int dr_bson_read(
    void * result,
    size_t capacity,
    const void * input,
    size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em);

int dr_bson_read_to_buffer(
    struct mem_buffer * result, 
    const void * input,
    size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em);

int dr_bson_write(
    void * output, size_t output_capacity, uint8_t is_open,
    const void * input, size_t input_capacity, LPDRMETA meta,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
