#ifndef PLUGIN_TILEDMAP_DATA_SCENE_I_H
#define PLUGIN_TILEDMAP_DATA_SCENE_I_H
#include "cpe/utils/hash.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin_tiledmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_data_scene {
    plugin_tiledmap_module_t m_module;
    ui_data_src_t m_src;
    TILEDMAP_SCENE m_data;
    uint32_t m_layer_count;
    plugin_tiledmap_data_layer_list_t m_layer_list;
};

int plugin_scrollmap_data_scene_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_scrollmap_data_scene_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_scrollmap_data_scene_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_tiledmap_data_scene_update_usings(ui_data_src_t src);
    
#ifdef __cplusplus
}
#endif

#endif
