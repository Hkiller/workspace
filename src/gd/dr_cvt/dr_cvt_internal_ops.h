#ifndef GD_DR_CVT_INTERNAL_OPS_H
#define GD_DR_CVT_INTERNAL_OPS_H
#include "dr_cvt_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void dr_cvt_type_free_i(struct dr_cvt_type * t);
struct dr_cvt_type * dr_cvt_type_find(dr_cvt_manage_t mgr, const char * name);

uint32_t dr_cvt_type_hash(const struct dr_cvt_type * t);
int dr_cvt_type_cmp(const struct dr_cvt_type * l, const struct dr_cvt_type * r);
void dr_cvt_type_free_all(dr_cvt_manage_t mgr);

#define DR_CVT_REGISTER_TYPE(__app, __mgr, __name, __type)      \
    do {                                                        \
    extern dr_cvt_result_t dr_cvt_fun_ ## __type ## _encode(    \
        LPDRMETA meta,                                          \
        void * output, size_t * output_capacity,                \
        const void * input, size_t * input_capacity,            \
        void * ctx,                                             \
        error_monitor_t em, int debug);                         \
    extern dr_cvt_result_t                                      \
    dr_cvt_fun_ ## __type ## _decode(                           \
        LPDRMETA meta,                                          \
        void * output, size_t * output_capacity,                \
        const void * input, size_t * input_capacity,            \
        void * ctx,                                             \
        error_monitor_t em, int debug);                         \
    if (dr_cvt_type_create_ex(                                  \
        __mgr, __name,                                          \
            dr_cvt_fun_ ## __type ## _encode,                   \
            dr_cvt_fun_ ## __type ## _decode,                   \
        NULL) != 0)                                             \
    {                                                           \
        APP_CTX_ERROR(                                          \
            __app, "%s: register type %s fail!",                \
            dr_cvt_manage_name(__mgr), __name);                 \
        dr_cvt_manage_free(__mgr);                              \
        return -1;                                              \
    }                                                           \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif
