#ifndef PLUGIN_LAYOUT_RENDER_GROUP_I_H
#define PLUGIN_LAYOUT_RENDER_GROUP_I_H
#include "plugin/layout/plugin_layout_render_group.h"
#include "plugin_layout_render_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_render_group {
    plugin_layout_render_t m_render;
    TAILQ_ENTRY(plugin_layout_render_group) m_next;
    uint32_t m_node_count;
    plugin_layout_render_group_node_list_t m_nodes;
};

void plugin_layout_render_group_real_free(plugin_layout_render_group_t render_group);

#ifdef __cplusplus
}
#endif

#endif
