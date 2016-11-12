#ifndef USF_LOGIC_USE_OP_ASNYC_H
#define USF_LOGIC_USE_OP_ASNYC_H
#include "logic_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_op_exec_result_t
logic_op_asnyc_exec(
    logic_context_t context,
    logic_stack_node_t stack_node,
    logic_op_fun_t send_fun,
    logic_op_recv_fun_t recv_fun,
    void * user_data,
    cfg_t args);

logic_executor_type_t
logic_op_async_type_create(
    gd_app_context_t app,
    const char * group_name,
    const char * type_name,
    logic_op_fun_t send_fun,
    logic_op_recv_fun_t recv_fun,
    void * user_ctx,
    logic_op_ctx_fini_fun_t fini_fun,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
