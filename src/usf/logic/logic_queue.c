#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_queue.h"
#include "usf/logic/logic_context.h"
#include "logic_internal_ops.h"

logic_queue_t
logic_queue_create(logic_manage_t mgr, const char * queue_name) {
    char * buf;
    logic_queue_t queue;
    size_t name_len = cpe_hs_len_to_binary_len(strlen(queue_name));

    CPE_PAL_ALIGN_DFT(name_len);

    buf = mem_alloc(mgr->m_alloc, sizeof(struct logic_queue) + name_len);
    if (buf == NULL) return NULL;

    cpe_hs_init((cpe_hash_string_t)buf, name_len, queue_name);

    queue = (logic_queue_t)(buf + name_len);
    queue->m_mgr = mgr;
    queue->m_name = (cpe_hash_string_t)buf;
    queue->m_count = 0;
    queue->m_max_count = 0;
    
    TAILQ_INIT(&queue->m_contexts);

    cpe_hash_entry_init(&queue->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_queues, queue) != 0) {
        mem_free(mgr->m_alloc, buf);
        return NULL;
    }

    if (mgr->m_debug >= 3) {
        APP_CTX_INFO(
            mgr->m_app, "%s: queue %s create",
            logic_manage_name(mgr), queue_name);
    }

    return queue;
}

void logic_queue_free(logic_queue_t queue) {
    assert(queue);

    while(!TAILQ_EMPTY(&queue->m_contexts)) {
        logic_context_free(TAILQ_FIRST(&queue->m_contexts));
    }

    cpe_hash_table_remove_by_ins(&queue->m_mgr->m_queues, queue);
    
    if (queue->m_mgr->m_debug >= 3) {
        APP_CTX_INFO(
            queue->m_mgr->m_app, "%s: queue %s free",
            logic_manage_name(queue->m_mgr), cpe_hs_data(queue->m_name));
    }

    mem_free(queue->m_mgr->m_alloc, queue->m_name);
}

void logic_queue_free_all(logic_manage_t mgr) {
    struct cpe_hash_it queue_it;
    logic_queue_t queue;

    cpe_hash_it_init(&queue_it, &mgr->m_queues);

    queue = cpe_hash_it_next(&queue_it);
    while (queue) {
        logic_queue_t next = cpe_hash_it_next(&queue_it);
        logic_queue_free(queue);
        queue = next;
    }
}

logic_queue_t
logic_queue_find(logic_manage_t mgr, cpe_hash_string_t queue_name) {
    struct logic_queue key;

    key.m_name = queue_name;

    return (logic_queue_t)cpe_hash_table_find(&mgr->m_queues, &key);
}

uint32_t logic_queue_count(logic_queue_t queue) {
    return queue->m_count;
}

uint32_t logic_queue_max_count(logic_queue_t queue) {
    return queue->m_max_count;
}

void logic_queue_set_max_count(logic_queue_t queue, uint32_t max_count) {
    queue->m_max_count = max_count;
}

const char * logic_queue_name(logic_queue_t queue) {
    return cpe_hs_data(queue->m_name);
}

cpe_hash_string_t logic_queue_name_hs(logic_queue_t queue) {
    return queue->m_name;
}

logic_context_t logic_queue_head(logic_queue_t queue) {
    return TAILQ_FIRST(&queue->m_contexts);
}

int logic_queue_enqueue_head(logic_queue_t queue, logic_context_t context) {
    assert(context->m_queue_state == logic_context_queue_pending);

    if (queue->m_max_count > 0 && queue->m_count >= queue->m_max_count) return -1;

    assert(context->m_logic_queue == NULL);

    if (TAILQ_EMPTY(&queue->m_contexts)) {
        TAILQ_INSERT_HEAD(&queue->m_contexts, context, m_next_logic_queue);
    }
    else {
        TAILQ_INSERT_AFTER(&queue->m_contexts, TAILQ_FIRST(&queue->m_contexts), context, m_next_logic_queue);
        logic_context_dequeue(context);
    }

    context->m_logic_queue = queue;
    ++queue->m_count;

    if (queue->m_mgr->m_debug >= 2) {
        APP_CTX_INFO(
            queue->m_mgr->m_app, "%s: queue %s: enqueue head: context "FMT_UINT32_T", count=%d",
            logic_manage_name(queue->m_mgr), cpe_hs_data(queue->m_name), logic_context_id(context),
            queue->m_count);
    }

    assert(
        (context == TAILQ_FIRST(&queue->m_contexts) && context->m_queue_state == logic_context_queue_pending)
        || (context != TAILQ_FIRST(&queue->m_contexts) && context->m_queue_state == logic_context_queue_none));

    return 0;
}

int logic_queue_enqueue_tail(logic_queue_t queue, logic_context_t context) {
    assert(context->m_queue_state == logic_context_queue_pending);

    if (queue->m_max_count > 0 && queue->m_count >= queue->m_max_count) return -1;

    assert(context->m_logic_queue == NULL);

    if (TAILQ_EMPTY(&queue->m_contexts)) {
        TAILQ_INSERT_HEAD(&queue->m_contexts, context, m_next_logic_queue);
    }
    else {
        TAILQ_INSERT_TAIL(&queue->m_contexts, context, m_next_logic_queue);
        logic_context_dequeue(context);
        assert(context->m_queue_state == logic_context_queue_none);
    }

    context->m_logic_queue = queue;

    ++queue->m_count;

    if (queue->m_mgr->m_debug >= 2) {
        APP_CTX_INFO(
            queue->m_mgr->m_app, "%s: queue %s: enqueue tail: context "FMT_UINT32_T", count=%d",
            logic_manage_name(queue->m_mgr), cpe_hs_data(queue->m_name), logic_context_id(context),
            queue->m_count);
    }

    assert(
        (context == TAILQ_FIRST(&queue->m_contexts) && context->m_queue_state == logic_context_queue_pending)
        || (context != TAILQ_FIRST(&queue->m_contexts) && context->m_queue_state == logic_context_queue_none));

    return 0;
}

int logic_queue_enqueue_after(logic_context_t pre, logic_context_t context) {
    logic_queue_t queue;

    assert(pre);
    assert(context->m_queue_state == logic_context_queue_pending);
    assert(context->m_logic_queue == NULL);

    queue = pre->m_logic_queue;
    if (queue == NULL) return -1;
    if (queue->m_max_count > 0 && queue->m_count >= queue->m_max_count) {
        APP_CTX_ERROR(
            queue->m_mgr->m_app,
            "%s: queue %s: enqueue after "FMT_UINT32_T": context "FMT_UINT32_T", overflow, max-count=%d",
            logic_manage_name(queue->m_mgr), cpe_hs_data(queue->m_name),
            logic_context_id(pre),
            logic_context_id(context),
            queue->m_max_count);
        return -1;
    }

    TAILQ_INSERT_AFTER(&queue->m_contexts, pre, context, m_next_logic_queue);

    logic_context_dequeue(context);

    context->m_logic_queue = queue;

    ++queue->m_count;

    if (queue->m_mgr->m_debug >= 2) {
        APP_CTX_INFO(
            queue->m_mgr->m_app,
            "%s: queue %s: enqueue after "FMT_UINT32_T": context "FMT_UINT32_T", count=%d",
            logic_manage_name(queue->m_mgr), cpe_hs_data(queue->m_name),
            logic_context_id(pre),
            logic_context_id(context),
            queue->m_count);
    }

    assert(
        (context == TAILQ_FIRST(&queue->m_contexts) && context->m_queue_state == logic_context_queue_pending)
        || (context != TAILQ_FIRST(&queue->m_contexts) && context->m_queue_state == logic_context_queue_none));

    return 0;
}

void logic_queue_dequeue(logic_queue_t queue, logic_context_t context) {
    int is_first;
    logic_context_t wakup_context = NULL;

    assert(queue == context->m_logic_queue);
    assert(queue->m_count > 0);

    is_first = TAILQ_FIRST(&queue->m_contexts) == context;

    TAILQ_REMOVE(&queue->m_contexts, context, m_next_logic_queue);
    context->m_logic_queue = NULL;
    --queue->m_count;

    if (is_first) {
        if (!TAILQ_EMPTY(&queue->m_contexts)) {
            wakup_context = TAILQ_FIRST(&queue->m_contexts);
            assert(wakup_context);
            assert(wakup_context->m_queue_state == logic_context_queue_none);
            logic_context_enqueue(wakup_context, logic_context_queue_pending);
        }
    }
    else {
        assert(context->m_queue_state == logic_context_queue_none);
        logic_context_enqueue(context, logic_context_queue_pending);
    }

    if (queue->m_mgr->m_debug >= 2) {
        if (wakup_context) {
            APP_CTX_INFO(
                queue->m_mgr->m_app, "%s: queue %s: dequeue: context "FMT_UINT32_T",wakup "FMT_UINT32_T", count=%d",
                logic_manage_name(queue->m_mgr), cpe_hs_data(queue->m_name),
                logic_context_id(context),
                logic_context_id(wakup_context),
                queue->m_count);
        }
        else {
            APP_CTX_INFO(
                queue->m_mgr->m_app, "%s: queue %s: dequeue: context "FMT_UINT32_T", no wakup, count=%d",
                logic_manage_name(queue->m_mgr), cpe_hs_data(queue->m_name),
                logic_context_id(context),
                queue->m_count);
        }
    }
}

uint32_t logic_queue_hash(const struct logic_queue * queue) {
    return cpe_hs_value(queue->m_name);
}

int logic_queue_cmp(const struct logic_queue * l, const struct logic_queue * r) {
    return cpe_hs_cmp(l->m_name, r->m_name) == 0;
}

