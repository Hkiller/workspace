#ifndef DROW_PLUGIN_MOVING_MODULE_H
#define DROW_PLUGIN_MOVING_MODULE_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/dr/dr_types.h"
#include "plugin_moving_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_moving_module_t
plugin_moving_module_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_moving_module_free(plugin_moving_module_t mgr);

plugin_moving_module_t plugin_moving_module_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_moving_module_t plugin_moving_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_moving_module_app(plugin_moving_module_t mgr);
const char * plugin_moving_module_name(plugin_moving_module_t mgr);

ui_data_mgr_t plugin_moving_module_data_mgr(plugin_moving_module_t mgr);
    
LPDRMETA plugin_moving_module_moving_plan_meta(plugin_moving_module_t mgr);
LPDRMETA plugin_moving_module_moving_plan_track_meta(plugin_moving_module_t mgr);
LPDRMETA plugin_moving_module_moving_plan_point_meta(plugin_moving_module_t mgr);
LPDRMETA plugin_moving_module_moving_plan_node_meta(plugin_moving_module_t mgr);
LPDRMETA plugin_moving_module_moving_plan_segment_meta(plugin_moving_module_t mgr);

void plugin_moving_module_install_bin_loader(plugin_moving_module_t mgr);
void plugin_moving_module_install_bin_saver(plugin_moving_module_t mgr);

#ifdef __cplusplus
}
#endif

#endif 

