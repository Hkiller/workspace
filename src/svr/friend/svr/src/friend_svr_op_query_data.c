#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_pbuf.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "friend_svr_ops.h"
#include "protocol/svr/friend/svr_friend_pro.h"
#include "protocol/svr/friend/svr_friend_internal.h"

logic_op_exec_result_t
friend_svr_op_query_data_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    friend_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_FRIEND_REQ_QUERY_DATA * req;

    req_data = logic_context_data_find(ctx, "svr_friend_req_query_data");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: query: get request fail!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    if (friend_svr_db_send_query_data(svr, stack, "query", req->user_id, req->friend_count, req->friends) != 0) {
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
friend_svr_op_query_data_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    friend_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_FRIEND_REQ_QUERY_DATA const * req;
    logic_data_t res_data;
    SVR_FRIEND_RES_QUERY_DATA * res;
    logic_data_t query_result_data;
    uint8_t * query_result;
    uint32_t record_count;
    uint32_t i;
    uint32_t data_capacity;

    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) != logic_require_state_error) {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: query: db request error, errno=%d!",
                friend_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_DB);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: query: db request state error, state=%s!",
                friend_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
    }

    req_data = logic_context_data_find(ctx, "svr_friend_req_query_data");
    assert(req_data);
    req = logic_data_data(req_data);

    query_result_data = logic_require_data_find(require, dr_meta_name(svr->m_record_list_meta));
    if (query_result_data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: find query result!",
            friend_svr_name(svr), logic_require_name(require));
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
        return -1;
    }

    data_capacity = logic_data_capacity(query_result_data);
    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_query, sizeof(SVR_FRIEND_RES_QUERY) + data_capacity);
    if (res_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: %s: create response buf fail!", friend_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    res->user_id = req->user_id;
    res->data_len = 0;

    for(i = 0; i < record_count && data_capacity > 0; ++i) {
        const uint8_t * query_record;
        int rv;

        query_record = query_result + dr_entry_data_start_pos(svr->m_record_list_data_entry, 0);
        
        rv = dr_pbuf_write(
            res->data + res->data_len, data_capacity,
            query_record + 0 /*TODO*/, svr->m_data_size, svr->m_data_meta, svr->m_em);
        if (rv <= 0) {
            APP_CTX_ERROR(
                svr->m_app, "%s: %s: pbuf write record error, rv=%d!",
                friend_svr_name(svr), logic_require_name(require), rv);
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        assert(rv <= data_capacity);
        data_capacity -= rv;
        res->data_len += rv;
    }
    
    return logic_op_exec_result_true;
}
