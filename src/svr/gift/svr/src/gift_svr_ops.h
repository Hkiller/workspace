#ifndef SVR_GIFT_SVR_OPS_H
#define SVR_GIFT_SVR_OPS_H
#include "gift_svr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*common ops*/
logic_op_exec_result_t
gift_svr_op_check_state_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
    
/*ops*/
logic_op_exec_result_t
gift_svr_op_generate_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
gift_svr_op_generate_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
gift_svr_op_update_generate_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
gift_svr_op_update_generate_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
gift_svr_op_query_generate_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);

logic_op_exec_result_t
gift_svr_op_query_use_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
gift_svr_op_query_use_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);
    
logic_op_exec_result_t
gift_svr_op_use_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
gift_svr_op_use_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

/*internal*/
logic_op_exec_result_t
gift_svr_op_init_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
gift_svr_op_init_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
gift_svr_op_expire_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
gift_svr_op_expire_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

/*utils*/
uint8_t gift_svr_op_check_db_result(gift_svr_t svr, logic_context_t ctx, logic_require_t require);

#ifdef __cplusplus
}
#endif
    
#endif
