#ifndef PLUGIN_SCROLLMAP_SCRIPT_EXECUTOR_H
#define PLUGIN_SCROLLMAP_SCRIPT_EXECUTOR_H
#include "plugin_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*plugin_scrollmap_script_executor_fun_t)(
    plugin_scrollmap_script_executor_t executor, plugin_scrollmap_script_t script);
    
plugin_scrollmap_script_executor_t
plugin_scrollmap_script_executor_create(
    plugin_scrollmap_env_t env, const char * type, uint32_t capacity, plugin_scrollmap_script_executor_fun_t fun);
    
plugin_scrollmap_script_executor_t
plugin_scrollmap_script_executor_find(
    plugin_scrollmap_env_t env, const char * type);

void * plugin_scrollmap_script_executor_data(plugin_scrollmap_script_executor_t executor);
    
void plugin_scrollmap_script_executor_free(plugin_scrollmap_script_executor_t executor);

#ifdef __cplusplus
}
#endif

#endif
