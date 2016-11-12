#ifndef CPE_DR_DATA_CVT_H
#define CPE_DR_DATA_CVT_H
#include "cpe/utils/error.h"
#include "cpe/cfg/cfg_types.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void dr_data_cvt(
    void * output,
    size_t output_capacity,
    LPDRMETA output_meta,
    const void * input,
    size_t input_capacity,
    LPDRMETA input_meta,
    cfg_t cfg,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
