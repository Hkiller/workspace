#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/time_utils.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "protocol/svr/gift/svr_gift_pro.h"
#include "protocol/svr/gift/svr_gift_internal.h"
#include "gift_svr_ops.h"
#include "gift_svr_db_ops_use.h"

logic_op_exec_result_t
gift_svr_op_query_use_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    gift_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_GIFT_REQ_QUERY_USE const * req;

    req_data = logic_context_data_find(ctx, "svr_gift_req_query_use");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: query use: find req fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    require = logic_require_create(stack, "query");
    if (require == NULL) {
        CPE_ERROR(svr->m_em, "%s: query use: create require fail!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (gift_svr_db_send_use_query_by_generate(svr, require, req->generate_id) != 0) {
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }        
    
    return logic_op_exec_result_true;
}

logic_op_exec_result_t
gift_svr_op_query_use_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg) {
    gift_svr_t svr = user_data;
    uint32_t result_capacity;
    logic_data_t result_data;
    SVR_GIFT_RES_QUERY_USE * result;
    logic_data_t query_result_data;
    SVR_GIFT_USE_RECORD_LIST * query_result;
    uint16_t i;
    
    if (gift_svr_op_check_db_result(svr, ctx, require) != 0) {
        return logic_op_exec_result_false;
    }

    query_result_data = logic_require_data_find(require, "svr_gift_use_record_list");
    assert(query_result_data);
    query_result = logic_data_data(query_result_data);
    
    result_capacity = sizeof(SVR_GIFT_RES_QUERY_USE) + sizeof(SVR_GIFT_USE_FULL) * query_result->record_count;

    result_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_generate, result_capacity);
    if (result_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: generate: create result data fail, capacity=%d!", gift_svr_name(svr), result_capacity);
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    result = logic_data_data(result_data);

    for(i = 0; i < query_result->record_count; ++i) {
        SVR_GIFT_USE_RECORD const * query_record = &query_result->records[i];
        SVR_GIFT_USE_FULL * result_record = &result->records[result->record_count++];

        cpe_str_dup(result_record->cdkey, sizeof(result_record->cdkey), query_record->_id);
        result_record->generate_id = query_record->generate_id;
        result_record->state = query_record->state;
        result_record->state_data = query_record->state_data;
    }
    
    return logic_op_exec_result_true;
}
