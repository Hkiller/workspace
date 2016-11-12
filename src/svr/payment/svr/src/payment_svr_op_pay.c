#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_cli/mongo_cli_result.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "payment_svr_ops.h"
#include "payment_svr_db_ops.h"
#include "payment_svr_db_data_ops.h"
#include "payment_svr_db_bill_ops.h"
#include "payment_svr_meta_bag_info.h"

static logic_op_exec_result_t
payment_svr_op_pay_on_db_update(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, SVR_PAYMENT_REQ_PAY const * req, SVR_PAYMENT_RES_PAY * res, BAG_INFO * bag_info);

logic_op_exec_result_t
payment_svr_op_pay_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_PAYMENT_REQ_PAY const * req;
    BAG_INFO * bag_info;
    logic_data_t res_data;
    SVR_PAYMENT_RES_PAY * res;
    logic_require_t require;

    /*获取请求 */
    req_data = logic_context_data_find(ctx, "svr_payment_req_pay");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: pay: get request fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    /*检查bag_id */
    bag_info = payment_svr_meta_bag_info_find(svr, req->bag_id);
    if (bag_info == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: pay: bag info of %d not exist!", payment_svr_name(svr), req->bag_id);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_BAG_ID_ERROR);
        return logic_op_exec_result_false;
    }

    /*检查money types*/
    if (payment_svr_op_validate_money_types(svr, bag_info, &req->pay) != 0) {
        CPE_ERROR(svr->m_em, "%s: pay: validate moneies fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_MONEY_TYPE_ERROR);
        return logic_op_exec_result_false;
    }

    /*初始化一个空的返回结果 */
    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_pay, sizeof(SVR_PAYMENT_RES_PAY));
    if (res_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: pay: create response buf fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);
    res->result = 0;
    res->balance.count = 0;

    /*发送扣款请求 */
    require = logic_require_create(stack, "pay_update");
    if (require == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: pay: create logic require fail!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (payment_svr_db_send_remove_money(svr, bag_info, require, req->user_id, &req->pay) != 0) {
        logic_require_free(require);
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
payment_svr_op_pay_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    payment_svr_t svr = user_data;
    SVR_PAYMENT_REQ_PAY const * req;
    SVR_PAYMENT_RES_PAY * res;
    BAG_INFO * bag_info;

    req = logic_data_data(logic_context_data_find(ctx, "svr_payment_req_pay"));
    assert(req);

    res = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_res_pay)));
    assert(res);

    bag_info = payment_svr_meta_bag_info_find(svr, req->bag_id);
    assert(bag_info);
 
    if (strcmp(logic_require_name(require), "pay_update") == 0) {
        return payment_svr_op_pay_on_db_update(ctx, stack, require, svr, req, res, bag_info);
    }
    else {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: pay: unknown require %s!", payment_svr_name(svr), logic_require_name(require));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
}

logic_op_exec_result_t
payment_svr_op_pay_on_db_update(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    payment_svr_t svr, SVR_PAYMENT_REQ_PAY const * req, SVR_PAYMENT_RES_PAY * res, BAG_INFO * bag_info)
{
    mongo_cli_result_t update_result;
    int r;

    if ((r = payment_svr_db_validate_result(svr, require))) {
        logic_context_errno_set(ctx, r);
        return logic_op_exec_result_false;
    }

    update_result = mongo_cli_result_find(require);
    if (update_result == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: pay_update:: no update result!", payment_svr_name(svr));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (mongo_cli_result_n(update_result) > 1) {
        APP_CTX_ERROR(
            logic_context_app(ctx), "%s: pay: on_db_update:: update %d records !!!",
            payment_svr_name(svr), mongo_cli_result_n(update_result));
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (payment_svr_db_build_balance(svr, bag_info, require, &res->balance) != 0) {
        logic_context_errno_set(ctx, SVR_PAYMENT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (mongo_cli_result_n(update_result) == 1) { /*老用户更新成功，查询回数据 */
        PAYMENT_BILL_DATA bill_data;

        res->result = SVR_PAYMENT_ERRNO_SUCCESS;

        bzero(&bill_data, sizeof(bill_data));
        bill_data.way = payment_bill_way_out;
        bill_data.money = req->pay;
        cpe_str_dup(bill_data.product_id, sizeof(bill_data.product_id), req->product_id);

        payment_svr_db_add_bill(svr, bag_info, req->user_id, &bill_data, &res->balance);
    }
    else {/*没有符合条件的记录，应该是金币不足导致 */
        assert(mongo_cli_result_n(update_result) == 0);
        res->result = SVR_PAYMENT_ERRNO_NOT_ENOUTH_MONEY;
    }

    return logic_op_exec_result_true;
}
