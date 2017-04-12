#include <assert.h>
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "friend_svr_ops.h"
#include "protocol/svr/friend/svr_friend_pro.h"
#include "protocol/svr/friend/svr_friend_internal.h"

logic_op_exec_result_t
friend_svr_op_add_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    friend_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_FRIEND_REQ_ADD const * req;
    logic_data_t op_ctx_data;
    SVR_FRIEND_OP_ADD_CTX * op_ctx;
    int rv;

    req_data = logic_context_data_find(ctx, "svr_friend_req_add");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: read record: find req fail!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    op_ctx_data = 
        logic_context_data_get_or_create(
            ctx, svr->m_meta_op_add_ctx,
            dr_meta_size(svr->m_meta_op_add_ctx) + svr->m_record_size);
    if (op_ctx_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: read record: create op_add ctx fail!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    op_ctx =(SVR_FRIEND_OP_ADD_CTX*)logic_data_data(op_ctx_data);

    rv = dr_pbuf_read(
        op_ctx->record + svr->m_record_data_start_pos,
        svr->m_record_size - svr->m_record_data_start_pos,
        req->data,
        req->data_len,
        svr->m_data_meta,
        svr->m_em);
    if (rv <= 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: read record: decode data error, rv=%d!", friend_svr_name(svr), rv);
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    op_ctx->record_len = (uint16_t)rv;

    op_ctx->uid = req->user_id;
    friend_svr_record_set_uid(svr, req->user_id, op_ctx->record);

    op_ctx->fuid = friend_svr_record_fuid(svr, op_ctx->record);

    if (friend_svr_record_build_id(svr, op_ctx->record) != 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: read record: set _id error!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    friend_svr_record_set_state(
        svr,
        svr->m_runing_mode == friend_svr_runing_mode_one_way ? 0 : SVR_FRIEND_STATE_REQ_SEND,
        op_ctx->record);

    /*插入自己好友记录 */
    if (friend_svr_db_send_insert(svr, stack, "insert_self", op_ctx->record) != 0) {
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    /*插入对方好友记录 */
    if (svr->m_runing_mode == friend_svr_runing_mode_ack) {
        char buf[svr->m_record_size];
        int r;

        memcpy(buf, op_ctx->record, sizeof(buf));
        friend_svr_record_set_uid(svr, op_ctx->fuid, buf);
        friend_svr_record_set_fuid(svr, op_ctx->uid, buf);
        friend_svr_record_set_state(svr, SVR_FRIEND_STATE_REQ_RECV, buf);
        r = friend_svr_record_build_id(svr, buf);
        assert(r == 0);

        if (friend_svr_db_send_insert(svr, stack, "insert_other", buf) != 0) {
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
    }

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
friend_svr_op_add_on_insert_self_result(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, friend_svr_t svr);
static logic_op_exec_result_t
friend_svr_op_add_on_insert_other_result(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, friend_svr_t svr);
static logic_op_exec_result_t
friend_svr_op_add_on_query_self_result(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, friend_svr_t svr);
static logic_op_exec_result_t
friend_svr_op_add_on_query_other_result(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, friend_svr_t svr);

logic_op_exec_result_t
friend_svr_op_add_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    friend_svr_t svr = user_data;

    if (strcmp(logic_require_name(require), "insert_self") == 0) {
        return friend_svr_op_add_on_insert_self_result(ctx, stack, require, svr);
    }
    else if (strcmp(logic_require_name(require), "insert_other") == 0) {
        return friend_svr_op_add_on_insert_other_result(ctx, stack, require, svr);
    }
    else if (strcmp(logic_require_name(require), "query_self") == 0) {
        return friend_svr_op_add_on_query_self_result(ctx, stack, require, svr);
    }
    else if (strcmp(logic_require_name(require), "query_other") == 0) {
        return friend_svr_op_add_on_query_other_result(ctx, stack, require, svr);
    }
    else {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: request %s unknown!", friend_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

static logic_op_exec_result_t
friend_svr_op_add_on_insert_self_result(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, friend_svr_t svr) {
    SVR_FRIEND_OP_ADD_CTX const * op_ctx;

    op_ctx =(SVR_FRIEND_OP_ADD_CTX*)logic_data_data(logic_context_data_find(ctx, "svr_friend_op_add_ctx"));

    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) != logic_require_state_error) {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: add: db request state error, state=%s!",
                friend_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        else if (logic_require_error(require) == mongo_data_error_duplicate_key) {
            if (friend_svr_db_send_query_one(svr, stack, "query_self", op_ctx->uid, op_ctx->fuid) != 0) {
                APP_CTX_ERROR(logic_context_app(ctx), "%s: add: send query self req fail!", friend_svr_name(svr));
                logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
                return logic_op_exec_result_false;
            }
            return logic_op_exec_result_true;
        }
        else {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: add: db request error, errno=%d!",
                friend_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_DB);
            return logic_op_exec_result_false;
        }
    }

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
friend_svr_op_add_on_insert_other_result(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, friend_svr_t svr) {
    SVR_FRIEND_OP_ADD_CTX const * op_ctx;

    op_ctx =(SVR_FRIEND_OP_ADD_CTX*)logic_data_data(logic_context_data_find(ctx, "svr_friend_op_add_ctx"));

    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) != logic_require_state_error) {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: %s: db request state error, state=%s!",
                friend_svr_name(svr), logic_require_name(require), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        else if (logic_require_error(require) == mongo_data_error_duplicate_key) {
            if (friend_svr_db_send_query_one(svr, stack, "query_other", op_ctx->fuid, op_ctx->uid) != 0) {
                APP_CTX_ERROR(logic_context_app(ctx), "%s: add: send query other req fail!", friend_svr_name(svr));
                logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
                return logic_op_exec_result_false;
            }
            return logic_op_exec_result_true;
        }
        else {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: %s: db request error, errno=%d!",
                friend_svr_name(svr), logic_require_name(require), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_DB);
            return logic_op_exec_result_false;
        }
    }

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
friend_svr_op_add_on_query_self_result(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, friend_svr_t svr) {
    logic_data_t query_result_data;
    uint8_t * query_result;
    uint32_t record_count;
    SVR_FRIEND_RECORD * record;
    SVR_FRIEND_OP_ADD_CTX const * op_ctx;

    op_ctx =(SVR_FRIEND_OP_ADD_CTX*)logic_data_data(logic_context_data_find(ctx, "svr_friend_op_add_ctx"));


    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) != logic_require_state_error) {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: add: db request state error, state=%s!",
                friend_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: add: db request error, errno=%d!",
                friend_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_DB);
            return logic_op_exec_result_false;
        }
    }

    query_result_data = logic_require_data_find(require, dr_meta_name(svr->m_record_list_meta));
    if (query_result_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: find query result!",
            friend_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return -1;
    }
    query_result = logic_data_data(query_result_data);

    if (dr_entry_try_read_uint32(
            &record_count,
            query_result + dr_entry_data_start_pos(svr->m_record_list_count_entry, 0),
            svr->m_record_list_count_entry,
            svr->m_em) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: %s: read record count fail!", friend_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return -1;
    }

    if (record_count != 1) {
        CPE_ERROR(svr->m_em, "%s: %s: record-count %d error!", friend_svr_name(svr), logic_require_name(require), record_count);
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return -1;
    }

    record = (SVR_FRIEND_RECORD*)(query_result + dr_entry_data_start_pos(svr->m_record_list_data_entry, 0));

    switch(record->state) {
    case SVR_FRIEND_STATE_OK:
        CPE_ERROR(svr->m_em, "%s: %s: %s is already friend!", friend_svr_name(svr), logic_require_name(require), record->_id);
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_ALREADY_EXIST);
        return logic_op_exec_result_false;
    case SVR_FRIEND_STATE_REQ_SEND:
        return logic_op_exec_result_true;
    case SVR_FRIEND_STATE_REQ_RECV: {
        if (friend_svr_db_send_update_state(svr, NULL, NULL, op_ctx->uid, op_ctx->fuid, SVR_FRIEND_STATE_REQ_RECV, SVR_FRIEND_STATE_OK) != 0) {
            APP_CTX_ERROR(logic_context_app(ctx), "%s: add: update friend state: send request fail!", friend_svr_name(svr));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        return logic_op_exec_result_true;
    }
    default:
        CPE_ERROR(svr->m_em, "%s: %s: %s state %d unknown!", friend_svr_name(svr), logic_require_name(require), record->_id, record->state);
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

static logic_op_exec_result_t
friend_svr_op_add_on_query_other_result(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, friend_svr_t svr) {
    logic_data_t query_result_data;
    uint8_t * query_result;
    uint32_t record_count;
    SVR_FRIEND_RECORD * record;
    SVR_FRIEND_OP_ADD_CTX const * op_ctx;

    op_ctx =(SVR_FRIEND_OP_ADD_CTX*)logic_data_data(logic_context_data_find(ctx, "svr_friend_op_add_ctx"));


    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) != logic_require_state_error) {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: add: db request state error, state=%s!",
                friend_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: add: db request error, errno=%d!",
                friend_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_DB);
            return logic_op_exec_result_false;
        }
    }

    query_result_data = logic_require_data_find(require, dr_meta_name(svr->m_record_list_meta));
    if (query_result_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: find query result!",
            friend_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return -1;
    }
    query_result = logic_data_data(query_result_data);

    if (dr_entry_try_read_uint32(
            &record_count,
            query_result + dr_entry_data_start_pos(svr->m_record_list_count_entry, 0),
            svr->m_record_list_count_entry,
            svr->m_em) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: %s: read record count fail!", friend_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return -1;
    }

    if (record_count != 1) {
        CPE_ERROR(svr->m_em, "%s: %s: record-count %d error!", friend_svr_name(svr), logic_require_name(require), record_count);
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return -1;
    }

    record = (SVR_FRIEND_RECORD*)(query_result + dr_entry_data_start_pos(svr->m_record_list_data_entry, 0));

    switch(record->state) {
    case SVR_FRIEND_STATE_OK:
        CPE_INFO(svr->m_em, "%s: %s: %s is already friend!", friend_svr_name(svr), logic_require_name(require), record->_id);
        return logic_op_exec_result_true;
    case SVR_FRIEND_STATE_REQ_RECV:
        return logic_op_exec_result_true;
    case SVR_FRIEND_STATE_REQ_SEND:
        if (friend_svr_db_send_update_state(svr, NULL, NULL, op_ctx->fuid, op_ctx->uid, SVR_FRIEND_STATE_REQ_SEND, SVR_FRIEND_STATE_OK) != 0) {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: %s: update friend state: send request fail!",
                friend_svr_name(svr), logic_require_name(require));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        return logic_op_exec_result_true;
    default:
        CPE_ERROR(svr->m_em, "%s: %s: %s state %d unknown!", friend_svr_name(svr), logic_require_name(require), record->_id, record->state);
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}
