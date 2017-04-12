#ifndef CPE_TL_INTERNAL_OPS_H
#define CPE_TL_INTERNAL_OPS_H
#include "tl_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*node operations*/
struct tl_event_node * tl_event_node_alloc(tl_t tl, size_t capacity);
void tl_event_node_free(struct tl_event_node * node);
int tl_event_node_insert(struct tl_event_node * node);
int tl_event_queue_clear(struct tl_event_node_queue * queue);

#define tl_event_to_node(e)                                             \
    ((struct tl_event_node *)                                           \
     (((char *)e)                                                       \
      + (10000 - ((ptr_int_t)(&((struct tl_event_node *)10000)->m_event)))))

#define tl_event_node_remove_from_building_queue(i)  \
    TAILQ_REMOVE(                                       \
        &(i)->m_event.m_tl->m_manage->m_event_building_queue, \
        (i), m_next)

#define tl_manage_update_time(tm)                                   \
    if ((tm)->m_state != tl_manage_state_pause) {                   \
        tl_time_t nextTime =                                        \
            (tm)->m_time_get((tm)->m_time_ctx)                      \
            - (tm)->m_time_pause_eat;                               \
        if (nextTime > (tm)->m_time_current)                        \
            (tm)->m_time_current = nextTime;                        \
    }

#ifdef __cplusplus
}
#endif

#endif


