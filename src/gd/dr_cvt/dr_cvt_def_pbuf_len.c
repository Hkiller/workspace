#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_pbuf.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/dr_cvt/dr_cvt_manage.h"

dr_cvt_result_t dr_cvt_fun_pbuf_len_encode(
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    void * ctx,
    error_monitor_t em, int debug)
{
    int r;
    size_t reserve_size_size;
    size_t size_size;
    size_t data_capacity;
    uint8_t size_buffer[10];

    reserve_size_size = 1;

    data_capacity = *output_capacity - reserve_size_size;

    r = dr_pbuf_write(((char *)output) + reserve_size_size, data_capacity, input, *input_capacity, meta, em);
    if (r < 0) {
        CPE_ERROR(
            em, "encode %s: pbuf-len: fail, input buf "FMT_SIZE_T", output buf "FMT_SIZE_T,
            dr_meta_name(meta), data_capacity, *output_capacity);
        return dr_cvt_result_error;
    }

    size_size = dr_pbuf_encode32((uint32_t)r, size_buffer);
    if (size_size + r > *output_capacity) {
        CPE_ERROR(
            em, "encode %s: pbuf-len: fail(too small with len), input buf "FMT_SIZE_T", output buf "FMT_SIZE_T,
            dr_meta_name(meta), size_size + r, *output_capacity);
        return dr_cvt_result_error;
    }

    if (size_size != reserve_size_size) {
        memmove(((char *)output) + size_size, ((char *)output) + reserve_size_size, r);
    }
    memcpy(output, size_buffer, size_size);

    *output_capacity = size_size + r;

    if (debug) {
        CPE_INFO(
            em, "encode %s: pbuf-len: ok, "FMT_SIZE_T" data to output, input-size="FMT_SIZE_T,
            dr_meta_name(meta), *output_capacity, *input_capacity);
    }

    return dr_cvt_result_success;
}

dr_cvt_result_t
dr_cvt_fun_pbuf_len_decode(
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    void * ctx,
    error_monitor_t em, int debug)
{
    uint64_t size_buf;
    int r;
    int size_size;
    size_t data_size;

    size_size = dr_pbuf_decode_uint64((uint8_t *)input, &size_buf);
    if (size_size < 0) {
        CPE_ERROR(em, "decode %s: pbuf-len: fail, read size fail!", dr_meta_name(meta));
        return dr_cvt_result_error;
    }

    if ((size_t)size_size > *input_capacity) return dr_cvt_result_not_enough_input;

    data_size = (size_t)size_buf;
    if ((data_size + size_size) > *input_capacity) return dr_cvt_result_not_enough_input;
    *input_capacity = data_size + size_size;

    r = dr_pbuf_read(output, *output_capacity, (const char *)input + size_size, data_size, meta, em);
    if (r < 0) {
        CPE_ERROR(
            em, "decode %s: pbuf-len: fail, data size %d, output buf "FMT_SIZE_T,
            dr_meta_name(meta), size_size, *output_capacity);
        if (r == dr_code_error_not_enough_output) {
            return dr_cvt_result_not_enough_output;
        }
        else {
            return dr_cvt_result_error;
        }
    }

    *output_capacity = r;

    if (debug) {
        CPE_INFO(
            em, "decode %s: pbuf-len: ok, %d data to output, data-size="FMT_SIZE_T", input-size="FMT_SIZE_T,
            dr_meta_name(meta), r, data_size, *input_capacity);
    }

    return dr_cvt_result_success;
}

EXPORT_DIRECTIVE
int dr_pbuf_len_cvt_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    return dr_cvt_type_create(app, "pbuf-len", dr_cvt_fun_pbuf_len_encode, dr_cvt_fun_pbuf_len_decode, NULL);
}

EXPORT_DIRECTIVE
void dr_pbuf_len_cvt_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dr_cvt_type_free(app, "pbuf-len");
}
