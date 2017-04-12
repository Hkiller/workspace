#ifndef CPE_SPACK_TYPES_H
#define CPE_SPACK_TYPES_H
#include "cpe/utils/utils_types.h"
#include "cpe/vfs/vfs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spack_repo * spack_repo_t;
typedef struct spack_file * spack_file_t;
typedef struct spack_rfs * spack_rfs_t;

typedef struct spack_builder * spack_builder_t;
    
#ifdef __cplusplus
}
#endif

#endif
