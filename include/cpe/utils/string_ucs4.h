#ifndef CPE_UTILS_STRING_UCS4_H
#define CPE_UTILS_STRING_UCS4_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

cpe_str_ucs4_t cpe_str_ucs4_alloc(mem_allocrator_t alloc, size_t len);
cpe_str_ucs4_t cpe_str_ucs4_dup_str(mem_allocrator_t alloc, uint32_t const * src);    
cpe_str_ucs4_t cpe_str_ucs4_dup_len(mem_allocrator_t alloc, uint32_t const * src, size_t len);
cpe_str_ucs4_t cpe_str_ucs4_dup_range(mem_allocrator_t alloc, uint32_t const * begin, uint32_t const * end);
cpe_str_ucs4_t cpe_str_ucs4_dup(mem_allocrator_t alloc, cpe_str_ucs4_t src);
void cpe_str_ucs4_free(cpe_str_ucs4_t ucs4_str);

size_t cpe_str_ucs4_len(cpe_str_ucs4_t ucs4_str);
uint32_t * cpe_str_ucs4_data(cpe_str_ucs4_t ucs4_str);

/*转换 */
cpe_str_ucs4_t cpe_str_ucs4_from_utf8(mem_allocrator_t alloc, const char * src);
cpe_str_ucs4_t cpe_str_ucs4_from_utf8_range(mem_allocrator_t alloc, const char * begin, const char * end);
cpe_str_ucs4_t cpe_str_ucs4_from_utf8_len(mem_allocrator_t alloc, const char * src, size_t len);

char * cpe_str_ucs4_to_utf8(cpe_str_ucs4_t ucs4_str, mem_allocrator_t alloc);
char * cpe_str_utf8_from_ucs4_range(mem_allocrator_t alloc, uint32_t const * begin, uint32_t const * end);
char * cpe_str_utf8_from_ucs4_len(mem_allocrator_t alloc, uint32_t const * begin, size_t len);

/*转换缓冲大小 */
size_t cpe_str_ucs4_clen(const uint32_t * begin, const uint32_t * end);
size_t cpe_str_utf8_wlen(const char * begin, const char * end);

size_t cpe_char_ucs4_clen(uint32_t c);
size_t cpe_char_ucs4_to_utf8(char * r, size_t capacity, uint32_t c);

#ifdef __cplusplus
}
#endif

#endif
