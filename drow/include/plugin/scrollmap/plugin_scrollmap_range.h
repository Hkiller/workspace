#ifndef PLUGIN_SCROLLMAP_DATA_RANGE_H
#define PLUGIN_SCROLLMAP_DATA_RANGE_H
#include "cpe/utils/memory.h"
#include "plugin_scrollmap_types.h"
#include "plugin/scrollmap/plugin_scrollmap_data_scene.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_range_it {
    plugin_scrollmap_range_t (*next)(plugin_scrollmap_range_it_t it);
    char m_data[64];
};
    
plugin_scrollmap_range_t
plugin_scrollmap_range_create(
    plugin_scrollmap_layer_t layer, plugin_scrollmap_source_t source, float start_pos);

void plugin_scrollmap_range_free(plugin_scrollmap_range_t range);

plugin_scrollmap_range_state_t plugin_scrollmap_range_state(plugin_scrollmap_range_t range);
int plugin_scrollmap_range_cancel(plugin_scrollmap_range_t range);

void plugin_scrollmap_layer_active_ranges(plugin_scrollmap_range_it_t it, plugin_scrollmap_layer_t layer);
    
#define plugin_scrollmap_range_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
