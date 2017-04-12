#include "cpe/utils/error.h"
#include "fdb_repo_i.h"
#include "fdb_table_i.h"

fdb_repo_t fdb_repo_create(mem_allocrator_t alloc, error_monitor_t em) {
    fdb_repo_t repo;

    repo = mem_alloc(alloc, sizeof(struct fdb_repo));
    if (repo == NULL) {
        CPE_ERROR(em, "fdb_repo_create: alloc repo fail!");
        return NULL;
    }

    repo->m_alloc = alloc;
    repo->m_em = em;

    if (cpe_hash_table_init(
            &repo->m_tables,
            alloc,
            (cpe_hash_fun_t) fdb_table_hash,
            (cpe_hash_eq_t) fdb_table_eq,
            CPE_HASH_OBJ2ENTRY(fdb_table, m_hh),
            -1) != 0)
    {
        mem_free(alloc, repo);
        return NULL;
    }
    
    return repo;
}

void fdb_repo_free(fdb_repo_t repo) {

    fdb_table_free_all(repo);
    
    mem_free(repo->m_alloc, repo);
}
