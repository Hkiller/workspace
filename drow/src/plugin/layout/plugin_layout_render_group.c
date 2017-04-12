#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/utils/ui_rect.h"
#include "plugin_layout_render_group_i.h"
#include "plugin_layout_render_node_i.h"
#include "plugin_layout_render_group_node_i.h"

plugin_layout_render_group_t plugin_layout_render_group_create(plugin_layout_render_t render) {
    plugin_layout_module_t module = render->m_module;
    plugin_layout_render_group_t render_group;

    render_group = TAILQ_FIRST(&module->m_free_render_groups);
    if (render_group) {
        assert(module->m_free_group_count > 0);
        module->m_free_group_count--;
        TAILQ_REMOVE(&module->m_free_render_groups, render_group, m_next);
    }
    else {
        render_group = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_render_group));
        if (render_group == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_render_group_create: alloc fail!");
            return NULL;
        }
    }
    module->m_group_count++;
    
    render_group->m_render = render;
    render_group->m_node_count = 0;
    TAILQ_INIT(&render_group->m_nodes);
    
    TAILQ_INSERT_TAIL(&render->m_groups, render_group, m_next);

    return render_group;
}

void plugin_layout_render_group_free(plugin_layout_render_group_t render_group) {
    plugin_layout_module_t module = render_group->m_render->m_module;

    plugin_layout_render_group_clear(render_group);
    assert(render_group->m_node_count == 0);
    
    assert(module->m_group_count > 0);
    module->m_group_count--;
    TAILQ_REMOVE(&render_group->m_render->m_groups, render_group, m_next);

    module->m_free_group_count++;
    render_group->m_render = (plugin_layout_render_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_render_groups, render_group, m_next);
}

void plugin_layout_render_group_real_free(plugin_layout_render_group_t render_group) {
    plugin_layout_module_t module = (plugin_layout_module_t)render_group->m_render;

    assert(module->m_free_group_count > 0);
    module->m_free_group_count--;
    TAILQ_REMOVE(&module->m_free_render_groups, render_group, m_next);
    
    mem_free(module->m_alloc, render_group);
}

int plugin_layout_render_group_add_node(plugin_layout_render_group_t group, plugin_layout_render_node_t node) {
    plugin_layout_render_group_node_t group_node;

    if (plugin_layout_render_group_have_node(group, node)) return 0;

    group_node = plugin_layout_render_group_node_create(group, node);
    
    return group_node ? 0 : -1;
}

int plugin_layout_render_group_remove_node(plugin_layout_render_group_t group, plugin_layout_render_node_t node) {
    plugin_layout_render_group_node_t group_node;

    TAILQ_FOREACH(group_node, &node->m_groups, m_next_for_node) {
        if (group_node->m_group == group) {
            plugin_layout_render_group_node_free(group_node);
            return 0;
        }
    }

    return -1;
}

void plugin_layout_render_group_clear(plugin_layout_render_group_t render_group) {
    while(!TAILQ_EMPTY(&render_group->m_nodes)) {
        plugin_layout_render_group_node_free(TAILQ_FIRST(&render_group->m_nodes));
    }
}

uint8_t plugin_layout_render_group_have_node(plugin_layout_render_group_t group, plugin_layout_render_node_t node) {
    plugin_layout_render_group_node_t group_node;

    TAILQ_FOREACH(group_node, &node->m_groups, m_next_for_node) {
        if (group_node->m_group == group) return 1;
    }

    return 0;
}

uint32_t plugin_layout_render_group_node_count(plugin_layout_render_group_t group) {
    return group->m_node_count;
}

int plugin_layout_render_group_add_nodes(plugin_layout_render_group_t group, plugin_layout_render_group_t from_group) {
    int rv = 0;
    plugin_layout_render_group_node_t group_node;

    TAILQ_FOREACH(group_node, &from_group->m_nodes, m_next_for_group) {
        if (plugin_layout_render_group_add_node(group, group_node->m_node) != 0) rv = -1;
    }

    return rv;
}

static plugin_layout_render_node_t plugin_layout_render_group_node_next(struct plugin_layout_render_node_it * it) {
    plugin_layout_render_group_node_t * data = (plugin_layout_render_group_node_t *)(it->m_data);
    plugin_layout_render_group_node_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_group);

    return r->m_node;
}

void plugin_layout_render_group_nodes(plugin_layout_render_group_t group, plugin_layout_render_node_it_t it) {
    *(plugin_layout_render_group_node_t *)(it->m_data) = TAILQ_FIRST(&group->m_nodes);
    it->next = plugin_layout_render_group_node_next;
}

void plugin_layout_render_group_bound_rt(plugin_layout_render_group_t group, ui_rect_t rt) {
    plugin_layout_render_group_node_t group_node;
    uint8_t is_first_node = 1;
    
    for(group_node = TAILQ_FIRST(&group->m_nodes); group_node; group_node = TAILQ_NEXT(group_node, m_next_for_group)) {
        ui_rect_t bound_rt = &group_node->m_node->m_bound_rt;
        if (ui_rect_is_valid(bound_rt)) {
            if (is_first_node) {
                is_first_node = 0;
                *rt = *bound_rt;
            }
            else {
                ui_rect_inline_union(rt, bound_rt);
            }
        }
    }
}

plugin_layout_render_node_t plugin_layout_render_group_first_node(plugin_layout_render_group_t group) {
    plugin_layout_render_group_node_t first_node = TAILQ_FIRST(&group->m_nodes);
    return first_node ? first_node->m_node : NULL;
}

plugin_layout_render_node_t plugin_layout_render_group_last_node(plugin_layout_render_group_t group) {
    plugin_layout_render_group_node_t last_node = TAILQ_LAST(&group->m_nodes, plugin_layout_render_group_node_list);
    return last_node ? last_node->m_node : NULL;
}

void plugin_layout_render_group_adj_pos(plugin_layout_render_group_t group, ui_vector_2_t adj_pos) {
    plugin_layout_render_group_node_t group_node;

    TAILQ_FOREACH(group_node, &group->m_nodes, m_next_for_group) {
        plugin_layout_render_node_adj_pos(group_node->m_node, adj_pos);
    }
}
