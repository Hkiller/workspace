#ifndef SVR_SET_LOGIC_RSP_MANAGE_H
#define SVR_SET_LOGIC_RSP_MANAGE_H
#include "set_logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_logic_rsp_manage_t
set_logic_rsp_manage_create(
    gd_app_context_t app,
    const char * name,
    set_logic_rsp_manage_dp_scope_t scope,
    logic_manage_t logic_mgr,
    logic_executor_mgr_t executor_mgr,
    set_svr_stub_t stub,
    error_monitor_t em);

void set_logic_rsp_manage_free(set_logic_rsp_manage_t mgr);

set_logic_rsp_manage_t
set_logic_rsp_manage_find(gd_app_context_t app, cpe_hash_string_t name);

set_logic_rsp_manage_t
set_logic_rsp_manage_find_nc(gd_app_context_t app, const char * name);

set_logic_rsp_manage_t
set_logic_rsp_manage_default(gd_app_context_t app);

gd_app_context_t set_logic_rsp_manage_app(set_logic_rsp_manage_t mgr);
const char * set_logic_rsp_manage_name(set_logic_rsp_manage_t mgr);
cpe_hash_string_t set_logic_rsp_manage_name_hs(set_logic_rsp_manage_t mgr);

logic_manage_t set_logic_rsp_manage_logic(set_logic_rsp_manage_t mgr);

void set_logic_rsp_manage_set_context_op(
    set_logic_rsp_manage_t mgr,
    size_t ctx_capacity,
    set_logic_ctx_init_fun_t ctx_init,
    set_logic_ctx_fini_fun_t ctx_fini,
    void * ctx_ctx);

uint32_t set_logic_rsp_manage_flags(set_logic_rsp_manage_t mgr);
void set_logic_rsp_manage_flags_set(set_logic_rsp_manage_t mgr, uint32_t flag);
void set_logic_rsp_manage_flag_enable(set_logic_rsp_manage_t mgr, set_logic_rsp_manage_flag_t flag);
void set_logic_rsp_manage_flag_disable(set_logic_rsp_manage_t mgr, set_logic_rsp_manage_flag_t flag);
int set_logic_rsp_manage_flag_is_enable(set_logic_rsp_manage_t mgr, set_logic_rsp_manage_flag_t flag);

logic_context_t set_logic_rsp_manage_create_context(set_logic_rsp_manage_t mgr, dp_req_t pkg, error_monitor_t em);

logic_context_t
set_logic_rsp_manage_create_follow_op_by_name(
    set_logic_rsp_manage_t mgr, logic_context_t ctx, const char * rsp_name, error_monitor_t em);

logic_context_t
set_logic_rsp_manage_create_op_by_name(
    set_logic_rsp_manage_t mgr, logic_context_t ctx/*can be null*/, const char * rsp_name, error_monitor_t em);

void set_logic_rsp_manage_free_context(set_logic_rsp_manage_t mgr, logic_context_t op_context);

#ifdef __cplusplus
}
#endif

#endif
