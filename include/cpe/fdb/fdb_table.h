#ifndef CPE_FDB_TABLE_H
#define CPE_FDB_TABLE_H
#include "fdb_types.h"

#ifdef __cplusplus
extern "C" {
#endif

fdb_table_t fdb_table_create(fdb_repo_t db, LPDRMETA meta);
void fdb_table_free(fdb_table_t table);

fdb_table_t fdb_table_find_by_name(fdb_repo_t db, const char * name);

LPDRMETA fdb_table_meta(fdb_table_t table);
    
void * fdb_table_record_insert(fdb_table_t table, const void * data, const char ** duplicate_record);
int fdb_table_record_remove(fdb_table_t table, const void * record);
    
#ifdef __cplusplus
}
#endif

#endif
