#ifndef CPE_MEM_BUFFER_H
#define CPE_MEM_BUFFER_H
#include "cpe/pal/pal_queue.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

TAILQ_HEAD(mem_buffer_trunk_list, mem_buffer_trunk);

struct mem_buffer {
    struct mem_allocrator * m_default_allocrator;
    size_t m_size;
    size_t m_auto_inc_size;
    struct mem_buffer_trunk_list m_trunks;
};

struct mem_buffer_pos {
    struct mem_buffer * m_buffer;
    struct mem_buffer_trunk * m_trunk;
    size_t m_pos_in_trunk;
};

/* trunk operations */
struct mem_buffer_trunk *
mem_buffer_append_trunk(mem_buffer_t buffer, size_t capacity);
struct mem_buffer_trunk *
mem_buffer_append_trunk_after(mem_buffer_t buffer, struct mem_buffer_trunk * preTrunk, size_t capacity);
void mem_trunk_free(mem_buffer_t buffer, struct mem_buffer_trunk * trunk);

size_t mem_buffer_trunk_count(mem_buffer_t buffer);
struct mem_buffer_trunk * mem_buffer_trunk_at(mem_buffer_t buffer, size_t pos);

struct mem_buffer_trunk * mem_buffer_trunk_first(mem_buffer_t buffer);
struct mem_buffer_trunk * mem_buffer_trunk_next(struct mem_buffer_trunk * trunk);

void * mem_trunk_data(struct mem_buffer_trunk * trunk);
size_t mem_trunk_capacity(struct mem_buffer_trunk * trunk);
size_t mem_trunk_size(struct mem_buffer_trunk * trunk);
size_t mem_trunk_append(mem_buffer_t buffer, struct mem_buffer_trunk * trunk, const void * buf, size_t size);
void mem_trunk_set_size(mem_buffer_t buffer, struct mem_buffer_trunk * trunk, size_t size);

/* buffer basic operations */
size_t mem_buffer_size(mem_buffer_t buffer);
int mem_buffer_set_size(mem_buffer_t buffer, size_t size);

void mem_buffer_init(mem_buffer_t buffer, struct mem_allocrator * allocrator);
void mem_buffer_clear(mem_buffer_t buffer);
void mem_buffer_clear_data(mem_buffer_t buffer);

/* buffer position operations */
void mem_buffer_begin(mem_buffer_pos_t pos, mem_buffer_t buffer);
void mem_buffer_end(mem_buffer_pos_t pos, mem_buffer_t buffer);
void mem_pos_at(mem_buffer_pos_t pos, mem_buffer_t buffer, size_t n);
ssize_t mem_pos_seek(mem_buffer_pos_t pos, ssize_t n);
void * mem_pos_insert_alloc(mem_buffer_pos_t pos, size_t n);
ssize_t mem_pos_insert(mem_buffer_pos_t pos, const void * buf, size_t size);
size_t mem_pos_write(mem_buffer_pos_t l, const void * data, size_t n);

int mem_pos_eq(mem_buffer_pos_t l, mem_buffer_pos_t r);
int mem_pos_valide(mem_buffer_pos_t l);
ssize_t mem_pos_diff(mem_buffer_pos_t l, mem_buffer_pos_t r);
char mem_pos_data(mem_buffer_pos_t l);
size_t mem_pos_read(mem_buffer_pos_t l, void * data, size_t n);

/* buffer read write operations */
ssize_t mem_buffer_append(mem_buffer_t buffer, const void * buf, size_t size);
ssize_t mem_buffer_append_char(mem_buffer_t buffer, char);
ssize_t mem_buffer_read(void * buf, size_t size, mem_buffer_t buffer);
void * mem_buffer_make_continuous(mem_buffer_t buffer, size_t reserve);
void * mem_buffer_make_exactly(mem_buffer_t buffer);
void * mem_buffer_alloc(mem_buffer_t buffer, size_t size);

char * mem_buffer_strdup(mem_buffer_t buffer, const char * s);
char * mem_buffer_strdup_len(mem_buffer_t buffer, const char * s, size_t sLen);
char * mem_buffer_strdup_range(mem_buffer_t buffer, const char * s, const char * end);
int mem_buffer_strcat(mem_buffer_t buffer, const char * s);
int mem_buffer_printf(mem_buffer_t buffer, const char * fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
