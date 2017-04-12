#ifndef PLUGIN_APP_ENV_EXECUTOR_I_H
#define PLUGIN_APP_ENV_EXECUTOR_I_H
#include "plugin_app_env_module_i.h"
#include "plugin/app_env/plugin_app_env_executor.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_app_env_executor {
    plugin_app_env_module_t m_module;
    plugin_app_env_executor_type_t m_type;
    struct cpe_hash_entry m_hh;
    LPDRMETA m_apply_to;
    void * m_ctx;
    union {
        plugin_app_env_executor_oneway_exec_fun_t m_exec_oneway;
        plugin_app_env_executor_sync_exec_fun_t m_exec_sync;
        plugin_app_env_executor_async_exec_fun_t m_exec_async;
    };
};

void plugin_app_env_executor_free_all(plugin_app_env_module_t module);

uint32_t plugin_app_env_executor_hash(plugin_app_env_executor_t executor);
int plugin_app_env_executor_eq(plugin_app_env_executor_t l, plugin_app_env_executor_t r);
    
#ifdef __cplusplus
}
#endif

#endif
