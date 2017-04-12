#include <assert.h>
#include "cpe/pal/pal_time.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/tl/tl_errno.h"
#include "tl_internal_ops.h"

tl_time_t tl_time_source_usec(void * context) {
    struct timeval tp;
    int r;
    r = gettimeofday(&tp, NULL);
    assert(r == 0);
    return tp.tv_sec * 1000 * 1000 + tp.tv_usec;
}

tl_time_t tl_time_source_msec(void * context) {
    struct timeval tp;
    int r;
    r = gettimeofday(&tp, NULL);
    assert(r == 0);
    return (uint64_t)tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

tl_time_t tl_time_source_next_event(void * context) {
    tl_manage_t tm = (tl_manage_t)context;
    if (TAILQ_EMPTY(&tm->m_event_queue)) {
        return tm->m_time_current;
    }
    else {
        return TAILQ_FIRST(&tm->m_event_queue)->m_execute_time;
    }
}

tl_time_t tl_time_source_last_event(void * context) {
    tl_manage_t tm = (tl_manage_t)context;
    if (TAILQ_EMPTY(&tm->m_event_queue)) {
        return tm->m_time_current;
    }
    else {
        return TAILQ_LAST(&tm->m_event_queue, tl_event_node_queue)->m_execute_time;
    }
}

tl_time_span_t tl_time_cvt_sec2msec(tl_time_span_t sec, void * context) {
    return sec * 1000;
}

tl_time_span_t tl_time_cvt_sec2usec(tl_time_span_t sec, void * context) {
    return sec * 1000 * 1000;
}
