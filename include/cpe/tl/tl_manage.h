#ifndef CPE_TL_MANAGE_H
#define CPE_TL_MANAGE_H
#include "cpe/utils/memory.h"
#include "tl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*time line manager*/
typedef enum tl_manage_option {
    tl_set_time_source
    , tl_set_time_cvt
    , tl_set_time_source_context
} tl_manage_option_t;

tl_manage_t tl_manage_create(mem_allocrator_t alloc);
void tl_manage_free(tl_manage_t tm);
ptr_int_t tl_manage_tick(tl_manage_t tm, ptr_int_t count);
int tl_manage_set_opt(tl_manage_t tm, tl_manage_option_t opt,...);
tl_time_t tl_manage_time(tl_manage_t tm);
uint32_t tl_manage_time_sec(tl_manage_t tm);

tl_manage_state_t tl_manage_state(tl_manage_t tm);

void tl_manage_pause(tl_manage_t tm);
void tl_manage_resume(tl_manage_t tm);
void tl_manage_rate(tl_manage_t tm, float rate);

/*time line*/
typedef enum tl_option {
    tl_set_event_dispatcher
    , tl_set_event_enqueue
    , tl_set_event_construct
    , tl_set_event_destory
    , tl_set_event_op_context
} tl_option_t;

tl_t tl_create(tl_manage_t tm);
void tl_free(tl_t tl);
int tl_set_opt(tl_t tl, tl_option_t opt, ...);

/*event enqueue*/
int tl_event_enqueue_local(
    tl_event_t event,
    tl_time_span_t delay,
    tl_time_span_t span,
    int repeatCount,
    void * context);

/*time source*/
tl_time_t tl_time_source_msec(void * context);
tl_time_t tl_time_source_usec(void * context);
tl_time_t tl_time_source_next_event(void * context);
tl_time_t tl_time_source_last_event(void * context);

/*time convert*/
tl_time_span_t tl_time_cvt_sec2msec(tl_time_span_t sec, void *);
tl_time_span_t tl_time_cvt_sec2usec(tl_time_span_t sec, void *);

/**/
void tl_manage_update(tl_manage_t tl_mgr);

#ifdef __cplusplus
}
#endif

#endif
