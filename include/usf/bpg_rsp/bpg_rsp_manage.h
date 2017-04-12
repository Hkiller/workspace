#ifndef USF_BPG_RSP_MANAGE_H
#define USF_BPG_RSP_MANAGE_H
#include "bpg_rsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_rsp_manage_t
bpg_rsp_manage_create(
    gd_app_context_t app,
    const char * name,
    bpg_rsp_manage_dp_scope_t scope,
    logic_manage_t logic_mgr,
    logic_executor_mgr_t executor_mgr,
    bpg_pkg_manage_t pkg_manage,
    error_monitor_t em);

void bpg_rsp_manage_free(bpg_rsp_manage_t mgr);

bpg_rsp_manage_t
bpg_rsp_manage_find(gd_app_context_t app, cpe_hash_string_t name);

bpg_rsp_manage_t
bpg_rsp_manage_find_nc(gd_app_context_t app, const char * name);

bpg_rsp_manage_t
bpg_rsp_manage_default(gd_app_context_t app);

gd_app_context_t bpg_rsp_manage_app(bpg_rsp_manage_t mgr);
const char * bpg_rsp_manage_name(bpg_rsp_manage_t mgr);
cpe_hash_string_t bpg_rsp_manage_name_hs(bpg_rsp_manage_t mgr);

logic_manage_t bpg_rsp_manage_logic(bpg_rsp_manage_t mgr);

int bpg_rsp_manage_set_dispatch_at(bpg_rsp_manage_t mgr, const char * recv_at);

void bpg_rsp_manage_set_context_op(
    bpg_rsp_manage_t mgr,
    size_t ctx_capacity,
    bpg_logic_ctx_init_fun_t ctx_init,
    bpg_logic_ctx_fini_fun_t ctx_fini,
    bpg_logic_pkg_init_fun_t pkg_init,
    void * ctx_ctx);

bpg_pkg_dsp_t bpg_rsp_manage_commit_dsp(bpg_rsp_manage_t mgr);
void bpg_rsp_manage_set_commit_dsp(bpg_rsp_manage_t mgr, bpg_pkg_dsp_t dsp);
bpg_pkg_dsp_t bpg_rsp_manage_forward_dsp(bpg_rsp_manage_t mgr);
void bpg_rsp_manage_set_forward_dsp(bpg_rsp_manage_t mgr, bpg_pkg_dsp_t dsp);

uint32_t bpg_rsp_manage_flags(bpg_rsp_manage_t mgr);
void bpg_rsp_manage_flags_set(bpg_rsp_manage_t mgr, uint32_t flag);
void bpg_rsp_manage_flag_enable(bpg_rsp_manage_t mgr, bpg_rsp_manage_flag_t flag);
void bpg_rsp_manage_flag_disable(bpg_rsp_manage_t mgr, bpg_rsp_manage_flag_t flag);
int bpg_rsp_manage_flag_is_enable(bpg_rsp_manage_t mgr, bpg_rsp_manage_flag_t flag);

logic_context_t bpg_rsp_manage_create_context(bpg_rsp_manage_t mgr, dp_req_t pkg, error_monitor_t em);

logic_context_t
bpg_rsp_manage_create_follow_op_by_name(
    bpg_rsp_manage_t mgr, logic_context_t ctx, const char * rsp_name, error_monitor_t em);

logic_context_t
bpg_rsp_manage_create_op_by_name(
    bpg_rsp_manage_t mgr, logic_context_t ctx/*can be null*/, const char * rsp_name, error_monitor_t em);

void bpg_rsp_manage_free_context(bpg_rsp_manage_t mgr, logic_context_t op_context);

#ifdef __cplusplus
}
#endif

#endif
