#ifndef PLUGIN_MOVING_CONTROL_I_H
#define PLUGIN_MOVING_CONTROL_I_H
#include "plugin/moving/plugin_moving_control.h"
#include "plugin_moving_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_control {
    plugin_moving_env_t m_env;
    plugin_moving_plan_t m_plan;
    TAILQ_ENTRY(plugin_moving_control) m_next_for_env;
    ui_vector_2 m_origin_pos;
    plugin_moving_node_list_t m_nodes;
};

void plugin_moving_control_real_free(plugin_moving_control_t control);

void plugin_moving_control_calc_pos(ui_vector_2_t r, plugin_moving_plan_point_t begion_point, plugin_moving_plan_point_t end_point, float t);

#ifdef __cplusplus
}
#endif

#endif
