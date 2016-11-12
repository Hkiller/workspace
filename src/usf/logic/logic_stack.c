#include <assert.h>
#include "gd/app/app_context.h"
#include "usf/logic/logic_stack.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_data.h"
#include "logic_internal_ops.h"

static void logic_stack_require_clear(logic_stack_node_t stack);

#define LOGIC_STACK_INLINE_ITEM_COUNT \
    ( sizeof(((struct logic_stack*)0)->m_inline_items) \
      / sizeof(((struct logic_stack*)0)->m_inline_items[0]) )

void logic_stack_init(struct logic_stack * stack) {
    stack->m_item_pos = -1;
    stack->m_extern_items = NULL;
    stack->m_extern_items_capacity = 0;
}

void logic_stack_fini(struct logic_stack * stack, logic_context_t context) {
    while(stack->m_item_pos >= 0) {
        struct logic_stack_node * stack_item = logic_stack_node_at(stack, stack->m_item_pos);
        logic_stack_node_data_clear(stack_item);
        logic_stack_require_clear(stack_item);
        --stack->m_item_pos;
    }

    if (stack->m_extern_items) {
        mem_free(context->m_mgr->m_alloc, stack->m_extern_items);
    }
}

void logic_stack_push(struct logic_stack * stack, logic_context_t context, logic_executor_t executor) {
    struct logic_stack_node * stack_item;

REINTER:
    if (stack->m_item_pos + 1 < LOGIC_STACK_INLINE_ITEM_COUNT) {
        stack_item = &stack->m_inline_items[++stack->m_item_pos];
    }
    else {
        int32_t writePos = stack->m_item_pos + 1 - LOGIC_STACK_INLINE_ITEM_COUNT;
        if (writePos >= stack->m_extern_items_capacity) {
            int32_t new_capacity;
            struct logic_stack_node * new_buf;

            new_capacity = stack->m_extern_items_capacity + 16;
            new_buf = (struct logic_stack_node *)mem_alloc(context->m_mgr->m_alloc, sizeof(struct logic_stack_node) * new_capacity);
            if (new_buf == NULL) {
                context->m_errno = -1;
                context->m_state = logic_context_state_error;
                return;
            }

            if (stack->m_extern_items) {
                memcpy(new_buf, stack->m_extern_items, sizeof(struct logic_stack_node) * stack->m_extern_items_capacity);
                mem_free(context->m_mgr->m_alloc, stack->m_extern_items);
            }

            stack->m_extern_items = new_buf;
            stack->m_extern_items_capacity = new_capacity;
        }

        assert(writePos < stack->m_extern_items_capacity);
        stack_item = &stack->m_extern_items[writePos]; 
        ++stack->m_item_pos;
    }

    assert(stack_item);
    stack_item->m_executr = executor;
    stack_item->m_context = context;
    stack_item->m_require_waiting_count = 0;
    stack_item->m_rv = logic_op_exec_result_null;

    TAILQ_INIT(&stack_item->m_datas);
    TAILQ_INIT(&stack_item->m_requires);

    if (executor == NULL) {
        --stack->m_item_pos;
        return;
    }

    switch(executor->m_category) {
    case logic_executor_category_composite: {
        struct logic_executor_composite * composite = (struct logic_executor_composite *)executor;
        switch(composite->m_composite_type) {
        case logic_executor_composite_parallel:
            stack_item->m_rv =
                composite->m_args.m_parallel_policy == logic_executor_parallel_success_on_all
                ? logic_op_exec_result_true
                : logic_op_exec_result_false;
            break;
        case logic_executor_composite_selector:
            stack_item->m_rv = logic_op_exec_result_false;
            break;
        case logic_executor_composite_sequence:
            stack_item->m_rv = logic_op_exec_result_true;
            break;
        }

        if (!TAILQ_EMPTY(&composite->m_members)) {
            executor = TAILQ_FIRST(&composite->m_members);
        }
        else {
            executor = NULL;
        }
        goto REINTER;
    }
    case logic_executor_category_condition: {
        struct logic_executor_condition * condition = (struct logic_executor_condition *)executor;
        stack_item->m_rv = logic_op_exec_result_false;
        executor = condition->m_if;
        goto REINTER;
    }
    case logic_executor_category_decorator: {
        struct logic_executor_decorator * decorator = (struct logic_executor_decorator *)stack_item->m_executr;
        executor = decorator->m_inner;
        goto REINTER;
    }
    case logic_executor_category_action:
    default:
        break;
    }
}

void logic_stack_exec(struct logic_stack * stack, int32_t stop_stack_pos, logic_context_t ctx) {
    while(ctx->m_deleting == 0
          && ctx->m_state == logic_context_state_idle
          && stack->m_item_pos > stop_stack_pos)
    {
        struct logic_stack_node * stack_item = logic_stack_node_at(stack, stack->m_item_pos);

        if (stack_item->m_executr) {
            if (stack_item->m_executr->m_category == logic_executor_category_action) {
                struct logic_executor_action * action = (struct logic_executor_action *)stack_item->m_executr;
                if (action->m_type->m_op) {
                    stack_item->m_rv =
                        ((logic_op_fun_t)action->m_type->m_op)(ctx, stack_item, action->m_type->m_ctx, action->m_args);
                    if (stack_item->m_require_waiting_count > 0 && stack_item->m_rv != logic_op_exec_result_null) {
                        break;
                    }
                }
                else {
                    CPE_ERROR(gd_app_em(ctx->m_mgr->m_app), "logic_stack_exec: action logic op %s have no op!", logic_executor_name(stack_item->m_executr));
                }
            }
        }
        else {
            CPE_ERROR(gd_app_em(ctx->m_mgr->m_app), "stack item have no executor!");
        }

        logic_stack_node_data_clear(stack_item);
        logic_stack_require_clear(stack_item);
        --stack->m_item_pos;

        while(stack->m_item_pos > stop_stack_pos) {
            struct logic_stack_node * stack_item = logic_stack_node_at(stack, stack->m_item_pos);
            struct logic_stack_node * pre_stack_item = logic_stack_node_at(stack, stack->m_item_pos + 1);

            if (stack_item->m_executr->m_category == logic_executor_category_composite) {
                struct logic_executor_composite * composite = (struct logic_executor_composite *)stack_item->m_executr;
                logic_executor_t next = TAILQ_NEXT(pre_stack_item->m_executr, m_next);

                if (pre_stack_item->m_rv == logic_op_exec_result_null) {
                    stack_item->m_rv = logic_op_exec_result_null;
                }
                else {
                    switch(composite->m_composite_type) {
                    case logic_executor_composite_selector:
                        if (pre_stack_item->m_rv == logic_op_exec_result_false) {
                            if (next) logic_stack_push(stack, ctx, next);
                        }
                        else {
                            assert(pre_stack_item->m_rv == logic_op_exec_result_true);
                            stack_item->m_rv = logic_op_exec_result_true;
                        }
                        break;
                    case logic_executor_composite_sequence: {
                        if (pre_stack_item->m_rv == logic_op_exec_result_true) {
                            if (next) logic_stack_push(stack, ctx, next);
                        }
                        else {
                            assert(pre_stack_item->m_rv == logic_op_exec_result_false);
                            stack_item->m_rv = logic_op_exec_result_false;
                        }
                        break;
                    }
                    case logic_executor_composite_parallel: {
                        if (composite->m_args.m_parallel_policy == logic_executor_parallel_success_on_all) {
                            if(pre_stack_item->m_rv == logic_op_exec_result_false) {
                                if (pre_stack_item->m_rv != logic_op_exec_result_null) {
                                    stack_item->m_rv = logic_op_exec_result_false;
                                }
                            }
                        }
                        else {
                            if(pre_stack_item->m_rv == logic_op_exec_result_true) {
                                stack_item->m_rv = logic_op_exec_result_true;
                            }
                        }

                        if (next) {
                            logic_stack_push(stack, ctx, next);
                        }

                        break;
                    }
                    default:
                        break;
                    }
                }
            }
            else if (stack_item->m_executr->m_category == logic_executor_category_decorator) {
                struct logic_executor_decorator * decorator = (struct logic_executor_decorator *)stack_item->m_executr;
                switch(decorator->m_decorator_type) {
                case logic_executor_decorator_protect:
                    stack_item->m_rv = logic_op_exec_result_true;
                    break;
                case logic_executor_decorator_not:
                    if (pre_stack_item->m_rv == logic_op_exec_result_true) {
                        stack_item->m_rv = logic_op_exec_result_false;
                    }
                    else if (pre_stack_item->m_rv == logic_op_exec_result_false) {
                        stack_item->m_rv = logic_op_exec_result_true;
                    }
                    else {
                        stack_item->m_rv = logic_op_exec_result_null;
                    }
                    break;
                }
                --stack->m_item_pos;
                continue;
            }
            else if (stack_item->m_executr->m_category == logic_executor_category_condition) {
                struct logic_executor_condition * condition = (struct logic_executor_condition *)stack_item->m_executr;

                if (condition->m_if == pre_stack_item->m_executr) {
                    if (pre_stack_item->m_rv == logic_op_exec_result_true) {
                        logic_stack_push(stack, ctx, condition->m_do);
                    }
                    else if (pre_stack_item->m_rv == logic_op_exec_result_false) {
                        if (condition->m_else) {
                            logic_stack_push(stack, ctx, condition->m_else);
                        }
                        else {
                            stack_item->m_rv = logic_op_exec_result_false;
                        }
                    }
                    else {
                        stack_item->m_rv = logic_op_exec_result_null;
                    }
                }
                else if (condition->m_do == pre_stack_item->m_executr) {
                    stack_item->m_rv = pre_stack_item->m_rv;
                }
                else if (condition->m_else == pre_stack_item->m_executr) {
                    stack_item->m_rv = pre_stack_item->m_rv;
                }
                else {
                    stack_item->m_rv = logic_op_exec_result_null;
                }
            }

            break;
        }
    }
}

logic_executor_t logic_stack_node_executor(logic_stack_node_t stack) {
    return stack->m_executr;
}

logic_context_t logic_stack_node_context(logic_stack_node_t stack) {
    return stack->m_context;
}

void logic_stack_node_data_clear(logic_stack_node_t stack) {
    while(!TAILQ_EMPTY(&stack->m_datas)) {
        logic_data_free(TAILQ_FIRST(&stack->m_datas));
    }
}

void logic_stack_require_clear(logic_stack_node_t stack) {
    while(!TAILQ_EMPTY(&stack->m_requires)) {
        logic_require_t require = TAILQ_FIRST(&stack->m_requires);
        logic_require_disconnect_to_stack(require);
    }
}

struct logic_stack_require_next_data {
    logic_require_t m_next;
    logic_stack_node_t m_stack;
};

static logic_require_t logic_stack_require_next(struct logic_require_it * it) {
    struct logic_stack_require_next_data * data;
    logic_require_t r;

    assert(sizeof(struct logic_stack_require_next_data) <= sizeof(it->m_data));

    data = (struct logic_stack_require_next_data *)it->m_data;

    if (data->m_next == NULL) return NULL;

    r = data->m_next;
    data->m_next = TAILQ_NEXT(data->m_next, m_next_for_stack);
    return r;
}

void logic_stack_node_requires(logic_stack_node_t stack, logic_require_it_t it) {
    struct logic_stack_require_next_data * data;
    
    assert(sizeof(struct logic_stack_require_next_data) <= sizeof(it->m_data));

    data = (struct logic_stack_require_next_data *)it->m_data;

    data->m_stack = stack;
    data->m_next = TAILQ_FIRST(&stack->m_requires);
    it->next = logic_stack_require_next;
}

int logic_stack_node_have_waiting_require(logic_stack_node_t stack) {
    logic_require_t require;

    TAILQ_FOREACH(require, &stack->m_requires, m_next_for_stack) {
        if (require->m_state == logic_require_state_waiting) return 1;
    }

    return 0;
}

void logic_stack_node_cancel_all_requires(logic_stack_node_t stack) {
    logic_require_t require;

    TAILQ_FOREACH(require, &stack->m_requires, m_next_for_stack) {
        if(require->m_state == logic_require_state_waiting) {
            logic_require_cancel(require);
        }
    }
}
