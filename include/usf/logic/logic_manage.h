#ifndef USF_LOGIC_MANAGE_H
#define USF_LOGIC_MANAGE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/timer/timer_types.h"
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_manage_t
logic_manage_create(
    gd_app_context_t app,
    gd_timer_mgr_t timer_mgr,
    const char * name,
    mem_allocrator_t alloc);

void logic_manage_free(logic_manage_t mgr);

logic_manage_t
logic_manage_find(gd_app_context_t app, cpe_hash_string_t name);

logic_manage_t
logic_manage_find_nc(gd_app_context_t app, const char * name);

logic_manage_t
logic_manage_default(gd_app_context_t app);

gd_app_context_t logic_manage_app(logic_manage_t mgr);
error_monitor_t logic_manage_em(logic_manage_t mgr);

const char * logic_manage_name(logic_manage_t mgr);
cpe_hash_string_t logic_manage_name_hs(logic_manage_t mgr);

void logic_context_free_all(logic_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif

