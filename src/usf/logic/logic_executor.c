#include <assert.h>
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_context.h"
#include "logic_internal_ops.h"

logic_executor_t
logic_executor_action_create(logic_manage_t mgr, logic_executor_type_t type, cfg_t args) {
    struct logic_executor_action * executor;
    cfg_t executor_args;

    assert(mgr);

    if (type == NULL) return NULL;

    executor_args = NULL;
    if (args) {
        if (cfg_type(args) != CPE_CFG_TYPE_STRUCT) return NULL;
        executor_args = cfg_create(mgr->m_alloc);
        if (executor_args == NULL) return NULL;

        if (cfg_merge(executor_args, args, cfg_replace, NULL) != 0) {
            cfg_free(executor_args);
            return NULL;
        }
    }

    executor = (struct logic_executor_action *)mem_alloc(mgr->m_alloc, sizeof(struct logic_executor_action));
    if (executor == NULL) {
        if (executor_args) cfg_free(executor_args);
        return NULL;
    }

    executor->m_mgr = mgr;
    executor->m_category = logic_executor_category_action;
    executor->m_type = type;
    executor->m_args = executor_args;

    return (logic_executor_t)executor;
}

logic_executor_t logic_executor_condition_create(logic_manage_t mgr) {
    struct logic_executor_condition * executor;

    assert(mgr);

    executor = (struct logic_executor_condition *)mem_alloc(mgr->m_alloc, sizeof(struct logic_executor_condition));
    if (executor == NULL) return NULL;

    executor->m_mgr = mgr;
    executor->m_category = logic_executor_category_condition;
    executor->m_if = NULL;
    executor->m_do = NULL;
    executor->m_else = NULL;

    return (logic_executor_t)executor;
}

int logic_executor_condition_set_if(logic_executor_t executor, logic_executor_t check) {
    struct logic_executor_condition * condition;

    if (executor->m_category != logic_executor_category_condition) return -1;

    condition = (struct logic_executor_condition *)executor;

    if (condition->m_if) logic_executor_free(condition->m_if);
    condition->m_if = check;

    return 0;
}

int logic_executor_condition_set_do(logic_executor_t executor, logic_executor_t action) {
    struct logic_executor_condition * condition;

    if (executor->m_category != logic_executor_category_condition) return -1;

    condition = (struct logic_executor_condition *)executor;

    if (condition->m_do) logic_executor_free(condition->m_do);
    condition->m_do = action;

    return 0;
}

int logic_executor_condition_set_else(logic_executor_t executor, logic_executor_t action) {
    struct logic_executor_condition * condition;

    if (executor->m_category != logic_executor_category_condition) return -1;

    condition = (struct logic_executor_condition *)executor;

    if (condition->m_else) logic_executor_free(condition->m_else);
    condition->m_else = action;

    return 0;
}

logic_executor_t logic_executor_condition_if(logic_executor_t executor) {
    struct logic_executor_condition * condition;

    if (executor->m_category != logic_executor_category_condition) return NULL;

    condition = (struct logic_executor_condition *)executor;

    return condition->m_if;
}

logic_executor_t logic_executor_condition_do(logic_executor_t executor) {
    struct logic_executor_condition * condition;

    if (executor->m_category != logic_executor_category_condition) return NULL;

    condition = (struct logic_executor_condition *)executor;

    return condition->m_do;
}

logic_executor_t logic_executor_condition_else(logic_executor_t executor) {
    struct logic_executor_condition * condition;

    if (executor->m_category != logic_executor_category_condition) return NULL;

    condition = (struct logic_executor_condition *)executor;

    return condition->m_else;
}

logic_executor_t
logic_executor_composite_create(logic_manage_t mgr, logic_executor_composite_type_t composite_type) {
    struct logic_executor_composite * executor;

    assert(mgr);

    executor = (struct logic_executor_composite *)mem_alloc(mgr->m_alloc, sizeof(struct logic_executor_composite));
    if (executor == NULL) return NULL;

    executor->m_mgr = mgr;
    executor->m_category = logic_executor_category_composite;
    executor->m_composite_type = composite_type;
    if(composite_type == logic_executor_composite_parallel) {
        executor->m_args.m_parallel_policy = logic_executor_parallel_success_on_all;
    }

    TAILQ_INIT(&executor->m_members);

    return (logic_executor_t)executor;
}

int logic_executor_composite_add(logic_executor_t input_composite, logic_executor_t member) {
    struct logic_executor_composite * composite;

    assert(input_composite);
    if (member == NULL) return -1;
    if (input_composite->m_category != logic_executor_category_composite) return -1;

    composite = (struct logic_executor_composite *)input_composite;

    TAILQ_INSERT_TAIL(&composite->m_members, member, m_next);

    return 0;
}

int logic_executor_composite_parallel_set_policy(logic_executor_t parallel, logic_executor_parallel_policy_t policy) {
    struct logic_executor_composite * composite;

    if (parallel->m_category != logic_executor_category_composite) return -1;
    
    composite = (struct logic_executor_composite *)parallel;
    if (composite->m_composite_type != logic_executor_composite_parallel) return -1;

    composite->m_args.m_parallel_policy = policy;

    return 0;
}

logic_executor_t
logic_executor_decorator_create(logic_manage_t mgr, logic_executor_decorator_type_t decorator_type, logic_executor_t inner) {
    struct logic_executor_decorator * executor;

    assert(mgr);
    if (inner == NULL) return NULL;

    executor = (struct logic_executor_decorator *)mem_alloc(mgr->m_alloc, sizeof(struct logic_executor_decorator));
    if (executor == NULL) return NULL;

    executor->m_mgr = mgr;
    executor->m_category = logic_executor_category_decorator;
    executor->m_decorator_type = decorator_type;
    executor->m_inner = inner;

    return (logic_executor_t)executor;
}

void logic_executor_free(logic_executor_t executor) {
    if (executor == NULL) return;

    switch(executor->m_category) {
    case logic_executor_category_action: {
        struct logic_executor_action * action = (struct logic_executor_action *)executor;
        if (action->m_args) cfg_free(action->m_args);
        mem_free(action->m_mgr->m_alloc, action);
        break;
    }
    case logic_executor_category_composite: {
        logic_executor_t member;
        struct logic_executor_composite * composite = (struct logic_executor_composite *)executor;

        while(!TAILQ_EMPTY(&composite->m_members)) {
            member = TAILQ_FIRST(&composite->m_members);
            TAILQ_REMOVE(&composite->m_members, member, m_next);
            logic_executor_free(member);
        }

        mem_free(composite->m_mgr->m_alloc, composite);
        break;
    }
    case logic_executor_category_decorator: {
        struct logic_executor_decorator * decorator = (struct logic_executor_decorator *)executor;
        logic_executor_free(decorator->m_inner);
        mem_free(decorator->m_mgr->m_alloc, decorator);
        break;
    }
    case logic_executor_category_condition: {
        struct logic_executor_condition * condition = (struct logic_executor_condition *)executor;
        if (condition->m_if) logic_executor_free(condition->m_if);
        if (condition->m_do) logic_executor_free(condition->m_do);
        if (condition->m_else) logic_executor_free(condition->m_else);
        mem_free(condition->m_mgr->m_alloc, condition);
    }
    }
}

const char * logic_executor_name(logic_executor_t executor) {
    switch(executor->m_category) {
    case logic_executor_category_action:
        return ((struct logic_executor_action *)executor)->m_type->m_name;
    case logic_executor_category_condition:
        return "condition";
    case logic_executor_category_decorator:
        switch (((struct logic_executor_decorator *)executor)->m_decorator_type) {
        case logic_executor_decorator_protect:
            return "protect";
        case logic_executor_decorator_not:
            return "not";
        default:
            return "unknown-decorator-type";
        }
    case logic_executor_category_composite:
        switch (((struct logic_executor_composite *)executor)->m_composite_type) {
        case logic_executor_composite_selector:
            return "selector";
        case logic_executor_composite_sequence:
            return "sequence";
        case logic_executor_composite_parallel:
            return "parallel";
        default:
            return "unknown-composite-type";
        }
    default:
        return "unknown-executor-category";
    }
}

logic_executor_t logic_executor_clone(logic_manage_t mgr, logic_executor_t executor) {
    assert(executor);

    switch(executor->m_category) {
    case logic_executor_category_action: {
        struct logic_executor_action * action = (struct logic_executor_action *)(executor);
        return logic_executor_action_create(mgr, action->m_type, action->m_args);
    }
    case logic_executor_category_composite: {
        struct logic_executor_composite * composite;
        logic_executor_t member;
        logic_executor_t r;

        composite  = (struct logic_executor_composite*)(executor);

        r = logic_executor_composite_create(mgr, composite->m_composite_type);
        if (r == NULL) return NULL;

        TAILQ_FOREACH(member, &composite->m_members, m_next) {
            logic_executor_t member_r = logic_executor_clone(mgr, member);
            if (member_r == NULL || logic_executor_composite_add(r, member_r) != 0) {
                logic_executor_free(r);
                return NULL;
            }
        }

        ((struct logic_executor_composite *)r)->m_args = composite->m_args;
        
        return r;
    }
    case logic_executor_category_condition: {
        struct logic_executor_condition * condition;
        logic_executor_t r;

        condition  = (struct logic_executor_condition*)(executor);

        r = logic_executor_condition_create(mgr);
        if (r == NULL) return NULL;

        if (condition->m_if) {
            logic_executor_t if_r = logic_executor_clone(mgr, condition->m_if);
            if (if_r == NULL || logic_executor_condition_set_if(r, if_r) != 0) {
                logic_executor_free(r);
                return NULL;
            }
        }

        if (condition->m_do) {
            logic_executor_t do_r = logic_executor_clone(mgr, condition->m_do);
            if (do_r == NULL || logic_executor_condition_set_do(r, do_r) != 0) {
                logic_executor_free(r);
                return NULL;
            }
        }

        if (condition->m_else) {
            logic_executor_t else_r = logic_executor_clone(mgr, condition->m_else);
            if (else_r == NULL || logic_executor_condition_set_else(r, else_r) != 0) {
                logic_executor_free(r);
                return NULL;
            }
        }

        return r;
    }
    case logic_executor_category_decorator: {
        struct logic_executor_decorator * decorator;
        logic_executor_t r;
        logic_executor_t inner;

        decorator  = (struct logic_executor_decorator*)(executor);

        inner = logic_executor_clone(mgr, decorator->m_inner);
        if (inner == NULL) return NULL;

        r = logic_executor_decorator_create(mgr, decorator->m_decorator_type, inner);
        if (r == NULL) {
            logic_executor_free(inner);
            return NULL;
        }

        return r;
    }
    }

    return NULL;
}

void logic_executor_dump(logic_executor_t executor, write_stream_t stream, int level) {
    if (executor == NULL) return;

    switch(executor->m_category) {
    case logic_executor_category_action: {
        struct logic_executor_action * action = (struct logic_executor_action *)executor;
        stream_putc_count(stream, ' ', level << 2);
        if (action->m_args) {
            stream_printf(stream, "%s: ", logic_executor_name(executor));
            cfg_print_inline(action->m_args, stream);
        }
        else {
            stream_printf(stream, "%s", logic_executor_name(executor));
        }
        break;
    }
    case logic_executor_category_composite: {
        logic_executor_t member;
        struct logic_executor_composite * composite = (struct logic_executor_composite *)executor;
        stream_putc_count(stream, ' ', level << 2);
        stream_printf(stream, "%s:", logic_executor_name(executor));

        if (composite->m_composite_type == logic_executor_composite_parallel) {
            stream_putc(stream, '\n');
            stream_putc_count(stream, ' ', (level + 1) << 2);
            stream_printf(
                stream, "policy: %s",
                composite->m_args.m_parallel_policy == logic_executor_parallel_success_on_all
                ? "SUCCESS_ON_ALL"
                : "SUCCESS_ON_ONE");

            stream_putc(stream, '\n');
            stream_putc_count(stream, ' ', (level + 1) << 2);
            stream_printf(stream, "childs:");

            TAILQ_FOREACH(member, &composite->m_members, m_next) {
                stream_putc(stream, '\n');
                logic_executor_dump(member, stream, level + 2);
            }
        }
        else {
            TAILQ_FOREACH(member, &composite->m_members, m_next) {
                stream_putc(stream, '\n');
                logic_executor_dump(member, stream, level + 1);
            }
        }

        break;
    }
    case logic_executor_category_decorator: {
        struct logic_executor_decorator * decorator = (struct logic_executor_decorator *)executor;
        stream_putc_count(stream, ' ', level << 2);
        stream_printf(stream, "%s:", logic_executor_name(executor));
        stream_putc(stream, '\n');
        logic_executor_dump(decorator->m_inner, stream, level + 1);
        break;
    }
    case logic_executor_category_condition: {
        struct logic_executor_condition * condition = (struct logic_executor_condition *)executor;

        stream_putc_count(stream, ' ', level << 2);
        stream_printf(stream, "%s:", logic_executor_name(executor));

        if (condition->m_if) {
            stream_putc(stream, '\n');
            stream_putc_count(stream, ' ', (level + 1) << 2);
            stream_printf(stream, "if:\n");
            logic_executor_dump(condition->m_if, stream, level + 2);
        }

        if (condition->m_do) {
            stream_putc(stream, '\n');
            stream_putc_count(stream, ' ', (level + 1) << 2);
            stream_printf(stream, "do:\n");
            logic_executor_dump(condition->m_do, stream, level + 2);
        }

        if (condition->m_else) {
            stream_putc(stream, '\n');
            stream_putc_count(stream, ' ', (level + 1) << 2);
            stream_printf(stream, "else:\n");
            logic_executor_dump(condition->m_else, stream, level + 2);
        }

        break;
    }
    default:
        stream_putc_count(stream, ' ', level << 2);
        stream_printf(stream, "%s", logic_executor_name(executor));
        break;
    }
}
