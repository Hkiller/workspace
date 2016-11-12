#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "account_svr_ops.h"
#include "account_svr_login_info.h"

logic_op_exec_result_t
account_svr_op_query_login_info(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    account_svr_t svr = user_data;
    logic_data_t req_data;
    logic_data_t res_data;
    SVR_ACCOUNT_REQ_QUERY_LOGIN_INFO const * req;
    SVR_ACCOUNT_RES_QUERY_LOGIN_INFO * res;
    account_svr_login_info_t login_info;
    
    req_data = logic_context_data_find(ctx, "svr_account_req_query_login_info");
    if (req_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: query login info: get request fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    req = logic_data_data(req_data);
    assert(req);

    login_info = account_svr_login_info_find(svr, req->account_id);
    if (login_info == NULL) {
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_NOT_LOGIN);
        return logic_op_exec_result_false;
    }

    /*构造响应 */
    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_query_login_info, 0);
    if (res_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: query login info: create response fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    res->data = login_info->m_data;
    
    return logic_op_exec_result_true;
}

