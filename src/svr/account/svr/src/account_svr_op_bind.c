#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_cli/mongo_cli_result.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "account_svr_ops.h"
#include "account_svr_db_ops.h"
#include "protocol/svr/account/svr_account_pro.h"
#include "protocol/svr/account/svr_account_internal.h"

logic_op_exec_result_t
account_svr_op_bind_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_ACCOUNT_REQ_BIND * req;
    uint16_t from_svr_type;

    req_data = logic_context_data_find(ctx, "svr_account_req_bind");
    if (req_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: bind: get request fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    /*获取连接信息 */
    if (set_logic_rsp_context_get_conn_info(ctx, &from_svr_type, NULL) != 0) {
        APP_CTX_ERROR(svr->m_app, "%s: bind: get_carry_info fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    /*外部系统传入的请求，只能修改自己的绑定 */
    if (account_svr_is_conn_svr(svr, from_svr_type)) {
        uint64_t account_id;
        if (account_svr_conn_get_conn_info(svr, ctx, NULL, &account_id) != 0) {
            APP_CTX_ERROR(svr->m_app, "%s: bind: get account_id from conn info fail!", account_svr_name(svr));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }

        if (req->account_id == 0) {
            req->account_id = account_id;
        }
        else {
            if (req->account_id != account_id) {
                APP_CTX_ERROR(svr->m_app, "%s: bind: can`t bind to other account!", account_svr_name(svr));
                logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_NO_RIGHT);
                return logic_op_exec_result_false;
            }
        }
    }

    require = logic_require_create(stack, "bind_query");
    if (require == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: bind: create logic require fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (account_svr_db_send_bind(svr, require, req->account_id, &req->logic_id) != 0) {
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        logic_require_free(require);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
account_svr_op_bind_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    mongo_cli_result_t db_op_res;
    int32_t updated_count;

    /*检查数据库返回结果 */
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            APP_CTX_ERROR(
                svr->m_app, "%s: bind: db request error, errno=%d!",
                account_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                svr->m_app, "%s: bind: db request state error, state=%s!",
                account_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_DB);
            return logic_op_exec_result_false;
        }
    }


    db_op_res = mongo_cli_result_find(require);
    if (db_op_res == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: bind: find db op res fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_DB);
        return logic_op_exec_result_false;
    }

    updated_count = mongo_cli_result_n(db_op_res);
    if (updated_count != 1) {
        APP_CTX_ERROR(svr->m_app, "%s: bind: updated %d records, error!", account_svr_name(svr), updated_count);
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}
