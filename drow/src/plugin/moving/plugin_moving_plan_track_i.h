#ifndef PLUGIN_MOVING_PLAN_TRACK_I_H
#define PLUGIN_MOVING_PLAN_TRACK_I_H
#include "plugin/moving/plugin_moving_plan_track.h"
#include "plugin_moving_plan_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_plan_track {
    plugin_moving_plan_t m_plan;
    TAILQ_ENTRY(plugin_moving_plan_track) m_next_for_plan;
    plugin_moving_plan_point_list_t m_points;
    uint16_t m_point_count;
    MOVING_PLAN_TRACK m_data;
};

void plugin_moving_plan_track_real_free(plugin_moving_plan_track_t track);
    
#ifdef __cplusplus
}
#endif

#endif
