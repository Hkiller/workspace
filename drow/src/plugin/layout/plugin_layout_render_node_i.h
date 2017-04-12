#ifndef PLUGIN_LAYOUT_RENDER_NODE_I_H
#define PLUGIN_LAYOUT_RENDER_NODE_I_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_color.h"
#include "plugin/layout/plugin_layout_render_node.h"
#include "plugin_layout_render_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_render_node {
    plugin_layout_render_t m_render;
    TAILQ_ENTRY(plugin_layout_render_node) m_next;
    plugin_layout_render_group_node_list_t m_groups;
    plugin_layout_font_element_t m_element;
    ui_rect m_bound_rt;
    ui_rect m_render_rt;
    struct plugin_layout_font_draw m_font_draw;
};

plugin_layout_render_node_t plugin_layout_render_node_create(
    plugin_layout_render_t render, plugin_layout_font_element_t element, plugin_layout_font_draw_t font_draw);
void plugin_layout_render_node_free(plugin_layout_render_node_t render_node);
void plugin_layout_render_node_real_free(plugin_layout_render_node_t render_node);

#ifdef __cplusplus
}
#endif

#endif
