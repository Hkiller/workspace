#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream.h"
#include "priority_queue_i.h"

cpe_priority_queue_t
cpe_priority_queue_create(
    mem_allocrator_t alloc, error_monitor_t em,
    uint16_t ele_size, cpe_priority_queue_cmp_fun_t ele_cmp,
    uint16_t init_capacity)
{
    cpe_priority_queue_t queue;

    queue = mem_alloc(alloc, sizeof(struct cpe_priority_queue));
    if (queue == NULL) {
        CPE_ERROR(em, "cpe_priority_queue_create: alloc fail!");
        return NULL;
    }

    queue->m_alloc = alloc;
    queue->m_em = em;
    queue->m_ele_size = ele_size;
    queue->m_ele_cmp = ele_cmp;
    queue->m_node_capacity = init_capacity;
    queue->m_node_count = 1;
    queue->m_node_buf = NULL;

    if (queue->m_node_capacity > 0) {
        queue->m_node_buf = mem_alloc(alloc, ele_size * init_capacity);
        if (queue->m_node_buf == NULL) {
            CPE_ERROR(em, "cpe_priority_queue_create: alloc fail!");
            mem_free(alloc, queue);
            return NULL;
        }
    }

    return queue;
}

void cpe_priority_queue_free(cpe_priority_queue_t queue) {
    if (queue->m_node_buf) {
        mem_free(queue->m_alloc, queue->m_node_buf);
    }

    mem_free(queue->m_alloc, queue);
}

uint16_t cpe_priority_queue_count(cpe_priority_queue_t queue) {
    return queue->m_node_count - 1;
}

void * cpe_priority_queue_top(cpe_priority_queue_t queue) {
    return queue->m_node_count > 1 ? cpe_priority_queue_node_at(queue, 1)  : NULL;
}

int cpe_priority_queue_pop(cpe_priority_queue_t queue) {
  int n, m;

  if (queue->m_node_count <= 1) return -1;
 
  queue->m_node_count--;
    
  n = 1;
  while ((m = n * 2) < queue->m_node_count) {
      if (m + 1 < queue->m_node_count && cpe_priority_queue_node_cmp(queue, m, m + 1) > 0) m++;
 
      if (cpe_priority_queue_node_cmp(queue, queue->m_node_count, m) <= 0) break;

      memcpy(
          cpe_priority_queue_node_at(queue, n),
          cpe_priority_queue_node_at(queue, m),
          queue->m_ele_size);
      
      n = m;
  }

  memcpy(
      cpe_priority_queue_node_at(queue, n),
      cpe_priority_queue_node_at(queue, queue->m_node_count),
      queue->m_ele_size);
  
  return 0;
}

int cpe_priority_queue_insert(cpe_priority_queue_t queue, void * ele) {
    int m, n;
  
    if (queue->m_node_count + 1 > queue->m_node_capacity) {
        uint16_t new_capacity = queue->m_node_capacity < 16 ? 32 : queue->m_node_capacity * 2;
        void * new_buf = mem_alloc(queue->m_alloc, queue->m_ele_size * new_capacity);
        if (new_buf == NULL) {
            CPE_ERROR(queue->m_em, "cpe_priority_queue_insert: resize capacity to %d alloc fail!", new_capacity);
            return -1;
        }

        if (queue->m_node_buf) {
            memcpy(new_buf, queue->m_node_buf, queue->m_ele_size * queue->m_node_count);
            mem_free(queue->m_alloc, queue->m_node_buf);
        }

        queue->m_node_buf = new_buf;
        queue->m_node_capacity = new_capacity;
    }

    assert(queue->m_node_count + 1 <= queue->m_node_capacity);

    n = queue->m_node_count++;

    while ((m = n / 2) && queue->m_ele_cmp(ele, cpe_priority_queue_node_at(queue, m)) < 0) {
        memcpy(
            cpe_priority_queue_node_at(queue, n),
            cpe_priority_queue_node_at(queue, m),
            queue->m_ele_size);
        n = m;
    }

    memcpy(cpe_priority_queue_node_at(queue, n), ele, queue->m_ele_size);

    return 0;
}

void cpe_priority_queue_dump(write_stream_t s, cpe_priority_queue_t queue, cpe_priority_queue_print_fun_t p, const char * sep) {
    int i;

    for(i = 0; i < queue->m_node_count; ++i) {
        if (i > 0) stream_printf(s, "%s", sep);
        p(s, cpe_priority_queue_node_at(queue, i));
    }
}
