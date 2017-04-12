#ifndef PLUGIN_MOVING_PLAN_SEGMENT_I_H
#define PLUGIN_MOVING_PLAN_SEGMENT_I_H
#include "plugin/moving/plugin_moving_plan_segment.h"
#include "plugin_moving_plan_node_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_plan_segment {
    plugin_moving_plan_node_t m_node;
    TAILQ_ENTRY(plugin_moving_plan_segment) m_next_for_node;
    MOVING_PLAN_SEGMENT m_data;
};

void plugin_moving_plan_segment_real_free(plugin_moving_plan_segment_t segment);
    
#ifdef __cplusplus
}
#endif

#endif
