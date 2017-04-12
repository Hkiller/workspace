#ifndef CPE_SPACK_FILE_I_H
#define CPE_SPACK_FILE_I_H
#include "cpe/spack/spack_file.h"
#include "spack_repo_i.h"

struct spack_file {
    spack_repo_t m_repo;
    cpe_hash_entry m_hh;
    const char * m_path;
    uint32_t m_start;
    uint32_t m_size;
};

spack_file_t spack_file_create(spack_repo_t repo, const char * path, uint32_t start, uint32_t size);
void spack_file_free(spack_file_t file);

void spack_file_free_all(spack_repo_t db);

uint32_t spack_file_hash(spack_file_t file);
int spack_file_eq(spack_file_t l, spack_file_t r);

#endif
