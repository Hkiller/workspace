#include <assert.h>
#include <string.h>
#include "cpe/utils/stream_buffer.h"
#include "buffer_private.h"

void mem_buffer_init(struct mem_buffer * buffer, struct mem_allocrator * allocrator) {
    buffer->m_default_allocrator = allocrator;
    buffer->m_size = 0;
    buffer->m_auto_inc_size = 128;
    TAILQ_INIT(&buffer->m_trunks);
}

void mem_buffer_clear(struct mem_buffer * buffer) {
    while(!TAILQ_EMPTY(&buffer->m_trunks)) {
        mem_trunk_free(buffer, TAILQ_FIRST(&buffer->m_trunks));
    }

    assert(buffer->m_size == 0);
}

size_t mem_buffer_size(struct mem_buffer * buffer) {
    return buffer->m_size;
}

void mem_buffer_clear_data(mem_buffer_t buffer) {
    struct mem_buffer_trunk * trunk;

    TAILQ_FOREACH(trunk, &buffer->m_trunks, m_next) {
        trunk->m_size = 0;
    }

    buffer->m_size = 0;
}

struct mem_buffer_trunk *
mem_buffer_append_trunk(struct mem_buffer * buffer, size_t capacity) {
    struct mem_buffer_trunk * trunk = mem_trunk_alloc(buffer->m_default_allocrator, capacity);

    if (trunk) {
        TAILQ_INSERT_TAIL(&buffer->m_trunks, trunk, m_next);
    }

    return trunk;
}

struct mem_buffer_trunk *
mem_buffer_append_trunk_after(
    struct mem_buffer * buffer,
    struct mem_buffer_trunk * preTrunk,
    size_t capacity)
{
    struct mem_buffer_trunk * trunk = mem_trunk_alloc(buffer->m_default_allocrator, capacity);

    if (trunk) {
        TAILQ_INSERT_AFTER(&buffer->m_trunks, preTrunk, trunk, m_next);
    }

    return trunk;
}

struct mem_buffer_trunk *
mem_buffer_trunk_first(mem_buffer_t buffer) {
    return TAILQ_FIRST(&buffer->m_trunks);
}

struct mem_buffer_trunk *
mem_buffer_trunk_next(struct mem_buffer_trunk * trunk) {
    return trunk
        ? TAILQ_NEXT(trunk, m_next)
        : NULL;
}

ssize_t mem_buffer_read(void * buf, size_t size, struct mem_buffer * buffer) {
    size_t readedSize = 0;
    struct mem_buffer_trunk * trunk;

    if (!buf || size <= 0 || !buffer) return -1;

    trunk = TAILQ_FIRST(&buffer->m_trunks);

    while(readedSize < size && trunk != TAILQ_END(&trunk->m_trunks)) {
        size_t readSize = size - readedSize;
        if (readSize > trunk->m_size) {
            readSize = trunk->m_size;
        }

        memcpy((char*)buf + readedSize, mem_trunk_data(trunk), readSize);
        readedSize += readSize;

        trunk = TAILQ_NEXT(trunk, m_next);
    }

    return (ssize_t)readedSize;
}

ssize_t mem_buffer_append(struct mem_buffer * buffer, const void * buf, size_t size) {
    size_t writedSize = 0;
    size_t newTrunkSize = 0;
    struct mem_buffer_trunk * trunk = NULL;

    if (buf == NULL && size == 0) return 0;

    if (!buffer || !buf) return -1;

    trunk = TAILQ_LAST(&buffer->m_trunks, mem_buffer_trunk_list);

    if (trunk != TAILQ_END(&buffer->m_trunks)) {
        writedSize += mem_trunk_append(buffer, trunk, buf, size);
    }

    newTrunkSize = size - writedSize;
    if (newTrunkSize < buffer->m_auto_inc_size) {
        newTrunkSize = buffer->m_auto_inc_size;
    }

    trunk = mem_buffer_append_trunk(buffer, newTrunkSize);
    if (trunk == NULL) {
        return -1;
    }

    writedSize += mem_trunk_append(buffer, trunk, (const char*)buf + writedSize, size - writedSize);

    return size;
}

ssize_t mem_buffer_append_char(mem_buffer_t buffer, char data) {
    return mem_buffer_append(buffer, &data, 1);
}

void * mem_buffer_make_continuous(struct mem_buffer * buffer, size_t reserve) {
    struct mem_buffer_trunk * trunk;

    if (!buffer) return NULL;

    trunk = TAILQ_FIRST(&buffer->m_trunks);

    if (trunk == TAILQ_END(&buffer->m_trunks)) {
        assert(buffer->m_size == 0);
        if (reserve > 0) {
            if (reserve < buffer->m_auto_inc_size) reserve = buffer->m_auto_inc_size;

            trunk = mem_trunk_alloc(buffer->m_default_allocrator, reserve);
            if (trunk == NULL) {
                return NULL;
            }

            TAILQ_INSERT_HEAD(&buffer->m_trunks, trunk, m_next);
            buffer->m_size = trunk->m_size;

            return mem_trunk_data(trunk);
        }
        else {
            return NULL;
        }
    }

    if (TAILQ_NEXT(trunk, m_next) == TAILQ_END(&buffer->m_trunks)
        && trunk->m_capacity >= (trunk->m_size + reserve))
    {
        return mem_trunk_data(trunk);
    }

    if (reserve < buffer->m_auto_inc_size) reserve = buffer->m_auto_inc_size;

    trunk = mem_trunk_alloc(buffer->m_default_allocrator, buffer->m_size + reserve);
    if (trunk == NULL) {
        return NULL;
    }

    if (mem_buffer_read(mem_trunk_data(trunk), buffer->m_size, buffer) == -1) {
        mem_trunk_free(NULL, trunk);
        return NULL;
    }
    trunk->m_size = buffer->m_size;

    mem_buffer_clear(buffer);

    TAILQ_INSERT_HEAD(&buffer->m_trunks, trunk, m_next);
    buffer->m_size += trunk->m_size;

    return mem_trunk_data(trunk);
}

void * mem_buffer_make_exactly(struct mem_buffer * buffer) {
    struct mem_buffer_trunk * trunk;

    if (!buffer) return NULL;

    trunk = TAILQ_FIRST(&buffer->m_trunks);

    if (trunk == TAILQ_END(&buffer->m_trunks)) {
        return NULL;
    }

    if (TAILQ_NEXT(trunk, m_next) == TAILQ_END(&buffer->m_trunks)
        && trunk->m_capacity == (trunk->m_size))
    {
        return mem_trunk_data(trunk);
    }

    trunk = mem_trunk_alloc(buffer->m_default_allocrator, buffer->m_size);
    if (trunk == NULL) {
        return NULL;
    }

    if (mem_buffer_read(mem_trunk_data(trunk), buffer->m_size, buffer) == -1) {
        mem_trunk_free(NULL, trunk);
        return NULL;
    }
    trunk->m_size = buffer->m_size;

    mem_buffer_clear(buffer);

    TAILQ_INSERT_HEAD(&buffer->m_trunks, trunk, m_next);
    buffer->m_size += trunk->m_size;

    return mem_trunk_data(trunk);
}

void * mem_buffer_alloc(struct mem_buffer * buffer, size_t size) {
    void * result = NULL;
    struct mem_buffer_trunk * trunk = NULL;

    if (!buffer || size <= 0) return NULL;

    trunk = TAILQ_LAST(&buffer->m_trunks, mem_buffer_trunk_list);
    if (trunk == NULL || trunk->m_size + size > trunk->m_capacity) {
        trunk = mem_buffer_append_trunk(
            buffer,
            buffer->m_auto_inc_size > size
            ? buffer->m_auto_inc_size
            : size);
        if (trunk == NULL) {
            return NULL;
        }

        result = mem_trunk_data(trunk);
    }
    else {
        result = (char*)mem_trunk_data(trunk) + trunk->m_size;
    }

    trunk->m_size += size;
    buffer->m_size += size;

    return result;
}

char * mem_buffer_strdup(struct mem_buffer * buffer, const char * s) {
    size_t n;
    char * p;

    if (!s) return NULL;

    n = strlen(s);
    p = (char*)mem_buffer_alloc(buffer, n + 1);
    if (p == NULL) {
        return NULL;
    }

    memcpy(p, s, n + 1);
    return p;
}

char * mem_buffer_strdup_len(mem_buffer_t buffer, const char * s, size_t sLen) {
    char * p;

    if (!s) return NULL;

    p = (char*)mem_buffer_alloc(buffer, sLen + 1);
    if (p == NULL) return NULL;

    memcpy(p, s, sLen);

    p[sLen] = 0;

    return p;
}

char * mem_buffer_strdup_range(mem_buffer_t buffer, const char * s, const char * end) {
    return mem_buffer_strdup_len(buffer, s, end - s);
}

size_t mem_buffer_trunk_count(mem_buffer_t buffer) {
    size_t count;
    struct mem_buffer_trunk * trunk;

    count = 0;
    trunk = TAILQ_FIRST(&buffer->m_trunks);
    while (trunk != TAILQ_END(&buffer->m_trunks)) {
        ++count;
        trunk = TAILQ_NEXT(trunk, m_next);
    }

    return count;
}

int mem_buffer_strcat(mem_buffer_t buffer, const char * s) {
    struct mem_buffer_trunk * trunk;
    char * buf;
    size_t copySize;
    size_t copyLeft;

    if (buffer->m_size == 0)
        return mem_buffer_strdup(buffer, s) ? 0 : -1;

    trunk = TAILQ_LAST(&buffer->m_trunks, mem_buffer_trunk_list);
    assert(trunk);

    while(trunk && trunk->m_size == 0) {
        trunk = TAILQ_PREV(trunk, mem_buffer_trunk_list, m_next);
    }

    if (trunk == NULL) return -1;
    assert(trunk->m_size > 0);

    buf = (char*)mem_trunk_data(trunk);
    if (buf[trunk->m_size - 1] != 0) return -1;

    copyLeft = strlen(s) + 1;
    copySize = trunk->m_capacity - trunk->m_size + 1;
    if (copySize > copyLeft) copySize = copyLeft;

    memcpy(buf + (trunk->m_size - 1), s, copySize);
    trunk->m_size += (copySize - 1);
    buffer->m_size += (copySize - 1);
    copyLeft -= copySize;
    
    if (copyLeft > 0) {
        buf = (char*)mem_buffer_alloc(buffer, copyLeft);
        if (buf == NULL) return -1;
        memcpy(buf, s + copySize, copyLeft);
    }

    return 0;
}

int mem_buffer_printf(mem_buffer_t buffer, const char * fmt, ...) {
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    va_list args;
    int rv;
    
    mem_buffer_clear_data(buffer);

    va_start(args, fmt);
    rv = stream_vprintf((write_stream_t)&s, fmt, args);
    va_end(args);

    stream_putc((write_stream_t)&s, 0);
    
    return rv;
}

struct mem_buffer_trunk *
mem_buffer_trunk_at(mem_buffer_t buffer, size_t pos) {
    struct mem_buffer_trunk * trunk;

    trunk = TAILQ_FIRST(&buffer->m_trunks);
    while (pos > 0 && trunk != TAILQ_END(&buffer->m_trunks)) {
        --pos;
        trunk = TAILQ_NEXT(trunk, m_next);
    }

    return trunk;
}

int mem_buffer_set_size(mem_buffer_t buffer, size_t size) {
    struct mem_buffer_trunk * trunk;
    size_t delta;

    if (size == buffer->m_size) return 0;

    if (size > buffer->m_size) {
        return mem_buffer_alloc(buffer, size - buffer->m_size) ? 0 : -1;
    }

    /*size < buffer->m_size*/
    delta = buffer->m_size - size;
    trunk = TAILQ_LAST(&buffer->m_trunks, mem_buffer_trunk_list);

    while(trunk && delta > 0) {
        size_t releaseSize = trunk->m_size;
        if (releaseSize > delta) releaseSize = delta;
        trunk->m_size -= releaseSize;
        buffer->m_size -= releaseSize;
        delta -= releaseSize;
        trunk = TAILQ_PREV(trunk, mem_buffer_trunk_list, m_next);
    }

    return delta == 0 ? 0 : -1;
}
