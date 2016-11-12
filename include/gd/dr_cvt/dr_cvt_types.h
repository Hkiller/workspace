#ifndef GD_DR_CVT_TYPES_H
#define GD_DR_CVT_TYPES_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dr_cvt_manage * dr_cvt_manage_t;
typedef struct dr_cvt * dr_cvt_t;

typedef enum dr_cvt_result {
    dr_cvt_result_success = 0
    , dr_cvt_result_error
    , dr_cvt_result_not_enough_input
    , dr_cvt_result_not_enough_output
} dr_cvt_result_t;

typedef dr_cvt_result_t (*dr_cvt_fun_t)(
    LPDRMETA meta,
    void * output, size_t * output_capacity,
    const void * input, size_t * input_capacity,
    void * ctx,
    error_monitor_t em, int debug);

#ifdef __cplusplus
}
#endif

#endif
