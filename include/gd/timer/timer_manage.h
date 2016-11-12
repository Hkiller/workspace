#ifndef GD_TIMER_MANAGE_H
#define GD_TIMER_MANAGE_H
#include "cpe/utils/memory.h"
#include "cpe/tl/tl_types.h"
#include "gd/app/app_types.h"
#include "timer_types.h"

#ifdef __cplusplus
extern "C" {
#endif

gd_timer_mgr_t
gd_timer_mgr_create(
    gd_app_context_t app,
    const char * name,
    const char * tl_name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void gd_timer_mgr_free(gd_timer_mgr_t em);

gd_timer_mgr_t
gd_timer_mgr_find(gd_app_context_t app, cpe_hash_string_t name);

gd_timer_mgr_t
gd_timer_mgr_find_nc(gd_app_context_t app, const char * name);

gd_timer_mgr_t
gd_timer_mgr_default(gd_app_context_t app);

gd_app_context_t gd_timer_mgr_app(gd_timer_mgr_t mgr);
const char * gd_timer_mgr_name(gd_timer_mgr_t mgr);
cpe_hash_string_t gd_timer_mgr_name_hs(gd_timer_mgr_t mgr);

tl_t gd_timer_mgr_tl(gd_timer_mgr_t mgr);

int gd_timer_mgr_regist_timer(
    gd_timer_mgr_t mgr,
    gd_timer_id_t * id,
    gd_timer_process_fun_t fun, void * ctx,
    void * arg, void (*arg_fini)(void *),
    tl_time_span_t delay, tl_time_span_t span, int repeatCount);

void gd_timer_mgr_unregist_timer_by_ctx(gd_timer_mgr_t mgr, void * ctx);
void gd_timer_mgr_unregist_timer_by_id(gd_timer_mgr_t mgr, gd_timer_id_t timer_id);

int gd_timer_mgr_have_timer(gd_timer_mgr_t mgr, gd_timer_id_t timer_id);

#ifdef __cplusplus
}
#endif

#endif
