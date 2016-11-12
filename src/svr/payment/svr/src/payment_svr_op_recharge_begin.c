#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/time_utils.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_cli/mongo_cli_result.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "payment_svr_ops.h"
#include "payment_svr_db_recharge_ops.h"
#include "payment_svr_db_ops.h"
#include "payment_svr_adapter.h"
#include "payment_svr_adapter_type.h"
#include "payment_svr_meta_bag_info.h"

logic_op_exec_result_t
payment_svr_op_recharge_begin_on_db_insert(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD * record);

logic_op_exec_result_t
payment_svr_op_recharge_begin_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_PAYMENT_REQ_RECHARGE_BEGIN const * req;
    BAG_INFO * bag_info;
    payment_svr_adapter_t adapter;
    logic_data_t record_data;
    PAYMENT_RECHARGE_RECORD * record;
    logic_require_t require;
    uint8_t i;
    
    /*获取请求 */
    req_data = logic_context_data_find(ctx, "svr_payment_req_recharge_begin");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: get request fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    /*检查bag_id*/
    bag_info = payment_svr_meta_bag_info_find(svr, req->bag_id);
    if (bag_info == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: bag info of %d not exist!", payment_svr_name(svr), req->bag_id);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_BAG_ID_ERROR);
        return logic_op_exec_result_false;
    }

    /*检查充值信息 */
    if (req->money_count <= 0 || req->money_count > CPE_ARRAY_SIZE(record->moneies)) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: money count %d error!", payment_svr_name(svr), req->money_count);
        logic_context_errno_set(ctx, -1);
        return logic_op_exec_result_false;
    }
    
    for(i = 0; i < req->money_count; ++i) {
        if (!payment_svr_meta_bag_info_support_money_type(bag_info, req->moneies[i].type)) {
            APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: bag %d not support money type %d!", payment_svr_name(svr), req->bag_id, req->moneies[i].type);
            logic_context_errno_set(ctx, -1);
            return logic_op_exec_result_false;
        }
    }
    
    /*查找充值服务是否存在 */
    adapter = payment_svr_adapter_find_by_type_id(svr, req->service);
    if (adapter == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: service %d is unknown!", payment_svr_name(svr), req->service);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_RECHARGE_SERVICE_ERROR);
        return logic_op_exec_result_false;
    }
    
    /*构建支记录 */
    record_data = logic_context_data_get_or_create(ctx, svr->m_meta_recharge_record, sizeof(PAYMENT_RECHARGE_RECORD));
    if (record_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: create charge record buf fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    record = logic_data_data(record_data);

    snprintf(
        record->_id, sizeof(record->_id), "%1d%05d%03d-" FMT_UINT64_T "-" FMT_INT64_T,
        req->device_category, set_svr_stub_svr_id(svr->m_stub), req->service, req->account_id, (uint64_t)cur_time_ms());
    record->version = 1;
    record->account_id = req->account_id;
    record->region_id = req->region_id;
    record->user_id = req->user_id;
    record->service = req->service;
    record->device_category = req->device_category;
    cpe_str_dup(record->chanel, sizeof(record->chanel), req->chanel);
    
    record->bag_id = req->bag_id;
    cpe_str_dup(record->product_id, sizeof(record->product_id), req->product_id);
    record->money_count = req->money_count;
    for(i = 0; i < req->money_count; ++i) {
        record->moneies[i] = req->moneies[i];
    }
    
    record->cost = req->cost;
    record->begin_time = payment_svr_cur_time(svr);
    record->state = PAYMENT_RECHARGE_INPROCESS;
    
    /*发送插入请求 */
    require = logic_require_create(stack, "record_insert");
    if (require == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: create logic require fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    payment_svr_db_recharge_send_insert(svr, require, record);
        
    return logic_op_exec_result_true;
}

logic_op_exec_result_t
payment_svr_op_recharge_begin_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    PAYMENT_RECHARGE_RECORD * record;

    record = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_recharge_record)));
    assert(record);

    CPE_ERROR(svr->m_em, "xxxxx: recv %s\n", logic_require_name(require));
    
    if (strcmp(logic_require_name(require), "record_insert") == 0) {
        return payment_svr_op_recharge_begin_on_db_insert(ctx, stack, require, svr, record);
    }
    else {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: unknown require %s!", payment_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

logic_op_exec_result_t
payment_svr_op_recharge_begin_on_db_insert(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, PAYMENT_RECHARGE_RECORD * record)
{
    int r;
    logic_data_t res_data;
    SVR_PAYMENT_RES_RECHARGE_BEGIN * res;
    const char * notify_to;
    
    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    /*初始化一个空的返回结果 */
    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_recharge_begin, sizeof(SVR_PAYMENT_RES_RECHARGE_BEGIN));
    if (res_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: recharge: create response buf fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);
    cpe_str_dup(res->trade_id, sizeof(res->trade_id), record->_id);

    if ((notify_to = gd_app_arg_find(logic_context_app(ctx), "--notify-to"))) {
        SVR_PAYMENT_REQ_RECHARGE_BEGIN const * req;
        payment_svr_adapter_t adapter;
        
        req = logic_data_data(logic_context_data_find(ctx, "svr_payment_req_recharge_begin"));
        assert(req);
        
        adapter = payment_svr_adapter_find_by_type_id(svr, req->service);
        assert(adapter);

        snprintf(res->notify_to, sizeof(res->notify_to), "http://%s/%s", notify_to, adapter->m_type->m_service_name);
    }
    else {
        res->notify_to[0] = 0;
    }
    
    return logic_op_exec_result_true;
}
