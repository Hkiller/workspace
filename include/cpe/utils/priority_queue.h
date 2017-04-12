#ifndef CPE_UTILS_PRIORITY_QUEUE_H
#define CPE_UTILS_PRIORITY_QUEUE_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*cpe_priority_queue_cmp_fun_t)(void const * l, void const * r);
typedef void (*cpe_priority_queue_print_fun_t)(write_stream_t s, void const * e);
    
cpe_priority_queue_t
cpe_priority_queue_create(
    mem_allocrator_t alloc, error_monitor_t em,
    uint16_t ele_size, cpe_priority_queue_cmp_fun_t ele_cmp,
    uint16_t init_capacity);

void cpe_priority_queue_free(cpe_priority_queue_t queue);

uint16_t cpe_priority_queue_count(cpe_priority_queue_t queue);

void * cpe_priority_queue_top(cpe_priority_queue_t queue);
int cpe_priority_queue_pop(cpe_priority_queue_t queue);    

int cpe_priority_queue_insert(cpe_priority_queue_t queue, void * ele);

void cpe_priority_queue_dump(
    write_stream_t s, cpe_priority_queue_t queue, cpe_priority_queue_print_fun_t p, const char * sep);

#ifdef __cplusplus
}
#endif

#endif
