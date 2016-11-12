#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "dr_cvt_internal_types.h"

dr_cvt_result_t dr_cvt_fun_copy_encode(
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    void * ctx,
    error_monitor_t em, int debug)
{
    int32_t size;
    size_t require_size = sizeof(size) + *input_capacity;

    if (*output_capacity < require_size) {
        CPE_ERROR(
            em, "encode %s: copy: not enought output buf, require %d(input=%d), but only %d!",
            dr_meta_name(meta), (int)require_size, (int)*input_capacity, (int)*output_capacity);
        return dr_cvt_result_not_enough_output;
    }

    size = (int32_t)*input_capacity;
    memcpy(output, &size, sizeof(size));
    memcpy(((char*)output) + sizeof(size), input, *input_capacity);

    *output_capacity = require_size;

    if (debug) {
        CPE_INFO(
            em, "encode %s: copy: copy %d data to output, input-size=%d",
            dr_meta_name(meta), (int)require_size, (int)*input_capacity);
    }

    return dr_cvt_result_success;
}

dr_cvt_result_t
dr_cvt_fun_copy_decode(
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    void * ctx,
    error_monitor_t em, int debug)
{
    int32_t size;

    if (*input_capacity < sizeof(size)) {
        if (debug) {
            CPE_INFO(
                em, "decode %s: copy: not enought input data, require at least %d, but only %d!",
                dr_meta_name(meta), (int)sizeof(size), (int)*input_capacity);
        }
        *output_capacity = 0;
        *input_capacity = 0;
        return dr_cvt_result_not_enough_input;
    }

    memcpy(&size, input, sizeof(size));
    if (*input_capacity < sizeof(size) + size) {
        if (debug) {
            CPE_INFO(
                em, "decode %s: copy: not enought input data, require %d(size=%d), but only %d!",
                dr_meta_name(meta), (int)sizeof(size) + size, size, (int)*input_capacity);
        }
        *output_capacity = 0;
        *input_capacity = 0;
        return dr_cvt_result_not_enough_input;
    }

    if (*output_capacity < (size_t)size) {
        if (debug) {
            CPE_INFO(
                em, "decode %s: copy: not enought output buf, require %d, but only %d!",
                dr_meta_name(meta), (int)size, (int)*output_capacity);
        }

        return dr_cvt_result_not_enough_output;
    }

    memcpy(output, ((const char *)input) + sizeof(size), size);
    *output_capacity = size;
    *input_capacity = size + sizeof(size);

    if (debug) {
        CPE_INFO(
            em, "decode %s: copy: copy %d data to output, input-size=%d",
            dr_meta_name(meta), (int)size, (int)*input_capacity);
    }

    return dr_cvt_result_success;
}
