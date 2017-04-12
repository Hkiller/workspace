#ifndef DROW_PLUGIN_TILEDMAP_MANIP_BASIC_H
#define DROW_PLUGIN_TILEDMAP_MANIP_BASIC_H
#include "render/model_ed/ui_ed_types.h"
#include "render/cache/ui_cache_types.h"
#include "protocol/plugin/tiledmap/tiledmap_common.h"
#include "plugin/tiledmap/plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int plugin_tiledmap_scene_import(
    ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr, ui_cache_manager_t cache_mgr,
    const char * proj_path, error_monitor_t em);

int plugin_tiledmap_scene_export(
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr,
    const char * proj_path, error_monitor_t em);

int plugin_tiledmap_layers_to_pic(
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr,
    const char * scene_path, uint8_t layer_count, const char * * layers, const char * output, error_monitor_t em);

int plugin_tiledmap_layer_to_pixel_buf(
    ui_cache_pixel_buf_t pixel_buf, plugin_tiledmap_data_layer_t layer, TILEDMAP_PAIR const * pos_adj, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
