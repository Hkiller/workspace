#ifndef GD_TIMER_TYPES_H
#define GD_TIMER_TYPES_H
#include "cpe/timer/timer_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef cpe_timer_id_t gd_timer_id_t;

typedef struct gd_timer_mgr * gd_timer_mgr_t;
typedef cpe_timer_processor_t gd_timer_processor_t;
typedef cpe_timer_process_fun_t gd_timer_process_fun_t;

#define GD_TIMER_ID_INVALID CPE_TIMER_ID_INVALID

#ifdef __cplusplus
}
#endif

#endif
