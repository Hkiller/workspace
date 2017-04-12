#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "account_svr_ops.h"
#include "account_svr_db_ops.h"
#include "account_svr_conn_info.h"
#include "account_svr_login_info.h"
#include "account_svr_backend.h"

static logic_op_exec_result_t account_svr_op_login_send_query_db_req(
    account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, SVR_ACCOUNT_REQ_LOGIN const * req);
static SVR_ACCOUNT_REQ_LOGIN * account_svr_op_login_req(account_svr_t svr, logic_context_t ctx);

logic_op_exec_result_t
account_svr_op_login_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    SVR_ACCOUNT_REQ_LOGIN const * req;
    account_svr_backend_t backend;

    req = account_svr_op_login_req(svr, ctx);
    if (req == NULL) return logic_op_exec_result_false;

    backend = account_svr_backend_find(svr, req->logic_id.account_type);
    if (backend == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: login: account type %d not support!", account_svr_name(svr), req->logic_id.account_type);
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_NOT_SUPPORT_ACCOUNT_TYPE);
        return logic_op_exec_result_false;
    }

    if (strlen(req->logic_id.account) < 5) {
        APP_CTX_ERROR(svr->m_app, "%s: login: logic_id %s length too small, at least 5!", account_svr_name(svr), req->logic_id.account);
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (backend->m_token_to_id) {
        return account_svr_backend_check_send_logic_to_id_req(backend, stack, &req->logic_id);
    }
    else {
        return account_svr_op_login_send_query_db_req(svr, ctx, stack, req);
    }
}

static SVR_ACCOUNT_REQ_LOGIN * account_svr_op_login_req(account_svr_t svr, logic_context_t ctx) {
    logic_data_t req_data;

    req_data = logic_context_data_find(ctx, "svr_account_req_login");
    if (req_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: login: get request fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return NULL;
    }
    else {
        return logic_data_data(req_data);
    }
}

static logic_op_exec_result_t
account_svr_op_login_send_query_db_req(account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, SVR_ACCOUNT_REQ_LOGIN const * req) {
    logic_require_t require;

    require = logic_require_create(stack, "query_db");
    if (require == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: login: create logic require fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (account_svr_db_send_query_by_logic_id(svr, require, &req->logic_id, svr->m_meta_record_basic_list) != 0) {
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        logic_require_free(require);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
account_svr_op_login_recv_query_db(account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require) {
    logic_data_t db_res;
    SVR_ACCOUNT_BASIC_LIST * account_list;
    SVR_ACCOUNT_BASIC * account;
    logic_data_t res_data;
    SVR_ACCOUNT_RES_LOGIN * res;
    SVR_ACCOUNT_REQ_LOGIN const * req;
    uint16_t from_svr_type;
    uint16_t from_svr_id;
    account_svr_account_info_t local_account_info;

    /*检查数据库返回结果 */
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            APP_CTX_ERROR(
                svr->m_app, "%s: login: db request error, errno=%d!",
                account_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                svr->m_app, "%s: login: db request state error, state=%s!",
                account_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_DB);
            return logic_op_exec_result_false;
        }
    }

    /*获取数据库返回结果 */
    db_res = logic_require_data_find(require, dr_meta_name(svr->m_meta_record_basic_list));
    if (db_res == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: login: find db result fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    account_list = logic_data_data(db_res);

    req = account_svr_op_login_req(svr, ctx);
    assert(req);

    /*检查记录数 */
    if (account_list->count == 0) {
        mem_buffer_clear_data(&svr->m_dump_buffer);

        APP_CTX_ERROR(
            svr->m_app, "%s: login: account %s not exist!", account_svr_name(svr),
            dr_json_dump(&svr->m_dump_buffer, &req->logic_id, sizeof(req->logic_id), svr->m_meta_logic_id));

        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_NOT_EXIST);

        return logic_op_exec_result_false;
    }
    else if (account_list->count > 1) {
        mem_buffer_clear_data(&svr->m_dump_buffer);

        APP_CTX_ERROR(
            svr->m_app, "%s: login: find duplicate accounts\n%s!", account_svr_name(svr),
            dr_json_dump(&svr->m_dump_buffer, account_list, logic_data_capacity(db_res), svr->m_meta_record_basic_list));

        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);

        return logic_op_exec_result_false;
    }

    /*获取登陆请求 */
    account = &account_list->data[0];

    if ((local_account_info = account_svr_account_info_find(svr, &req->logic_id))) {
        if (local_account_info->m_state > account->account_state) {
            account->account_state = local_account_info->m_state;
            APP_CTX_INFO(
                svr->m_app, "%s: login: %d:%s state ==> %d!",
                account_svr_name(svr), req->logic_id.account_type, req->logic_id.account, account->account_state);
        }
    }

    /*获取连接信息 */
    if (set_logic_rsp_context_get_conn_info(ctx, &from_svr_type, &from_svr_id) != 0) {
        APP_CTX_ERROR(svr->m_app, "%s: login: get_carry_info fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    /*如果从连接服务过来的请求，需要将账号绑定到连接上去 */
    if (account_svr_is_conn_svr(svr, from_svr_type)) {
        if (account_svr_conn_bind_account(
                svr, ctx, from_svr_id, from_svr_type, account->_id, account->account_state,
                req->device_category, req->device_cap, req->chanel)
            != 0)
        {
            APP_CTX_ERROR(svr->m_app, "%s: login: set_carry_info not exist!", account_svr_name(svr));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
    }

    /*构造响应 */
    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_login, 0);
    if (res_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: login: create response fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    res->account_id = account->_id;
    res->account_state = account->account_state;

    account_svr_login_info_update(svr, account->_id, &req->logic_id, ctx);

    return logic_op_exec_result_true;

}

static logic_op_exec_result_t
account_svr_op_login_recv_token_to_id(account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require) {
    SVR_ACCOUNT_REQ_LOGIN * req;
    logic_data_t login_info_data;
    SVR_ACCOUNT_LOGIN_INFO * login_info;

    req = account_svr_op_login_req(svr, ctx);
    assert(req);

    /*检查返回结果 */
    if (logic_require_state(require) != logic_require_state_done) {
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    login_info_data = logic_require_data_find(require, dr_meta_name(svr->m_meta_login_info));
    assert(login_info_data);

    login_info = logic_data_data(login_info_data);

    req->logic_id = login_info->logic_id;

    logic_context_data_copy(ctx, login_info_data);

    return account_svr_op_login_send_query_db_req(svr, ctx, stack, req);
}

logic_op_exec_result_t
account_svr_op_login_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    const char * require_name;

    require_name = logic_require_name(require);
    if (strcmp(require_name, "query_db") == 0) {
        return account_svr_op_login_recv_query_db(svr, ctx, stack, require);
    }
    else if (strcmp(require_name, "token_to_id") == 0) {
        return account_svr_op_login_recv_token_to_id(svr, ctx, stack, require);
    }
    else {
        APP_CTX_ERROR(svr->m_app, "%s: login: unknown require %s!", account_svr_name(svr), require_name);
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
}
