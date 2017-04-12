#ifndef USF_LOGIC_USE_OP_REGISTER_H
#define USF_LOGIC_USE_OP_REGISTER_H
#include "logic_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_op_register_t
logic_op_register_create(
    gd_app_context_t app, const char * group_name, mem_allocrator_t alloc, error_monitor_t em);
void logic_op_register_free(logic_op_register_t op_register);

int logic_op_register_create_op(
    logic_op_register_t op_register,
    const char * type_name,
    logic_op_fun_t send_fun,
    logic_op_recv_fun_t recv_fun,
    void * user_ctx,
    logic_op_ctx_fini_fun_t fini_fun);

int logic_op_register_create_ops(
    logic_op_register_t op_register,
    uint32_t op_def_count,
    logic_op_register_def_t op_defs,
    void * user_ctx);

#ifdef __cplusplus
}
#endif

#endif
