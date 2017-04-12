#ifndef DROW_PLUGIN_TILEDMAP_DATA_SCENE_H
#define DROW_PLUGIN_TILEDMAP_DATA_SCENE_H
#include "protocol/plugin/tiledmap/tiledmap_info.h"
#include "plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*scene */
plugin_tiledmap_data_scene_t plugin_tiledmap_data_scene_create(plugin_tiledmap_module_t module, ui_data_src_t src);
void plugin_tiledmap_data_scene_free(plugin_tiledmap_data_scene_t sprite);

plugin_tiledmap_module_t plugin_tiledmap_data_scene_module(plugin_tiledmap_data_scene_t scene);
TILEDMAP_SCENE * plugin_tiledmap_data_scene_data(plugin_tiledmap_data_scene_t scene);

uint32_t plugin_tiledmap_data_scene_layer_count(plugin_tiledmap_data_scene_t scene);
void plugin_tiledmap_data_scene_layers(plugin_tiledmap_data_layer_it_t layer_it, plugin_tiledmap_data_scene_t scene);

void plugin_tiledmap_data_scene_use_srcs(ui_data_src_it_t it, plugin_tiledmap_data_scene_t scene);
void plugin_tiledmap_data_scene_use_refs(ui_data_src_ref_it_t it, plugin_tiledmap_data_scene_t scene);

void plugin_tiledmap_data_scene_config_rect(plugin_tiledmap_data_scene_t scene, ui_rect_t rect);
int plugin_tiledmap_data_scene_rect(plugin_tiledmap_data_scene_t scene, ui_rect_t rect);

#ifdef __cplusplus
}
#endif

#endif
