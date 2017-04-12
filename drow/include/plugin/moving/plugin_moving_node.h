#ifndef DROW_PLUGIN_MOVING_NODE_H
#define DROW_PLUGIN_MOVING_NODE_H
#include "plugin_moving_types.h"
#include "protocol/plugin/moving/moving_common.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_moving_node_t plugin_moving_node_create(plugin_moving_control_t control, plugin_moving_plan_node_t plan_node, uint32_t loop_count);
void plugin_moving_node_free(plugin_moving_node_t moving_node);

void plugin_moving_node_set_update_fun(plugin_moving_node_t node, plugin_moving_pos_update_fun_t update_fun, void * update_ctx);
plugin_moving_pos_update_fun_t plugin_moving_node_update_fun(plugin_moving_node_t node);
void * plugin_moving_node_update_ctx(plugin_moving_node_t node);

plugin_moving_control_t plugin_moving_node_control(plugin_moving_node_t node);
const char * plugin_moving_node_name(plugin_moving_node_t node);
plugin_moving_plan_node_t plugin_moving_node_plan_node(plugin_moving_node_t node);

float plugin_moving_node_time_scale(plugin_moving_node_t node);
void plugin_moving_node_set_time_scale(plugin_moving_node_t node, float time_scale);
    
plugin_moving_node_state_t plugin_moving_node_state(plugin_moving_node_t node);
ui_vector_2_t plugin_moving_node_pos(plugin_moving_node_t node);

plugin_moving_plan_segment_t plugin_moving_node_cur_segment(plugin_moving_node_t node);

void plugin_moving_node_update(plugin_moving_node_t node, float delta);

#ifdef __cplusplus
}
#endif

#endif

