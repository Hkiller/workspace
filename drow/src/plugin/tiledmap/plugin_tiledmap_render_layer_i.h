#ifndef PLUGIN_TILEDMAP_RENDER_LAYER_I_H
#define PLUGIN_TILEDMAP_RENDER_LAYER_I_H
#include "plugin/tiledmap/plugin_tiledmap_render_layer.h"
#include "plugin_tiledmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_render_layer {
    plugin_tiledmap_layer_t m_layer;    
};

int plugin_tiledmap_render_layer_regist(plugin_tiledmap_module_t module);
void plugin_tiledmap_render_layer_unregist(plugin_tiledmap_module_t module);

#ifdef __cplusplus
}
#endif

#endif
