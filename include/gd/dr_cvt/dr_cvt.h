#ifndef GD_DR_CVT_H
#define GD_DR_CVT_H
#include "dr_cvt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dr_cvt_t dr_cvt_create(gd_app_context_t app, const char * name);
dr_cvt_t dr_cvt_create_ex(dr_cvt_manage_t mgr, const char * name);
void dr_cvt_free(dr_cvt_t cvt);

const char * dr_cvt_name(dr_cvt_t cvt);

dr_cvt_result_t dr_cvt_encode(
    dr_cvt_t cvt, 
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    error_monitor_t em, int debug);

dr_cvt_result_t dr_cvt_decode(
    dr_cvt_t cvt, 
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    error_monitor_t em, int debug);

#ifdef __cplusplus
}
#endif

#endif
