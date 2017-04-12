#ifndef DROW_PLUGIN_UI_MODULE_H
#define DROW_PLUGIN_UI_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "plugin/package/plugin_package_types.h"
#include "plugin/editor/plugin_editor_types.h"
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_module_t
plugin_ui_module_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr, plugin_package_module_t package_module,
    ui_runtime_module_t runtime, plugin_editor_module_t editor_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_ui_module_free(plugin_ui_module_t mgr);

plugin_ui_module_t plugin_ui_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_ui_module_t plugin_ui_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_ui_module_app(plugin_ui_module_t mgr);
const char * plugin_ui_module_name(plugin_ui_module_t mgr);

float plugin_ui_module_cfg_fps(plugin_ui_module_t mgr);
    
ui_data_mgr_t plugin_ui_module_data_mgr(plugin_ui_module_t mgr);
plugin_editor_module_t plugin_ui_module_editor(plugin_ui_module_t mgr);

uint8_t plugin_ui_module_app_pause(plugin_ui_module_t mgr);
void plugin_ui_module_set_app_pause(plugin_ui_module_t mgr, uint8_t is_pause);

uint32_t plugin_ui_module_control_base_size(plugin_ui_module_t mgr);
uint32_t plugin_ui_module_animation_base_size(plugin_ui_module_t mgr);

#ifdef __cplusplus
}
#endif

#endif 

