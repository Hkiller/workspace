#ifndef PLUGIN_SCROLLMAP_RENDER_I_H
#define PLUGIN_SCROLLMAP_RENDER_I_H
#include "plugin/scrollmap/plugin_scrollmap_render.h"
#include "plugin_scrollmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_render {
    plugin_scrollmap_layer_t m_layer;    
};

int plugin_scrollmap_render_regist(plugin_scrollmap_module_t module);
void plugin_scrollmap_render_unregist(plugin_scrollmap_module_t module);

#ifdef __cplusplus
}
#endif

#endif
