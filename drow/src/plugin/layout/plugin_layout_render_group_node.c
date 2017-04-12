#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_layout_render_group_node_i.h"
#include "plugin_layout_render_group_i.h"
#include "plugin_layout_render_node_i.h"

plugin_layout_render_group_node_t
plugin_layout_render_group_node_create(plugin_layout_render_group_t group, plugin_layout_render_node_t node) {
    plugin_layout_module_t module = node->m_render->m_module;
    plugin_layout_render_group_node_t group_node;

    group_node = TAILQ_FIRST(&module->m_free_render_group_nodes);
    if (group_node) {
        assert(module->m_free_group_node_count > 0);
        module->m_free_group_node_count--;
        TAILQ_REMOVE(&module->m_free_render_group_nodes, group_node, m_next_for_group);
    }
    else {
        group_node = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_render_group_node));
        if (group_node == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_render_group_node_create: alloc fail!");
            return NULL;
        }
    }

    group->m_node_count++;
    group_node->m_group = group;
    group_node->m_node = node;

    TAILQ_INSERT_TAIL(&group->m_nodes, group_node, m_next_for_group);
    TAILQ_INSERT_TAIL(&node->m_groups, group_node, m_next_for_node);
    
    module->m_group_node_count++;
    return group_node;
}

void plugin_layout_render_group_node_free(plugin_layout_render_group_node_t group_node) {
    plugin_layout_module_t module = group_node->m_group->m_render->m_module;

    assert(group_node->m_group->m_node_count > 0);
    group_node->m_group->m_node_count--;
    
    TAILQ_REMOVE(&group_node->m_group->m_nodes, group_node, m_next_for_group);
    TAILQ_REMOVE(&group_node->m_node->m_groups, group_node, m_next_for_node);

    assert(module->m_group_node_count > 0);
    module->m_group_node_count--;
    
    module->m_free_group_node_count++;
    group_node->m_group = (plugin_layout_render_group_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_render_group_nodes, group_node, m_next_for_group);
}

void plugin_layout_render_group_node_real_free(plugin_layout_render_group_node_t group_node) {
    plugin_layout_module_t module = (plugin_layout_module_t)group_node->m_group;

    assert(module->m_free_group_node_count > 0);
    module->m_free_group_node_count--;
    TAILQ_REMOVE(&module->m_free_render_group_nodes, group_node, m_next_for_group);
    
    mem_free(module->m_alloc, group_node);
}
