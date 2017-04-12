#include "gd/app/app_log.h"
#include "usf/logic/logic_context.h"
#include "gift_svr_ops.h"

logic_op_exec_result_t
gift_svr_op_check_state_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    gift_svr_t svr = user_data;
    if (svr->m_init_state != gift_svr_init_success) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: check state: svr not init, retry later!", gift_svr_name(svr));
        logic_context_errno_set(ctx, SVR_GIFT_ERRNO_INTERNAL);
        return logic_op_exec_result_false;
    }
    
    return logic_op_exec_result_true;
}

