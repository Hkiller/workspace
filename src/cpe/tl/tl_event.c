#include <assert.h>
#include "tl_internal_ops.h"

struct tl_event_node *
tl_event_node_alloc(tl_t tl, size_t capacity) {
    struct tl_event_node * node = 
        (struct tl_event_node *)mem_alloc(
            tl->m_manage->m_alloc,
            sizeof(struct tl_event_node) + capacity);
    if (node == NULL) return NULL;

    node->m_state = tl_event_node_state_in_building_queue;
    node->m_event.m_tl = tl;
    node->m_event.m_capacity = capacity;

    if (tl->m_event_construct) {
        tl->m_event_construct(&node->m_event, tl->m_event_op_context);
    }

    TAILQ_INSERT_TAIL(&tl->m_events, node, m_next_in_tl);
    TAILQ_INSERT_HEAD(&tl->m_manage->m_event_building_queue, node, m_next);

    return node;
}

void tl_event_node_free_i(struct tl_event_node * node) {
    if (node->m_event.m_tl->m_event_destory) {
        node->m_event.m_tl->m_event_destory(
            &node->m_event,
            node->m_event.m_tl->m_event_op_context);
    }

    TAILQ_REMOVE(&node->m_event.m_tl->m_events, node, m_next_in_tl);

    mem_free(node->m_event.m_tl->m_manage->m_alloc, node);
}

void tl_event_node_free(struct tl_event_node * node) {
    if (node == NULL) return;

    switch(node->m_state) {
    case tl_event_node_state_deleting:
        return;
    case tl_event_node_state_runing:
        node->m_state = tl_event_node_state_deleting;
        return;
    case tl_event_node_state_free:
        tl_event_node_free_i(node);
        break;
    case tl_event_node_state_in_building_queue:
        TAILQ_REMOVE(&node->m_event.m_tl->m_manage->m_event_building_queue, node, m_next);
        tl_event_node_free_i(node);
        break;
    case tl_event_node_state_in_event_queue:
        TAILQ_REMOVE(&node->m_event.m_tl->m_manage->m_event_queue, node, m_next);
        tl_event_node_free_i(node);
        break;
    }
}

int tl_event_node_insert(struct tl_event_node * node) {
    struct tl_event_node * insertPos;
    tl_manage_t tm;

    assert(node);
    assert(node->m_event.m_tl);
    assert(node->m_event.m_tl->m_manage);

    tm = node->m_event.m_tl->m_manage;

    insertPos = TAILQ_FIRST(&tm->m_event_queue);
    while(insertPos != TAILQ_END(&tm->m_event_queue)
          && insertPos->m_execute_time <= node->m_execute_time)
    {
        insertPos = TAILQ_NEXT(insertPos, m_next);
    }

    if (insertPos == TAILQ_END(&tm->m_event_queue)) {
        TAILQ_INSERT_TAIL(&tm->m_event_queue, node, m_next);
    }
    else {
        TAILQ_INSERT_BEFORE(insertPos, node, m_next);
    }

    node->m_state = tl_event_node_state_in_event_queue;

    return 0;
}

int tl_event_queue_clear(struct tl_event_node_queue * queue) {
    int count = 0;

    assert(queue);

    while(!TAILQ_EMPTY(queue)) {
        tl_event_node_free(TAILQ_FIRST(queue));
        ++count;
    }

    return count;
}

