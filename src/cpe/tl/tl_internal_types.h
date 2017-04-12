#ifndef CPE_TL_INTERNAL_TYPES_H
#define CPE_TL_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/tl/tl_types.h"

#define CPE_TL_ACTION_MAX (128)
#define CPE_TL_ACTION_SIZE (128)

#ifdef __cplusplus
extern "C" {
#endif

struct tl_event {
    tl_t m_tl;
    size_t m_capacity;
};

struct tl_free_event {
    mem_allocrator_t m_alloc;
    struct tl_event m_event;
};

enum tl_event_node_state {
    tl_event_node_state_free
    , tl_event_node_state_runing
    , tl_event_node_state_deleting
    , tl_event_node_state_in_building_queue
    , tl_event_node_state_in_event_queue
};

struct tl_event_node {
    enum tl_event_node_state m_state;
    TAILQ_ENTRY(tl_event_node) m_next;
    TAILQ_ENTRY(tl_event_node) m_next_in_tl;
    tl_time_t m_execute_time;
    tl_time_span_t m_span;
    int m_repeatCount;

    struct tl_event m_event;
};

TAILQ_HEAD(tl_event_node_queue, tl_event_node);

union tl_action {
    struct tl_event m_event;
    char m_reserve[CPE_TL_ACTION_SIZE];
};

struct tl_intercept {
    tl_t m_tl;
    const char * m_name;
    tl_intercept_fun_t m_intercept_fun;
    void * m_intercept_ctx;

    TAILQ_ENTRY(tl_intercept) m_next;
};

TAILQ_HEAD(tl_intercept_list, tl_intercept);

struct tl {
    tl_manage_t m_manage;

    tl_event_enqueue_t m_event_enqueue;
    tl_event_process_t m_event_dispatcher;
    tl_event_process_t m_event_construct;
    tl_event_process_t m_event_destory;
    void * m_event_op_context;

    struct tl_event_node_queue m_events;
    struct tl_intercept_list m_intercepts;

    TAILQ_ENTRY(tl) m_next;
};

TAILQ_HEAD(tl_queue, tl);

struct tl_manage {
    mem_allocrator_t m_alloc;

    /*time*/
    tl_time_fun_t m_time_get;
    tl_time_cvt_fun_t m_time_cvt;
    void * m_time_ctx;
    tl_time_t m_time_current;

    tl_manage_state_t m_state;
    tl_time_span_t m_time_pause_eat;

    /*tl*/
    //int m_tl_count;
    struct tl_queue m_tls;

    /*action*/
    int m_action_begin_pos;
    int m_action_end_pos;
    union tl_action m_action_queue[CPE_TL_ACTION_MAX];

    /*event*/
    struct tl_event_node_queue m_event_queue;
    struct tl_event_node_queue m_event_building_queue;

    float m_rate;
    float m_to_rate;
};

#ifdef __cplusplus
}
#endif

#endif
