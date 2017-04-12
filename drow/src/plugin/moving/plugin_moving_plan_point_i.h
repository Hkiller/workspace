#ifndef PLUGIN_MOVING_PLAN_POINT_I_H
#define PLUGIN_MOVING_PLAN_POINT_I_H
#include "plugin/moving/plugin_moving_plan_point.h"
#include "plugin_moving_plan_track_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_plan_point {
    plugin_moving_plan_track_t m_track;
    TAILQ_ENTRY(plugin_moving_plan_point) m_next_for_track;
    plugin_moving_plan_point_list_t m_points;
    MOVING_PLAN_POINT m_data;
};

void plugin_moving_plan_point_real_free(plugin_moving_plan_point_t point);
    
#ifdef __cplusplus
}
#endif

#endif
