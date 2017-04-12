#include "cpe/pal/pal_platform.h"
#include "cpe/utils/error.h"
#include "spack_repo_i.h"
#include "spack_file_i.h"
#include "spack_proto_i.h"

static int spack_repo_build_files(spack_repo_t repo);

spack_repo_t spack_repo_create(mem_allocrator_t alloc, error_monitor_t em, void const * data, uint32_t data_size) {
    spack_repo_t repo;

    repo = mem_alloc(alloc, sizeof(struct spack_repo));
    if (repo == NULL) {
        CPE_ERROR(em, "spack_repo_create: alloc repo fail!");
        return NULL;
    }

    repo->m_alloc = alloc;
    repo->m_em = em;
    repo->m_buf = data;
    repo->m_buf_size = data_size;

    if (cpe_hash_table_init(
            &repo->m_files,
            alloc,
            (cpe_hash_fun_t) spack_file_hash,
            (cpe_hash_eq_t) spack_file_eq,
            CPE_HASH_OBJ2ENTRY(spack_file, m_hh),
            -1) != 0)
    {
        mem_free(alloc, repo);
        return NULL;
    }

    if (spack_repo_build_files(repo) != 0) {
        spack_repo_free(repo);
        return NULL;
    }
    
    return repo;
}

void spack_repo_free(spack_repo_t repo) {
    spack_file_free_all(repo);
    mem_free(repo->m_alloc, repo);
}

static int spack_repo_build_files(spack_repo_t repo) {
    struct spack_head const * head = repo->m_buf;
    struct spack_entry const * entry;
    uint32_t entry_count;
    uint32_t i;

    if (repo->m_buf_size < sizeof(*head)) {
        CPE_ERROR(repo->m_em, "spack_repo_build_files: buf-size=%d too small!", (int)repo->m_buf_size);
        return -1;
    }

    CPE_COPY_NTOH32(&entry_count, &head->m_entry_count);
    if (entry_count * sizeof(struct spack_entry) + sizeof(struct spack_head) > repo->m_buf_size) {
        CPE_ERROR(repo->m_em, "spack_repo_build_files: entry count %d, buf-size %d too small", entry_count, (int)repo->m_buf_size);
        return -1;
    }

    entry = (void const *)(head + 1);
    for(i = 0; i < entry_count; ++i, entry++) {
        uint32_t name_pos;
        uint32_t entry_data_start;
        uint32_t entry_data_size;

        CPE_COPY_NTOH32(&name_pos, &entry->m_name);
        CPE_COPY_NTOH32(&entry_data_start, &entry->m_data_start);
        CPE_COPY_NTOH32(&entry_data_size, &entry->m_data_size);

        if (name_pos > repo->m_buf_size) {
            CPE_ERROR(
                repo->m_em, "spack_repo_build_files: entry %d name pos %d overflow, buf-size=%d",
                i, name_pos, (int)repo->m_buf_size);
            return -1;
        }

        if (entry_data_start + entry_data_size > repo->m_buf_size) {
            CPE_ERROR(
                repo->m_em, "spack_repo_build_files: entry %d data (%d~%d) overflow, buf-size=%d",
                i, entry_data_size, entry_data_start + entry_data_size, (int)repo->m_buf_size);
            return -1;
        }

        if (spack_file_create(repo, ((const char *)repo->m_buf) + name_pos,  entry_data_start, entry_data_size) == NULL) {
            return -1;
        }
    }
            
    return 0;
}
