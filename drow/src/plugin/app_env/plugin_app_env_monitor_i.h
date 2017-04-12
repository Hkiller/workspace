#ifndef PLUGIN_APP_ENV_MONITOR_I_H
#define PLUGIN_APP_ENV_MONITOR_I_H
#include "plugin_app_env_module_i.h"
#include "plugin/app_env/plugin_app_env_monitor.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_app_env_monitor {
    plugin_app_env_module_t m_module;
    struct cpe_hash_entry m_hh;
    const char * m_meta_name;
    void * m_ctx;
    plugin_app_env_monitor_exec_fun_t m_fun;
    plugin_app_env_monitor_clear_fun_t m_clear_fun;
};

void plugin_app_env_monitor_free_all(plugin_app_env_module_t module);

uint32_t plugin_app_env_monitor_hash(plugin_app_env_monitor_t monitor);
int plugin_app_env_monitor_eq(plugin_app_env_monitor_t l, plugin_app_env_monitor_t r);
    
#ifdef __cplusplus
}
#endif

#endif
