#ifndef USF_BPG_RSP_H
#define USF_BPG_RSP_H
#include "bpg_rsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bpg_rsp_t bpg_rsp_create(bpg_rsp_manage_t mgr, const char * name);
void bpg_rsp_free(bpg_rsp_t rsp);

bpg_rsp_t bpg_rsp_find(bpg_rsp_manage_t mgr, const char * name);

dp_rsp_t bpg_rsp_dp(bpg_rsp_t rsp);

logic_executor_t bpg_rsp_executor(bpg_rsp_t rsp);
void bpg_rsp_set_executor(bpg_rsp_t rsp, logic_executor_ref_t executor);

const char * bpg_rsp_queue(bpg_rsp_t rsp);
int bpg_rsp_set_queue(bpg_rsp_t rsp, const char * queue_name);

const char * bpg_rsp_name(bpg_rsp_t rsp);
tl_time_span_t bpg_rsp_timeout_ms(bpg_rsp_t rsp);
void bpg_rsp_set_timeout_ms(bpg_rsp_t rsp, tl_time_span_t timeout_ms);

uint32_t bpg_rsp_flags(bpg_rsp_t rsp);
void bpg_rsp_flags_set(bpg_rsp_t rsp, uint32_t flag);
void bpg_rsp_flag_enable(bpg_rsp_t rsp, bpg_rsp_flag_t flag);
void bpg_rsp_flag_disable(bpg_rsp_t rsp, bpg_rsp_flag_t flag);
int bpg_rsp_flag_is_enable(bpg_rsp_t rsp, bpg_rsp_flag_t flag);

int bpg_rsp_build(bpg_rsp_manage_t mgr, cfg_t cfg, LPDRMETALIB metalib, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
