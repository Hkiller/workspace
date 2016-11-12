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
friend_svr_op_ack_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    friend_svr_t svr = user_data;
    logic_data_t req_data;
    SVR_FRIEND_REQ_ACK const * req;

    if (svr->m_runing_mode != friend_svr_runing_mode_ack) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: ack: svr runing-mode is not ack!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }

    req_data = logic_context_data_find(ctx, "svr_friend_req_ack");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: ack: read record: find req fail!", friend_svr_name(svr));
        logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    if (!req->accept) {/*拒绝好友请求 */
        if (friend_svr_db_send_remove(svr, NULL, NULL, req->user_id, req->friend_id) != 0) {
            APP_CTX_ERROR(logic_context_app(ctx), "%s: ack: send remove req fail!", friend_svr_name(svr));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        if (friend_svr_db_send_remove(svr, NULL, NULL,req->friend_id , req->user_id) != 0) {
            APP_CTX_ERROR(logic_context_app(ctx), "%s: ack: send remove req fail!", friend_svr_name(svr));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }
        return logic_op_exec_result_true;
    }
    else {/*同意好友请求 */
        if (friend_svr_db_send_update_state(svr, NULL, NULL, req->user_id, req->friend_id, SVR_FRIEND_STATE_REQ_RECV, SVR_FRIEND_STATE_OK) != 0) {
            APP_CTX_ERROR(logic_context_app(ctx), "%s: ack: send update self state req fail!", friend_svr_name(svr));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        if (friend_svr_db_send_update_state(svr, NULL, NULL, req->friend_id, req->user_id, SVR_FRIEND_STATE_REQ_SEND, SVR_FRIEND_STATE_OK) != 0) {
            APP_CTX_ERROR(logic_context_app(ctx), "%s: ack: send update other state req fail!", friend_svr_name(svr));
            logic_context_errno_set(ctx, SVR_FRIEND_ERRNO_INTERNAL);
            return logic_op_exec_result_false;
        }

        return logic_op_exec_result_true;
    }
}

logic_op_exec_result_t
friend_svr_op_ack_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    return logic_op_exec_result_true;
}
