#include <assert.h>
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "mail_svr_ops.h"
#include "protocol/svr/mail/svr_mail_pro.h"
#include "protocol/svr/mail/svr_mail_internal.h"

logic_op_exec_result_t
mail_svr_op_remove_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    mail_svr_t svr = user_data;
    logic_require_t require;
    logic_data_t req_data;
    SVR_MAIL_REQ_REMOVE_MAIL const * req;

    req_data = logic_context_data_find(ctx, "svr_mail_req_remove_mail");
    if (req_data == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: get request fail!", mail_svr_name(svr));
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    req = logic_data_data(req_data);

    require = logic_require_create(stack, "remove_mail");
    if (require == NULL) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: add: create logic require fail!", mail_svr_name(svr));
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (mail_svr_db_send_remove(svr, require, req->mail_id, req->receiver_gid) != 0) {
        logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}

logic_op_exec_result_t
mail_svr_op_remove_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    mail_svr_t svr = user_data;

    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_state(require) != logic_require_state_error) {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: remove: db request error, errno=%d!",
                mail_svr_name(svr), logic_require_error(require));
            logic_context_errno_set(ctx, SVR_MAIL_ERROR_INTERNAL);
            return logic_op_exec_result_false;
        }
        else {
            APP_CTX_ERROR(
                logic_context_app(ctx), "%s: remove: db request state error, state=%s!",
                mail_svr_name(svr), logic_require_state_name(logic_require_state(require)));
            logic_context_errno_set(ctx, SVR_MAIL_ERROR_DB);
            return logic_op_exec_result_false;
        }
    }

    return logic_op_exec_result_true;
}
