#ifndef CPE_UTILS_HTTP_ARG_H
#define CPE_UTILS_HTTP_ARG_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cpe_http_arg {
    char * name;
    char * value;
};

int cpe_http_args_parse_inline(cpe_http_arg_t args, uint16_t * arg_count, char * input, error_monitor_t em);
int cpe_http_args_remove(cpe_http_arg_t args, uint16_t * arg_count, const char * name);
void cpe_http_args_pprint(write_stream_t s, cpe_http_arg_t args, uint16_t arg_count);    
char * cpe_http_args_to_string(cpe_http_arg_t args, uint16_t arg_count, mem_buffer_t buffer);
cpe_http_arg_t cpe_http_args_find(cpe_http_arg_t args, uint16_t arg_count, const char * name);
char * cpe_http_args_find_value(cpe_http_arg_t args, uint16_t arg_count, const char * name);    

uint8_t cpe_http_args_all_exists(cpe_http_arg_t args, uint16_t arg_count, const char * * names, uint16_t name_count);

#ifdef __cplusplus
}
#endif

#endif
