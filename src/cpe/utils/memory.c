#include <stdlib.h>
#include "cpe/utils/memory.h"

void * mem_alloc(struct mem_allocrator * alloc, size_t size) {
    if (alloc) {
        return alloc->m_alloc(size, alloc);
    }
    else {
        return malloc(size);
    }
}

void * mem_calloc(struct mem_allocrator * alloc, size_t size) {
    if (alloc) {
        return alloc->m_calloc(size, alloc);
    }
    else {
        return calloc(size, 1);
    }
}

void mem_free(struct mem_allocrator * alloc, void * p) {
    if (alloc) {
        alloc->m_free(p, alloc);
    }
    else {
        free(p);
    }
}

static void * mem_do_null_alloc(size_t size, struct mem_allocrator * allocrator) {
    return NULL;
}

static void mem_do_null_free(void * p, struct mem_allocrator * allocrator) {
}

struct mem_allocrator * mem_allocrator_null(void) {
    static struct mem_allocrator s_ins = {
        mem_do_null_alloc,
        mem_do_null_alloc,
        mem_do_null_free
    };

    return &s_ins;
}
