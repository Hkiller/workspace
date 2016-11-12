#ifndef CPE_UTILS_RWPIPE_MMAP_H
#define CPE_UTILS_RWPIPE_MMAP_H
#include "rwpipe.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MSC_VER

rwpipe_t rwpipe_mmap_init(const char * key_file, int * fd, uint32_t capacity, int force_new, error_monitor_t em);
rwpipe_t rwpipe_mmap_attach(const char * key_file, int * fd, error_monitor_t em);
int rwpipe_mmap_sync(rwpipe_t q, error_monitor_t em);

#endif

#ifdef __cplusplus
}
#endif

#endif
