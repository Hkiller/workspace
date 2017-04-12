#ifndef DROW_PLUGIN_LAYOUT_MODULE_H
#define DROW_PLUGIN_LAYOUT_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_module_t
plugin_layout_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    const char * name, error_monitor_t em);
    
void plugin_layout_module_free(plugin_layout_module_t module);

plugin_layout_module_t plugin_layout_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_layout_module_t plugin_layout_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_layout_module_app(plugin_layout_module_t module);
const char * plugin_layout_module_name(plugin_layout_module_t module);

plugin_layout_font_id_t plugin_layout_module_default_font_id(plugin_layout_module_t module);
void plugin_layout_module_set_default_font_id(plugin_layout_module_t module, plugin_layout_font_id_t default_font_id);

uint32_t plugin_layout_module_node_count(plugin_layout_module_t module);
uint32_t plugin_layout_module_free_node_count(plugin_layout_module_t module);

#ifdef __cplusplus
}
#endif

#endif 
