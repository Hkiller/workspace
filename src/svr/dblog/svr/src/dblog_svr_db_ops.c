#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "dblog_svr_db_ops.h"
#include "dblog_svr_meta.h"

int dblog_svr_db_insert(
    dblog_svr_t svr, uint16_t svr_id, uint64_t log_time,
    void const * data, size_t data_size, dblog_svr_meta_t meta)
{
    mongo_pkg_t db_pkg;
    int pkg_r = 0;
    char id[64];

    db_pkg = mongo_driver_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: log: from %d-%d meta %d(%s): get db fail",
            dblog_svr_name(svr), meta->m_svr_type, svr_id, dr_meta_id(meta->m_meta), dr_meta_name(meta->m_meta));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_db(db_pkg, svr->m_db_ns);
    mongo_pkg_set_collection(db_pkg, meta->m_collection);
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r |= mongo_pkg_doc_open(db_pkg);

    snprintf(id, sizeof(id), "%d-%d-"FMT_UINT64_T, meta->m_svr_type, svr_id, log_time);
    pkg_r |= mongo_pkg_append_string(db_pkg, "_id", id);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "svr_type", (int32_t)meta->m_svr_type);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "svr_id", (int32_t)svr_id);
    pkg_r |= mongo_pkg_append_object_open(db_pkg, meta->m_meta, data, data_size);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "time", (int64_t)log_time);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: log: from %d-%d meta %d(%s): build db req fail!",
            dblog_svr_name(svr), meta->m_svr_type, svr_id, dr_meta_id(meta->m_meta), dr_meta_name(meta->m_meta));
        return -1;
    }

    if (mongo_driver_send(svr->m_db, db_pkg) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: log: from %d-%d meta %d(%s): db op fail!",
            dblog_svr_name(svr), meta->m_svr_type, svr_id, dr_meta_id(meta->m_meta), dr_meta_name(meta->m_meta));
        return -1;
    }

    return 0;
}
