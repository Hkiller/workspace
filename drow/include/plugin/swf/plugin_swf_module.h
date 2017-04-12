#ifndef PLUGIN_SWF_MODULE_H
#define PLUGIN_SWF_MODULE_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "plugin_swf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_swf_module_t
plugin_swf_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    uint8_t debug, const char * name, error_monitor_t em);

void plugin_swf_module_free(plugin_swf_module_t module);

gd_app_context_t plugin_swf_module_app(plugin_swf_module_t module);
const char * plugin_swf_module_name(plugin_swf_module_t module);

plugin_swf_module_t plugin_swf_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_swf_module_t plugin_swf_module_find_nc(gd_app_context_t app, const char * name);

ui_data_mgr_t plugin_swf_module_data_mgr(plugin_swf_module_t module);
ui_runtime_module_t plugin_swf_module_runtime(plugin_swf_module_t module);

void plugin_swf_module_install_bin_loader(plugin_swf_module_t module);
void plugin_swf_module_install_bin_saver(plugin_swf_module_t module);

#ifdef __cplusplus
}
#endif

#endif
