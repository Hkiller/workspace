#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "spack_building_file_i.h"

spack_building_file_t
spack_building_file_create(spack_builder_t builder, const char * path, uint32_t size) {
    spack_building_file_t file;

    file = mem_alloc(builder->m_alloc, sizeof(struct spack_building_file));
    if (file == NULL) {
        CPE_ERROR(builder->m_em, "spack_building_file_create: alloc fail!");
        return NULL;
    }

    file->m_builder = builder;
    file->m_path = mem_buffer_strdup(&builder->m_string_buff, path);
    if (file->m_path == NULL) {
        CPE_ERROR(builder->m_em, "spack_building_file_create: path %s dup fail!", path);
        mem_free(builder->m_alloc, file->m_path);
        return NULL;
    }
    file->m_size = size;

    cpe_hash_entry_init(&file->m_hh);
    if (cpe_hash_table_insert(&builder->m_files, file) != 0) {
        CPE_ERROR(builder->m_em, "spack_building_file_create: file %s insert fail!", path);
        mem_free(builder->m_alloc, file->m_path);
        return NULL;
    }
    
    return file;
}

void spack_building_file_free(spack_building_file_t file) {
    cpe_hash_table_remove_by_ins(&file->m_builder->m_files, file);
}

void spack_building_file_free_all(spack_builder_t builder) {
    struct cpe_hash_it file_it;
    spack_building_file_t file;

    cpe_hash_it_init(&file_it, &builder->m_files);

    file = cpe_hash_it_next(&file_it);
    while (file) {
        spack_building_file_t next = cpe_hash_it_next(&file_it);
        spack_building_file_free(file);
        file = next;
    }
}    

uint32_t spack_building_file_hash(spack_building_file_t file) {
    return cpe_hash_str(file->m_path, strlen(file->m_path));
}

int spack_building_file_eq(spack_building_file_t l, spack_building_file_t r) {
    return strcmp(l->m_path, r->m_path) == 0 ? 1 : 0;
}
