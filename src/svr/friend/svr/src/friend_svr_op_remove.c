#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "friend_svr_ops.h"
#include "protocol/svr/friend/svr_friend_pro.h"
#include "protocol/svr/friend/svr_friend_internal.h"

logic_op_exec_result_t
friend_svr_op_remove_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    friend_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_FRIEND_REQ_REMOVE const * req;

    req_data = logic_context_data_find(ctx, "svr_friend_req_remove");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: remove: get request fail!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    if (friend_svr_db_send_remove(svr, stack, "remove", req->user_id, req->friend_id) != 0) {
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    if (friend_svr_db_send_remove(svr, stack, "remove", req->friend_id, req->user_id) != 0) {
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
friend_svr_op_remove_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    friend_svr_t svr = user_data;

    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) != logic_require_state_error) {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: remove: db request error, errno=%d!",
                friend_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: remove: db request state error, state=%s!",
                friend_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_DB);
            return logic_op_exec_result_false;
        }
    }

    return logic_op_exec_result_true;
}
