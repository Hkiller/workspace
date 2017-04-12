#ifndef DROW_PLUGIN_MOVING_PLAN_H
#define DROW_PLUGIN_MOVING_PLAN_H
#include "cpe/utils/buffer.h"
#include "plugin_moving_types.h"
#include "protocol/plugin/moving/moving_info.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_moving_plan_t plugin_moving_plan_create(plugin_moving_module_t module, ui_data_src_t src);
void plugin_moving_plan_free(plugin_moving_plan_t data_moving_group);

ui_data_src_t plugin_moving_plan_src(plugin_moving_plan_t plan);
    
MOVING_PLAN * plugin_moving_plan_data(plugin_moving_plan_t moving_plan);

void plugin_moving_plan_tracks(plugin_moving_plan_track_it_t track_it, plugin_moving_plan_t plan);
void plugin_moving_plan_nodes(plugin_moving_plan_node_it_t track_it, plugin_moving_plan_t plan);

const char * plugin_moving_plan_dump(mem_buffer_t buffer, plugin_moving_plan_t plan);

#ifdef __cplusplus
}
#endif

#endif

