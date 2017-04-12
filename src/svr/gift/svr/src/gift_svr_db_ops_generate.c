#include <assert.h>
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "gift_svr_db_ops_generate.h"

int gift_svr_db_send_generate_record_query(gift_svr_t svr, logic_require_t require) {
    int pkg_r;
    mongo_pkg_t db_pkg;
    
    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_generate_record_query: get db pkg fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "gift_generate");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_generate_record_query: build query req fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }
    
    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_generate_record_list_meta, 32, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_generate_record_query: send db request fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}

int gift_svr_db_send_generate_insert(gift_svr_t svr, logic_require_t require, void const * record_data) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_generate_insert: get db pkg fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "gift_generate");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_append(db_pkg, svr->m_generate_record_meta, record_data, svr->m_generate_record_size);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_generate_insert: build insert req fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_generate_insert: send db request fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}

int gift_svr_db_send_generate_remove(gift_svr_t svr, logic_require_t require, uint32_t generate_id) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_generate_remove: get db pkg fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "gift_generate");
    mongo_pkg_set_op(db_pkg, mongo_db_op_delete);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "_id", (int32_t)generate_id);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_generate_remove: build delete req fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_generate_remove: send db request fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}

int gift_svr_db_send_generate_update_duration(gift_svr_t svr, logic_require_t require, uint32_t generate_id, uint32_t begin_time, uint32_t expire_time) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send_generate_remove: get db pkg fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "gift_generate");
    mongo_pkg_set_op(db_pkg, mongo_db_op_update);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "_id", (int32_t)generate_id);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "begin_time", (int32_t)begin_time);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "expire_time", (int32_t)expire_time);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);
    
    if (pkg_r) {
        CPE_ERROR(svr->m_em, "%s: send_generate_update_duration: build req fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: send_generate_update_duration: send db request fail!", gift_svr_name(svr));
        if (require) logic_require_set_error(require);
        return -1;
    }

    return 0;
}
