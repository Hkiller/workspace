#ifndef PLUGIN_TILEDMAP_DATA_LAYER_I_H
#define PLUGIN_TILEDMAP_DATA_LAYER_I_H
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin_tiledmap_data_scene_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_data_layer {
    plugin_tiledmap_data_scene_t m_scene;
    TAILQ_ENTRY(plugin_tiledmap_data_layer) m_next_for_scene;
    TILEDMAP_LAYER m_data;
    uint32_t m_tile_count;
    plugin_tiledmap_data_tile_list_t m_tile_list;
};

#ifdef __cplusplus
}
#endif

#endif
