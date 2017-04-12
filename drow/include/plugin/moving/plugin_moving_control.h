#ifndef DROW_PLUGIN_MOVING_CONTROL_H
#define DROW_PLUGIN_MOVING_CONTROL_H
#include "plugin_moving_types.h"
#include "protocol/plugin/moving/moving_common.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_moving_control_t plugin_moving_control_create(plugin_moving_env_t env, plugin_moving_plan_t plan);
void plugin_moving_control_free(plugin_moving_control_t moving_control);

ui_vector_2_t plugin_moving_control_origin_pos(plugin_moving_control_t control);
void plugin_moving_control_set_origin_pos(plugin_moving_control_t control, ui_vector_2_t pos);

plugin_moving_plan_t plugin_moving_control_plan(plugin_moving_control_t control);

int plugin_moving_control_update(plugin_moving_control_t control, float delta);

void plugin_moving_control_adj_origin_pos_for_point(plugin_moving_control_t control, plugin_moving_plan_point_t point, ui_vector_2_t point_pos);
int plugin_moving_control_adj_origin_pos_for_track_end_at(plugin_moving_control_t control, plugin_moving_plan_track_t track, ui_vector_2_t pos);
int plugin_moving_control_adj_origin_pos_for_track_begin_at(plugin_moving_control_t control, plugin_moving_plan_track_t track, ui_vector_2_t pos);
int plugin_moving_control_adj_origin_pos_for_node_end_at(plugin_moving_control_t control, plugin_moving_plan_node_t node, ui_vector_2_t pos);
int plugin_moving_control_adj_origin_pos_for_node_begin_at(plugin_moving_control_t control, plugin_moving_plan_node_t node, ui_vector_2_t pos);
    
#ifdef __cplusplus
}
#endif

#endif

