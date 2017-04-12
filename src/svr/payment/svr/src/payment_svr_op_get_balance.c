#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "payment_svr_ops.h"
#include "payment_svr_db_ops.h"
#include "payment_svr_db_data_ops.h"
#include "payment_svr_meta_bag_info.h"

logic_op_exec_result_t
payment_svr_op_get_balance_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_PAYMENT_REQ_GET_BALANCE const * req;
    logic_require_t require;
    BAG_INFO * bag_info;

    req_data = logic_context_data_find(ctx, "svr_payment_req_get_balance");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: get_balance: get request fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    bag_info = payment_svr_meta_bag_info_find(svr, req->bag_id);
    if (bag_info == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: get_balance: bag info of %d not exist!", payment_svr_name(svr), req->bag_id);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_BAG_ID_ERROR);
        return logic_op_exec_result_false;
    }

    require = logic_require_create(stack, "get_balance");
    if (require == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: get_balance: create logic require fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (payment_svr_db_send_query_money(svr, bag_info, require, req->user_id) != 0) {
        logic_require_free(require);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    
    return logic_op_exec_result_true;
}

logic_op_exec_result_t
payment_svr_op_get_balance_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_PAYMENT_REQ_GET_BALANCE const * req;
    logic_data_t res_data;
    SVR_PAYMENT_RES_GET_BALANCE * res;
    BAG_INFO * bag_info;
    int r;

    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    req_data = logic_context_data_find(ctx, "svr_payment_req_get_balance");
    assert(req_data);
    req = logic_data_data(req_data);

    bag_info = payment_svr_meta_bag_info_find(svr, req->bag_id);
    if (bag_info == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: get_balance: bag info of %d not exist!", payment_svr_name(svr), req->bag_id);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_BAG_ID_ERROR);
        return logic_op_exec_result_false;
    }

    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_get_balance, sizeof(SVR_PAYMENT_RES_GET_BALANCE));
    if (res_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: get_balance: create response buf fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    if (payment_svr_db_build_balance(svr, bag_info, require, &res->balance) != 0) {
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}
