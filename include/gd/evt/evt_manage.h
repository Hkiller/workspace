#ifndef CPE_DP_EVT_MANAGE_H
#define CPE_DP_EVT_MANAGE_H
#include "cpe/utils/memory.h"
#include "cpe/tl/tl_types.h"
#include "evt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

gd_evt_mgr_t
gd_evt_mgr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void gd_evt_mgr_free(gd_evt_mgr_t em);

gd_evt_mgr_t
gd_evt_mgr_find(gd_app_context_t app, cpe_hash_string_t name);

gd_evt_mgr_t
gd_evt_mgr_find_nc(gd_app_context_t app, const char * name);

gd_evt_mgr_t
gd_evt_mgr_default(gd_app_context_t app);

gd_app_context_t gd_evt_mgr_app(gd_evt_mgr_t mgr);
const char * gd_evt_mgr_name(gd_evt_mgr_t mgr);
cpe_hash_string_t gd_evt_mgr_name_hs(gd_evt_mgr_t mgr);

tl_t gd_evt_mgr_tl(gd_evt_mgr_t mgr);

void gd_evg_mgr_set_carry_info(gd_evt_mgr_t em, LPDRMETA carry_meta, size_t carry_size);

int gd_evt_mgr_register_evt(gd_evt_mgr_t mgr, LPDRMETA cmd_meta);
int gd_evt_mgr_register_evt_in_metalib(gd_evt_mgr_t mgr, LPDRMETALIB metalib);

int gd_evt_mgr_regist_responser(
    gd_evt_mgr_t mgr,
    evt_processor_id_t * id,
    const char * oid, gd_evt_process_fun_t fun, void * ctx,
    void * arg, void (*arg_fini)(void *));

void gd_evt_mgr_unregist_responser(gd_evt_mgr_t mgr, void * ctx);

#ifdef __cplusplus
}
#endif

#endif
