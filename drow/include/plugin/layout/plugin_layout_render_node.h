#ifndef PLUGIN_LAYOUT_RENDER_NODE_H
#define PLUGIN_LAYOUT_RENDER_NODE_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_render_node_it {
    plugin_layout_render_node_t (*next)(struct plugin_layout_render_node_it * it);
    char m_data[64];
};

uint32_t plugin_layout_render_node_charter(plugin_layout_render_node_t node);
ui_rect_t plugin_layout_render_node_bound_rt(plugin_layout_render_node_t node);
void plugin_layout_render_node_adj_pos(plugin_layout_render_node_t node, ui_vector_2_t adj_pos);

#define plugin_layout_render_node_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
