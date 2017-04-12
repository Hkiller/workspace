#include <stdarg.h>
#include <assert.h>
#include "cpe/tl/tl_manage.h"
#include "cpe/tl/tl_errno.h"
#include "cpe/tl/tl_intercept.h"
#include "tl_internal_types.h"
#include "tl_internal_ops.h"

tl_manage_t tl_manage_create(mem_allocrator_t alloc) {
    int i;

    tl_manage_t tm = mem_alloc(alloc, sizeof(struct tl_manage));
    if (tm == NULL) return NULL;

    tm->m_alloc = alloc;
    
    tm->m_time_get = tl_time_source_msec;
    tm->m_time_cvt = 0;
    tm->m_time_ctx = 0;
    tm->m_time_current = tm->m_time_get(tm->m_time_ctx);
    tm->m_state = tl_manage_state_runing;
    tm->m_time_pause_eat = 0;
    tm->m_to_rate = tm->m_rate = 1.f;

    tm->m_action_begin_pos = tm->m_action_end_pos = 0;

    for(i = 0; i < CPE_TL_ACTION_MAX; ++i) {
        union tl_action * action = &tm->m_action_queue[i];
        action->m_event.m_tl = NULL;
        action->m_event.m_capacity = CPE_TL_ACTION_MAX - sizeof(struct tl_event);
    }
 
    TAILQ_INIT(&tm->m_tls);

    TAILQ_INIT(&tm->m_event_queue);
    TAILQ_INIT(&tm->m_event_building_queue);

    return tm;
}

static void tl_action_queue_clear(tl_manage_t tm) {
    if (tm->m_action_begin_pos > tm->m_action_end_pos) {
        for(; tm->m_action_begin_pos < CPE_TL_ACTION_MAX;
            ++tm->m_action_begin_pos)
        {
            union tl_action * action = &tm->m_action_queue[tm->m_action_begin_pos];
            if (action->m_event.m_tl->m_event_destory) {
                action->m_event.m_tl->m_event_destory(
                    &action->m_event,
                    action->m_event.m_tl->m_event_op_context);
            }
        }

        assert(tm->m_action_begin_pos >= CPE_TL_ACTION_MAX);
        tm->m_action_begin_pos = 0;
    }

    for(; tm->m_action_begin_pos < tm->m_action_end_pos; ++tm->m_action_begin_pos) {
        union tl_action * action = &tm->m_action_queue[tm->m_action_begin_pos];
        if (action->m_event.m_tl->m_event_destory) {
            action->m_event.m_tl->m_event_destory(
                &action->m_event,
                action->m_event.m_tl->m_event_op_context);
        }
    }
}

void tl_queue_clear(tl_manage_t tm) {
    while(!TAILQ_EMPTY(&tm->m_tls)) {
        tl_free(TAILQ_FIRST(&tm->m_tls));
    }
}

void tl_manage_free(tl_manage_t tm) {
    if (tm == NULL) return;

    tl_event_queue_clear(&tm->m_event_building_queue);
    tl_event_queue_clear(&tm->m_event_queue);
    tl_action_queue_clear(tm);
    tl_queue_clear(tm);

    mem_free(tm->m_alloc, tm);
}

int tl_manage_set_opt(tl_manage_t tm, tl_manage_option_t opt,...) {
    int rv = -1;
    va_list ap;
    va_start(ap, opt);

    switch(opt) {
        case tl_set_time_source: {
            tl_time_fun_t ts = va_arg(ap, tl_time_fun_t);
            if (ts == NULL) {
                rv = CPE_TL_ERROR_NO_TIME_SOURCE;
            }
            else {
                rv = 0;
                tm->m_state = tl_manage_state_runing;
                tm->m_time_get = ts;
                tm->m_time_current = ts(tm->m_time_ctx);
                tm->m_time_pause_eat = 0;
            }
            break;
        }
        case tl_set_time_cvt: {
            rv = 0;
            tm->m_time_cvt = va_arg(ap, tl_time_cvt_fun_t);
            break;
        }
        case tl_set_time_source_context: {
            rv = 0;
            tm->m_time_ctx = va_arg(ap, void *);
            break;
        }
        default:
            rv = CPE_TL_ERROR_BAD_ARG;
            break;
    }

    va_end(ap);

    return rv;
}

tl_t tl_create(tl_manage_t tm) {
    tl_t tl;

    tl = mem_alloc(tm->m_alloc, sizeof(struct tl));
    if (tl == NULL) return NULL;

    tl->m_manage = tm;
    tl->m_event_dispatcher = NULL;
    tl->m_event_enqueue = tl_event_enqueue_local;
    tl->m_event_construct = NULL;
    tl->m_event_destory = NULL;
    tl->m_event_op_context = NULL;
    TAILQ_INIT(&tl->m_events);
    TAILQ_INIT(&tl->m_intercepts);

    TAILQ_INSERT_TAIL(&tm->m_tls, tl, m_next);

    return tl;
}

void tl_free(tl_t tl) {
    if (tl == NULL) return;

    while(!TAILQ_EMPTY(&tl->m_events)) {
        tl_event_node_free(TAILQ_FIRST(&tl->m_events));
    }

    while(!TAILQ_EMPTY(&tl->m_intercepts)) {
        tl_intercept_free(TAILQ_FIRST(&tl->m_intercepts));
    }

    TAILQ_REMOVE(&tl->m_manage->m_tls, tl, m_next);

    mem_free(tl->m_manage->m_alloc, tl);
}

tl_time_t tl_manage_time(tl_manage_t tm) {
    return tm->m_time_current;
}

uint32_t tl_manage_time_sec(tl_manage_t tm) {
    return (uint32_t)(tm->m_time_current / 1000);
}

tl_manage_state_t tl_manage_state(tl_manage_t tm) {
    return tm->m_state;
}

void tl_manage_pause(tl_manage_t tm) {
    if (tm->m_state == tl_manage_state_pause) return;
    tl_manage_update_time(tm);
    tm->m_state = tl_manage_state_pause;
}

void tl_manage_resume(tl_manage_t tm) {
    tl_time_t pause_time;
    tl_time_span_t new_eat;

    if (tm->m_state != tl_manage_state_pause) return;

    pause_time = tm->m_time_current;

    tm->m_state = tl_manage_state_runing;
    tl_manage_update_time(tm);

    new_eat = tm->m_time_current - pause_time;
    tm->m_time_pause_eat += new_eat;
    tm->m_time_current -= new_eat;
}

int tl_set_opt(tl_t tl, tl_option_t opt, ...) {
    int rv = -1;
    va_list ap;
    va_start(ap, opt);

    switch(opt) {
        case tl_set_event_dispatcher: {
            rv = 0;
            tl->m_event_dispatcher = va_arg(ap, tl_event_process_t);
            break;
        }
        case tl_set_event_enqueue: {
            tl_event_enqueue_t enqueue = va_arg(ap, tl_event_enqueue_t);
            if (enqueue == NULL) {
                rv = CPE_TL_ERROR_EVENT_NO_ENQUEUE;
            }
            else {
                rv = 0;
                tl->m_event_enqueue = enqueue;
            }
            break;
        }
        case tl_set_event_construct: {
            rv = 0;
            tl->m_event_construct = va_arg(ap, tl_event_process_t);
            break;
        }
        case tl_set_event_destory: {
            rv = 0;
            tl->m_event_destory = va_arg(ap, tl_event_process_t);
            break;
        }
        case tl_set_event_op_context: {
            rv = 0;
            tl->m_event_op_context = va_arg(ap, void *);
            break;
        }
        default:
            rv = CPE_TL_ERROR_BAD_ARG;
            break;
    }

    va_end(ap);

    return rv;
}

void tl_manage_rate(tl_manage_t tm, float rate) {
    tm->m_to_rate = rate;
}

void tl_manage_update(tl_manage_t tm) {
    tl_manage_update_time(tm);
}
