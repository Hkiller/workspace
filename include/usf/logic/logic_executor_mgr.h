#ifndef USF_LOGIC_EXECUTOR_MGR_H
#define USF_LOGIC_EXECUTOR_MGR_H
#include "cpe/utils/stream.h"
#include "cpe/utils/hash_string.h"
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_executor_mgr_t
logic_executor_mgr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t  em);

void logic_executor_mgr_free(logic_executor_mgr_t mgr);

logic_executor_mgr_t
logic_executor_mgr_find(gd_app_context_t app, cpe_hash_string_t name);

logic_executor_mgr_t
logic_executor_mgr_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t logic_executor_mgr_app(logic_executor_mgr_t mgr);
const char * logic_executor_mgr_name(logic_executor_mgr_t mgr);
cpe_hash_string_t logic_executor_mgr_name_hs(logic_executor_mgr_t mgr);

logic_executor_ref_t
logic_executor_mgr_import(
    logic_executor_mgr_t mgr,
    const char * name,
    logic_manage_t logic_mgr,
    logic_executor_type_group_t type_group,
    cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif

