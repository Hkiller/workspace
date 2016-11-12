#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "fdb_repo_i.h"
#include "fdb_table_i.h"

int fdb_repo_load_schema(fdb_repo_t repo, LPDRMETALIB metalib, cfg_t def) {
    struct cfg_it child_it;
    cfg_t child_cfg;

    cfg_it_init(&child_it, def);
    while((child_cfg = cfg_it_next(&child_it))) {
        const char * tag;
        
        child_cfg = cfg_child_only(child_cfg);

        tag = cfg_name(child_cfg);
    }

    return 0;
}

int fdb_repo_load_schema_and_data(fdb_repo_t repo, LPDRMETALIB metalib, cfg_t def) {
    struct cfg_it child_it;
    cfg_t child_cfg;
    int rv;

    rv = 0;
    
    cfg_it_init(&child_it, def);
    while((child_cfg = cfg_it_next(&child_it))) {
        const char * table_name;
        fdb_table_t table;
        
        child_cfg = cfg_child_only(child_cfg);

        table_name = cfg_name(child_cfg);

        table = fdb_table_find_by_name(repo, table_name);
        if (table == NULL) {
            LPDRMETA table_meta = dr_lib_find_meta_by_name(metalib, table_name);
            if (table_name == NULL) {
                CPE_ERROR(repo->m_em, "fdb_repo_load_schema_and_data: table %s not exist in metalib!", table_name);
                rv = -1;
                continue;
            }

            table = fdb_table_create(repo, table_meta);
            if (table == NULL) {
                CPE_ERROR(repo->m_em, "fdb_repo_load_schema_and_data: table %s create fail!", table_name);
                rv = -1;
                continue;
            }
        }
    }

    return rv;
}
