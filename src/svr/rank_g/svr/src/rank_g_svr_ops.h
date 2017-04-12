#ifndef SVR_RANK_G_SVR_OPS_H
#define SVR_RANK_G_SVR_OPS_H
#include "rank_g_svr.h"
#include "protocol/svr/rank_g/svr_rank_g_pro.h"

#ifdef __cplusplus
extern "C" {
#endif

/*rank_g request ops*/
logic_op_exec_result_t
rank_g_svr_op_update_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
rank_g_svr_op_update_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
rank_g_svr_op_remove_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
rank_g_svr_op_remove_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
rank_g_svr_op_query_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
rank_g_svr_op_query_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
rank_g_svr_op_query_with_data_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
rank_g_svr_op_query_with_data_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);

logic_op_exec_result_t
rank_g_svr_op_query_season_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);

logic_op_exec_result_t
rank_g_svr_op_query_data_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
    
logic_op_exec_result_t
rank_g_svr_op_dump_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);

logic_op_exec_result_t
rank_g_svr_op_init_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
    
/*internal*/
logic_op_exec_result_t
rank_g_svr_op_change_season_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg);
logic_op_exec_result_t
rank_g_svr_op_change_season_recv(logic_context_t ctx, logic_stack_node_t stack, logic_require_t require, void * user_data, cfg_t cfg);
logic_context_t rank_g_svr_op_change_season_start(rank_g_svr_t svr, rank_g_svr_index_t index);
    
#ifdef __cplusplus
}
#endif

#endif
