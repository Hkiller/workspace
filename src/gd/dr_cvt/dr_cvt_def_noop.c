#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "dr_cvt_internal_types.h"

dr_cvt_result_t dr_cvt_fun_noop_encode(
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    void * ctx,
    error_monitor_t em, int debug)
{
    if (*output_capacity < *input_capacity) {
        CPE_ERROR(
            em, "encode %s: noop: not enought output buf, require %d, but only %d!",
            dr_meta_name(meta), (int)*input_capacity, (int)*output_capacity);
        return dr_cvt_result_not_enough_output;
    }

    memcpy(((char*)output), input, *input_capacity);

    *output_capacity = *input_capacity;

    if (debug) {
        CPE_INFO(
            em, "encode %s: noop: noop %d data to output, input-size=%d",
            dr_meta_name(meta), (int)*output_capacity, (int)*input_capacity);
    }

    return dr_cvt_result_success;
}

dr_cvt_result_t
dr_cvt_fun_noop_decode(
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    void * ctx,
    error_monitor_t em, int debug)
{
    if (*output_capacity < (size_t)*input_capacity) {
        if (debug) {
            CPE_INFO(
                em, "decode %s: noop: not enought output buf, require %d, but only %d!",
                dr_meta_name(meta), (int)*input_capacity, (int)*output_capacity);
        }

        return dr_cvt_result_not_enough_output;
    }

    memcpy(output, input, *input_capacity);
    *output_capacity = *input_capacity;

    if (debug) {
        CPE_INFO(
            em, "decode %s: noop: noop %d data to output, input-size=%d",
            dr_meta_name(meta), (int)*input_capacity, (int)*input_capacity);
    }

    return dr_cvt_result_success;
}
