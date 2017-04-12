#include "cpe/pal/pal_string.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "spack_file_i.h"

spack_file_t spack_file_create(spack_repo_t repo, const char * path, uint32_t start, uint32_t size) {
    spack_file_t file;

    file = mem_alloc(repo->m_alloc, sizeof(struct spack_file));
    if (file == NULL) {
        CPE_ERROR(repo->m_em, "spack_file_create: alloc spack_file fail!");
        return NULL;
    }

    file->m_repo = repo;
    file->m_path = path;
    file->m_start = start;
    file->m_size = size;

    cpe_hash_entry_init(&file->m_hh);
    if (cpe_hash_table_insert(&repo->m_files, file) != 0) {
        CPE_ERROR(repo->m_em, "spack_file_create: file %s insert fail!", path);
        mem_free(repo->m_alloc, file);
        return NULL;
    }
    
    return file;
}

void spack_file_free(spack_file_t file) {
    spack_repo_t repo = file->m_repo;
    cpe_hash_table_remove_by_ins(&repo->m_files, file);
    mem_free(repo->m_alloc, file);
}

void spack_file_free_all(spack_repo_t repo) {
    struct cpe_hash_it file_it;
    spack_file_t file;

    cpe_hash_it_init(&file_it, &repo->m_files);

    file = cpe_hash_it_next(&file_it);
    while (file) {
        spack_file_t next = cpe_hash_it_next(&file_it);
        spack_file_free(file);
        file = next;
    }
}

spack_file_t spack_file_find_by_path(spack_repo_t db, const char * path) {
    struct spack_file key;
    key.m_path = path;
    return cpe_hash_table_find(&db->m_files, &key);
}

const char * spack_file_path(spack_file_t file) {
    return file->m_path;
}

uint32_t spack_file_size(spack_file_t file) {
    return file->m_size;
}

void const * spack_file_data(spack_file_t file) {
    return ((const char *)file->m_repo) + file->m_start;
}
    
uint32_t spack_file_hash(spack_file_t file) {
    return cpe_hash_str(file->m_path, strlen(file->m_path));
}

int spack_file_eq(spack_file_t l, spack_file_t r) {
    return strcmp(l->m_path, r->m_path) == 0 ? 1 : 0;
}

