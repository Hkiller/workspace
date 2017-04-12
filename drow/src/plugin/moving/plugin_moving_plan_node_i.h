#ifndef PLUGIN_MOVING_PLAN_NODE_I_H
#define PLUGIN_MOVING_PLAN_NODE_I_H
#include "plugin/moving/plugin_moving_plan_node.h"
#include "plugin_moving_plan_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_plan_node {
    plugin_moving_plan_t m_plan;
    TAILQ_ENTRY(plugin_moving_plan_node) m_next_for_plan;
    plugin_moving_plan_segment_list_t m_segments;
    uint16_t m_segment_count;
    MOVING_PLAN_NODE m_data;
};

void plugin_moving_plan_node_real_free(plugin_moving_plan_node_t node);
    
#ifdef __cplusplus
}
#endif

#endif
