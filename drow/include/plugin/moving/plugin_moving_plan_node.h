#ifndef DROW_PLUGIN_MOVING_PLAN_NODE_H
#define DROW_PLUGIN_MOVING_PLAN_NODE_H
#include "plugin_moving_types.h"
#include "protocol/plugin/moving/moving_info.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_plan_node_it {
    plugin_moving_plan_node_t (*next)(struct plugin_moving_plan_node_it * it);
    char m_data[64];
};
    
plugin_moving_plan_node_t plugin_moving_plan_node_create(plugin_moving_plan_t plan);
void plugin_moving_plan_node_free(plugin_moving_plan_node_t plan_node);

plugin_moving_plan_node_t plugin_moving_plan_node_find_by_name(plugin_moving_plan_t plan, const char * name);

int plugin_moving_plan_node_end_pos(plugin_moving_plan_node_t node, ui_vector_2_t end_pos);
int plugin_moving_plan_node_begin_pos(plugin_moving_plan_node_t node, ui_vector_2_t begin_pos);
    
MOVING_PLAN_NODE * plugin_moving_plan_node_data(plugin_moving_plan_node_t plan_node);

void plugin_moving_plan_node_segments(plugin_moving_plan_segment_it_t segment_it, plugin_moving_plan_node_t node);
    
#define plugin_moving_plan_node_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

