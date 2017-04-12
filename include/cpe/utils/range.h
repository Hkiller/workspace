#ifndef CPE_UTILS_RANGEMGR_H
#define CPE_UTILS_RANGEMGR_H
#include "cpe/pal/pal_types.h"
#include "memory.h"
#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cpe_range {
    ptr_int_t m_start;
    ptr_int_t m_end;
};

typedef struct cpe_range_mgr {
    mem_allocrator_t m_alloc;
    size_t m_range_capacity;
    size_t m_range_count;
    struct cpe_range * m_ranges;
    void * m_debug_info;
} * cpe_range_mgr_t;

typedef struct cpe_range_it {
    cpe_range_mgr_t m_mgr;
    int m_pos;
} * cpe_range_it_t;

int cpe_range_mgr_init(cpe_range_mgr_t ra, mem_allocrator_t alloc);
void cpe_range_mgr_fini(cpe_range_mgr_t ra);
void cpe_range_mgr_clear(cpe_range_mgr_t ra);
void cpe_range_mgr_dump(write_stream_t stream, cpe_range_mgr_t ra);
int cpe_range_mgr_reserve_for_put(cpe_range_mgr_t ra, int put_count);
int cpe_range_mgr_is_empty(cpe_range_mgr_t ra);

int cpe_range_is_conflict(cpe_range_mgr_t ra, ptr_int_t start, ptr_int_t end);

int cpe_range_is_valid(struct cpe_range r);
int cpe_range_size(struct cpe_range r);

ptr_int_t cpe_range_get_one(cpe_range_mgr_t ra);
int cpe_range_put_one(cpe_range_mgr_t ra, ptr_int_t value);

void cpe_range_set_debug(cpe_range_mgr_t ra, int is_debug);

struct cpe_range cpe_range_get_range(cpe_range_mgr_t ra, size_t require_count);
int cpe_range_put_range(cpe_range_mgr_t ra, ptr_int_t start, ptr_int_t end);

int cpe_range_remove_one(cpe_range_mgr_t ra, ptr_int_t);

struct cpe_range cpe_range_find(cpe_range_mgr_t ra, ptr_int_t value);

void cpe_range_mgr_ranges(cpe_range_it_t it, cpe_range_mgr_t ra);
struct cpe_range cpe_range_it_next(cpe_range_it_t it);

extern struct cpe_range cpe_range_invalid;

struct cpe_urange {
    ptr_uint_t m_start;
    ptr_uint_t m_end;
};

typedef struct cpe_urange_mgr {
    mem_allocrator_t m_alloc;
    size_t m_urange_capacity;
    size_t m_urange_count;
    struct cpe_urange * m_uranges;
    void * m_debug_info;
} * cpe_urange_mgr_t;

typedef struct cpe_urange_it {
    cpe_urange_mgr_t m_mgr;
    int m_pos;
} * cpe_urange_it_t;

int cpe_urange_mgr_init(cpe_urange_mgr_t ra, mem_allocrator_t alloc);
void cpe_urange_mgr_fini(cpe_urange_mgr_t ra);
void cpe_urange_mgr_clear(cpe_urange_mgr_t ra);
void cpe_urange_mgr_dump(write_stream_t stream, cpe_urange_mgr_t ra);
int cpe_urange_mgr_reserve_for_put(cpe_urange_mgr_t ra, int put_count);
int cpe_urange_mgr_is_empty(cpe_urange_mgr_t ra);

int cpe_urange_is_conflict(cpe_urange_mgr_t ra, ptr_uint_t start, ptr_uint_t end);

int cpe_urange_is_valid(struct cpe_urange r);
int cpe_urange_size(struct cpe_urange r);

int cpe_urange_get_one(cpe_urange_mgr_t ra, ptr_uint_t * begin);
int cpe_urange_put_one(cpe_urange_mgr_t ra, ptr_uint_t value);

void cpe_urange_set_debug(cpe_urange_mgr_t ra, int is_debug);

struct cpe_urange cpe_urange_get_urange(cpe_urange_mgr_t ra, size_t require_count);
int cpe_urange_put_urange(cpe_urange_mgr_t ra, ptr_uint_t start, ptr_uint_t end);

struct cpe_urange cpe_urange_find(cpe_urange_mgr_t ra, ptr_uint_t value);

void cpe_urange_mgr_uranges(cpe_urange_it_t it, cpe_urange_mgr_t ra);
struct cpe_urange cpe_urange_it_next(cpe_urange_it_t it);

extern struct cpe_urange cpe_urange_invalid;

#ifdef __cplusplus
}
#endif

#endif
