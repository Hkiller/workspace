#ifndef DROW_PLUGIN_MOVING_TYPES_H
#define DROW_PLUGIN_MOVING_TYPES_H
#include "cpe/pal/pal_types.h"
#include "render/utils/ui_utils_types.h"
#include "render/model/ui_model_types.h"
#include "protocol/plugin/moving/moving_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_moving_module * plugin_moving_module_t;
typedef struct plugin_moving_env * plugin_moving_env_t;
typedef struct plugin_moving_control * plugin_moving_control_t;
typedef struct plugin_moving_node * plugin_moving_node_t;
    
typedef struct plugin_moving_plan * plugin_moving_plan_t;
typedef struct plugin_moving_plan_point * plugin_moving_plan_point_t;
typedef struct plugin_moving_plan_track_it * plugin_moving_plan_track_it_t;
typedef struct plugin_moving_plan_track * plugin_moving_plan_track_t;
typedef struct plugin_moving_plan_point_it * plugin_moving_plan_point_it_t;
typedef struct plugin_moving_plan_node * plugin_moving_plan_node_t;
typedef struct plugin_moving_plan_node_it * plugin_moving_plan_node_it_t;
typedef struct plugin_moving_plan_segment * plugin_moving_plan_segment_t;
typedef struct plugin_moving_plan_segment_it * plugin_moving_plan_segment_it_t;

typedef enum plugin_moving_node_state {
    plugin_moving_node_state_init = 1,
    plugin_moving_node_state_working = 2,
    plugin_moving_node_state_done = 3
} plugin_moving_node_state_t;

typedef enum plugin_moving_node_event {
    plugin_moving_node_event_state_updated = 1,
    plugin_moving_node_event_segment_begin = 2,
    plugin_moving_node_event_segment_end = 3,
} plugin_moving_node_event_t;

typedef void (*plugin_moving_pos_update_fun_t)(void * ctx, plugin_moving_node_t node, plugin_moving_node_event_t evt);
    
#ifdef __cplusplus
}
#endif

#endif
