#ifndef PLUGIN_SCROLLMAP_BLOCK_H
#define PLUGIN_SCROLLMAP_BLOCK_H
#include "plugin_scrollmap_types.h"
#include "protocol/plugin/scrollmap/scrollmap_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_block_it {
    plugin_scrollmap_block_t (*m_next)(plugin_scrollmap_block_it_t it);
    char m_data[16];
};

plugin_scrollmap_layer_t plugin_scrollmap_block_layer(plugin_scrollmap_block_t block);
plugin_scrollmap_range_t plugin_scrollmap_block_range(plugin_scrollmap_block_t block);
SCROLLMAP_TILE const * plugin_scrollmap_block_tile(plugin_scrollmap_block_t block);
SCROLLMAP_PAIR const * plugin_scrollmap_block_pos(plugin_scrollmap_block_t block);

#define plugin_scrollmap_block_it_next(__it) ((__it)->m_next)(__it)

#ifdef __cplusplus
}
#endif

#endif
