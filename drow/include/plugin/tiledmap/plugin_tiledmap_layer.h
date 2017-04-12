#ifndef PLUGIN_TILEDMAP_LAYER_H
#define PLUGIN_TILEDMAP_LAYER_H
#include "protocol/plugin/tiledmap/tiledmap_info.h"
#include "plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_layer_it {
    plugin_tiledmap_layer_t (*next)(struct plugin_tiledmap_layer_it * it);
    char m_data[64];
};

plugin_tiledmap_layer_t plugin_tiledmap_layer_create(plugin_tiledmap_env_t env, const char * name);
void plugin_tiledmap_layer_free(plugin_tiledmap_layer_t layer);

plugin_tiledmap_layer_t plugin_tiledmap_layer_find(plugin_tiledmap_env_t env, const char * name);

int plugin_tiledmap_layer_load(
    plugin_tiledmap_layer_t layer, ui_rect_t to_rect,
    plugin_tiledmap_data_layer_t layer_data, ui_rect_t from_rect,
    plugin_tiledmap_fill_base_policy_t base_policy,
    plugin_tiledmap_fill_repeat_policy_t repeat_policy);
                                  
const char * plugin_tiledmap_layer_name(plugin_tiledmap_layer_t layer);
int plugin_tiledmap_layer_rect(plugin_tiledmap_layer_t layer, ui_rect_t rect);

ui_transform_t plugin_tiledmap_layer_trans(plugin_tiledmap_layer_t layer);
void plugin_tiledmap_layer_set_trans(plugin_tiledmap_layer_t layer, ui_transform_t trans);

float plugin_tiledmap_layer_alpha(plugin_tiledmap_layer_t layer);
void plugin_tiledmap_layer_set_alpha(plugin_tiledmap_layer_t layer, float alpha);
    
void plugin_tiledmap_layer_tiles(plugin_tiledmap_tile_it_t tile_it, plugin_tiledmap_layer_t layer);

#define plugin_tiledmap_layer_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
