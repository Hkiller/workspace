#ifndef SVR_DBLOG_SVR_DB_H
#define SVR_DBLOG_SVR_DB_H
#include "dblog_svr_i.h"

int dblog_svr_db_insert(
    dblog_svr_t svr, uint16_t svr_id, uint64_t log_time,
    void const * data, size_t data_size, dblog_svr_meta_t meta);

#endif
