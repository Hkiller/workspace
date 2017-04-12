#ifndef PLUGIN_SPINE_MODULE_H
#define PLUGIN_SPINE_MODULE_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_spine_module_t
plugin_spine_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    const char * name, error_monitor_t em);

void plugin_spine_module_free(plugin_spine_module_t module);

gd_app_context_t plugin_spine_module_app(plugin_spine_module_t module);
const char * plugin_spine_module_name(plugin_spine_module_t module);

plugin_spine_module_t plugin_spine_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_spine_module_t plugin_spine_module_find_nc(gd_app_context_t app, const char * name);

ui_data_mgr_t plugin_spine_module_data_mgr(plugin_spine_module_t module);
ui_runtime_module_t plugin_spine_module_runtime(plugin_spine_module_t module);

void plugin_spine_module_install_bin_loader(plugin_spine_module_t module);
void plugin_spine_module_install_bin_saver(plugin_spine_module_t module);

LPDRMETA plugin_spine_module_meta_data_part(plugin_spine_module_t module);
LPDRMETA plugin_spine_module_meta_data_part_state(plugin_spine_module_t module);
LPDRMETA plugin_spine_module_meta_data_part_transition(plugin_spine_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
