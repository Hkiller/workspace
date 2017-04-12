#ifndef CPE_POM_GRP_SHM_H
#define CPE_POM_GRP_SHM_H
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"
#include "pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int pom_grp_shm_init(
    LPDRMETALIB metalib, pom_grp_meta_t grp_meta,
    int shm_key, int shm_size, int force, error_monitor_t em);

int pom_grp_shm_info(
    int shm_key, write_stream_t stream, int ident, error_monitor_t em);

int pom_grp_shm_rm(int shm_key, error_monitor_t em);

int pom_grp_shm_dump(int shm_key, write_stream_t stream, int ident, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif

