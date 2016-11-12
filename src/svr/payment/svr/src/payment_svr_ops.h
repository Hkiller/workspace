#ifndef SVR_PAYMENT_SVR_OPS_H
#define SVR_PAYMENT_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "svr/center/agent/center_agent_types.h" 
#include "protocol/svr/payment/svr_payment_pro.h"
#include "payment_svr.h"

/*ops*/
logic_op_exec_result_t
payment_svr_op_get_balance_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
payment_svr_op_get_balance_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
payment_svr_op_pay_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
payment_svr_op_pay_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
payment_svr_op_recharge_begin_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
payment_svr_op_recharge_begin_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
payment_svr_op_recharge_commit_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
payment_svr_op_recharge_commit_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
payment_svr_op_notify_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
payment_svr_op_notify_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

/*recharge*/
logic_op_exec_result_t
payment_svr_op_recharge_db_update(
    logic_context_t ctx, logic_stack_node_t stack,
    payment_svr_t svr, SVR_PAYMENT_REQ_RECHARGE_BEGIN const * req, BAG_INFO * bag_info);


#endif
