#ifndef PLUGIN_TILEDMAP_DATA_TILE_I_H
#define PLUGIN_TILEDMAP_DATA_TILE_I_H
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin_tiledmap_data_layer_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_data_tile {
    plugin_tiledmap_data_layer_t m_layer;
    TAILQ_ENTRY(plugin_tiledmap_data_tile) m_next_for_layer;
    TAILQ_ENTRY(plugin_tiledmap_data_tile) m_next_for_use;
    TILEDMAP_TILE m_data;
};

#ifdef __cplusplus
}
#endif

#endif
