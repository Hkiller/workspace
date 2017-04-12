#ifndef PLUGIN_MOVING_NODE_I_H
#define PLUGIN_MOVING_NODE_I_H
#include "render/utils/ui_vector_2.h"
#include "plugin/moving/plugin_moving_node.h"
#include "plugin_moving_control_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_node {
    plugin_moving_control_t m_control;
    TAILQ_ENTRY(plugin_moving_node) m_next_for_control;
    plugin_moving_plan_node_t m_plan_node;
    plugin_moving_plan_track_t m_track;
    plugin_moving_node_state_t m_state;
    float m_time_scale;
    float m_runing_time;
    uint32_t m_loop_count;
    ui_vector_2 m_pos;
    plugin_moving_pos_update_fun_t m_update_fun;
    void * m_update_ctx;

    uint8_t m_segment_is_begin;
    plugin_moving_plan_segment_t m_segment;
    plugin_moving_plan_point_t m_begin_point;
    plugin_moving_plan_point_t m_end_point;
    float m_segment_start_duration;
    float m_segment_duration;
    float m_point_start_duration;
    float m_point_duration;
};

void plugin_moving_node_real_free(plugin_moving_node_t node);
    
#ifdef __cplusplus
}
#endif

#endif
