#ifndef CPE_SPACK_RFS_H
#define CPE_SPACK_RFS_H
#include "cpe/vfs/vfs_types.h"
#include "spack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spack_rfs_t spack_rfs_create(vfs_mgr_t vfs, const char * path, mem_allocrator_t alloc, error_monitor_t em);
void spack_rfs_free(spack_rfs_t rfs);

const char * spack_rfs_path(spack_rfs_t rfs);

void spack_rfs_clear(spack_rfs_t rfs);
    
int spack_rfs_attach_data(spack_rfs_t rfs, void * data, size_t data_size);
    
int spack_rfs_append_data(spack_rfs_t rfs, void * data, size_t data_size);
int spack_rfs_append_complete(spack_rfs_t rfs);
    
#ifdef __cplusplus
}
#endif

#endif
