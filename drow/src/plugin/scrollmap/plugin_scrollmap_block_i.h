#ifndef PLUGIN_SCROLLMAP_BLOCK_I_H
#define PLUGIN_SCROLLMAP_BLOCK_I_H
#include "plugin/scrollmap/plugin_scrollmap_block.h"
#include "plugin_scrollmap_layer_i.h"
#include "plugin_scrollmap_tile_i.h"
#include "plugin_scrollmap_range_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_block {
    plugin_scrollmap_layer_t m_layer;
    TAILQ_ENTRY(plugin_scrollmap_block) m_next_for_layer;
    plugin_scrollmap_range_t m_range;
    TAILQ_ENTRY(plugin_scrollmap_block) m_next_for_range;
    plugin_scrollmap_tile_t m_tile;
    SCROLLMAP_PAIR m_pos;
};

plugin_scrollmap_block_t plugin_scrollmap_block_create(
    plugin_scrollmap_layer_t layer, plugin_scrollmap_range_t range,
    SCROLLMAP_TILE const * tile, SCROLLMAP_PAIR const * pos);

void plugin_scrollmap_block_free(plugin_scrollmap_block_t block);

void plugin_scrollmap_block_real_free(plugin_scrollmap_block_t block);

#ifdef __cplusplus
}
#endif

#endif
