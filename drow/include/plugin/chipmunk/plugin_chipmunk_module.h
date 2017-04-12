#ifndef DROW_PLUGIN_CHIPMUNK_MODULE_H
#define DROW_PLUGIN_CHIPMUNK_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_module_t
plugin_chipmunk_module_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr, ui_runtime_module_t runtime,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_chipmunk_module_free(plugin_chipmunk_module_t mgr);

plugin_chipmunk_module_t plugin_chipmunk_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_chipmunk_module_t plugin_chipmunk_module_find_nc(gd_app_context_t app, const char * name);

ui_data_mgr_t plugin_chipmunk_module_data_mgr(plugin_chipmunk_module_t mgr);

gd_app_context_t plugin_chipmunk_module_app(plugin_chipmunk_module_t mgr);
const char * plugin_chipmunk_module_name(plugin_chipmunk_module_t mgr);

LPDRMETA plugin_chipmunk_module_data_scene_meta(plugin_chipmunk_module_t mgr);
LPDRMETA plugin_chipmunk_module_data_body_meta(plugin_chipmunk_module_t mgr);
LPDRMETA plugin_chipmunk_module_data_fixture_meta(plugin_chipmunk_module_t mgr);
LPDRMETA plugin_chipmunk_module_data_polygon_node_meta(plugin_chipmunk_module_t mgr);
LPDRMETA plugin_chipmunk_module_data_constraint_meta(plugin_chipmunk_module_t mgr);

#ifdef __cplusplus
}
#endif

#endif 

