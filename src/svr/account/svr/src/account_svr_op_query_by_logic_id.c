#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_context.h"
#include "svr/set/logic/set_logic_rsp_carry_info.h"
#include "account_svr_ops.h"
#include "protocol/svr/account/svr_account_pro.h"
#include "protocol/svr/account/svr_account_internal.h"

logic_op_exec_result_t
account_svr_op_query_by_logic_id_send(
    logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg)
{
    return logic_op_exec_result_true;
}

logic_op_exec_result_t
account_svr_op_query_by_logic_id_recv(
    logic_context_t ctx, logic_stack_node_t stack, logic_require_t require,
    void * user_data, cfg_t cfg)
{
    return logic_op_exec_result_true;
}
