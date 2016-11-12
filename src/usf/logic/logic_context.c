#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/app_log.h"
#include "gd/timer/timer_manage.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_queue.h"
#include "logic_internal_ops.h"

logic_context_t
logic_context_create(logic_manage_t mgr, logic_context_id_t id, size_t capacity) {
    return logic_context_create_ex(mgr, id, capacity, mgr->m_context_timout_ms);
}

logic_context_t
logic_context_create_ex(logic_manage_t mgr, logic_context_id_t id, size_t capacity, tl_time_span_t timeout) {
    char * buf;
    logic_context_t context;

    buf = mem_alloc(mgr->m_alloc, sizeof(struct logic_context) + capacity);
    if (buf == NULL) return NULL;

    context = (logic_context_t)buf;

    context->m_mgr = mgr;

    TAILQ_INIT(&context->m_requires);
    cpe_hash_entry_init(&context->m_hh);

    if (id != INVALID_LOGIC_CONTEXT_ID) {
        context->m_id = id;
        if (cpe_hash_table_insert_unique(&mgr->m_contexts, context) != 0) {
            mem_free(mgr->m_alloc, buf);
            return NULL;
        }
    }
    else {
        int id_try_count;
        for(id_try_count = 0; id_try_count < 2000; ++id_try_count) {
            context->m_id = context->m_mgr->m_context_id++;
            if (cpe_hash_table_insert_unique(&mgr->m_contexts, context) == 0) {
                break;
            }
        }

        if (id_try_count >= 2000) {
            mem_free(mgr->m_alloc, buf);
            return NULL;
        }
    }

    context->m_errno = 0;
    context->m_state = logic_context_state_init;
    context->m_capacity = capacity;
    context->m_timer_id = GD_TIMER_ID_INVALID;
    context->m_flags = 0;
    context->m_runing = 0;
    context->m_deleting = 0;
    context->m_commit_op = NULL;
    context->m_commit_ctx = NULL;
    context->m_require_waiting_count = 0;
    context->m_queue_state = logic_context_queue_none;
    context->m_logic_queue = NULL;

    logic_stack_init(&context->m_stack);
    TAILQ_INIT(&context->m_datas);

    logic_context_enqueue(context, logic_context_queue_pending);

    if (mgr->m_timer_mgr && timeout > 0) {
        if (logic_context_timeout_start(context, timeout) != 0) {
            logic_context_free(context);
            return NULL;
        }
    }

    if (context->m_mgr->m_debug >= 3) {
        APP_CTX_INFO(
            context->m_mgr->m_app, "%s: context "FMT_UINT32_T" create",
            logic_manage_name(context->m_mgr), logic_context_id(context));
    }

    return context;
}

void logic_context_free(logic_context_t context) {
    assert(context);

    if (context->m_runing) {
        context->m_deleting = 1;
        return;
    }

    logic_context_timeout_stop(context);

    while(!TAILQ_EMPTY(&context->m_requires)) {
        logic_require_free(TAILQ_FIRST(&context->m_requires));
    }

    while(!TAILQ_EMPTY(&context->m_datas)) {
        logic_data_free(TAILQ_FIRST(&context->m_datas));
    }

    logic_stack_fini(&context->m_stack, context);

    cpe_hash_table_remove_by_ins(&context->m_mgr->m_contexts, context);

    if (context->m_logic_queue) {
        logic_queue_dequeue(context->m_logic_queue, context);
        assert(context->m_logic_queue == NULL);
    }

    logic_context_dequeue(context);

    if (context->m_mgr->m_debug >= 3) {
        APP_CTX_INFO(
            context->m_mgr->m_app, "%s: context "FMT_UINT32_T" free",
            logic_manage_name(context->m_mgr), logic_context_id(context));
    }

    mem_free(context->m_mgr->m_alloc, context);
}

void logic_context_free_all(logic_manage_t mgr) {
    struct cpe_hash_it context_it;
    logic_context_t context;

    cpe_hash_it_init(&context_it, &mgr->m_contexts);

    context = cpe_hash_it_next(&context_it);
    while (context) {
        logic_context_t next = cpe_hash_it_next(&context_it);
        logic_context_free(context);
        context = next;
    }
}

logic_context_t
logic_context_find(logic_manage_t mgr, logic_context_id_t id) {
    struct logic_context key;

    key.m_id = id;
    return (logic_context_t)cpe_hash_table_find(&mgr->m_contexts, &key);
}

logic_context_id_t
logic_context_id(logic_context_t context) {
    return context->m_id;
}

int32_t logic_context_errno(logic_context_t context) {
    return context->m_errno;
}

void logic_context_errno_set(logic_context_t context, int32_t v) {
    context->m_errno = v;
}

void logic_context_set_commit(
    logic_context_t context,
    logic_context_commit_fun_t op,
    void * ctx)
{
    context->m_commit_op = op;
    context->m_commit_ctx = ctx;
}

logic_manage_t
logic_context_mgr(logic_context_t context) {
    return context->m_mgr;
}

gd_app_context_t
logic_context_app(logic_context_t context) {
    return context->m_mgr->m_app;
}

logic_context_state_t
logic_context_state(logic_context_t context) {
    return logic_context_state_i(context);
}

size_t logic_context_capacity(logic_context_t context) {
    return context->m_capacity;
}

void * logic_context_data(logic_context_t context) {
    return context + 1;
}

uint32_t logic_context_hash(const struct logic_context * context) {
    return context->m_id;
}

int logic_context_cmp(const struct logic_context * l, const struct logic_context * r) {
    return l->m_id == r->m_id;
}

uint32_t logic_context_flags(logic_context_t context) {
    return context->m_flags;
}

void logic_context_flags_set(logic_context_t context, uint32_t flag) {
    context->m_flags = flag;
}

void logic_context_flag_enable(logic_context_t context, logic_context_flag_t flag) {
    context->m_flags |= flag;
}

void logic_context_flag_disable(logic_context_t context, logic_context_flag_t flag) {
    context->m_flags &= ~((uint32_t)flag);
}

int logic_context_flag_is_enable(logic_context_t context, logic_context_flag_t flag) {
    return context->m_flags & flag;
}

void logic_context_data_dump_to_cfg(logic_context_t context, cfg_t cfg) {
    logic_data_t data;

    TAILQ_FOREACH(data, &context->m_datas, m_next) {
        cfg_t data_node;
        data_node = cfg_struct_add_struct(cfg, dr_meta_name(data->m_meta), cfg_replace);

        dr_cfg_write(data_node, logic_data_data(data), data->m_meta, NULL);
    }
}

logic_queue_t logic_context_queue(logic_context_t context) {
    return context->m_logic_queue;
}

void logic_context_execute(logic_context_t context) {
    logic_context_state_t old_state;
    logic_context_state_t new_state;

    if (context->m_state != logic_context_state_idle) return;
    assert(context->m_runing == 0);

    logic_context_dequeue(context);

    old_state = logic_context_state_i(context);

    context->m_runing = 1;
    logic_stack_exec(&context->m_stack, -1, context);
    context->m_runing = 0;

    if (context->m_deleting) {
        logic_context_free(context);
        return;
    }

    if (context->m_errno == 0 && context->m_stack.m_item_pos == -1) {
        if (context->m_stack.m_inline_items[0].m_rv == logic_op_exec_result_null) {
            context->m_errno = -1;
            context->m_state = logic_context_state_error;
        }
        else {
            context->m_state = logic_context_state_done; 
        }
    }

    new_state = context->m_state;

    /*logic_context_do_state_change may free context!!!*/
    logic_context_do_state_change(context, old_state);

    if (new_state == logic_context_state_waiting) {
        if (context->m_queue_state == logic_context_queue_none)
        logic_context_enqueue(context, logic_context_queue_waiting);
    }
}

int logic_context_bind(logic_context_t context, logic_executor_t executor) {
    if (executor == NULL) return -1;
    if (context->m_state != logic_context_state_init) return -1;

    logic_stack_push(&context->m_stack, context, executor);
    context->m_state = logic_context_state_idle;
    return 0;
}

void logic_context_cancel(logic_context_t context) {
    logic_context_state_t old_state;

    old_state = logic_context_state_i(context);
    context->m_state = logic_context_state_cancel;

    logic_context_do_state_change(context, old_state);
}

void logic_context_timeout(logic_context_t context) {
    logic_context_state_t old_state;

    old_state = logic_context_state_i(context);
    context->m_state = logic_context_state_timeout;

    logic_context_do_state_change(context, old_state);
}

void logic_context_do_state_change(logic_context_t context, logic_context_state_t old_sate) {
    logic_context_state_t cur_state;

    if (context->m_runing) return;

    if (old_sate > logic_context_state_idle) return;

    cur_state = logic_context_state_i(context);
    if (cur_state < logic_context_state_idle) return;

    if (cur_state == logic_context_state_idle) {
        if (context->m_flags & logic_context_flag_execute_immediately) {
            logic_context_execute(context);
        }
        else {
            logic_context_enqueue(context, logic_context_queue_pending);
        }
    }
    else {
        if (context->m_commit_op) {
            context->m_commit_op(context, context->m_commit_ctx);
        }
        else {
            logic_context_free(context);
        }
    }
}

int logic_context_timeout_is_start(logic_context_t context) {
    return context->m_timer_id != GD_TIMER_ID_INVALID;
}

static void logic_context_do_timeout(void * ctx, gd_timer_id_t timer_id, void * arg) {
    logic_context_t context = ctx;

    APP_CTX_ERROR(
        context->m_mgr->m_app, "%s: context "FMT_UINT32_T" timeout",
        logic_manage_name(context->m_mgr), logic_context_id(context));

    logic_context_timeout(context);
}

void logic_context_timeout_stop(logic_context_t context) {
    logic_manage_t mgr;

    if (context->m_timer_id != GD_TIMER_ID_INVALID) {
        mgr = context->m_mgr;
        assert(mgr->m_timer_mgr);

        gd_timer_mgr_unregist_timer_by_id(mgr->m_timer_mgr, context->m_timer_id);
        context->m_timer_id = GD_TIMER_ID_INVALID;
    }
}

int logic_context_timeout_start(logic_context_t context, tl_time_span_t timeout_ms) {
    logic_manage_t mgr;

    mgr = context->m_mgr;
    if (mgr->m_timer_mgr == NULL) return -1;

    logic_context_timeout_stop(context);

    if (gd_timer_mgr_regist_timer(
            mgr->m_timer_mgr,
            &context->m_timer_id,
            logic_context_do_timeout, context, NULL, NULL, timeout_ms, timeout_ms, 1) != 0)
    {
        context->m_timer_id = GD_TIMER_ID_INVALID;
        return -1;
    }

    assert(context->m_timer_id != GD_TIMER_ID_INVALID);
    return 0;
}


void logic_context_dequeue(logic_context_t context) {
    switch(context->m_queue_state) {
    case logic_context_queue_waiting:
        TAILQ_REMOVE(&context->m_mgr->m_waiting_contexts, context, m_next);
        context->m_queue_state = logic_context_queue_none;
        --context->m_mgr->m_waiting_count;
        break;
    case logic_context_queue_pending:
        TAILQ_REMOVE(&context->m_mgr->m_pending_contexts, context, m_next);
        context->m_queue_state = logic_context_queue_none;
        --context->m_mgr->m_pending_count;
        break;
    case logic_context_queue_none:
        break;
    }
}

void logic_context_enqueue(logic_context_t context, enum logic_context_queue_state queue_type) {
    logic_context_dequeue(context);

    switch(queue_type) {
    case logic_context_queue_waiting:
        TAILQ_INSERT_TAIL(&context->m_mgr->m_waiting_contexts, context, m_next);
        context->m_queue_state = queue_type;
        ++context->m_mgr->m_waiting_count;
        break;
    case logic_context_queue_pending:
        TAILQ_INSERT_TAIL(&context->m_mgr->m_pending_contexts, context, m_next);
        context->m_queue_state = queue_type;
        ++context->m_mgr->m_pending_count;
        break;
    case logic_context_queue_none:
        break;
    }
}

const char * logic_context_state_name(logic_context_state_t state) {
    switch(state) {
    case logic_context_state_init:
        return "init";
    case logic_context_state_waiting:
        return "waiting";
    case logic_context_state_idle:
        return "idle";
    case logic_context_state_error:
        return "error";
    case logic_context_state_done:
        return "done";
    case logic_context_state_cancel:
        return "cancel";
    case logic_context_state_timeout:
        return "timeout";
    default:
        return "unknown";
    }
}
