#ifndef USF_LOGIC_CONTEXT_H
#define USF_LOGIC_CONTEXT_H
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_context_t
logic_context_create(
    logic_manage_t mgr,
    logic_require_id_t id,
    size_t capacity);

logic_context_t
logic_context_create_ex(
    logic_manage_t mgr,
    logic_require_id_t id,
    size_t capacity,
    tl_time_span_t timeout);

void logic_context_free(logic_context_t context);

logic_context_t logic_context_find(logic_manage_t mgr, logic_context_id_t id);

logic_context_id_t logic_context_id(logic_context_t context);
logic_context_state_t logic_context_state(logic_context_t context);
logic_manage_t logic_context_mgr(logic_context_t context);
gd_app_context_t logic_context_app(logic_context_t context);
int32_t logic_context_errno(logic_context_t context);
void logic_context_errno_set(logic_context_t context, int32_t v);
size_t logic_context_capacity(logic_context_t context);
void * logic_context_data(logic_context_t context);

const char * logic_context_state_name(logic_context_state_t state);

void logic_context_set_commit(logic_context_t context, logic_context_commit_fun_t op, void * ctx);

int logic_context_timeout_is_start(logic_context_t context);
int logic_context_timeout_start(logic_context_t context, tl_time_span_t timeout_ms);
void logic_context_timeout_stop(logic_context_t context);

uint32_t logic_context_flags(logic_context_t context);
void logic_context_flags_set(logic_context_t context, uint32_t flag);
void logic_context_flag_enable(logic_context_t context, logic_context_flag_t flag);
void logic_context_flag_disable(logic_context_t context, logic_context_flag_t flag);
int logic_context_flag_is_enable(logic_context_t context, logic_context_flag_t flag);

void logic_context_data_dump_to_cfg(logic_context_t context, cfg_t cfg);

int logic_context_bind(logic_context_t context, logic_executor_t executor);
void logic_context_execute(logic_context_t context);
logic_queue_t logic_context_queue(logic_context_t context);

void logic_context_cancel(logic_context_t context);
void logic_context_timeout(logic_context_t context);

#ifdef __cplusplus
}
#endif

#endif
