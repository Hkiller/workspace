#ifndef USF_LOGIC_REQUIRE_H
#define USF_LOGIC_REQUIRE_H
#include "cpe/utils/hash_string.h"
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_require_t
logic_require_create(logic_stack_node_t stack, const char * require_name);
void logic_require_free(logic_require_t require);

logic_require_t
logic_require_find(logic_manage_t mgr, logic_require_id_t id);

logic_require_id_t logic_require_id(logic_require_t require);
const char * logic_require_name(logic_require_t require);
logic_manage_t logic_require_mgr(logic_require_t require);
logic_context_t logic_require_context(logic_require_t require);
logic_stack_node_t logic_require_stack(logic_require_t require);

logic_require_state_t logic_require_state(logic_require_t require);

int logic_require_timeout_is_start(logic_require_t require);
int logic_require_timeout_start(logic_require_t require, tl_time_span_t timeout_ms);
void logic_require_timeout_stop(logic_require_t require);

logic_require_state_t
logic_require_state(logic_require_t require);

const char * logic_require_state_name(logic_require_state_t state);

logic_stack_node_t logic_require_stack(logic_require_t require);
void logic_require_disconnect_to_stack(logic_require_t require);

int32_t logic_require_error(logic_require_t require);

void logic_require_set_done(logic_require_t require);
void logic_require_set_error(logic_require_t require);
void logic_require_set_timeout(logic_require_t require);
void logic_require_set_error_ex(logic_require_t require, int32_t err);
void logic_require_cancel(logic_require_t require);

void logic_require_data_dump_to_cfg(logic_require_t require, cfg_t cfg);

#define logic_require_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

