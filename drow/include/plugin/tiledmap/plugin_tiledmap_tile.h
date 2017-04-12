#ifndef PLUGIN_TILEDMAP_TILE_H
#define PLUGIN_TILEDMAP_TILE_H
#include "protocol/plugin/tiledmap/tiledmap_info.h"
#include "render/cache/ui_cache_types.h"
#include "render/model/ui_model_types.h"
#include "plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_tile_it {
    plugin_tiledmap_tile_t (*next)(struct plugin_tiledmap_tile_it * it);
    char m_data[64];
};

plugin_tiledmap_layer_t plugin_tiledmap_tile_layer(plugin_tiledmap_tile_t tile);
    
plugin_tiledmap_data_tile_t plugin_tiledmap_tile_data(plugin_tiledmap_tile_t tile);
ui_vector_2_t plugin_tiledmap_tile_pos(plugin_tiledmap_tile_t tile);

ui_cache_res_t plugin_tiledmap_tile_using_texture(plugin_tiledmap_tile_t tile);
ui_data_img_block_t plugin_tiledmap_tile_using_img_block(plugin_tiledmap_tile_t tile);
ui_data_frame_t plugin_tiledmap_tile_useing_frame(plugin_tiledmap_tile_t tile);
    
#define plugin_tiledmap_tile_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
