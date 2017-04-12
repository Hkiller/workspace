#ifndef PLUGIN_APP_ENV_MONITOR_H
#define PLUGIN_APP_ENV_MONITOR_H
#include "plugin_app_env_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_app_env_monitor_exec_fun_t)(
    void * ctx,
    LPDRMETA req_meta, void const * req_data, size_t req_size);

typedef void (*plugin_app_env_monitor_clear_fun_t)(void * ctx);
    
plugin_app_env_monitor_t
plugin_app_env_monitor_create(
    plugin_app_env_module_t module, const char * meta_name,
    void * ctx, plugin_app_env_monitor_exec_fun_t exec_fun, plugin_app_env_monitor_clear_fun_t clear_fun);
    
void plugin_app_env_monitor_free(plugin_app_env_monitor_t monitor);

void plugin_app_env_monitor_free_by_ctx(plugin_app_env_module_t module, void * ctx);

#ifdef __cplusplus
}
#endif

#endif
