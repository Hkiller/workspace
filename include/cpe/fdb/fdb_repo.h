#ifndef CPE_FDB_REPO_H
#define CPE_FDB_REPO_H
#include "cpe/cfg/cfg_types.h"
#include "fdb_types.h"

#ifdef __cplusplus
extern "C" {
#endif

fdb_repo_t fdb_repo_create(mem_allocrator_t alloc, error_monitor_t em);
void fdb_repo_free(fdb_repo_t repo);

int fdb_repo_load_schema(fdb_repo_t repo, LPDRMETALIB metalib, cfg_t def);
int fdb_repo_load_schema_and_data(fdb_repo_t repo, LPDRMETALIB metalib, cfg_t def);    
    
#ifdef __cplusplus
}
#endif

#endif
