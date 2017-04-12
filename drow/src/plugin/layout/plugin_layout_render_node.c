#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_layout_render_node_i.h"
#include "plugin_layout_render_group_node_i.h"
#include "plugin_layout_font_element_i.h"

plugin_layout_render_node_t
plugin_layout_render_node_create(plugin_layout_render_t render, plugin_layout_font_element_t element, plugin_layout_font_draw_t font_draw) {
    plugin_layout_module_t module = render->m_module;
    plugin_layout_render_node_t render_node;

    render_node = TAILQ_FIRST(&module->m_free_render_nodes);
    if (render_node) {
        assert(module->m_free_node_count > 0);
        module->m_free_node_count--;
        TAILQ_REMOVE(&module->m_free_render_nodes, render_node, m_next);
    }
    else {
        render_node = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_render_node));
        if (render_node == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_render_node_create: alloc fail!");
            return NULL;
        }
    }
    module->m_node_count++;
    render->m_node_count++;
    
    bzero(render_node, sizeof(*render_node));
    render_node->m_render = render;
    render_node->m_element = element;
    if (font_draw) {
        render_node->m_font_draw = *font_draw;
    }
    else {
        bzero(&render_node->m_font_draw, sizeof(render_node->m_font_draw));
    }

    if (element) element->m_node_count++;
    
    TAILQ_INIT(&render_node->m_groups);
    
    TAILQ_INSERT_TAIL(&render->m_nodes, render_node, m_next);
    
    return render_node;
}

void plugin_layout_render_node_free(plugin_layout_render_node_t render_node) {
    plugin_layout_module_t module = render_node->m_render->m_module;

    while(!TAILQ_EMPTY(&render_node->m_groups)) {
        plugin_layout_render_group_node_free(TAILQ_FIRST(&render_node->m_groups));
    }

    if (render_node->m_element) {
        assert(render_node->m_element->m_node_count > 0);
        render_node->m_element->m_node_count--;
    }

    assert(render_node->m_render->m_node_count > 0);
    render_node->m_render->m_node_count--;
    TAILQ_REMOVE(&render_node->m_render->m_nodes, render_node, m_next);

    assert(module->m_node_count > 0);
    module->m_node_count--;
    module->m_free_node_count++;
    render_node->m_render = (plugin_layout_render_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_render_nodes, render_node, m_next);
}

void plugin_layout_render_node_real_free(plugin_layout_render_node_t render_node) {
    plugin_layout_module_t module = (plugin_layout_module_t)render_node->m_render;

    assert(module->m_free_node_count > 0);
    module->m_free_node_count--;

    TAILQ_REMOVE(&module->m_free_render_nodes, render_node, m_next);
    mem_free(module->m_alloc, render_node);
}

uint32_t plugin_layout_render_node_charter(plugin_layout_render_node_t node) {
    return node->m_element ? node->m_element->m_charter : 0;
}

ui_rect_t plugin_layout_render_node_bound_rt(plugin_layout_render_node_t node) {
    return &node->m_bound_rt;
}

void plugin_layout_render_node_adj_pos(plugin_layout_render_node_t render_node, ui_vector_2_t adj_pos) {
    ui_rect_adj_by_pt(&render_node->m_render_rt, adj_pos);
    ui_rect_adj_by_pt(&render_node->m_bound_rt, adj_pos);
}
