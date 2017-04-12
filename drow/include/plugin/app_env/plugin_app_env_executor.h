#ifndef PLUGIN_APP_ENV_EXECUTOR_H
#define PLUGIN_APP_ENV_EXECUTOR_H
#include "plugin_app_env_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*plugin_app_env_executor_oneway_exec_fun_t)(
    void * ctx,
    LPDRMETA req_meta, void const * req_data, size_t req_size);
    
typedef int (*plugin_app_env_executor_sync_exec_fun_t)(
    void * ctx,
    LPDRMETA req_meta, void const * req_data, size_t req_size,
    dr_data_t * result, mem_buffer_t result_alloc);

typedef int (*plugin_app_env_executor_async_exec_fun_t)(
    void * ctx,
    LPDRMETA req_meta, void const * req_data, size_t req_size,
    uint32_t request_id);

plugin_app_env_executor_t plugin_app_env_executor_create_oneway(
    plugin_app_env_module_t module, LPDRMETA apply_to, void * ctx, plugin_app_env_executor_oneway_exec_fun_t fun);
plugin_app_env_executor_t plugin_app_env_executor_create_sync(
    plugin_app_env_module_t module, LPDRMETA apply_to, void * ctx, plugin_app_env_executor_sync_exec_fun_t fun);
plugin_app_env_executor_t plugin_app_env_executor_create_async(
    plugin_app_env_module_t module, LPDRMETA apply_to, void * ctx, plugin_app_env_executor_async_exec_fun_t fun);
void plugin_app_env_executor_free(plugin_app_env_executor_t executor);

void plugin_app_env_executor_free_by_ctx(plugin_app_env_module_t module, void * ctx);

plugin_app_env_executor_t plugin_app_env_executor_find(plugin_app_env_module_t module, LPDRMETA req_meta);

plugin_app_env_executor_type_t plugin_app_env_executor_type(plugin_app_env_executor_t executor);

struct plugin_app_env_executor_def {
    const char * meta;
    plugin_app_env_executor_type_t type;
    union {
        plugin_app_env_executor_oneway_exec_fun_t oneway;
        plugin_app_env_executor_sync_exec_fun_t sync;
        plugin_app_env_executor_async_exec_fun_t async;
    } fun;
};

int plugin_app_env_executor_bulck_create(
    plugin_app_env_module_t module, LPDRMETALIB metalib, void * ctx, plugin_app_env_executor_def_t defs, uint8_t def_count);

#ifdef __cplusplus
}
#endif

#endif
