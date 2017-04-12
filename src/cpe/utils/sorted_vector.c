#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/sorted_vector.h"
#include "cpe/utils/algorithm.h"

void cpe_sorted_vector_init(
    cpe_sorted_vector_t v,
    void * data, size_t capacity, size_t size, size_t width, int (*compar)(const void *, const void *))
{
    assert(size <= capacity);

    v->m_head = data;
    v->m_width = width;
    v->m_size = size;
    v->m_capacity = capacity;
    v->m_compar = compar;
}

size_t cpe_sorted_vector_size(cpe_sorted_vector_t v) {
    return v->m_size;
}

size_t cpe_sorted_vector_capacity(cpe_sorted_vector_t v) {
    return v->m_capacity;
}

void cpe_sorted_vector_set_size(cpe_sorted_vector_t v, size_t size) {
    assert(size <= v->m_capacity);
    v->m_size = size;
}

uint8_t cpe_sorted_vector_is_full(cpe_sorted_vector_t v) {
    return v->m_size >= v->m_capacity;
}

void * cpe_sorted_vector_begin(cpe_sorted_vector_t v) {
    return v->m_head;
}

void * cpe_sorted_vector_end(cpe_sorted_vector_t v) {
    return ((char*)v->m_head) + (v->m_width * v->m_size);
}

void * cpe_sorted_vector_lower_bound(cpe_sorted_vector_t v, void const * key) {
    return cpe_lower_bound(v->m_head, v->m_size, key, v->m_width, v->m_compar);
}

void * cpe_sorted_vector_upper_bound(cpe_sorted_vector_t v, void const * key) {
    return cpe_upper_bound(v->m_head, v->m_size, key, v->m_width, v->m_compar);
}

void * cpe_sorted_vector_find_first(cpe_sorted_vector_t v, void const * key) {
    void * end = cpe_sorted_vector_end(v);
    void * r = cpe_sorted_vector_lower_bound(v, key);

    if (r == end || v->m_compar(r, key) != 0) return NULL;

    return r;
}

void cpe_sorted_vector_erase(cpe_sorted_vector_t v, void * data) {
    void * end;
    size_t left;

    end = cpe_sorted_vector_end(v);

    assert(data >= v->m_head && data < end);

    left = ((char*)end) - ((char*)data);

    assert((left % v->m_width) == 0);
    assert(left >= v->m_width);

    memmove(data, ((char*)data) + v->m_width, left - v->m_width);
    v->m_size--;
}

int cpe_sorted_vector_insert_at(cpe_sorted_vector_t v, void * insert_pos, void const * key) {
    void * end;

    if (v->m_size + 1 > v->m_capacity) return -1;

    end = cpe_sorted_vector_end(v);

    assert(insert_pos >= v->m_head);
    assert(insert_pos <= end);

    if (insert_pos != end) {
        memmove(
            ((char*)insert_pos) + v->m_width,
            insert_pos,
            ((char*)end) - ((char*)insert_pos));
    }

    memcpy(insert_pos, key, v->m_width);

    v->m_size++;

    return 0;
}

int cpe_sorted_vector_insert_unique(cpe_sorted_vector_t v, void const * key) {
    void * end;
    void * insert_pos;

    if (v->m_size + 1 > v->m_capacity) return -1;

    end = cpe_sorted_vector_end(v);
    
    insert_pos = cpe_sorted_vector_lower_bound(v, key);

    if (insert_pos != end && v->m_compar(insert_pos, key) == 0) return -1;

    if (insert_pos != end) {
        memmove(
            ((char*)insert_pos) + v->m_width,
            insert_pos,
            ((char*)end) - ((char*)insert_pos));
    }

    memcpy(insert_pos, key, v->m_width);

    v->m_size++;

    return 0;
}
