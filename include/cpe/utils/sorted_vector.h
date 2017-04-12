#ifndef CPE_UTILS_SORTED_VECTOR_H
#define CPE_UTILS_SORTED_VECTOR_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cpe_sorted_vector {
    void * m_head;
    size_t m_width;
    size_t m_size;
    size_t m_capacity;
    int (*m_compar)(const void *, const void *);
} * cpe_sorted_vector_t;

void cpe_sorted_vector_init(
    cpe_sorted_vector_t v,
    void * data, size_t capacity, size_t size, size_t width, int (*compar)(const void *, const void *));

size_t cpe_sorted_vector_size(cpe_sorted_vector_t v);
void cpe_sorted_vector_set_size(cpe_sorted_vector_t v, size_t size);

uint8_t cpe_sorted_vector_is_full(cpe_sorted_vector_t v);
size_t cpe_sorted_vector_capacity(cpe_sorted_vector_t v);

void * cpe_sorted_vector_begin(cpe_sorted_vector_t v);
void * cpe_sorted_vector_end(cpe_sorted_vector_t v);

void * cpe_sorted_vector_lower_bound(cpe_sorted_vector_t v, void const * key);
void * cpe_sorted_vector_upper_bound(cpe_sorted_vector_t v, void const * key);

void * cpe_sorted_vector_find_first(cpe_sorted_vector_t v, void const * key);

void cpe_sorted_vector_erase(cpe_sorted_vector_t v, void * data);

int cpe_sorted_vector_insert_at(cpe_sorted_vector_t v, void * insert_pos, void const * key);
int cpe_sorted_vector_insert_unique(cpe_sorted_vector_t v, void const * key);

#define CPE_SORTED_VECTOR_INIT_FROM_ARRAY(v, __array, __compar)                  \
    cpe_sorted_vector_init(v, (__array), CPE_ARRAY_SIZE(__array), 0, sizeof((__array)[0]), __compar);

#ifdef __cplusplus
}
#endif

#endif
