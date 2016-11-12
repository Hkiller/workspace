#ifndef USF_LOGIC_STACK_H
#define USF_LOGIC_STACK_H
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_executor_t logic_stack_node_executor(logic_stack_node_t stack);
logic_context_t logic_stack_node_context(logic_stack_node_t stack);
void logic_stack_node_data_clear(logic_stack_node_t stack);

void logic_stack_node_requires(logic_stack_node_t stack, logic_require_it_t it);
int logic_stack_node_have_waiting_require(logic_stack_node_t stack);
void logic_stack_node_cancel_all_requires(logic_stack_node_t stack);

#ifdef __cplusplus
}
#endif

#endif
