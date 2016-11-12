#ifndef CPE_MEM_ALLOCRATOR_DEBUG_H
#define CPE_MEM_ALLOCRATOR_DEBUG_H
#include "memory.h"
#include "error.h"
#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

mem_allocrator_t
mem_allocrator_debug_create(
    mem_allocrator_t alloc,
    mem_allocrator_t parent,
    int stack_size,
    error_monitor_t em);

void mem_allocrator_debug_free(mem_allocrator_t dalloc);

void mem_allocrator_debug_dump(write_stream_t stream, int ident, mem_allocrator_t dalloc);
int mem_allocrator_debug_alloc_count(mem_allocrator_t dalloc);
int mem_allocrator_debug_free_count(mem_allocrator_t dalloc);
int mem_allocrator_debug_alloc_size(mem_allocrator_t dalloc);
int mem_allocrator_debug_free_size(mem_allocrator_t dalloc);

#ifdef __cplusplus
}
#endif

#endif
