#ifndef USF_LOGIC_INTERNAL_OPS_H
#define USF_LOGIC_INTERNAL_OPS_H
#include "logic_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*context ops*/
uint32_t logic_context_hash(const struct logic_context * context);
int logic_context_cmp(const struct logic_context * l, const struct logic_context * r);
void logic_context_free_all(logic_manage_t mgr);
void logic_context_do_state_change(logic_context_t context, logic_context_state_t old_sate);

void logic_context_dequeue(logic_context_t context);
void logic_context_enqueue(logic_context_t context, enum logic_context_queue_state queue_type);

#define logic_context_state_i(context)                  \
    (context->m_errno                                   \
     ? logic_context_state_error                        \
     : ((context->m_state == logic_context_state_idle   \
         && context->m_require_waiting_count)           \
         ? logic_context_state_waiting                  \
         : context->m_state))

/*require ops*/
uint32_t logic_require_hash(const struct logic_require * require);
int logic_require_cmp(const struct logic_require * l, const struct logic_require * r);
void logic_require_free_all(logic_manage_t mgr);

/*queue ops*/
uint32_t logic_queue_hash(const struct logic_queue * queue);
int logic_queue_cmp(const struct logic_queue * l, const struct logic_queue * r);
void logic_queue_free_all(logic_manage_t mgr);

/*data ops*/
uint32_t logic_data_hash(const struct logic_data * data);
int logic_data_cmp(const struct logic_data * l, const struct logic_data * r);
void logic_data_free_all(logic_manage_t mgr);

/*stack ops*/
void logic_stack_init(struct logic_stack * stack);
void logic_stack_fini(struct logic_stack * stack, logic_context_t context);
void logic_stack_push(struct logic_stack * stack, logic_context_t context, logic_executor_t executor);
    
void logic_stack_exec(struct logic_stack * stack, int32_t stop_stack_pos, logic_context_t ctx);

/*executor type ops*/
uint32_t logic_executor_type_hash(const struct logic_executor_type * type);
int logic_executor_type_cmp(const struct logic_executor_type * l, const struct logic_executor_type * r);
void logic_executor_type_free_all(logic_executor_type_group_t group);

/*executor type ops*/
uint32_t logic_executor_ref_hash(const struct logic_executor_ref * type);
int logic_executor_ref_cmp(const struct logic_executor_ref * l, const struct logic_executor_ref * r);
void logic_executor_ref_free_all(logic_executor_mgr_t mgr);

logic_executor_ref_t logic_executor_ref_create(logic_executor_mgr_t mgr, const char * name, logic_executor_t executor);

#define logic_stack_node_at(stack, pos)                                 \
    ((pos) < LOGIC_STACK_INLINE_ITEM_COUNT                              \
     ? &stack->m_inline_items[(pos)]                                    \
     : &stack->m_extern_items[(pos) - LOGIC_STACK_INLINE_ITEM_COUNT])   \

#ifdef __cplusplus
}
#endif

#endif
