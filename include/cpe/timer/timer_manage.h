#ifndef CPE_TIMER_MANAGE_H
#define CPE_TIMER_MANAGE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/tl/tl_types.h"
#include "timer_types.h"

#ifdef __cplusplus
extern "C" {
#endif

cpe_timer_mgr_t cpe_timer_mgr_create(tl_manage_t tl_mgr, mem_allocrator_t alloc, error_monitor_t em);
void cpe_timer_mgr_free(cpe_timer_mgr_t em);

tl_t cpe_timer_mgr_tl(cpe_timer_mgr_t mgr);

int cpe_timer_mgr_regist_timer(
    cpe_timer_mgr_t mgr,
    cpe_timer_id_t * id,
    cpe_timer_process_fun_t fun, void * ctx,
    void * arg, void (*arg_fini)(void *),
    tl_time_span_t delay, tl_time_span_t span, int repeatCount);

void cpe_timer_mgr_unregist_timer_by_ctx(cpe_timer_mgr_t mgr, void * ctx);
void cpe_timer_mgr_unregist_timer_by_id(cpe_timer_mgr_t mgr, cpe_timer_id_t timer_id);

int cpe_timer_mgr_have_timer(cpe_timer_mgr_t mgr, cpe_timer_id_t timer_id);

#ifdef __cplusplus
}
#endif

#endif
