#ifndef CPE_FDB_INDEX_I_H
#define CPE_FDB_INDEX_I_H
#include "cpe/fdb/fdb_index.h"
#include "fdb_repo_i.h"

struct fdb_index {
    fdb_table_t m_table;
};

void fdb_index_free_all(fdb_repo_t db);

#endif
