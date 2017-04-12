#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "gift_svr_ops.h"
#include "gift_svr_db_ops_use.h"
#include "gift_svr_generate_record.h"

static logic_op_exec_result_t gift_svr_op_use_build_result(gift_svr_t svr, logic_context_t ctx);
static logic_op_exec_result_t gift_svr_op_use_recv_origin_query(gift_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require);
static logic_op_exec_result_t gift_svr_op_use_recv_origin_update(gift_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require);
static logic_op_exec_result_t gift_svr_op_use_recv_user_query(gift_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require);
static logic_op_exec_result_t gift_svr_op_use_recv_user_insert(gift_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require);

static logic_op_exec_result_t gift_svr_op_use_recv_user_insert(gift_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require);

logic_op_exec_result_t
gift_svr_op_use_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    gift_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_GIFT_REQ_USE const * req;

    req_data = logic_context_data_find(ctx, "svr_gift_req_use");
    if (req_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: add: get request fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    require = logic_require_create(stack, "origin_query");
    if (require == NULL) {
        CPE_ERROR(svr->m_em, "%s: gift_use: create logic require fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (gift_svr_db_send_use_query(svr, require, req->cdkey) != 0) {
        CPE_ERROR(svr->m_em, "%s: gift_use: send db request fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
gift_svr_op_use_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    gift_svr_t svr = user_data;
    const char * require_name;

    require_name = logic_require_name(require);
    if (strcmp(require_name, "origin_query") == 0) {
        return gift_svr_op_use_recv_origin_query(svr, ctx, stack, require);
    }
    else if (strcmp(require_name, "origin_update") == 0) {
        return gift_svr_op_use_recv_origin_update(svr, ctx, stack, require);
    }
    else if (strcmp(require_name, "user_query") == 0) {
        return gift_svr_op_use_recv_user_query(svr, ctx, stack, require);
    }
    else if (strcmp(require_name, "user_insert") == 0) {
        return gift_svr_op_use_recv_user_insert(svr, ctx, stack, require);
    }
    else {
        CPE_ERROR(svr->m_em, "%s: login: unknown require %s!", gift_svr_name(svr), require_name);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

logic_op_exec_result_t
gift_svr_op_use_recv_origin_query(gift_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require) {
    logic_data_t query_result_data;
    SVR_GIFT_USE_RECORD_LIST * query_result;
    SVR_GIFT_USE_RECORD * origin_record;
    void const * generate_record;
    SVR_GIFT_GENERATE_RECORD const * generate_record_common;
    logic_data_t req_data;
    SVR_GIFT_REQ_USE const * req;
    logic_data_t context_record_data;

    if (gift_svr_op_check_db_result(svr, ctx, require) != 0) return logic_op_exec_result_false;

    req_data = logic_context_data_find(ctx, "svr_gift_req_use");
    if (req_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: add: get request fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    query_result_data = logic_require_data_find(require, dr_meta_name(svr->m_use_record_list_meta));
    assert(query_result_data);
    query_result = logic_data_data(query_result_data);

    if (query_result->record_count == 0) { /*cdkey不存在 */
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_NOTEXIST);
        return logic_op_exec_result_true;
    }

    if (query_result->record_count > 1) {
        CPE_ERROR(svr->m_em, "%s: origin_query: cdkey %s record overflow!", gift_svr_name(svr), query_result->records[1]._id);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    origin_record = &query_result->records[0];

    context_record_data = logic_context_data_get_or_create(ctx, svr->m_use_record_meta, dr_meta_size(svr->m_use_record_meta));
    if (context_record_data == NULL) {
        CPE_ERROR(svr->m_em, "%s: origin_query: copy origin record to ctx fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    memcpy(logic_data_data(context_record_data), &query_result->records[0], sizeof(query_result->records[0]));

    generate_record = gift_svr_record_find(svr, origin_record->generate_id);
    if (generate_record == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: origin_query: cdkey %s generator %d not exist!",
            gift_svr_name(svr), origin_record->_id, origin_record->generate_id);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    generate_record_common = (SVR_GIFT_GENERATE_RECORD const *)generate_record;
    if (generate_record_common->use_policy == svr_gift_use_once_global) { /*全局使用一次 */
        if (origin_record->state == svr_gift_use_state_used) {
            logic_context_errno_set(ctx, SVR_GIFT_ERRNO_ALREADY_USED);
            return logic_op_exec_result_true;
        }
        else {
            logic_require_t update_require;

            update_require = logic_require_create(stack, "origin_update");
            if (require == NULL) {
                CPE_ERROR(svr->m_em, "%s: gift_use: create require fail!", gift_svr_name(svr));
                logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
                return logic_op_exec_result_false;
            }

            origin_record->state = svr_gift_use_state_used;
            origin_record->state_data.use.user_id = req->user_id;
            origin_record->state_data.use.region_id = req->region_id;
            origin_record->state_data.use.use_time = gift_svr_cur_time(svr);

            if (gift_svr_db_send_use_update_state(svr, update_require, svr_gift_use_state_not_used, origin_record) != 0) {
                CPE_ERROR(svr->m_em, "%s: gift_use: send origin update request fail!", gift_svr_name(svr));
                logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
                return logic_op_exec_result_false;
            }

            return logic_op_exec_result_true;
        }
    }
    else if (generate_record_common->use_policy == svr_gift_use_once_per_user) {  /*每用户使用一次 */
        logic_require_t query_require;
        char id_buf[64];

        query_require = logic_require_create(stack, "user_query");
        if (require == NULL) {
            CPE_ERROR(svr->m_em, "%s: gift_use: create require fail!", gift_svr_name(svr));
            logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        snprintf(id_buf, sizeof(id_buf), "%s-" FMT_UINT64_T, origin_record->_id, req->user_id);

        if (gift_svr_db_send_use_query(svr, query_require, id_buf) != 0) {
            CPE_ERROR(svr->m_em, "%s: gift_use: send query user record request fail!", gift_svr_name(svr));
            logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        return logic_op_exec_result_true;
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: origin_query: cdkey %s generator %d use policy %d unknown!",
            gift_svr_name(svr), origin_record->_id, origin_record->generate_id, generate_record_common->use_policy);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

logic_op_exec_result_t
gift_svr_op_use_recv_origin_update(gift_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require) {
    if (gift_svr_op_check_db_result(svr, ctx, require) != 0) return logic_op_exec_result_false;

    /*TODO: 判断记录数，确保没有被改动 */

    return gift_svr_op_use_build_result(svr, ctx);
}

logic_op_exec_result_t
gift_svr_op_use_recv_user_query(gift_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require) {
    logic_data_t query_result_data;
    SVR_GIFT_USE_RECORD_LIST * query_result;
    logic_data_t req_data;
    SVR_GIFT_USE_RECORD * origin_record;
    SVR_GIFT_REQ_USE const * req;
    void const * generate_record;
    logic_data_t origin_record_data;
    logic_require_t inset_require;
    char id_buf[64];

    if (gift_svr_op_check_db_result(svr, ctx, require) != 0) return logic_op_exec_result_false;

    req_data = logic_context_data_find(ctx, "svr_gift_req_use");
    assert(req_data);
    req = logic_data_data(req_data);

    query_result_data = logic_require_data_find(require,dr_meta_name(svr->m_use_record_list_meta));
    assert(query_result_data);
    query_result = logic_data_data(query_result_data);

    origin_record_data = logic_context_data_find(ctx, dr_meta_name(svr->m_use_record_meta));
    assert(origin_record_data);
    origin_record = logic_data_data(origin_record_data);

    generate_record = gift_svr_record_find(svr, origin_record->generate_id);
    assert(generate_record);

    if(query_result->record_count > 1) { /*数据错误 */
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if(query_result->record_count == 1) { /*cdkey存在不可以使用 */
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_ALREADY_USED);
        return logic_op_exec_result_false;
    }

    inset_require = logic_require_create(stack, "user_insert");
    if (require == NULL) {
        CPE_ERROR(svr->m_em, "%s: gift_use: create require fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    snprintf(id_buf, sizeof(id_buf), "%s-" FMT_UINT64_T, origin_record->_id, req->user_id);
    cpe_str_dup(origin_record->_id, sizeof(origin_record->_id), id_buf);
    origin_record->state = svr_gift_use_state_used;
    origin_record->state_data.use.user_id = req->user_id;
    origin_record->state_data.use.region_id = req->region_id;
    origin_record->state_data.use.use_time = gift_svr_cur_time(svr);

    if (gift_svr_db_send_use_insert(svr, inset_require, origin_record) != 0) {
        CPE_ERROR(svr->m_em, "%s: gift_use: send origin insert request fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
gift_svr_op_use_recv_user_insert(gift_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require) {
    if (gift_svr_op_check_db_result(svr, ctx, require) != 0) return logic_op_exec_result_false;

    /*应该判断是否主键重复错误，如果是主键重复，应该返回已经使用 */

    return gift_svr_op_use_build_result(svr, ctx);
}

logic_op_exec_result_t gift_svr_op_use_build_result(gift_svr_t svr, logic_context_t ctx) {
    logic_data_t result_data;
    size_t result_capacity;
    SVR_GIFT_RES_USE * result;
    logic_data_t origin_record_data;
    SVR_GIFT_USE_RECORD * origin_record;
    void const * generate_record;
    int rv;

    origin_record_data = logic_context_data_find(ctx, dr_meta_name(svr->m_use_record_meta));
    assert(origin_record_data);
    origin_record = logic_data_data(origin_record_data);

    generate_record = gift_svr_record_find(svr, origin_record->generate_id);
    assert(generate_record);

    result_capacity = sizeof(SVR_GIFT_RES_USE) + svr->m_data_size * 3 + 32;

    result_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_use, result_capacity);
    if (result_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: use: create result data fail, capacity=%d!", gift_svr_name(svr), (int)result_capacity);
        return logic_op_exec_result_false;
    }
    result = logic_data_data(result_data);

    rv = dr_pbuf_write(
        result->data, result_capacity - sizeof(SVR_GIFT_RES_USE),
        ((char*)generate_record) + svr->m_generate_record_data_start_pos, svr->m_data_size, svr->m_data_meta,
        svr->m_em);
    if (rv < 0) {
        CPE_ERROR(
            svr->m_em, "%s: use: build result: encode data fail, capacity=%d, rv=%d!",
            gift_svr_name(svr), (int)(result_capacity - sizeof(SVR_GIFT_RES_USE)), rv);
        return logic_op_exec_result_false;
    }
    result->data_len = rv;

    return logic_op_exec_result_true;
}
