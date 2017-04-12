#ifndef DROW_PLUGIN_TILEDMAP_DATA_LAYER_H
#define DROW_PLUGIN_TILEDMAP_DATA_LAYER_H
#include "protocol/plugin/tiledmap/tiledmap_info.h"
#include "plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_data_layer_it {
    plugin_tiledmap_data_layer_t (*next)(struct plugin_tiledmap_data_layer_it * it);
    char m_data[64];
};

/*layer*/
plugin_tiledmap_data_layer_t plugin_tiledmap_data_layer_create(plugin_tiledmap_data_scene_t scene);
void plugin_tiledmap_data_layer_free(plugin_tiledmap_data_layer_t layer);

plugin_tiledmap_data_scene_t plugin_tiledmap_data_layer_scene(plugin_tiledmap_data_layer_t layer);
TILEDMAP_LAYER * plugin_tiledmap_data_layer_data(plugin_tiledmap_data_layer_t layer);

const char * plugin_tiledmap_data_layer_name(plugin_tiledmap_data_layer_t layer);
plugin_tiledmap_data_layer_t plugin_tiledmap_data_layer_find_by_name(plugin_tiledmap_data_scene_t scene, const char * name);
uint32_t plugin_tiledmap_data_layer_tile_count(plugin_tiledmap_data_layer_t layer);
void plugin_tiledmap_data_layer_tiles(plugin_tiledmap_data_tile_it_t tile_it, plugin_tiledmap_data_layer_t layer);

void plugin_tiledmap_data_layer_config_rect(plugin_tiledmap_data_layer_t layer, ui_rect_t rect);
int plugin_tiledmap_data_layer_rect(plugin_tiledmap_data_layer_t layer, ui_rect_t rect);

#define plugin_tiledmap_data_layer_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
