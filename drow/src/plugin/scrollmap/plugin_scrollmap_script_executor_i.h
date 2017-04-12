#ifndef PLUGIN_SCROLLMAP_SCRIPT_EXECUTOR_I_H
#define PLUGIN_SCROLLMAP_SCRIPT_EXECUTOR_I_H
#include "plugin/scrollmap/plugin_scrollmap_script_executor.h"
#include "plugin_scrollmap_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_script_executor {
    plugin_scrollmap_env_t m_env;
    cpe_hash_entry m_hh;
    const char * m_type;
    plugin_scrollmap_script_executor_fun_t m_exec_fun;
};

void plugin_scrollmap_script_executor_free_all(plugin_scrollmap_env_t env);

uint32_t plugin_scrollmap_script_executor_hash(plugin_scrollmap_script_executor_t script_executor);
int plugin_scrollmap_script_executor_eq(plugin_scrollmap_script_executor_t l, plugin_scrollmap_script_executor_t r);

#ifdef __cplusplus
}
#endif

#endif
