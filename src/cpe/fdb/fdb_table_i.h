#ifndef CPE_FDB_TABLE_I_H
#define CPE_FDB_TABLE_I_H
#include "cpe/fdb/fdb_table.h"
#include "fdb_repo_i.h"

struct fdb_table {
    fdb_repo_t m_repo;
    cpe_hash_entry m_hh;
    const char * m_name;
    LPDRMETA m_meta;
    void * m_records;
    uint32_t m_record_count;
    uint32_t m_record_capacity;
};

void fdb_table_free_all(fdb_repo_t db);

uint32_t fdb_table_hash(fdb_table_t table);
int fdb_table_eq(fdb_table_t l, fdb_table_t r);

#endif
