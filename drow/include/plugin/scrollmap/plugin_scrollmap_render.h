#ifndef PLUGIN_SCROLLMAP_RENDER_H
#define PLUGIN_SCROLLMAP_RENDER_H
#include "plugin_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_scrollmap_layer_t plugin_scrollmap_render_layer(plugin_scrollmap_render_t render);
void plugin_scrollmap_render_set_layer(plugin_scrollmap_render_t render, plugin_scrollmap_layer_t layer);

#ifdef __cplusplus
}
#endif

#endif
