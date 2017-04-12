#ifndef CPE_SPACK_TABLE_H
#define CPE_SPACK_TABLE_H
#include "spack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

spack_file_t spack_file_find_by_path(spack_repo_t repo, const char * path);

const char * spack_file_path(spack_file_t file);
uint32_t spack_file_size(spack_file_t file);
void const * spack_file_data(spack_file_t file);    
    
#ifdef __cplusplus
}
#endif

#endif
