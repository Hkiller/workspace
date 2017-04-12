#ifndef CPE_TL_TYPES_H
#define CPE_TL_TYPES_H
#include "cpe/pal/pal_types.h"

typedef struct tl_event * tl_event_t;
typedef struct tl * tl_t;
typedef struct tl_manage * tl_manage_t;

typedef uint64_t tl_time_t;
typedef int64_t tl_time_span_t;

typedef enum tl_manage_state {
    tl_manage_state_runing
    , tl_manage_state_pause
} tl_manage_state_t;

typedef struct tl_intercept * tl_intercept_t;

typedef tl_time_t (*tl_time_fun_t)(void * context);
typedef tl_time_span_t (*tl_time_cvt_fun_t)(tl_time_span_t delta, void * context);

typedef int (*tl_event_enqueue_t)(
    tl_event_t event, tl_time_span_t delay, tl_time_span_t span, int repeatCount,
    void * context);
typedef void (*tl_event_process_t)(tl_event_t event, void * context);

typedef int (*tl_intercept_fun_t)(tl_event_t event, void * ctx);

#endif


