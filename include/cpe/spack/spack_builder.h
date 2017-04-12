#ifndef CPE_SPACK_BUILDER_H
#define CPE_SPACK_BUILDER_H
#include "spack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spack_builder_t spack_builder_create(vfs_mgr_t vfs, const char * root, mem_allocrator_t alloc, error_monitor_t em);
void spack_builder_free(spack_builder_t builder);

int spack_builder_add(spack_builder_t builder, const char * file);

int spack_builder_build(spack_builder_t builder, write_stream_t ws);
    
#ifdef __cplusplus
}
#endif

#endif
