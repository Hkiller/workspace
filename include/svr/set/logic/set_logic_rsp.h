#ifndef SVR_SET_LOGIC_RSP_H
#define SVR_SET_LOGIC_RSP_H
#include "set_logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

set_logic_rsp_t set_logic_rsp_create(set_logic_rsp_manage_t mgr, const char * name);
void set_logic_rsp_free(set_logic_rsp_t rsp);

set_logic_rsp_t set_logic_rsp_find(set_logic_rsp_manage_t mgr, const char * name);

dp_rsp_t set_logic_rsp_dp(set_logic_rsp_t rsp);

logic_executor_t set_logic_rsp_executor(set_logic_rsp_t rsp);
void set_logic_rsp_set_executor(set_logic_rsp_t rsp, logic_executor_ref_t executor);

const char * set_logic_rsp_queue(set_logic_rsp_t rsp);
int set_logic_rsp_set_queue(set_logic_rsp_t rsp, const char * queue_name);

const char * set_logic_rsp_name(set_logic_rsp_t rsp);
tl_time_span_t set_logic_rsp_timeout_ms(set_logic_rsp_t rsp);
void set_logic_rsp_set_timeout_ms(set_logic_rsp_t rsp, tl_time_span_t timeout_ms);

uint32_t set_logic_rsp_flags(set_logic_rsp_t rsp);
void set_logic_rsp_flags_set(set_logic_rsp_t rsp, uint32_t flag);
void set_logic_rsp_flag_enable(set_logic_rsp_t rsp, set_logic_rsp_flag_t flag);
void set_logic_rsp_flag_disable(set_logic_rsp_t rsp, set_logic_rsp_flag_t flag);
int set_logic_rsp_flag_is_enable(set_logic_rsp_t rsp, set_logic_rsp_flag_t flag);

int set_logic_rsp_build(set_logic_rsp_manage_t mgr, cfg_t cfg, LPDRMETALIB metalib, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
