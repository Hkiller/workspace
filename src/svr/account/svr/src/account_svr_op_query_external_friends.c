#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "account_svr_ops.h"
#include "account_svr_login_info.h"
#include "account_svr_backend.h"
#include "account_svr_db_ops.h"

logic_op_exec_result_t
account_svr_op_query_external_friends_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    account_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_ACCOUNT_REQ_QUERY_EXTERNAL_FRIENDS const * req;
    account_svr_login_info_t login_info;
    account_svr_backend_t backend;
    logic_data_t res_data;

    res_data = logic_context_data_dyn_reserve_for_append(ctx, svr->m_meta_res_query_external_friends, 100);
    if (res_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: query external friends: init reserve response fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    req_data = logic_context_data_find(ctx, "svr_account_req_query_external_friends");
    if (req_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: query external friends: get request fail!", account_svr_name(svr));
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

    backend = account_svr_backend_find(svr, login_info->m_data.logic_id.account_type);
    if (backend == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: query external friends: account type %d not support!", account_svr_name(svr), login_info->m_data.logic_id.account_type);
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_NOT_SUPPORT_ACCOUNT_TYPE);
        return logic_op_exec_result_false;
    }
    
    if (backend->m_query_friends == NULL) {
        APP_CTX_ERROR(
            svr->m_app, "%s: query external friends: account type %d not support query friends!",
            account_svr_name(svr), login_info->m_data.logic_id.account_type);
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    return account_svr_backend_check_send_query_friends_req(backend, stack, login_info);
}

static
logic_op_exec_result_t
account_svr_op_query_external_friends_on_recv_db_query_one(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    logic_data_t db_res;
    SVR_ACCOUNT_BASIC_LIST * account_list;
    SVR_ACCOUNT_LOGIC_ID_LIST * friend_list;
    logic_data_t res_data;
    SVR_ACCOUNT_RES_QUERY_EXTERNAL_FRIENDS * res;
    SVR_ACCOUNT_ROLE_INFO * r_record;
    uint32_t pos;
    
    /*检查数据库返回结果 */
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            APP_CTX_ERROR(
                svr->m_app, "%s: query external friends: db request error, errno=%d!",
                account_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                svr->m_app, "%s: query external friends: db request state error, state=%s!",
                account_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_DB);
            return logic_op_exec_result_false;
        }
    }

    /*获取数据库返回结果 */
    db_res = logic_require_data_find(require, dr_meta_name(svr->m_meta_record_basic_list));
    if (db_res == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: query external friends: find db result fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    account_list = logic_data_data(db_res);

    if (account_list->count < 1) return logic_op_exec_result_true;

    friend_list = logic_data_data(logic_stack_data_find(stack, dr_meta_name(svr->m_meta_logic_id_list)));
    assert(friend_list);

    pos = atoi(logic_require_name(require) + strlen("db_query_"));
    
    /*构造响应 */
    res_data = logic_context_data_dyn_reserve_for_append(ctx, svr->m_meta_res_query_external_friends, 1);
    if (res_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: query external friends: create response fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    r_record = res->friends + res->friend_count++;

    r_record->account_id = account_list->data[0]._id;
    r_record->has_external = 1;
    r_record->external.data.logic_id = friend_list->data[pos];
    
    return logic_op_exec_result_true;
}

static
logic_op_exec_result_t
account_svr_op_query_external_friends_on_recv_db_query_bulk(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    logic_data_t db_res;
    SVR_ACCOUNT_BASIC_LIST * account_list;
    logic_data_t res_data;
    SVR_ACCOUNT_RES_QUERY_EXTERNAL_FRIENDS * res;
    uint32_t i;
    
    /*检查数据库返回结果 */
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) == logic_require_state_error) {
            APP_CTX_ERROR(
                svr->m_app, "%s: query external friends: bulk: db request error, errno=%d!",
                account_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                svr->m_app, "%s: query external friends: bulk: db request state error, state=%s!",
                account_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_DB);
            return logic_op_exec_result_false;
        }
    }

    /*获取数据库返回结果 */
    db_res = logic_require_data_find(require, dr_meta_name(svr->m_meta_record_basic_list));
    if (db_res == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: query external friends: bulk: find db result fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    account_list = logic_data_data(db_res);

    /*构造响应 */
    res_data = logic_context_data_get_or_create(ctx, svr->m_meta_res_query_external_friends, account_list->count);
    if (res_data == NULL) {
        APP_CTX_ERROR(svr->m_app, "%s: query external friends: bulk: create response fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    res = logic_data_data(res_data);

    for(i = 0; i < account_list->count; ++i) {
        SVR_ACCOUNT_BASIC const * db_record = account_list->data + i;
        SVR_ACCOUNT_ROLE_INFO * r_record = res->friends + res->friend_count;

        r_record->account_id = db_record->_id;
        r_record->has_external = 0;

        res->friend_count++;
    }
    
    return logic_op_exec_result_true;
}

static
logic_op_exec_result_t
account_svr_op_query_external_friends_on_recv_friends(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg)
{
    account_svr_t svr = user_data;
    SVR_ACCOUNT_LOGIC_ID_LIST * friend_list;
    logic_data_t external_result;
    logic_require_t db_require;
    SVR_ACCOUNT_REQ_QUERY_EXTERNAL_FRIENDS const * req;
    
    if (logic_require_state(require) != logic_require_state_done) {
        APP_CTX_ERROR(svr->m_app, "%s: query external friends: extern query fail!", account_svr_name(svr));
        logic_context_errno_set(ctx, logic_require_error(require));
        return logic_op_exec_result_false;
    }

    external_result = logic_require_data_find(require, dr_meta_name(svr->m_meta_logic_id_list));
    assert(external_result);
    friend_list = logic_data_data(external_result);
    assert(friend_list);

    if (friend_list->count == 0) {
        return logic_op_exec_result_true;
    }
    
    req = logic_data_data(logic_context_data_find(ctx, "svr_account_req_query_external_friends"));
    assert(req);

    APP_CTX_ERROR(svr->m_app, "%s: query external friends: xxxxxx: friend count=%d!", account_svr_name(svr), friend_list->count);
    
    if (req->with_external) {
        uint32_t i;

        if (logic_stack_data_copy(stack, external_result) == NULL) {
            APP_CTX_ERROR(svr->m_app, "%s: query external friends: copy data fail!", account_svr_name(svr));
            return logic_op_exec_result_false;
        }
        
        for(i = 0; i < friend_list->count; ++i) {
            char query_name[64];
            snprintf(query_name, sizeof(query_name), "db_query_%d", i);
            
            db_require = logic_require_create(stack, query_name);
            if (db_require == NULL) {
                APP_CTX_ERROR(svr->m_app, "%s: query external friends: create db query fail!", account_svr_name(svr));
                return logic_op_exec_result_false;
            }

            if (account_svr_db_send_query_by_logic_id(
                    svr, db_require, &friend_list->data[i], svr->m_meta_record_basic_list)
                != 0)
            {
                APP_CTX_ERROR(svr->m_app, "%s: query external friends: build db query fail!", account_svr_name(svr));
                logic_require_free(db_require);
                return logic_op_exec_result_false;
            }
        }
    }
    else {
        db_require = logic_require_create(stack, "db_query_bulk");
        if (db_require == NULL) {
            APP_CTX_ERROR(svr->m_app, "%s: query external friends: create db query fail!", account_svr_name(svr));
            return logic_op_exec_result_false;
        }

        if (account_svr_db_send_bulk_query_by_logic_id(
                svr, db_require, friend_list->data, friend_list->count, svr->m_meta_record_basic_list)
            != 0)
        {
            APP_CTX_ERROR(svr->m_app, "%s: query external friends: build db query fail!", account_svr_name(svr));
            logic_require_free(db_require);
            return logic_op_exec_result_false;
        }

        APP_CTX_ERROR(svr->m_app, "%s: query external friends: send bulk query!", account_svr_name(svr));
    }
    
    return logic_op_exec_result_true;
}

logic_op_exec_result_t
account_svr_op_query_external_friends_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg) {
    account_svr_t svr = user_data;
    const char * require_name;

    require_name = logic_require_name(require);
    if (cpe_str_start_with(require_name, "query_friends")) { /*兼容分页requiest */
        return account_svr_op_query_external_friends_on_recv_friends(ctx, stack, require, user_data, cfg);
    }
    else if (strcmp(require_name, "db_query_bulk") == 0) {
        return account_svr_op_query_external_friends_on_recv_db_query_bulk(ctx, stack, require, user_data, cfg);
    }
    else if (cpe_str_start_with(require_name, "db_query_")) {
        return account_svr_op_query_external_friends_on_recv_db_query_one(ctx, stack, require, user_data, cfg);
    }
    else {
        APP_CTX_ERROR(svr->m_app, "%s: query external friends: unknown require %s!", account_svr_name(svr), require_name);
        logic_context_errno_set(ctx, SVR_ACCOUNT_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
}
