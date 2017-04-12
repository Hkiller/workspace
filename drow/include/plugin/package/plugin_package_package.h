#ifndef UI_PLUGIN_PACKAGE_PACKAGE_H
#define UI_PLUGIN_PACKAGE_PACKAGE_H
#include "plugin_package_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_package_package_t plugin_package_package_create(plugin_package_module_t module, const char * name, plugin_package_package_state_t state);
void plugin_package_package_free(plugin_package_package_t package);

plugin_package_package_t plugin_package_package_find(plugin_package_module_t module, const char * name);

plugin_package_module_t plugin_package_package_module(plugin_package_package_t package);
plugin_package_package_using_state_t plugin_package_package_using_state(plugin_package_package_t package);
    
const char * plugin_package_package_name(plugin_package_package_t package);
    
plugin_package_package_state_t plugin_package_package_state(plugin_package_package_t package);
const char * plugin_package_package_state_str(plugin_package_package_t package);
    
const char * plugin_package_package_path(plugin_package_package_t package);
int plugin_package_package_set_path(plugin_package_package_t package, const char * path);

int plugin_package_package_install(plugin_package_package_t package);
int plugin_package_package_uninstall(plugin_package_package_t package);

int plugin_package_package_load_async(plugin_package_package_t package, plugin_package_load_task_t task);
int plugin_package_package_load_async_r(plugin_package_package_t package, plugin_package_load_task_t task);
int plugin_package_package_load_sync(plugin_package_package_t package);
int plugin_package_package_load_sync_r(plugin_package_package_t package);    
int plugin_package_package_unload(plugin_package_package_t package);

uint32_t plugin_package_package_total_size(plugin_package_package_t package);
void plugin_package_package_set_total_size(plugin_package_package_t package, uint32_t size);

float plugin_package_package_progress(plugin_package_package_t package);
int plugin_package_package_set_progress(plugin_package_package_t package, float progress);

uint8_t plugin_package_package_is_installed(plugin_package_package_t package);

ui_cache_group_t plugin_package_package_resources(plugin_package_package_t package);
ui_data_src_group_t plugin_package_package_srcs(plugin_package_package_t package);

uint8_t plugin_package_package_is_in_group(plugin_package_package_t package, plugin_package_group_t group);

int plugin_package_package_add_base_package(plugin_package_package_t package, plugin_package_package_t base_package);
uint8_t plugin_package_package_has_base_package(plugin_package_package_t package, plugin_package_package_t base_package);
    
void plugin_package_package_extern_packages(plugin_package_package_t package, plugin_package_package_it_t extern_packages);
void plugin_package_package_base_packages(plugin_package_package_t package, plugin_package_package_it_t base_packages);
    
const char * plugin_package_package_state_to_str(plugin_package_package_state_t state);

const char * plugin_package_package_dump_using(plugin_package_package_t package, mem_buffer_t buffer);
    
/*it*/
struct plugin_package_package_it {
    plugin_package_package_t (*next)(plugin_package_package_it_t it);
    char m_data[64];
};

#define plugin_package_package_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

