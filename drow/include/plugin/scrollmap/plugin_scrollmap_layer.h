#ifndef PLUGIN_SCROLLMAP_LAYER_H
#define PLUGIN_SCROLLMAP_LAYER_H
#include "plugin_scrollmap_types.h"
#include "protocol/plugin/scrollmap/scrollmap_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_layer_it {
    plugin_scrollmap_layer_t (*next)(plugin_scrollmap_layer_it_t it);
    char m_data[64];
};
    
plugin_scrollmap_layer_t
plugin_scrollmap_layer_create(
    plugin_scrollmap_env_t env, const char * name);

plugin_scrollmap_layer_t
plugin_scrollmap_layer_find(
    plugin_scrollmap_env_t env, const char * name);

void plugin_scrollmap_layer_free(plugin_scrollmap_layer_t layer);

plugin_scrollmap_env_t plugin_scrollmap_layer_env(plugin_scrollmap_layer_t layer);
const char * plugin_scrollmap_layer_name(plugin_scrollmap_layer_t layer);
    
void plugin_scrollmap_layer_cancel_loop(plugin_scrollmap_layer_t layer);

void plugin_scrollmap_layer_blocks(plugin_scrollmap_block_it_t it, plugin_scrollmap_layer_t layer);

void plugin_scrollmap_env_layers(plugin_scrollmap_layer_it_t it, plugin_scrollmap_env_t env);

#define plugin_scrollmap_layer_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
