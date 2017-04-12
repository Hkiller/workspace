#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_pbuf.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/dr_cvt/dr_cvt_manage.h"

dr_cvt_result_t dr_cvt_fun_pbuf_encode(
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    void * ctx,
    error_monitor_t em, int debug)
{
    int r;

    r = dr_pbuf_write(output, *output_capacity, input, *input_capacity, meta, em);
    if (r < 0) {
        CPE_ERROR(
            em, "encode %s: pbuf: fail, input buf "FMT_SIZE_T", output buf "FMT_SIZE_T,
            dr_meta_name(meta), *input_capacity, *output_capacity);
        switch(r) {
        case dr_code_error_not_enough_output:
            return dr_cvt_result_not_enough_output;
        case dr_code_error_not_enough_input:
            return dr_cvt_result_not_enough_input;
        default:
            return dr_cvt_result_error;
        }
    }

    *output_capacity = r;

    if (debug) {
        CPE_INFO(
            em, "encode %s: pbuf: ok, %d data to output, input-size="FMT_SIZE_T,
            dr_meta_name(meta), r, *input_capacity);
    }

    return dr_cvt_result_success;
}

dr_cvt_result_t
dr_cvt_fun_pbuf_decode(
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    void * ctx,
    error_monitor_t em, int debug)
{
    int r;

    r = dr_pbuf_read(output, *output_capacity, input, *input_capacity, meta, em);
    if (r < 0) {
        CPE_ERROR(
            em, "decode %s: pbuf: fail, input buf "FMT_SIZE_T", output buf "FMT_SIZE_T,
            dr_meta_name(meta), *input_capacity, *output_capacity);
        switch(r) {
        case dr_code_error_not_enough_output:
            return dr_cvt_result_not_enough_output;
        case dr_code_error_not_enough_input:
            return dr_cvt_result_not_enough_input;
        default:
            return dr_cvt_result_error;
        }
    }

    *output_capacity = r;

    if (debug) {
        CPE_INFO(
            em, "decode %s: pbuf: ok, %d data to output, input-size="FMT_SIZE_T,
            dr_meta_name(meta), r, *input_capacity);
    }

    return dr_cvt_result_success;
}

EXPORT_DIRECTIVE
int dr_pbuf_cvt_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    return dr_cvt_type_create(app, "pbuf", dr_cvt_fun_pbuf_encode, dr_cvt_fun_pbuf_decode, NULL);
}

EXPORT_DIRECTIVE
void dr_pbuf_cvt_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dr_cvt_type_free(app, "pbuf");
}
