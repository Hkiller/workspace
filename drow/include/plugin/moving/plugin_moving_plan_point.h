#ifndef DROW_PLUGIN_MOVING_PLAN_POINT_H
#define DROW_PLUGIN_MOVING_PLAN_POINT_H
#include "plugin_moving_types.h"
#include "protocol/plugin/moving/moving_info.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_moving_plan_point_it {
    plugin_moving_plan_point_t (*next)(struct plugin_moving_plan_point_it * it);
    char m_data[64];
};

plugin_moving_plan_point_t plugin_moving_plan_point_create(plugin_moving_plan_track_t plan_track);
void plugin_moving_plan_point_free(plugin_moving_plan_point_t plan_point);

MOVING_PLAN_POINT * plugin_moving_plan_point_data(plugin_moving_plan_point_t plan_point);

#define plugin_moving_plan_point_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

