#include <string.h>
#include "cpe/utils/bitarry.h"
#include "cpe/utils/memory.h"

#define CPE_BA_CLR_NOT_USED(ba, numBytes, numBits)                      \
    ((uint8_t*)(ba))[numBytes - 1] &=                                       \
        (((uint8_t)0xFF) >> ((numBits % 8) ? (8 - (numBits % 8)) : 0))

cpe_ba_t
cpe_ba_create(mem_allocrator_t alloc, size_t bit_capacity) {
    size_t s = cpe_ba_bytes_from_bits(bit_capacity);
    return (cpe_ba_t)mem_alloc(alloc, s);
}

cpe_ba_t cpe_ba_create_from_string(mem_allocrator_t alloc, const char * str) {
    cpe_ba_t  r;
    size_t bit_capacity = strlen(str);

    r = cpe_ba_create(alloc, bit_capacity);
    if (r == NULL) return NULL;

    cpe_ba_set_from_string(r, bit_capacity, str);

    return r;
}

void cpe_ba_set_from_string(cpe_ba_t ba, size_t bit_capacity, const char * str) {
    int i;
    size_t len = strlen(str);
    if (len > bit_capacity) len = bit_capacity;

    cpe_ba_set_all(ba, bit_capacity, cpe_ba_false);

    for(i = 0; i < (int)bit_capacity; ++i) {
        if (str[i] != '0') {
            cpe_ba_set(ba, i, cpe_ba_true);
        }
    }
}

void cpe_ba_copy(cpe_ba_t target, cpe_ba_t source, size_t bit_capacity) {
    size_t s = cpe_ba_bytes_from_bits(bit_capacity);

    if (s > 8) {
        memcpy(target, source, s);
    }
    else {
        register size_t i;
        for(i = 0; i < s; ++i) {
            ((uint8_t *)target)[i] = ((uint8_t *)source)[i];
        }
    }

    CPE_BA_CLR_NOT_USED(target, s, bit_capacity);
}

void cpe_ba_set_all(cpe_ba_t ba, size_t bit_capacity, cpe_ba_value_t value) {
    size_t s = cpe_ba_bytes_from_bits(bit_capacity);
    uint8_t setval = value ? 0xFF : 0;
    register size_t i;

    for (i = 0; i < s; i++) {
        ((uint8_t*)ba)[i] = setval;
    }

    CPE_BA_CLR_NOT_USED(ba, s, bit_capacity);
}

size_t cpe_ba_bytes_from_bits(size_t nbits) {
    return (nbits >> 3) + (nbits % 8 ? 1 : 0);
}

ssize_t cpe_ba_next_pos(cpe_ba_t ba, size_t bit_capacity) {
    size_t s = cpe_ba_bytes_from_bits(bit_capacity);
    register size_t i;
    uint8_t * b = (uint8_t *)ba;
    register uint8_t r;
    ssize_t rv;

    for (i = 0; (i < s) && (((uint32_t *)b)[i >> 2] == 0xFFFFFFFF); i += 4);

    for (; i < s && b[i] == 0xFF; i ++);

    if (i >= s) return -1;

    rv = i << 3;

    r = b[i];

    s = bit_capacity - rv;
    if (s > 8) s = 8;


    for(i = 0; i < s && (r & (1 << i)); ++i, ++rv);

    return rv;
}

size_t cpe_ba_count(cpe_ba_t ba, size_t bit_capacity) {
    register size_t count;
    register size_t i;
    size_t s = cpe_ba_bytes_from_bits(bit_capacity);
    uint8_t * b = (uint8_t*)ba;

    static const uint8_t bitcount[256] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1,                                        \
        2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3,       \
        4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2,       \
        3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2,       \
        3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3,       \
        4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4,       \
        5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1,       \
        2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3,       \
        4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3,       \
        4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5,       \
        6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3,       \
        4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4,       \
        5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5,       \
        6, 6, 7, 6, 7, 7, 8};

    for (count = 0L, i = 0; i < s; i++) {
        count += bitcount[b[i]];
    }

    return count;
}

char * cpe_ba_to_str(char * dest, cpe_ba_t ba, size_t bit_capacity) {
    register size_t i;

    if (dest == NULL || ba == NULL) return NULL;

    for (i = 0; i < bit_capacity; i++) {
        dest[i] = cpe_ba_get(ba, i) ? '1' : '0';
    }
    dest[bit_capacity] = '\0';

    return dest;
}

char * cpe_ba_to_str_create(mem_allocrator_t alloc, cpe_ba_t ba, size_t bit_capacity) {
    return cpe_ba_to_str(
        (char*)mem_alloc(alloc, bit_capacity + 1),
        ba,
        bit_capacity);
}

void cpe_ba_complement(cpe_ba_t ba, size_t bit_capacity) {
    size_t s = cpe_ba_bytes_from_bits(bit_capacity);
    register size_t i;
    uint8_t * b = (uint8_t*)ba;

    for (i = 0; i < s; i++) {
        b[i] = ~b[i];
    }

    CPE_BA_CLR_NOT_USED(ba, s, bit_capacity);
}
