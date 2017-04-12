#ifndef DROW_PLUGIN_MOVING_PLAN_TRACK_H
#define DROW_PLUGIN_MOVING_PLAN_TRACK_H
#include "plugin_moving_types.h"
#include "protocol/plugin/moving/moving_info.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_plan_track_it {
    plugin_moving_plan_track_t (*next)(struct plugin_moving_plan_track_it * it);
    char m_data[64];
};
    
plugin_moving_plan_track_t plugin_moving_plan_track_create(plugin_moving_plan_t plan);
void plugin_moving_plan_track_free(plugin_moving_plan_track_t track);

plugin_moving_plan_track_t plugin_moving_plan_track_find_by_id(plugin_moving_plan_t plan, uint16_t track_id);
    
MOVING_PLAN_TRACK * plugin_moving_plan_track_data(plugin_moving_plan_track_t track);

void plugin_moving_plan_track_points(plugin_moving_plan_point_it_t track_it, plugin_moving_plan_track_t track);
    
#define plugin_moving_plan_track_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

