#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "friend_svr_ops.h"

int friend_svr_db_send_query(
    friend_svr_t svr, logic_stack_node_t stack, const char * req_name,
    uint64_t user_id, uint8_t state_count, uint8_t * states)
{
    logic_require_t require = NULL;
    int pkg_r;
    mongo_pkg_t db_pkg;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: get db pkg fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "friend_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "user_id", (int64_t)user_id);

    if (state_count == 1) {
        pkg_r |= mongo_pkg_append_int32(db_pkg, "state", (int32_t)states[0]);
    }
    else if (state_count > 1) {

    }

    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    /*需要的列 */
    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "user_id", 1);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "state", 1);
    pkg_r |= mongo_pkg_append_int32(db_pkg, dr_entry_name(svr->m_record_fuid_entry), 1);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build query req fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    if (req_name) {
        require = logic_require_create(stack, req_name);
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: %s: create logic require fail!", friend_svr_name(svr), req_name);
            return -1;
        }
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 32, NULL, NULL, 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    return 0;
}

int friend_svr_db_send_query_data(
    friend_svr_t svr, logic_stack_node_t stack, const char * req_name,
    uint64_t user_id, uint16_t friend_count, uint64_t * friends)
{
    logic_require_t require = NULL;
    int pkg_r;
    mongo_pkg_t db_pkg;
    char buf[64];

    if (friend_count == 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: friend_cont=0, error!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: get db pkg fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "friend_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);

    if (friend_count == 1) {
        snprintf(buf, sizeof(buf), FMT_UINT64_T"-"FMT_UINT64_T, user_id, friends[0]);
        pkg_r |= mongo_pkg_append_string(db_pkg, "_id", buf);
    }
    else {
        uint16_t i;

        pkg_r |= mongo_pkg_append_start_object(db_pkg, "_id");
        pkg_r |= mongo_pkg_append_start_array(db_pkg, "$in");

        for(i = 0; i < friend_count; ++i) {
            char idx[12];
            snprintf(idx, sizeof(idx), "%d", i);
            snprintf(buf, sizeof(buf), FMT_UINT64_T"-"FMT_UINT64_T, user_id, friends[i]);
            pkg_r |= mongo_pkg_append_string(db_pkg, idx, buf);
        }

        pkg_r |= mongo_pkg_append_finish_object(db_pkg);
        pkg_r |= mongo_pkg_append_finish_object(db_pkg);
    }

    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build query req fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    if (req_name) {
        require = logic_require_create(stack, req_name);
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: %s: create logic require fail!", friend_svr_name(svr), req_name);
            return -1;
        }
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 32, NULL, NULL, 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    return 0;
}

int friend_svr_db_send_query_one(friend_svr_t svr, logic_stack_node_t stack, const char * req_name, uint64_t user_id, uint64_t friend_uid) {
    logic_require_t require = NULL;
    int pkg_r;
    mongo_pkg_t db_pkg;
    char buf[64];

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: get db pkg fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "friend_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    snprintf(buf, sizeof(buf), FMT_UINT64_T"-"FMT_UINT64_T, user_id, friend_uid);
    pkg_r |= mongo_pkg_append_string(db_pkg, "_id", buf);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    /*需要的列 */
    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "user_id", 1);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "state", 1);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build query req fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    if (req_name) {
        require = logic_require_create(stack, req_name);
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: %s: create logic require fail!", friend_svr_name(svr), req_name);
            return -1;
        }
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 32, NULL, NULL, 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    return 0;
}

int friend_svr_db_send_insert(friend_svr_t svr, logic_stack_node_t stack, const char * req_name, void const * record) {
    logic_require_t require = NULL;
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s: get db pkg fail!", friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "friend_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_append(db_pkg, svr->m_record_meta, record, svr->m_record_size);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build insert req fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    if (req_name) {
        require = logic_require_create(stack, req_name);
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: %s: create logic require fail!", friend_svr_name(svr), req_name);
            return -1;
        }
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    return 0;
}

int friend_svr_db_send_remove(friend_svr_t svr, logic_stack_node_t stack, const char * req_name, uint64_t uid, uint64_t fuid) {
    logic_require_t require = NULL;
    mongo_pkg_t db_pkg;
    int pkg_r;
    char buf[64];

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "friend_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_delete);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    snprintf(buf, sizeof(buf), FMT_UINT64_T"-"FMT_UINT64_T, uid, fuid);
    pkg_r |= mongo_pkg_append_string(db_pkg, "_id", buf);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build delete req fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    if (req_name) {
        require = logic_require_create(stack, req_name);
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: %s: create logic require fail!", friend_svr_name(svr), req_name);
            return -1;
        }
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    return 0;
}

int friend_svr_db_send_update_state(
    friend_svr_t svr, logic_stack_node_t stack, const char * req_name,
    uint64_t uid, uint64_t fuid, uint8_t from_state, uint8_t to_state)
{
    logic_require_t require = NULL;
    mongo_pkg_t db_pkg;
    int pkg_r;
    char buf[64];

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "friend_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_update);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    snprintf(buf, sizeof(buf), FMT_UINT64_T"-"FMT_UINT64_T, uid, fuid);
    pkg_r |= mongo_pkg_append_string(db_pkg, "_id", buf);
    pkg_r |= mongo_pkg_append_int32(db_pkg, "state", from_state);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_start_object(db_pkg, "$set");
    pkg_r |= mongo_pkg_append_int32(db_pkg, "state", to_state);
    pkg_r |= mongo_pkg_append_finish_object(db_pkg);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build insert req fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    if (req_name) {
        require = logic_require_create(stack, req_name);
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: %s: create logic require fail!", friend_svr_name(svr), req_name);
            return -1;
        }
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            friend_svr_name(svr), req_name ? req_name : "???");
        return -1;
    }

    return 0;
}

