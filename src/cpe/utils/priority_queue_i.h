#ifndef CPE_UTILS_PRIORITY_QUEUE_I_H
#define CPE_UTILS_PRIORITY_QUEUE_I_H
#include "cpe/utils/priority_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cpe_priority_queue {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint16_t m_ele_size;
    cpe_priority_queue_cmp_fun_t m_ele_cmp;
    uint16_t m_node_capacity;
    uint16_t m_node_count;
    void * m_node_buf;
};

#define cpe_priority_queue_node_at(__queue, __pos) \
    ((void*)(((char*)((__queue)->m_node_buf)) + (__queue)->m_ele_size * (__pos)))

#define cpe_priority_queue_node_cmp(__queue, __l_pos, __r_pos) \
    ((__queue)->m_ele_cmp(                              \
        cpe_priority_queue_node_at(__queue, __l_pos),\
        cpe_priority_queue_node_at(__queue, __r_pos)))
    
#ifdef __cplusplus
}
#endif

#endif


