#ifndef CPE_TIMER_TYPES_H
#define CPE_TIMER_TYPES_H
#include "cpe/utils/hash_string.h"
#include "cpe/tl/tl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t cpe_timer_id_t;

typedef struct cpe_timer_mgr * cpe_timer_mgr_t;
typedef struct cpe_timer_processor * cpe_timer_processor_t;

typedef void (*cpe_timer_process_fun_t)(void * ctx, cpe_timer_id_t timer_id, void * arg);

#define CPE_TIMER_ID_INVALID ((cpe_timer_id_t)-1)

#ifdef __cplusplus
}
#endif

#endif
