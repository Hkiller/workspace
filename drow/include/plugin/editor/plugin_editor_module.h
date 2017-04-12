#ifndef DROW_PLUGIN_EDITOR_MODULE_H
#define DROW_PLUGIN_EDITOR_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "plugin_editor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_editor_module_t
plugin_editor_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    const char * name, error_monitor_t em);
    
void plugin_editor_module_free(plugin_editor_module_t module);

plugin_editor_module_t plugin_editor_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_editor_module_t plugin_editor_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_editor_module_app(plugin_editor_module_t module);
const char * plugin_editor_module_name(plugin_editor_module_t module);

plugin_editor_editing_t plugin_editor_module_active_editing(plugin_editor_module_t module);
void plugin_editor_module_set_active_editing(plugin_editor_module_t module, plugin_editor_editing_t editing);

int plugin_editor_module_copy_to_clipboard(plugin_editor_module_t module, plugin_layout_render_t render);
int plugin_editor_module_past_from_clipboard(plugin_editor_module_t module, plugin_layout_render_t render);
                                            
#ifdef __cplusplus
}
#endif

#endif 
