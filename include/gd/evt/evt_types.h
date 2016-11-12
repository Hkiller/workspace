#ifndef CPE_DP_EVT_TYPES_H
#define CPE_DP_EVT_TYPES_H
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "cpe/tl/tl_types.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t evt_processor_id_t;

typedef struct gd_evt_mgr * gd_evt_mgr_t;
typedef struct gd_evt * gd_evt_t;
typedef struct gd_evt_dsp * gd_evt_dsp_t;
typedef struct gd_evt_processor * gd_evt_responser_t;

typedef void (*gd_evt_process_fun_t)(gd_evt_t evt, void * ctx, void * arg);

#ifdef __cplusplus
}
#endif

#endif
