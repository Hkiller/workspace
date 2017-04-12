#ifndef CPE_MEM_BUFFER_PRIVATE_H
#define CPE_MEM_BUFFER_PRIVATE_H
#include "cpe/utils/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mem_buffer_trunk {
    struct mem_allocrator * m_allocrator;
    size_t m_capacity;
    size_t m_size;
    TAILQ_ENTRY(mem_buffer_trunk) m_next;
};

struct mem_buffer_trunk *
mem_trunk_alloc(struct mem_allocrator * allocrator, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif

