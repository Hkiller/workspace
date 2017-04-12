#ifndef CPE_UTILS_HASHSTRING_H
#define CPE_UTILS_HASHSTRING_H
#include "cpe/pal/pal_string.h"
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef char cpe_hash_string_buf[];

size_t cpe_hs_len(cpe_hash_string_t hs);
size_t cpe_hs_binary_len(cpe_hash_string_t hs);
uint32_t cpe_hs_value(cpe_hash_string_t hs);

cpe_hash_string_t cpe_hs_create(mem_allocrator_t alloc, const char * data);
void cpe_hs_copy(cpe_hash_string_t target, cpe_hash_string_t source);
void cpe_hs_init(cpe_hash_string_t target, size_t capacity, const char * source);
cpe_hash_string_t cpe_hs_copy_create(mem_allocrator_t alloc, cpe_hash_string_t source);
void cpe_hs_strcat(cpe_hash_string_t target, size_t capacity, const char * data);
void cpe_hs_printf(cpe_hash_string_t target, size_t capacity, const char * fmt, ...);

#define cpe_hs_data(hs) (((const char *)(hs)) + 8)
#define cpe_hs_from_str(str) ((cpe_hash_string_t)((((const char *)(str)) - 8)))

#define cpe_hs_cmp(__l, __r)                                    \
    ( cpe_hs_value(__l) == cpe_hs_value(__r)                    \
      ? strcmp(cpe_hs_data(__l), cpe_hs_data(__r))              \
      : (((int)cpe_hs_value(__l)) - ((int)cpe_hs_value(__r))))

#define cpe_hs_len_to_binary_len(__len) ((__len) + 8 + 1)

#define CPE_HS_BUF_MAKE(__str) {"\0\0\0\0\0\0\0\0" __str}

#define CPE_HS_DEF_VAR(__arg_name, __str)                               \
    cpe_hash_string_buf __arg_name ## _buf = CPE_HS_BUF_MAKE(__str);   \
    cpe_hash_string_t __arg_name = (cpe_hash_string_t)& __arg_name ## _buf

#ifdef __cplusplus
}
#endif

#endif
