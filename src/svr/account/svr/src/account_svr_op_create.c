#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "gd/utils/id_generator.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "usf/mongo_use/id_generator.h"
#include "account_svr_ops.h"
#include "account_svr_db_ops.h"
#include "account_svr_backend.h"
#include "account_svr_login_info.h"
#include "protocol/svr/account/svr_account_pro.h"
#include "protocol/svr/account/svr_account_internal.h"


static logic_op_exec_result_t account_svr_op_create_send_insert_db_req(
    account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, SVR_ACCOUNT_REQ_CREATE const * req);
static logic_op_exec_result_t account_svr_op_create_recv_token_to_id(
    account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require);
static logic_op_exec_result_t account_svr_op_create_recv_insert_db(
    account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require);
static SVR_ACCOUNT_REQ_CREATE * account_svr_op_create_req(
    account_svr_t svr, logic_context_t ctx);

logic_op_exec_result_t
account_svr_op_create_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    SVR_ACCOUNT_REQ_CREATE const * req;
    account_svr_backend_t backend;

    req = account_svr_op_create_req(svr, ctx);
    if (req == NULL) {
        return logic_op_exec_result_false;
    }

    backend = account_svr_backend_find(svr, req->logic_id.account_type);
    if (backend == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: create: account type %d not support!", account_svr_name(svr), req->logic_id.account_type);
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_NOT_SUPPORT_ACCOUNT_TYPE);
        return logic_op_exec_result_false;
    }

    if (backend->m_token_to_id) {
        return account_svr_backend_check_send_logic_to_id_req(backend, stack, &req->logic_id);
    }
    else {
        return account_svr_op_create_send_insert_db_req(svr, ctx, stack, req);
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
account_svr_op_create_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    const char * require_name;

    require_name = logic_require_name(require);
    if (strcmp(require_name, "db_insert") == 0) {
        return account_svr_op_create_recv_insert_db(svr, ctx, stack, require);
    }
    else if (strcmp(require_name, "token_to_id") == 0) {
        return account_svr_op_create_recv_token_to_id(svr, ctx, stack, require);
    }
    else {
        APP_CTX_ERROR(svr->m_app, "%s: create: unknown require %s!", account_svr_name(svr), require_name);
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
}

static SVR_ACCOUNT_REQ_CREATE * account_svr_op_create_req(account_svr_t svr, logic_context_t ctx) {
    logic_data_t req_data;

    req_data = logic_context_data_find(ctx, "svr_account_req_create");
    if (req_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: create: get request fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return NULL;
    }
    else {
        return logic_data_data(req_data);
    }
}

static logic_op_exec_result_t account_svr_op_create_send_insert_db_req(
    account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, SVR_ACCOUNT_REQ_CREATE const * req)
{
    logic_require_t require;
    logic_data_t res_data;
    SVR_ACCOUNT_RES_CREATE * res;

    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_create, 0);
    if (res_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: create: create result fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    res = logic_data_data(res_data);
    assert(res);

    if (gd_id_generator_generate(&res->account_id, (gd_id_generator_t)svr->m_id_generator, "account_id") != 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: create: gen account id fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    require = logic_require_create(stack, "db_insert");
    if (require == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: create: create logic require fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (account_svr_db_send_insert(svr, require, res->account_id, &req->logic_id, 0) != 0) {
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        logic_require_free(require);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
account_svr_op_create_recv_insert_db(account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require) {
    SVR_ACCOUNT_REQ_CREATE * req;
    SVR_ACCOUNT_RES_CREATE * res;

    /*检查数据库返回结果 */
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            APP_CTX_ERROR(
                svr->m_app, "%s: create: db request error, errno=%d!",
                account_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                svr->m_app, "%s: create: db request state error, state=%s!",
                account_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_DB);
            return logic_op_exec_result_false;
        }
    }

    req = account_svr_op_create_req(svr, ctx);
    assert(req);

    res = logic_data_data(logic_context_data_find(ctx, dr_meta_name(svr->m_meta_res_create)));
    assert(res);

    account_svr_login_info_update(svr, res->account_id, &req->logic_id, ctx);

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
account_svr_op_create_recv_token_to_id(account_svr_t svr, logic_context_t ctx, logic_stack_node_t stack, logic_require_t require) {
    SVR_ACCOUNT_REQ_CREATE * req;
    logic_data_t login_info_data;
    SVR_ACCOUNT_LOGIN_INFO * login_info;

    req = account_svr_op_create_req(svr, ctx);
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

    return account_svr_op_create_send_insert_db_req(svr, ctx, stack, req);
}
