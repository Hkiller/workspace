#ifndef USF_LOGIC_OP_EXECUTOR_TYPE_H
#define USF_LOGIC_OP_EXECUTOR_TYPE_H
#include "cpe/utils/stream.h"
#include "cpe/utils/hash_string.h"
#include "logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*logic_executor_type_group operations*/
logic_executor_type_group_t
logic_executor_type_group_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc);

void logic_executor_type_group_free(logic_executor_type_group_t group);

logic_executor_type_group_t
logic_executor_type_group_find(gd_app_context_t app, cpe_hash_string_t name);
logic_executor_type_group_t
logic_executor_type_group_find_nc(gd_app_context_t app, const char * name);

logic_executor_type_group_t
logic_executor_type_group_default(
    gd_app_context_t app);

gd_app_context_t logic_executor_type_group_app(logic_executor_type_group_t group);
const char * logic_executor_type_group_name(logic_executor_type_group_t group);
cpe_hash_string_t logic_executor_type_group_name_hs(logic_executor_type_group_t group);

void logic_executor_type_group_types(logic_executor_type_it_t it, logic_executor_type_group_t group);

/*logic_executor_type operations*/
logic_executor_type_t
logic_executor_type_create(logic_executor_type_group_t group, const char * name);
void logic_executor_type_free(logic_executor_type_t t);

int logic_executor_type_bind(
    logic_executor_type_t type, logic_op_fun_t fun, void * ctx, logic_op_ctx_fini_fun_t ctx_fini);

const char * logic_executor_type_name(logic_executor_type_t type);
void * logic_executor_type_ctx(logic_executor_type_t type);

logic_executor_type_t
logic_executor_type_find(logic_executor_type_group_t group, const char * name);

#define logic_executor_type_next(it) ((it)->next ? (it)->next(it) : NULL)

logic_executor_type_t
logic_executor_type_create_global(
    gd_app_context_t app, const char * group_name, const char * name,
    logic_op_fun_t op_fun,
    void * op_ctx,
    logic_op_ctx_fini_fun_t ctx_fini,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif

