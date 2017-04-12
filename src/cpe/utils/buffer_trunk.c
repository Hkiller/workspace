#include <assert.h>
#include <string.h>
#include "buffer_private.h"

void * mem_trunk_data(struct mem_buffer_trunk * trunk) {
    return (void *)(trunk + 1);
}

size_t mem_trunk_capacity(struct mem_buffer_trunk * trunk) {
    return trunk->m_capacity;
}

size_t mem_trunk_size(struct mem_buffer_trunk * trunk) {
    return trunk->m_size;
}

size_t mem_trunk_append(struct mem_buffer * buffer, struct mem_buffer_trunk * trunk, const void * buf, size_t size) {
    size_t writeSize = trunk->m_capacity - trunk->m_size;
    if (size < writeSize) {
        writeSize = size;
    }

    memcpy((char*)mem_trunk_data(trunk) + trunk->m_size, buf, writeSize);

    buffer->m_size += writeSize;
    trunk->m_size += writeSize;

    return writeSize;
}

void mem_trunk_set_size(struct mem_buffer * buffer, struct mem_buffer_trunk * trunk, size_t size) {
    assert(size <= trunk->m_capacity);

    buffer->m_size -= trunk->m_size;
    buffer->m_size += size;
    trunk->m_size = size;
}

struct mem_buffer_trunk *
mem_trunk_alloc(struct mem_allocrator * allocrator, size_t capacity) {
    size_t allocSize = capacity + sizeof(struct mem_buffer_trunk);
    struct mem_buffer_trunk * trunk =
        (struct mem_buffer_trunk *)mem_alloc(allocrator, allocSize);
    if (trunk == NULL) {
        return NULL;
    }

    trunk->m_allocrator = allocrator;
    trunk->m_capacity = capacity;
    trunk->m_size = 0;

    return trunk;
}

void mem_trunk_free(struct mem_buffer * buffer, struct mem_buffer_trunk * trunk) {
    if (trunk == NULL) return;

    if (buffer) {
        TAILQ_REMOVE(&buffer->m_trunks, trunk, m_next);
        buffer->m_size -= trunk->m_size;
    }

    mem_free(trunk->m_allocrator, trunk);
}
