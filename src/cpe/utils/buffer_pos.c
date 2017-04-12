#include <assert.h>
#include <string.h>
#include "buffer_private.h"

void mem_buffer_begin(struct mem_buffer_pos * pos, struct mem_buffer * buffer) {
    assert(pos);
    assert(buffer);

    pos->m_buffer = buffer;
    pos->m_pos_in_trunk = 0;

    pos->m_trunk = TAILQ_FIRST(&buffer->m_trunks);
    while(pos->m_trunk != TAILQ_END(&buffer->m_trunks) && pos->m_trunk->m_size <= 0) {
        pos->m_trunk = TAILQ_NEXT(pos->m_trunk, m_next);
    }
}

void mem_buffer_end(struct mem_buffer_pos * pos, struct mem_buffer * buffer) {
    assert(pos);
    assert(buffer);

    pos->m_buffer = buffer;
    pos->m_trunk = TAILQ_END(&buffer->m_trunks);
    pos->m_pos_in_trunk = 0;
}

ssize_t mem_pos_seek(struct mem_buffer_pos * pos, ssize_t n) {
    ssize_t done = 0;

    assert(pos);
    assert(pos->m_buffer);

    if (n > 0) {
        while(pos->m_trunk && n > 0) {
            int left = (int)(pos->m_trunk->m_size - pos->m_pos_in_trunk - 1);
            if (left >= n) {
                pos->m_pos_in_trunk += n;
                done += n;
                n = 0;
            }
            else {
                done += (left + 1);
                n -= (left + 1);

                do {
                    pos->m_trunk = TAILQ_NEXT(pos->m_trunk, m_next);
                } while(pos->m_trunk && pos->m_trunk->m_size == 0);

                pos->m_pos_in_trunk = 0;
            }
        }
    }
    else if (n < 0) {
        n = -n;
        while(n > 0) {
            ssize_t d;

            if (pos->m_trunk == NULL || pos->m_pos_in_trunk == 0) {
                struct mem_buffer_trunk * pre;
                pre = 
                    pos->m_trunk == NULL
                    ? TAILQ_LAST(&pos->m_buffer->m_trunks, mem_buffer_trunk_list)
                    : TAILQ_PREV(pos->m_trunk, mem_buffer_trunk_list, m_next);
                
                while(pre && pre != TAILQ_FIRST(&pos->m_buffer->m_trunks) && pre->m_size <= 0) {
                    pre = TAILQ_PREV(pre, mem_buffer_trunk_list, m_next);
                }

                if (pre && pre->m_size > 0) {
                    pos->m_trunk = pre;
                    pos->m_pos_in_trunk = pre->m_size - 1;
                    done -= 1;
                    n -= 1;
                }
                else {
                    break;
                }
            }

            assert(pos->m_trunk);
            assert(pos->m_trunk->m_size > 0);

            d = (ssize_t)pos->m_pos_in_trunk;
            if (d == 0) continue;
            if (d >= n) d = n;

            assert(pos->m_pos_in_trunk >= (size_t)d);
            pos->m_pos_in_trunk -= d;
            done -= d;
            n -= d;
        }
    }

    return done;
}

void mem_pos_at(struct mem_buffer_pos * pos, struct mem_buffer * buffer, size_t n) {
    assert(pos);
    assert(buffer);

    mem_buffer_begin(pos, buffer);
    mem_pos_seek(pos, n);
}

int mem_pos_eq(mem_buffer_pos_t l, mem_buffer_pos_t r) {
    assert(l);
    assert(r);

    return l->m_buffer == r->m_buffer
        && l->m_trunk == r->m_trunk
        && l->m_pos_in_trunk == r->m_pos_in_trunk
        ? 1
        : 0;
}

static ssize_t mem_pos_diff_i(mem_buffer_pos_t l, mem_buffer_pos_t r) {
    ssize_t from_l = 0;
    struct mem_buffer_trunk * trunk = l->m_trunk;

    while(trunk && trunk != r->m_trunk) {
        from_l += (trunk->m_size - l->m_pos_in_trunk);
        trunk = TAILQ_NEXT(trunk, m_next);
        l->m_pos_in_trunk = 0;
    }

    if (trunk == r->m_trunk) {
        if (trunk) {
            assert(trunk == r->m_trunk);
            return from_l + r->m_pos_in_trunk - l->m_pos_in_trunk;
        }
        else {
            return from_l;
        }
    }
    else {
        return -1;
    }
}

ssize_t mem_pos_diff(mem_buffer_pos_t l, mem_buffer_pos_t r) {
    ssize_t result;

    assert(l);
    assert(r);
    assert(l->m_buffer == r->m_buffer);

    result = mem_pos_diff_i(l, r);
    if (result >= 0) return result;

    return -  mem_pos_diff_i(r, l);
}

int mem_pos_valide(mem_buffer_pos_t l) {
    return l
        && l->m_buffer
        && l->m_trunk
        && l->m_pos_in_trunk < l->m_trunk->m_size
        ? 1
        : 0;
}

char mem_pos_data(mem_buffer_pos_t l) {
    assert(mem_pos_valide(l));
    return ((char *)mem_trunk_data(l->m_trunk))[l->m_pos_in_trunk];
}

size_t mem_pos_read(mem_buffer_pos_t l, void * data, size_t n) {
    size_t readed_size = 0;
    struct mem_buffer_trunk * trunk = l->m_trunk;
    size_t pos_in_trunk = l->m_pos_in_trunk;
    
    while(n > 0 && trunk) {
        size_t once_size;
        
        assert(pos_in_trunk <= trunk->m_size);
        once_size = trunk->m_size - pos_in_trunk;

        if (once_size > n) once_size = n;
        
        memcpy(data, ((char*)mem_trunk_data(trunk)) + pos_in_trunk, once_size);

        readed_size += once_size;
        n -= once_size;
        data = ((char*)data) + once_size;
        trunk = TAILQ_NEXT(trunk, m_next);
        pos_in_trunk = 0;
    }
    
    return readed_size;
}

size_t mem_pos_write(mem_buffer_pos_t l, void const * data, size_t n) {
    size_t writed_size = 0;
    struct mem_buffer_trunk * trunk = l->m_trunk;
    size_t pos_in_trunk = l->m_pos_in_trunk;
    
    while(n > 0 && trunk) {
        size_t once_size;
        struct mem_buffer_trunk * next_data_trunk;

        next_data_trunk = TAILQ_NEXT(trunk, m_next);
        while(next_data_trunk && next_data_trunk->m_size == 0) {
            next_data_trunk = TAILQ_NEXT(next_data_trunk, m_next);
        }
        
        assert(pos_in_trunk <= trunk->m_size);
        once_size = (next_data_trunk ? trunk->m_size : trunk->m_capacity) - pos_in_trunk;

        if (once_size > n) once_size = n;
        
        memcpy(((char*)mem_trunk_data(trunk)) + pos_in_trunk, data, once_size);

        if (next_data_trunk == NULL) {
            trunk->m_size = pos_in_trunk + once_size;
            assert(trunk->m_size <= trunk->m_capacity);
        }
        
        writed_size += once_size;
        n -= once_size;
        data = ((const char*)data) + once_size;
        trunk = next_data_trunk ? next_data_trunk : TAILQ_NEXT(trunk, m_next);
        pos_in_trunk = 0;
    }

    if (n > 0) {
        trunk = mem_buffer_append_trunk(l->m_buffer, n);
        if (trunk) {
            memcpy(((char*)mem_trunk_data(trunk)), data, n);
            trunk->m_size = n;
            l->m_buffer->m_size += n;
            writed_size += n;
        }
    }

    return writed_size;
}

void * mem_pos_insert_alloc(mem_buffer_pos_t pos, size_t n) {
    struct mem_buffer_trunk * trunk;

    assert(pos);
    assert(pos->m_buffer);

    if (n <= 0) return NULL;

    if (pos->m_trunk == NULL) {
        return mem_buffer_alloc(pos->m_buffer, n);
    }

    if (pos->m_pos_in_trunk == 0) {
        /*pos at begin of trunk, alloc a new trunk to store data*/
        trunk = mem_trunk_alloc(pos->m_buffer->m_default_allocrator, n);
        if (trunk == NULL) {
            return NULL;
        }

        TAILQ_INSERT_BEFORE(pos->m_trunk, trunk, m_next);
        pos->m_buffer->m_size += n;
        trunk->m_size = n;
        return mem_trunk_data(trunk);
    }
    else if (pos->m_trunk->m_capacity >= pos->m_pos_in_trunk + n) {
        /*new data can store in current trunk*/

        char * trunkBegin = (char *)mem_trunk_data(pos->m_trunk);
        char * result = trunkBegin + pos->m_pos_in_trunk;

        /*can`t store all data in current buffer, move overflow data to a new trunk*/
        ssize_t appendSize = pos->m_trunk->m_size + n - pos->m_trunk->m_capacity;
        if (appendSize > 0) {
            trunk = mem_trunk_alloc(pos->m_buffer->m_default_allocrator, appendSize);
            if (trunk == NULL) {
                return NULL;
            }

            TAILQ_INSERT_AFTER(&pos->m_buffer->m_trunks, pos->m_trunk, trunk, m_next);
            memcpy(
                mem_trunk_data(trunk),
                trunkBegin + pos->m_trunk->m_size - appendSize,
                appendSize);
            
            trunk->m_size = appendSize;
            pos->m_trunk->m_size -= appendSize;
        }

        if (pos->m_pos_in_trunk < pos->m_trunk->m_size) {
            memmove(
                trunkBegin + pos->m_pos_in_trunk + n,
                trunkBegin + pos->m_pos_in_trunk,
                pos->m_trunk->m_size - pos->m_pos_in_trunk);
        }
        else {
        }

        pos->m_trunk->m_size += pos->m_trunk->m_size - appendSize + n;

        pos->m_pos_in_trunk += n;

        if (pos->m_pos_in_trunk >= pos->m_trunk->m_size) {
            pos->m_pos_in_trunk = pos->m_pos_in_trunk - pos->m_trunk->m_size;
            pos->m_trunk = trunk;
        }

        assert(pos->m_pos_in_trunk < pos->m_trunk->m_size);
        assert(pos->m_trunk->m_size <= pos->m_trunk->m_capacity);
        pos->m_buffer->m_size += n;

        return result;
    }
    else {
        /*new data can not store in current trunk*/
        ssize_t moveSize = pos->m_trunk->m_size - pos->m_pos_in_trunk;
        trunk = mem_trunk_alloc(pos->m_buffer->m_default_allocrator, n + moveSize);
        if (trunk == NULL) {
            return NULL;
        }

        TAILQ_INSERT_AFTER(&pos->m_buffer->m_trunks, pos->m_trunk, trunk, m_next);

        assert(moveSize > 0);
        memcpy(
            (char *)mem_trunk_data(trunk) + n,
            (char *)mem_trunk_data(pos->m_trunk) + pos->m_pos_in_trunk,
            moveSize);

        trunk->m_size = n + moveSize;
        pos->m_trunk->m_size = pos->m_pos_in_trunk;
        pos->m_trunk = trunk;
        pos->m_pos_in_trunk = n;
        pos->m_buffer->m_size += n;

        return mem_trunk_data(trunk);
    }
}

ssize_t mem_pos_insert(mem_buffer_pos_t pos, const void * buf, size_t size) {
    void * p = mem_pos_insert_alloc(pos, size);
    if (p == NULL) return size == 0 ? 0 : -1;

    assert(buf);

    memcpy(p, buf, size);
    return (ssize_t)size;
}
