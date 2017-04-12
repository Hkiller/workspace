#ifndef PLUGIN_LAYOUT_RENDER_GROUP_NODE_I_H
#define PLUGIN_LAYOUT_RENDER_GROUP_NODE_I_H
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_render_group_node {
    plugin_layout_render_group_t m_group;
    TAILQ_ENTRY(plugin_layout_render_group_node) m_next_for_group;
    plugin_layout_render_node_t m_node;
    TAILQ_ENTRY(plugin_layout_render_group_node) m_next_for_node;
};

plugin_layout_render_group_node_t
plugin_layout_render_group_node_create(plugin_layout_render_group_t group, plugin_layout_render_node_t node);
void plugin_layout_render_group_node_free(plugin_layout_render_group_node_t group_node);

void plugin_layout_render_group_node_real_free(plugin_layout_render_group_node_t group_node);

#ifdef __cplusplus
}
#endif

#endif
