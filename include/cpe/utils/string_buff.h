#ifndef CPE_UTILS_STRING_BUFF_H
#define CPE_UTILS_STRING_BUFF_H
#include "error.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cpe_string_buff * cpe_string_buff_t;

cpe_string_buff_t cpe_string_buff_create(mem_allocrator_t alloc, error_monitor_t em);
void cpe_string_buff_free(cpe_string_buff_t str_buff);

const char * cpe_string_buff_add(cpe_string_buff_t str_buff, const char * str, uint32_t * str_id);
void cpe_string_buff_remove(cpe_string_buff_t str_buff, const char * str);

const char * 
#ifdef __cplusplus
}
#endif

#endif
