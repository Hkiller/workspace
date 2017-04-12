#ifndef CPE_SPACK_REPO_BUILDING_FILE_I_H
#define CPE_SPACK_REPO_BUILDING_FILE_I_H
#include "spack_builder_i.h"

struct spack_building_file {
    spack_builder_t m_builder;
    cpe_hash_entry m_hh;
    char * m_path;
    uint32_t m_size;
};

spack_building_file_t spack_building_file_create(spack_builder_t builder, const char * path, uint32_t size);
void spack_building_file_free(spack_building_file_t file);

void spack_building_file_free_all(spack_builder_t db);

uint32_t spack_building_file_hash(spack_building_file_t building_file);
int spack_building_file_eq(spack_building_file_t l, spack_building_file_t r);

#endif
