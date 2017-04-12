#ifndef PLUGIN_SCROLLMAP_DATA_SCENE_I_H
#define PLUGIN_SCROLLMAP_DATA_SCENE_I_H
#include "plugin/scrollmap/plugin_scrollmap_data_scene.h"
#include "plugin_scrollmap_module_i.h"
#include "protocol/plugin/scrollmap/scrollmap_file.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_data_scene {
    plugin_scrollmap_module_t m_module;
    ui_data_src_t m_src;
    uint32_t m_tile_count;
    plugin_scrollmap_data_tile_list_t m_tiles;
    uint32_t m_layer_count;
    plugin_scrollmap_data_layer_list_t m_layers;
};

int plugin_scrollmap_data_scene_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_scrollmap_data_scene_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_scrollmap_data_scene_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

int plugin_scrollmap_data_scene_regist(plugin_scrollmap_module_t module);
void plugin_scrollmap_data_scene_unregist(plugin_scrollmap_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
