#ifndef CPE_UTILS_STRING_URL_H
#define CPE_UTILS_STRING_URL_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t cpe_url_encode(char * result, size_t result_capacity, const char * input, size_t input_size, error_monitor_t em);
ssize_t cpe_url_decode(char * result, size_t result_capacity, const char * input, size_t input_size, error_monitor_t em);
size_t cpe_url_decode_inline(char * str, size_t len);
    
#ifdef __cplusplus
}
#endif

#endif
