#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_bson.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic_use/logic_data_dyn.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mail_svr_ops.h"

static mongo_cli_pkg_parse_result_t
mail_svr_db_query_pkg_parse(
    void * ctx,
    logic_require_t require, mongo_pkg_t pkg, logic_data_t * result_data, LPDRMETA result_meta,
    mongo_cli_pkg_parse_evt_t evt, const void * bson_input, size_t bson_capacity);

int mail_svr_db_send_query(
    mail_svr_t svr, logic_require_t require,
    SVR_MAIL_QUERY_CONDITION const * condition, uint32_t after_time, uint32_t require_count, LPDRMETA result_meta)
{
    int pkg_r;
    mongo_pkg_t db_pkg;
    int i, r;
    LPDRMETA record_meta;
    struct dr_meta_dyn_info result_dyn_info;

    r = dr_meta_find_dyn_info(result_meta, &result_dyn_info);
    assert(r == 0);
    assert(result_dyn_info.m_type == dr_meta_dyn_info_type_array);
    assert(result_dyn_info.m_data.m_array.m_array_entry != NULL);

    record_meta = dr_entry_ref_meta(result_dyn_info.m_data.m_array.m_array_entry);
    assert(record_meta != NULL);

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: get db pkg fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "mail");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    switch(condition->type) {
    case SVR_MAIL_QUERY_CONDITION_TYPE_BY_SENDER:
        pkg_r |= mongo_pkg_append_int64(db_pkg, "sender_gid", (int64_t)condition->data.by_sender.sender_gid);
        break;
    case SVR_MAIL_QUERY_CONDITION_TYPE_BY_RECEIVER:
        pkg_r |= mongo_pkg_append_int64(db_pkg, "receiver_gid", (int64_t)condition->data.by_receiver.receiver_gid);
        break;
    case SVR_MAIL_QUERY_CONDITION_TYPE_BY_ID:
        pkg_r |= mongo_pkg_append_int64(db_pkg, "_id", (int64_t)condition->data.by_id.mail_id);
        break;
    default:
        CPE_ERROR(
            svr->m_em, "%s: %s: unknown condition type %d!",
            mail_svr_name(svr), logic_require_name(require), condition->type);
        return -1;
    }
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    for(i = 0; i < dr_meta_entry_num(record_meta); ++i) {
        const char * entry_name = dr_entry_name(dr_meta_entry_at(record_meta, i));
        if (strcmp(entry_name, "mail_id") == 0
            || strcmp(entry_name, "sender_data_len") == 0
            || strcmp(entry_name, "attach_len") == 0) continue;

        if (strcmp(entry_name, "sender_data") == 0) {
            pkg_r |= mongo_pkg_append_int32(db_pkg, "sender", 1);
            continue;
        }

        pkg_r |= mongo_pkg_append_int32(db_pkg, entry_name, 1);
    }
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build query req fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, result_meta, 16, NULL, mail_svr_db_query_pkg_parse, svr) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int mail_svr_db_send_query_global(mail_svr_t svr, logic_require_t require, uint32_t after_time){
    int pkg_r;
    mongo_pkg_t db_pkg;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: get db pkg fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "mail_global");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_start_object(db_pkg, "$lt");
    pkg_r |= mongo_pkg_append_int32(db_pkg, "send_time", after_time);
    pkg_r |= mongo_pkg_append_finish_object(db_pkg);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build query req fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_global_list_meta, 16, NULL, mail_svr_db_query_pkg_parse, svr) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int mail_svr_db_send_insert(mail_svr_t svr, logic_require_t require, void const * record) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "mail");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_append(db_pkg, svr->m_record_meta, record, svr->m_record_size);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build insert req fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 0, NULL, NULL, NULL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int mail_svr_db_send_global_insert(mail_svr_t svr, logic_require_t require, void const * record) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "mail_global");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_append(db_pkg, svr->m_record_global_meta, record, svr->m_record_global_size);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build insert req fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_global_list_meta, 0, NULL, NULL, NULL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int mail_svr_db_send_remove(mail_svr_t svr, logic_require_t require, uint64_t mail_id, uint64_t receiver_gid) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "mail");
    mongo_pkg_set_op(db_pkg, mongo_db_op_delete);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "_id", (int64_t)mail_id);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "receiver_gid", (int64_t)receiver_gid);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build delete req fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 0, NULL, NULL, NULL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int mail_svr_db_send_update(mail_svr_t svr, logic_require_t require, uint64_t mail_id, uint64_t receiver_gid, SVR_MAIL_OP const * ops, uint8_t op_count) {
    mongo_pkg_t db_pkg;
    int pkg_r;
    uint8_t i;
    uint8_t is_start;

    if (op_count == 0) {
        CPE_ERROR(svr->m_em, "%s: %s:: no update op!", mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "mail");
    mongo_pkg_set_op(db_pkg, mongo_db_op_update);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "_id", (int64_t)mail_id);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "receiver_gid", (int64_t)receiver_gid);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    pkg_r |= mongo_pkg_doc_open(db_pkg);

    is_start = 0;
    for(i = 0; i < op_count; ++i) {
        SVR_MAIL_OP const * op = ops + i;

        switch(op->type) {
        case SVR_MAIL_OP_TYPE_UPDATE_STATE:
            if (!is_start) pkg_r |= mongo_pkg_append_start_object(db_pkg, "$set");
            mongo_pkg_append_int32(db_pkg, "state", op->data.update_state.state);
            break;
        case SVR_MAIL_OP_TYPE_UPDATE_ATTACH: {
            char buf[svr->m_record_attach_capacity];
            int rv;

            if (!is_start) pkg_r |= mongo_pkg_append_start_object(db_pkg, "$set");

            rv = dr_pbuf_read(
                buf, sizeof(buf),
                op->data.update_attach.attach, op->data.update_attach.attach_len,
                svr->m_attach_meta, svr->m_em);
            if (rv <= 0) {
                CPE_ERROR(svr->m_em, "%s: %s: decode attach error, rv=%d!", mail_svr_name(svr), logic_require_name(require), rv);
                return -1;
            }

            pkg_r |= mongo_pkg_append_object(db_pkg, "attach", svr->m_attach_meta, buf, sizeof(buf));
            break;
        }
        default:
            CPE_ERROR(
                svr->m_em, "%s: %s: unknown db op type %d!",
                mail_svr_name(svr), logic_require_name(require), op->type);
            break;
        }
    }
    pkg_r |= mongo_pkg_append_finish_object(db_pkg);

    is_start = 0;
    for(i = 0; i < op_count; ++i) {
        SVR_MAIL_OP const * op = ops + i;

        switch(op->type) {
        case SVR_MAIL_OP_TYPE_UPDATE_STATE:
            break;
        case SVR_MAIL_OP_TYPE_UPDATE_ATTACH:
            break;
        default:
            CPE_ERROR(
                svr->m_em, "%s: %s: unknown db op type %d!",
                mail_svr_name(svr), logic_require_name(require), op->type);
            break;
        }
    }
    if (is_start) pkg_r |= mongo_pkg_append_finish_object(db_pkg);

    pkg_r |= mongo_pkg_doc_close(db_pkg);
    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build delete req fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 0, NULL, NULL, NULL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            mail_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

static void maoi_svr_db_read_record_full(mail_svr_t svr, logic_require_t require, void * result, void const * db_record) {
    SVR_MAIL_RECORD_TMPL const * db_record_tmpl = db_record;
    SVR_MAIL_FULL * mail_full = result;
    int rv;

    mail_full->mail_id = db_record_tmpl->_id;
    mail_full->sender_gid = db_record_tmpl->sender_gid;
    mail_full->send_time = db_record_tmpl->send_time;
    mail_full->state = db_record_tmpl->state;
    cpe_str_dup(mail_full->title, sizeof(mail_full->title), db_record_tmpl->title);
    cpe_str_dup(mail_full->body, sizeof(mail_full->body), db_record_tmpl->body);

    mail_full->sender_data_len = 0;
    if (svr->m_sender_meta) {
        rv = dr_pbuf_write(
            mail_full->sender_data, sizeof(mail_full->sender_data),
            ((const char *)db_record) + svr->m_record_sender_start_pos, svr->m_record_sender_capacity, svr->m_sender_meta, svr->m_em);
        if (rv < 0) {
            CPE_ERROR(
                svr->m_em, "%s: %s: parse result: encode sender fail!",
                mail_svr_name(svr), logic_require_name(require));
        }
        else {
            mail_full->sender_data_len = rv;
        }
    }

    mail_full->attach_len = 0;
    if (svr->m_attach_meta) {
        rv = dr_pbuf_write(
            mail_full->attach, sizeof(mail_full->attach),
            ((const char *)db_record) + svr->m_record_attach_start_pos, svr->m_record_attach_capacity,
            svr->m_attach_meta, svr->m_em);
        if (rv < 0) {
            CPE_ERROR(
                svr->m_em, "%s: %s: parse result: encode attach fail!",
                mail_svr_name(svr), logic_require_name(require));
        }
        else {
            mail_full->attach_len = rv;
        }
    }
}

static void maoi_svr_db_read_record_basic(mail_svr_t svr, logic_require_t require, void * result, void const * db_record) {
    SVR_MAIL_RECORD_TMPL const * db_record_tmpl = db_record;
    SVR_MAIL_BASIC * mail_basic = result;
    int rv;

    mail_basic->mail_id = db_record_tmpl->_id;
    mail_basic->sender_gid = db_record_tmpl->sender_gid;
    mail_basic->send_time = db_record_tmpl->send_time;
    mail_basic->state = db_record_tmpl->state;
    cpe_str_dup(mail_basic->title, sizeof(mail_basic->title), db_record_tmpl->title);

    mail_basic->sender_data_len = 0;
    if (svr->m_sender_meta) {
        rv = dr_pbuf_write(
            mail_basic->sender_data, sizeof(mail_basic->sender_data),
            ((const char *)db_record) + svr->m_record_sender_start_pos, svr->m_record_sender_capacity,
            svr->m_sender_meta, svr->m_em);
        if (rv < 0) {
            CPE_ERROR(
                svr->m_em, "%s: %s: parse result: encode sender fail!",
                mail_svr_name(svr), logic_require_name(require));
        }
        else {
            mail_basic->sender_data_len = rv;
        }
    }
}

static void maoi_svr_db_read_record_detail(mail_svr_t svr, logic_require_t require, void * result, void const * db_record) {
    SVR_MAIL_RECORD_TMPL const * db_record_tmpl = db_record;
    SVR_MAIL_FULL * mail_detail = result;
    int rv;

    mail_detail->mail_id = db_record_tmpl->_id;
    cpe_str_dup(mail_detail->body, sizeof(mail_detail->body), db_record_tmpl->body);

    mail_detail->attach_len = 0;
    if (svr->m_attach_meta) {
        rv = dr_pbuf_write(
            mail_detail->attach, sizeof(mail_detail->attach),
            ((const char *)db_record) + svr->m_record_attach_start_pos, svr->m_record_attach_capacity,
            svr->m_attach_meta, svr->m_em);
        if (rv < 0) {
            CPE_ERROR(
                svr->m_em, "%s: %s: parse result: encode attach fail!",
                mail_svr_name(svr), logic_require_name(require));
        }
        else {
            mail_detail->attach_len = rv;
        }
    }
}

static mongo_cli_pkg_parse_result_t
mail_svr_db_query_pkg_parse(
    void * ctx,
    logic_require_t require, mongo_pkg_t pkg, logic_data_t * result_data, LPDRMETA result_meta,
    mongo_cli_pkg_parse_evt_t evt, const void * bson_input, size_t bson_capacity)
{
    mail_svr_t svr = ctx;
    char db_record_buf[svr->m_record_size];
    void * result;

    if (evt == mongo_cli_pkg_parse_end) return mongo_cli_pkg_parse_success;

    if (evt == mongo_cli_pkg_parse_begin) {
        ssize_t data_capacity;

        assert(result_meta);

        data_capacity = dr_meta_calc_dyn_size(result_meta, mongo_pkg_doc_count(pkg));
        *result_data =
            logic_context_data_get_or_create(
                logic_require_context(require), result_meta, data_capacity);
        if (*result_data == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: %s: parse result: create result data fail!",
                mail_svr_name(svr), logic_require_name(require));
            return mongo_cli_pkg_parse_fail;
        }

        return mongo_cli_pkg_parse_next;
    }

    if (dr_bson_read(db_record_buf, sizeof(db_record_buf), bson_input, bson_capacity, svr->m_record_meta, svr->m_em) < 0) {
        CPE_ERROR(svr->m_em, "%s: %s: parse result: bson read fail!", mail_svr_name(svr), logic_require_name(require));
        return mongo_cli_pkg_parse_fail;
    }

    result = logic_data_record_append_auto_inc(result_data);
    if (result == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: parse result: append record fail, record count is %d!",
            mail_svr_name(svr), logic_require_name(require), (int)logic_data_record_count(*result_data));
        return mongo_cli_pkg_parse_fail;
    }

    if (strcmp(dr_meta_name(result_meta), "svr_mail_full") == 0) {
        maoi_svr_db_read_record_full(svr, require, result, db_record_buf);
    }
    else if (strcmp(dr_meta_name(result_meta), "svr_mail_basic") == 0) {
        maoi_svr_db_read_record_basic(svr, require, result, db_record_buf);
    }
    else if (strcmp(dr_meta_name(result_meta), "svr_mail_detail") == 0) {
        maoi_svr_db_read_record_detail(svr, require, result, db_record_buf);
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: %s: parse result: unknown result meta %s",
            mail_svr_name(svr), logic_require_name(require), dr_meta_name(result_meta));
        return mongo_cli_pkg_parse_fail;
    }

    return mongo_cli_pkg_parse_next;
}
