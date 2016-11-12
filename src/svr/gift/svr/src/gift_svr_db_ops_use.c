#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "gift_svr_db_ops_use.h"

int gift_svr_db_send_use_query(gift_svr_t svr, logic_require_t require, const char * id) {
    int pkg_r;
    mongo_pkg_t db_pkg;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_use_query: get db pkg fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "gift_use");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_string(db_pkg, "_id", id);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_use_query: build query req fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_use_record_list_meta, 2, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_use_query: send db request fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}

int gift_svr_db_send_use_query_by_generate(gift_svr_t svr, logic_require_t require, uint32_t generate_id) {
    int pkg_r;
    mongo_pkg_t db_pkg;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_use_query: get db pkg fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "gift_use");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "generate_id", (int32_t)generate_id);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_use_query: build query req fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_use_record_list_meta, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_use_query: send db request fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}

int gift_svr_db_send_use_insert(gift_svr_t svr, logic_require_t require, SVR_GIFT_USE_RECORD const * record) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_use_insert: get db pkg fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "gift_use");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_append(db_pkg, svr->m_use_record_meta, record, sizeof(*record));

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_use_insert: build insert req fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_use_insert: send db request fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}

int gift_svr_db_send_use_update_state(gift_svr_t svr, logic_require_t require, uint8_t from_state, SVR_GIFT_USE_RECORD const * record) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_use_update: get db pkg fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "gift_use");
    mongo_pkg_set_op(db_pkg, mongo_db_op_update);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_string(db_pkg, "_id", record->_id);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "state", from_state);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_start_object(db_pkg, "$set");
    pkg_r |= mongo_pkg_append_int32(db_pkg, "state", record->state);

    if (record->state == svr_gift_use_state_used) {
        pkg_r |= mongo_pkg_append_start_object(db_pkg, "state_data");
        pkg_r |= mongo_pkg_append_object(
            db_pkg, "use",
            svr->m_use_record_use_meta, &record->state_data.use, sizeof(record->state_data.use));
        pkg_r |= mongo_pkg_append_finish_object(db_pkg);
    }
    else {
        pkg_r |= mongo_pkg_append_null(db_pkg, "state_data");
    }
    pkg_r |= mongo_pkg_append_finish_object(db_pkg);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_use_update: build insert req fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_use_update: send db request fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}

int gift_svr_db_send_use_remove_by_generate_id(gift_svr_t svr, logic_require_t require, uint32_t generate_id) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_use_remove_by_generate_id: get db pkg fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "gift_use");
    mongo_pkg_set_op(db_pkg, mongo_db_op_delete);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "generate_id", (int32_t)generate_id);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_use_remove_by_generate_id: build delete req fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_use_remove_by_generate_id: send db request fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}
