#ifndef CPE_UTILS_RWPIPE_SHM_H
#define CPE_UTILS_RWPIPE_SHM_H
#include "rwpipe.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MSC_VER

rwpipe_t rwpipe_shm_init(int shmid, uint32_t capacity, int force_new, error_monitor_t em);
rwpipe_t rwpipe_shm_attach(int shmid, error_monitor_t em);
rwpipe_t rwpipe_shm_init_by_path(const char * key_file, uint32_t capacity, int force_new, error_monitor_t em);
rwpipe_t rwpipe_shm_attach_by_path(const char * key_file, error_monitor_t em);

#endif

#ifdef __cplusplus
}
#endif

#endif
