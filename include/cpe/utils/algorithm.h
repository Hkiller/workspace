#ifndef CPE_UTILS_ALGORITHM_H
#define CPE_UTILS_ALGORITHM_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void * cpe_lower_bound(void * first, size_t nel, const void * key, size_t width, int (*compar)(const void *, const void *));
void * cpe_upper_bound(void * first, size_t nel, const void * key, size_t width, int (*compar)(const void *, const void *));

int cpe_comap_uint32(const void *, const void *);

#ifdef __cplusplus
}
#endif

#endif
