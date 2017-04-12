#include <assert.h>
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_context.h"
#include "rank_g_svr_ops.h"

logic_op_exec_result_t
rank_g_svr_op_init_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;

    if (rank_g_svr_load_init_records(svr) != 0) {
        APP_CTX_ERROR(logic_context_app(ctx), "%s: init: load init records fail!", rank_g_svr_name(svr));
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }
    
    return logic_op_exec_result_true;
}
