#ifndef PLUGIN_TILEDMAP_TILE_I_H
#define PLUGIN_TILEDMAP_TILE_I_H
#include "render/utils/ui_vector_2.h"
#include "plugin/tiledmap/plugin_tiledmap_tile.h"
#include "plugin_tiledmap_layer_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_tile {
    plugin_tiledmap_layer_t m_layer;
    plugin_tiledmap_data_tile_t m_data_tile;
    TAILQ_ENTRY(plugin_tiledmap_tile) m_next;
    ui_vector_2 m_pos;
    void * m_using_product;
    ui_cache_res_t m_using_texture;
};

plugin_tiledmap_tile_t
plugin_tiledmap_tile_create(
    plugin_tiledmap_layer_t layer, plugin_tiledmap_data_tile_t data_tile, ui_vector_2_t pos, ui_data_src_t using_src);
void plugin_tiledmap_tile_free(plugin_tiledmap_tile_t tile);

void plugin_tiledmap_tile_real_free(plugin_tiledmap_tile_t tile);
    
#ifdef __cplusplus
}
#endif

#endif
