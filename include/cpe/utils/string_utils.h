#ifndef CPE_UTILS_STRING_H
#define CPE_UTILS_STRING_H
#include "stream.h"
#include "error.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

char * cpe_str_dup(char * buf, size_t capacity, const char * str);
char * cpe_str_dup_trim(char * buf, size_t capacity, const char * str);
char * cpe_str_dup_range(char * buf, size_t capacity, const char * begin, const char * end);
char * cpe_str_dup_range_trim(char * buf, size_t capacity, const char * begin, const char * end);
char * cpe_str_dup_len(char * buf, size_t capacity, const char * begin, size_t size);

char * cpe_str_mem_dup(mem_allocrator_t alloc, const char * str);
char * cpe_str_mem_dup_range(mem_allocrator_t alloc, const char * str, const char * end);
char * cpe_str_mem_dup_len(mem_allocrator_t alloc, const char * str, size_t len);
char * cpe_str_mem_dup_trim(mem_allocrator_t alloc, const char * str);

typedef struct cpe_str_buf {
    size_t m_capacity;
    size_t m_size;
    char * m_buf;
    int m_overflow;
} * cpe_str_buf_t;

int cpe_str_buf_is_overflow(cpe_str_buf_t buf);
int cpe_str_buf_append(cpe_str_buf_t buf, const char * data, size_t data_len);
int cpe_str_buf_cat(cpe_str_buf_t buf, const char * data);
int cpe_str_buf_cat_printf(cpe_str_buf_t buf, const char * format, ...);
int cpe_str_buf_cpy(cpe_str_buf_t buf, const char * data);
int cpe_str_buf_printf(cpe_str_buf_t buf, const char * format, ...);

void cpe_str_toupper(char * data);
void cpe_str_tolower(char * data);

const char * cpe_str_char_not_in_pair(const char * p, char f, const char * start_set, const char * end_set);
const char * cpe_str_char_range(const char * begin, const char * end, char c);

int cpe_str_cmp_part(const char * part_str, size_t part_str_len, const char * full_str);

uint8_t cpe_str_start_with(const char * full_str, const char * prefix);
    
uint64_t cpe_str_parse_byte_size_with_dft(const char * astring, uint64_t dft);
int cpe_str_parse_byte_size(uint64_t * result, const char * astring);

uint64_t cpe_str_parse_timespan_ms_with_dft(const char * astring, uint64_t dft);
int cpe_str_parse_timespan_ms(uint64_t * result, const char * astring);

char * cpe_str_trim_head(char * p);
char * cpe_str_trim_tail(char * p, const char * head);

char * cpe_str_mask_uint16(uint16_t v, char * buf, size_t buf_size);

int cpe_str_to_bool(uint8_t * r, const char * p);
int cpe_str_to_int8(int8_t * r, const char * p);
int cpe_str_to_uint8(uint8_t * r, const char * p);
int cpe_str_to_int16(int16_t * r, const char * p);
int cpe_str_to_uint16(uint16_t * r, const char * p);
int cpe_str_to_int32(int32_t * r, const char * p);
int cpe_str_to_uint32(uint32_t * r, const char * p);
int cpe_str_to_int64(int64_t * r, const char * p);
int cpe_str_to_uint64(uint64_t * r, const char * p);
int cpe_str_to_float(float * r, const char * p);
int cpe_str_to_double(double * r, const char * p);

const char * cpe_str_format_ipv4(char * buf, size_t capacity, uint32_t ip);
uint8_t cpe_str_is_ipv4(const char * ip);

char * cpe_str_read_and_remove_arg(char * p, const char * arg_name, char sep /*,*/, char pair /*=*/);
int cpe_str_read_arg(char * r, size_t r_capacity, const char * p, const char * arg_name, char sep /*,*/, char pair /*=*/);
int cpe_str_read_arg_range(char * r, size_t r_capacity, const char * i, const char * i_end, const char * arg_name, char sep /*,*/, char pair /*=*/);
char * cpe_str_read_arg_mem_dup(mem_allocrator_t alloc, const char * p, const char * arg_name, char sep /*,*/, char pair /*=*/);
char * cpe_str_read_arg_range_mem_dup(mem_allocrator_t alloc, const char * i, const char * i_end, const char * arg_name, char sep /*,*/, char pair /*=*/);
    
uint8_t cpe_str_is_in_list(const char * check, const char * list, char sep);
void cpe_str_list_for_each(const char * list, char sep, void (*visit_fun)(void * ctx, const char * value), void * ctx);

#define CPE_STR_BUF_INIT(__b, __size) { __size, 0, __b, 0 }

#ifdef __cplusplus
}
#endif

#endif
