#ifndef DROW_PLUGIN_TILEDMAP_DATA_TILE_H
#define DROW_PLUGIN_TILEDMAP_DATA_TILE_H
#include "protocol/plugin/tiledmap/tiledmap_info.h"
#include "plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_data_tile_it {
    plugin_tiledmap_data_tile_t (*next)(struct plugin_tiledmap_data_tile_it * it);
    char m_data[64];
};

/*tile*/
plugin_tiledmap_data_tile_t plugin_tiledmap_data_tile_create(plugin_tiledmap_data_layer_t layer);
void plugin_tiledmap_data_tile_free(plugin_tiledmap_data_tile_t tile);

plugin_tiledmap_data_tile_t plugin_tiledmap_data_tile_find_by_name(plugin_tiledmap_data_layer_t layer, const char * name);
    
TILEDMAP_TILE * plugin_tiledmap_data_tile_data(plugin_tiledmap_data_tile_t tile);

uint32_t plugin_tiledmap_data_tile_src_id(plugin_tiledmap_data_tile_t tile);
ui_data_src_type_t plugin_tiledmap_data_tile_src_type(plugin_tiledmap_data_tile_t tile);

#define plugin_tiledmap_data_tile_it_next(it) ((it)->next ? (it)->next(it) : NULL)

int plugin_tiledmap_data_tile_rect(
    plugin_tiledmap_data_tile_t tile, ui_rect_t rect, ui_data_src_t * src_cache);
    
#ifdef __cplusplus
}
#endif

#endif
