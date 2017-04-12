#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_moving_plan_segment_i.h"

/*moving_group */
plugin_moving_plan_segment_t
plugin_moving_plan_segment_create(plugin_moving_plan_node_t node) {
    plugin_moving_module_t module = node->m_plan->m_module;
    plugin_moving_plan_segment_t segment;

    segment = TAILQ_FIRST(&module->m_free_plan_segments);
    if (segment) {
        TAILQ_REMOVE(&module->m_free_plan_segments, segment, m_next_for_node);
    }
    else {
        segment = mem_alloc(module->m_alloc, sizeof(struct plugin_moving_plan_segment));
        if (segment == NULL) {
            CPE_ERROR(module->m_em, "create plan segment: alloc segment fail!");
            return NULL;
        }
    }

    segment->m_node = node;
    bzero(&segment->m_data, sizeof(segment->m_data));

    node->m_segment_count++;
    TAILQ_INSERT_TAIL(&node->m_segments, segment, m_next_for_node);
    
    return segment;;
}

void plugin_moving_plan_segment_free(plugin_moving_plan_segment_t segment) {
    plugin_moving_plan_node_t node = segment->m_node;
    plugin_moving_module_t module = node->m_plan->m_module;

    node->m_segment_count--;
    TAILQ_REMOVE(&node->m_segments, segment, m_next_for_node);
    
    segment->m_node = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_plan_segments, segment, m_next_for_node);
}

void plugin_moving_plan_segment_real_free(plugin_moving_plan_segment_t segment) {
    plugin_moving_module_t module = (void*)segment->m_node;

    TAILQ_REMOVE(&module->m_free_plan_segments, segment, m_next_for_node);
    mem_free(module->m_alloc, segment);
}

MOVING_PLAN_SEGMENT *
plugin_moving_plan_segment_data(plugin_moving_plan_segment_t segment) {
    return &segment->m_data;
}
