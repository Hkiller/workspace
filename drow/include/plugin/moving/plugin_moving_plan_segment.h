#ifndef DROW_PLUGIN_MOVING_PLAN_SEGMENT_H
#define DROW_PLUGIN_MOVING_PLAN_SEGMENT_H
#include "plugin_moving_types.h"
#include "protocol/plugin/moving/moving_info.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_plan_segment_it {
    plugin_moving_plan_segment_t (*next)(struct plugin_moving_plan_segment_it * it);
    char m_data[64];
};

plugin_moving_plan_segment_t plugin_moving_plan_segment_create(plugin_moving_plan_node_t node);
void plugin_moving_plan_segment_free(plugin_moving_plan_segment_t segment);

MOVING_PLAN_SEGMENT * plugin_moving_plan_segment_data(plugin_moving_plan_segment_t segment);

#define plugin_moving_plan_segment_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

