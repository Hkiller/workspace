#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "payment_svr_db_recharge_ops.h"

int payment_svr_db_recharge_send_insert(payment_svr_t svr, logic_require_t require, PAYMENT_RECHARGE_RECORD const * record) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge insert: get db pkg fail!", payment_svr_name(svr));
        if (require) logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "payment_recharge");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_append(db_pkg, svr->m_meta_recharge_record, record, sizeof(*record));

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: recharge insert: build db pkg fail!", payment_svr_name(svr));
        if (require) logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, NULL) != 0) {
        CPE_ERROR(svr->m_em, "%s: recharge insert: send db request fail!", payment_svr_name(svr));
        if (require) logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    return 0;
}

int payment_svr_db_recharge_send_qurey_by_id(payment_svr_t svr, logic_require_t require, const char * recharge_id) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: recharge query: get db pkg fail!", payment_svr_name(svr));
        if (require) logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "payment_recharge");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |=  mongo_pkg_append_string(db_pkg, "_id", recharge_id);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: recharge query: build db pkg fail!", payment_svr_name(svr));
        if (require) logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_meta_recharge_record_list, 1, NULL, NULL, NULL) != 0) {
        CPE_ERROR(svr->m_em, "%s: recharge query: send db request fail!", payment_svr_name(svr));
        if (require) logic_require_set_error_ex(require, SVR_PAYMENT_ERRNO_INTERNAL);
        return -1;
    }

    return 0;
}

int payment_svr_db_recharge_send_update_state(payment_svr_t svr, logic_require_t require, PAYMENT_RECHARGE_RECORD const * record) {
    mongo_pkg_t db_pkg;
    int pkg_r;
    LPDRMETAENTRY entry;
    
    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: db: rechrage: send_update_state: get db pkg fail!", payment_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "payment_recharge");
    mongo_pkg_set_op(db_pkg, mongo_db_op_update);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_string(db_pkg, "_id", record->_id);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "state", (int32_t)PAYMENT_RECHARGE_INPROCESS);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "version", (int32_t)(record->version - 1));
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_start_object(db_pkg, "$set");
    pkg_r |= mongo_pkg_append_int32(db_pkg, "version", (int32_t)record->version);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "commit_time", (int32_t)record->commit_time);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "state", (int32_t)record->state);    
    pkg_r |= mongo_pkg_append_int64(db_pkg, "error", (int32_t)record->error);    
    pkg_r |= mongo_pkg_append_string(db_pkg, "error_msg", record->error_msg);

    pkg_r |= mongo_pkg_append_start_object(db_pkg, "vendor_record");
    if ((entry = dr_meta_find_entry_by_id(svr->m_meta_vendor_record, record->service))) {
        pkg_r |= mongo_pkg_append_object(
            db_pkg, dr_entry_name(entry),
            dr_entry_ref_meta(entry), &record->vendor_record, sizeof(record->vendor_record));
    }
    pkg_r |= mongo_pkg_append_finish_object(db_pkg);

    pkg_r |= mongo_pkg_append_finish_object(db_pkg);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: db: recharge: send_update_update: build insert req fail!", payment_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: db: recharge: send_update_state: send db request fail!", payment_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}
