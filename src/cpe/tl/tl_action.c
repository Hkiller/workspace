#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/tl/tl_action.h"
#include "cpe/tl/tl_errno.h"
#include "tl_internal_types.h"
#include "tl_internal_ops.h"

tl_event_t tl_event_create(tl_t tl, size_t dataSize) {
    struct tl_event_node * node = tl_event_node_alloc(tl, dataSize);
    if (node == NULL) return NULL;

    return &node->m_event;
}

tl_event_t tl_event_clone(tl_event_t e, mem_allocrator_t alloc) {
    struct tl_free_event * r;

    r = (struct tl_free_event *)mem_alloc(alloc, sizeof(struct tl_free_event) + e->m_capacity);
    if (r == NULL) return NULL;

    r->m_alloc = alloc;
    r->m_event.m_tl = NULL;
    r->m_event.m_capacity = e->m_capacity;
    memcpy(r + 1, e + 1, e->m_capacity);

    return &r->m_event;
}

void tl_event_free(tl_event_t e) {
    struct tl_free_event * fe;

    if (e == NULL) return;

    if (e->m_tl == NULL) {
        fe = (struct tl_free_event *)
            (((char*)e)
             - (sizeof(struct tl_free_event) - sizeof(struct tl_event)));
        mem_free(fe->m_alloc, fe);
    }
    else {
        tl_event_node_free(tl_event_to_node(e));
    }
}

tl_event_t tl_action_add(tl_t tl) {
    union tl_action * action;
    int nextEndPos;
    tl_manage_t tm;

    if (tl == NULL || tl->m_manage == NULL) return NULL;

   tm = tl->m_manage;

    nextEndPos = tm->m_action_end_pos + 1;
    if (nextEndPos >= CPE_TL_ACTION_MAX) {
        nextEndPos = 0;
    }

    if (nextEndPos == tm->m_action_begin_pos) {
        return NULL;
    }

    action = &tm->m_action_queue[tm->m_action_end_pos];
    tm->m_action_end_pos = nextEndPos;

    action->m_event.m_tl = tl;
    if (tl->m_event_construct) {
        tl->m_event_construct(&action->m_event, tl->m_event_op_context);
    }

    return &action->m_event;
}

void * tl_event_data(tl_event_t event) {
    return (void*)(event + 1);
}

size_t tl_event_capacity(tl_event_t event) {
    return event->m_capacity;
}

tl_t tl_event_tl(tl_event_t event) {
    return event->m_tl;
}

tl_event_t
tl_event_from_data(void * data) {
    return (tl_event_t)( ((char *)data) - sizeof(struct tl_event));
}

int tl_event_enqueue_local(
    tl_event_t event,
    tl_time_span_t delay,
    tl_time_span_t span,
    int repeatCount,
    void * context)
{
    int r;
    struct tl_event_node * input = tl_event_to_node(event);

    input->m_execute_time =
    event->m_tl->m_manage->m_time_current + (tl_time_span_t)((float)delay / event->m_tl->m_manage->m_rate);
    input->m_span = span;// / event->m_tl->m_manage->m_rate;
    input->m_repeatCount = repeatCount;

    /*be careful, input not be managed by manage!!!*/
    tl_event_node_remove_from_building_queue(input);
    input->m_state = tl_event_node_state_free;

    r = tl_event_node_insert(input);
    if (r != 0) {
        tl_event_node_free(input);
        return r;
    }

    return 0;
}

int tl_event_in_queue(tl_event_t event) {
    struct tl_event_node * checkEvent;
    struct tl_event_node * input;
    struct tl_event_node_queue * queue;

    if (event == NULL || event->m_tl == NULL || event->m_tl->m_manage == NULL) {
        return 0;
    }

    queue = &event->m_tl->m_manage->m_event_queue;
    input = tl_event_to_node(event);

    for(checkEvent = TAILQ_FIRST(queue);
        checkEvent != TAILQ_END(queue);
        checkEvent = TAILQ_NEXT(checkEvent, m_next))
    {
        if (checkEvent == input) return 1;
    }

    return 0;
}

int tl_event_in_building_queue(tl_event_t event) {
    struct tl_event_node * checkEvent;
    struct tl_event_node * input;
    struct tl_event_node_queue * queue;

    if (event == NULL || event->m_tl == NULL || event->m_tl->m_manage == NULL) {
        return 0;
    }

    queue = &event->m_tl->m_manage->m_event_building_queue;
    input = tl_event_to_node(event);

    for(checkEvent = TAILQ_FIRST(queue);
        checkEvent != TAILQ_END(queue);
        checkEvent = TAILQ_NEXT(checkEvent, m_next))
    {
        if (checkEvent == input) return 1;
    }

    return 0;
}

int tl_event_send_ex(
    tl_event_t event,
    tl_time_span_t delay,
    tl_time_span_t span,
    int repeatCount)
{
    tl_t tl;
    tl_manage_t tm;
    tl_intercept_t intercept;

    if (event == NULL || event->m_tl == NULL || event->m_tl->m_manage == NULL)
        return CPE_TL_ERROR_BAD_ARG;

    tl = event->m_tl;

    tm = tl->m_manage;
    assert(tm);
    if (tm->m_time_cvt) {
        delay = tm->m_time_cvt(delay, tm->m_time_ctx);
        span = tm->m_time_cvt(span, tm->m_time_ctx);
    }

    if (repeatCount == 0) return CPE_TL_ERROR_BAD_ARG;
    if (delay < 0) return CPE_TL_ERROR_BAD_ARG;
    if (span < 0 || (repeatCount != 1 && span == 0)) return CPE_TL_ERROR_BAD_ARG;
    if (!tl_event_in_building_queue(event)) return CPE_TL_ERROR_EVENT_UNKNOWN;

    TAILQ_FOREACH(intercept, &tl->m_intercepts, m_next) {
        if (intercept->m_intercept_fun(event, intercept->m_intercept_ctx)) {
            return CPE_TL_ERROR_NONE;
        }
    }

    if (tl->m_event_enqueue) {
        return tl->m_event_enqueue(
            event, delay, span, repeatCount,
            tl->m_event_op_context);
    }
    else {
        return CPE_TL_ERROR_EVENT_NO_ENQUEUE;
    }
}
