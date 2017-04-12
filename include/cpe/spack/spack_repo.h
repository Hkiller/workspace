#ifndef CPE_SPACK_REPO_H
#define CPE_SPACK_REPO_H
#include "spack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spack_repo_t spack_repo_create(mem_allocrator_t alloc, error_monitor_t em, void const * data, uint32_t data_size);
void spack_repo_free(spack_repo_t repo);

#ifdef __cplusplus
}
#endif

#endif
