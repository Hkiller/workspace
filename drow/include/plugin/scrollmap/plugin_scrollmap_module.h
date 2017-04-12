#ifndef PLUGIN_SCROLLMAP_MODULE_H
#define PLUGIN_SCROLLMAP_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "render/runtime/ui_runtime_types.h"
#include "plugin/moving/plugin_moving_types.h"
#include "plugin_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_scrollmap_module_t
plugin_scrollmap_module_create(
    gd_app_context_t app,
    ui_data_mgr_t data_mgr,
    ui_runtime_module_t runtime,
    plugin_moving_module_t moving_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_scrollmap_module_free(plugin_scrollmap_module_t repo);

plugin_scrollmap_module_t plugin_scrollmap_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_scrollmap_module_t plugin_scrollmap_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_scrollmap_module_app(plugin_scrollmap_module_t module);
const char * plugin_scrollmap_module_name(plugin_scrollmap_module_t module);

LPDRMETA plugin_scrollmap_module_tile_meta(plugin_scrollmap_module_t module);
LPDRMETA plugin_scrollmap_module_layer_meta(plugin_scrollmap_module_t module);
LPDRMETA plugin_scrollmap_module_block_meta(plugin_scrollmap_module_t module);
LPDRMETA plugin_scrollmap_module_script_meta(plugin_scrollmap_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
