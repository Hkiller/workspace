#ifndef PLUGIN_SCROLLMAP_DATA_TILE_I_H
#define PLUGIN_SCROLLMAP_DATA_TILE_I_H
#include "plugin_scrollmap_data_scene_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_data_tile {
    plugin_scrollmap_data_scene_t m_scene;
    TAILQ_ENTRY(plugin_scrollmap_data_tile) m_next;
    SCROLLMAP_TILE m_data;
};

void plugin_scrollmap_data_tile_real_free(plugin_scrollmap_data_tile_t tile);
    
#ifdef __cplusplus
}
#endif

#endif
