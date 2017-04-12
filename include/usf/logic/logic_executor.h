#ifndef USF_LOGIC_OP_EXECUTOR_H
#define USF_LOGIC_OP_EXECUTOR_H
#include "cpe/utils/stream.h"
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*action*/
logic_executor_t
logic_executor_action_create(logic_manage_t mgr, logic_executor_type_t type, cfg_t args);

/*condition*/
logic_executor_t logic_executor_condition_create(logic_manage_t mgr);
int logic_executor_condition_set_if(logic_executor_t condition, logic_executor_t check);
int logic_executor_condition_set_do(logic_executor_t condition, logic_executor_t action);
int logic_executor_condition_set_else(logic_executor_t condition, logic_executor_t action);

logic_executor_t logic_executor_condition_if(logic_executor_t condition);
logic_executor_t logic_executor_condition_do(logic_executor_t condition);
logic_executor_t logic_executor_condition_else(logic_executor_t condition);

/*composite*/
logic_executor_t
logic_executor_composite_create(logic_manage_t mgr, logic_executor_composite_type_t composite_type);

int logic_executor_composite_add(logic_executor_t composite, logic_executor_t member);
int logic_executor_composite_parallel_set_policy(logic_executor_t parallel, logic_executor_parallel_policy_t policy);

/*decorator*/
logic_executor_t
logic_executor_decorator_create(logic_manage_t mgr, logic_executor_decorator_type_t decorator_type, logic_executor_t inner);

/*common operations*/
const char * logic_executor_name(logic_executor_t executor);
void logic_executor_free(logic_executor_t executor);
void logic_executor_dump(logic_executor_t executor, write_stream_t stream, int level);

logic_executor_t
logic_executor_clone(logic_manage_t mgr, logic_executor_t executor);

#ifdef __cplusplus
}
#endif

#endif

