#ifndef PLUGIN_TILEDMAP_RENDER_LAYER_H
#define PLUGIN_TILEDMAP_RENDER_LAYER_H
#include "plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_tiledmap_layer_t plugin_tiledmap_render_layer_layer(plugin_tiledmap_render_layer_t render);
void plugin_tiledmap_render_layer_set_layer(plugin_tiledmap_render_layer_t render, plugin_tiledmap_layer_t layer);

#ifdef __cplusplus
}
#endif

#endif
