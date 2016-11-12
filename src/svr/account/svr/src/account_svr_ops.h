#ifndef SVR_ACCOUNT_SVR_OPS_H
#define SVR_ACCOUNT_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "protocol/svr/account/svr_account_pro.h"
#include "protocol/svr/account/svr_account_internal.h"
#include "account_svr_module.h"

/*service ops*/
logic_op_exec_result_t
account_svr_op_create_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_create_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
account_svr_op_login_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_login_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
account_svr_op_bind_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_bind_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
account_svr_op_unbind_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_unbind_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
account_svr_op_query_by_logic_id_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_query_by_logic_id_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
account_svr_op_query_by_account_id_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
account_svr_op_query_by_account_id_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
account_svr_op_query_login_info(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);

#endif
