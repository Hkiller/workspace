#ifndef CPE_UTILS_BITARR_H
#define CPE_UTILS_BITARR_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cpe_ba_value {
    cpe_ba_false = 0,
    cpe_ba_true = 1
} cpe_ba_value_t;

typedef struct cpe_ba * cpe_ba_t;

cpe_ba_t cpe_ba_create(mem_allocrator_t alloc, size_t bit_capacity);
void cpe_ba_copy(cpe_ba_t target, cpe_ba_t source, size_t bit_capacity);
size_t cpe_ba_count(cpe_ba_t ba, size_t bit_capacity);
void cpe_ba_set_all(cpe_ba_t ba, size_t bit_capacity, cpe_ba_value_t value);

cpe_ba_t cpe_ba_create_from_string(mem_allocrator_t alloc, const char * str);
void cpe_ba_set_from_string(cpe_ba_t ba, size_t bit_capacity, const char * str);
char * cpe_ba_to_str(char * dest, cpe_ba_t ba, size_t bit_capacity);
char * cpe_ba_to_str_create(mem_allocrator_t alloc, cpe_ba_t ba, size_t bit_capacity);

void cpe_ba_complement(cpe_ba_t ba, size_t bit_capacity);
ssize_t cpe_ba_next_pos(cpe_ba_t ba, size_t bit_capacity);

/*
  size_t cpe_ba_bytes_from_bits(size_t bit_capacity); */

#define cpe_ba_bytes_from_bits_m(nbits) ((((size_t)nbits) >> 3) + (((size_t)nbits) % 8 ? 1 : 0))

size_t cpe_ba_bytes_from_bits(size_t nbits);
#define cpe_ba_bits_from_bytes(nbits) ((size_t)(nbits << 3))

/*
  cpe_ba_value_t cpe_ba_get(cpe_ba_t ba, size_t pos); */
#define cpe_ba_get(ba, pos) \
    ((((const uint8_t *)(ba))[((size_t)pos) >> 3] & (1 << ((size_t)pos) % 8) ) \
     ? cpe_ba_true : cpe_ba_false)
    
/*
  void cpe_ba_set(cpe_ba_t ba, size_t pos, cpe_ba_value_t value); */
#define cpe_ba_set(ba, pos, value)                                      \
    if (value) ((uint8_t *)(ba))[((size_t)pos) >> 3] |= (1 << (((size_t)pos) % 8) ); \
    else ((uint8_t *)(ba))[((size_t)pos) >> 3] &= ~(1 << (((size_t)pos) % 8) )

/*
  void cpe_ba_toggle(cpe_ba_t ba, size_t pos); */
#define cpe_ba_toggle(ba, pos, value)                       \
    ((const uint8_t *)(ba))[((size_t)pos) >> 3] ^= (1 << (((size_t)pos) % 8) )


#ifdef __cplusplus
}
#endif

#endif
