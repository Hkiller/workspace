#ifndef CPE_AOM_SHM_H
#define CPE_AOM_SHM_H
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"
#include "aom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int aom_shm_init(
    LPDRMETA meta,
    int shm_key, int shm_size, int force, error_monitor_t em);

int aom_shm_info(
    int shm_key, write_stream_t stream, int ident, error_monitor_t em);

int aom_shm_rm(int shm_key, error_monitor_t em);

int aom_shm_dump(int shm_key, write_stream_t stream, int ident, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif

