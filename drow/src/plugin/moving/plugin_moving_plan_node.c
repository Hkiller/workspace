#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_moving_plan_node_i.h"
#include "plugin_moving_plan_segment_i.h"

/*moving_group */
plugin_moving_plan_node_t
plugin_moving_plan_node_create(plugin_moving_plan_t plan) {
    plugin_moving_module_t module = plan->m_module;
    plugin_moving_plan_node_t node;

    node = TAILQ_FIRST(&module->m_free_plan_nodes);
    if (node) {
        TAILQ_REMOVE(&module->m_free_plan_nodes, node, m_next_for_plan);
    }
    else {
        node = mem_alloc(module->m_alloc, sizeof(struct plugin_moving_plan_node));
        if (node == NULL) {
            CPE_ERROR(module->m_em, "create plan node: alloc node fail!");
            return NULL;
        }
    }

    node->m_plan = plan;
    bzero(&node->m_data, sizeof(node->m_data));
    node->m_segment_count = 0;
    TAILQ_INIT(&node->m_segments);
    
    plan->m_node_count++;
    TAILQ_INSERT_TAIL(&plan->m_nodes, node, m_next_for_plan);
    
    return node;;
}

void plugin_moving_plan_node_free(plugin_moving_plan_node_t node) {
    plugin_moving_plan_t plan = node->m_plan;
    plugin_moving_module_t module = plan->m_module;

    while(!TAILQ_EMPTY(&node->m_segments)) {
        plugin_moving_plan_segment_free(TAILQ_FIRST(&node->m_segments));
    }
    assert(node->m_segment_count == 0);

    plan->m_node_count--;
    TAILQ_REMOVE(&plan->m_nodes, node, m_next_for_plan);
    
    node->m_plan = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_plan_nodes, node, m_next_for_plan);
}

void plugin_moving_plan_node_real_free(plugin_moving_plan_node_t node) {
    plugin_moving_module_t module = (void*)node->m_plan;

    TAILQ_REMOVE(&module->m_free_plan_nodes, node, m_next_for_plan);
    mem_free(module->m_alloc, node);
}

plugin_moving_plan_node_t plugin_moving_plan_node_find_by_name(plugin_moving_plan_t plan, const char * name) {
    plugin_moving_plan_node_t node;

    TAILQ_FOREACH(node, &plan->m_nodes, m_next_for_plan) {
        if (strcmp(node->m_data.name, name) == 0) return node;
    }
    
    return NULL;
}

MOVING_PLAN_NODE *
plugin_moving_plan_node_data(plugin_moving_plan_node_t node) {
    return &node->m_data;
}

static plugin_moving_plan_segment_t plugin_moving_plan_segments_next(struct plugin_moving_plan_segment_it * it) {
    plugin_moving_plan_segment_t * data = (plugin_moving_plan_segment_t *)(it->m_data);
    plugin_moving_plan_segment_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_node);

    return r;
}

void plugin_moving_plan_node_segments(plugin_moving_plan_segment_it_t it, plugin_moving_plan_node_t node) {
    *(plugin_moving_plan_segment_t *)(it->m_data) = TAILQ_FIRST(&node->m_segments);
    it->next = plugin_moving_plan_segments_next;
}
