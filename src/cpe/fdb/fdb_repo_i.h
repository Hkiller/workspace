#ifndef CPE_FDB_REPO_I_H
#define CPE_FDB_REPO_I_H
#include "cpe/utils/hash.h"
#include "cpe/fdb/fdb_repo.h"

struct fdb_repo {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    struct cpe_hash_table m_tables;
};

#endif
