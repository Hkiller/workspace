#ifndef CPE_UTILS_MMAP_H
#define CPE_UTILS_MMAP_H
#include "cpe/pal/pal_types.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

void * mmap_file_load(const char * file, const char * mod, size_t * size, error_monitor_t em);
void mmap_unload(void * p, size_t size);

#ifdef __cplusplus
}
#endif

#endif
