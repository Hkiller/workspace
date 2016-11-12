#ifndef SVR_DBLOG_SVR_META_H
#define SVR_DBLOG_SVR_META_H
#include "dblog_svr_i.h"

struct dblog_svr_meta {
    dblog_svr_t m_svr;
    struct cpe_hash_entry m_hh;
    uint16_t m_svr_type;
    uint16_t m_meta_id;
    LPDRMETA m_meta;
    char m_collection[64]; 
};

dblog_svr_meta_t dblog_svr_meta_create(dblog_svr_t svr, uint16_t svr_type, const char * svr_type_name, LPDRMETA meta);
void dblog_svr_meta_free(dblog_svr_meta_t meta);

dblog_svr_meta_t dblog_svr_meta_find(dblog_svr_t svr, uint16_t svr_type, uint16_t meta_id);

void dblog_svr_meta_free_all(dblog_svr_t svr);

uint32_t dblog_svr_meta_hash(dblog_svr_meta_t meta);
int dblog_svr_meta_eq(dblog_svr_meta_t l, dblog_svr_meta_t r);
    

#endif
