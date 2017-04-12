#ifndef PLUGIN_SCROLLMAP_TYPES_H
#define PLUGIN_SCROLLMAP_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/pal/pal_types.h"
#include "cpe/dr/dr_types.h"
#include "render/utils/ui_utils_types.h"
#include "render/model/ui_model_types.h"
#include "protocol/plugin/scrollmap/scrollmap_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum plugin_scrollmap_range_state {
    plugin_scrollmap_range_state_idle = 1
    , plugin_scrollmap_range_state_active = 2
    , plugin_scrollmap_range_state_canceling = 3
    , plugin_scrollmap_range_state_done = 4
} plugin_scrollmap_range_state_t;

typedef enum plugin_scrollmap_moving_way {
    plugin_scrollmap_moving_left = 1
    , plugin_scrollmap_moving_right = 2
    , plugin_scrollmap_moving_up = 3
    , plugin_scrollmap_moving_down = 4
} plugin_scrollmap_moving_way_t;

typedef enum plugin_scrollmap_resize_policy {
    plugin_scrollmap_resize_policy_fix = 1
    , plugin_scrollmap_resize_policy_percent = 2
} plugin_scrollmap_resize_policy_t;

typedef enum plugin_scrollmap_script_state {
    plugin_scrollmap_script_state_wait, /*等待触发条件 */
    plugin_scrollmap_script_state_delay, /*等待延时 */
    plugin_scrollmap_script_state_runing, /*正在运行 */
} plugin_scrollmap_script_state_t;
    
typedef struct plugin_scrollmap_module * plugin_scrollmap_module_t;
typedef struct plugin_scrollmap_env * plugin_scrollmap_env_t;
typedef struct plugin_scrollmap_source * plugin_scrollmap_source_t;
typedef struct plugin_scrollmap_source_it * plugin_scrollmap_source_it_t;
typedef struct plugin_scrollmap_range * plugin_scrollmap_range_t;
typedef struct plugin_scrollmap_range_it * plugin_scrollmap_range_it_t;
typedef struct plugin_scrollmap_layer * plugin_scrollmap_layer_t;
typedef struct plugin_scrollmap_layer_it * plugin_scrollmap_layer_it_t;
typedef struct plugin_scrollmap_block * plugin_scrollmap_block_t;
typedef struct plugin_scrollmap_block_it * plugin_scrollmap_block_it_t;
typedef struct plugin_scrollmap_data_scene * plugin_scrollmap_data_scene_t;
typedef struct plugin_scrollmap_data_tile * plugin_scrollmap_data_tile_t;
typedef struct plugin_scrollmap_data_tile_it * plugin_scrollmap_data_tile_it_t;
typedef struct plugin_scrollmap_data_layer * plugin_scrollmap_data_layer_t;
typedef struct plugin_scrollmap_data_layer_it * plugin_scrollmap_data_layer_it_t;
typedef struct plugin_scrollmap_data_block * plugin_scrollmap_data_block_t;
typedef struct plugin_scrollmap_data_block_it * plugin_scrollmap_data_block_it_t;
typedef struct plugin_scrollmap_data_script * plugin_scrollmap_data_script_t;    
typedef struct plugin_scrollmap_data_script_it * plugin_scrollmap_data_script_it_t;
typedef struct plugin_scrollmap_script * plugin_scrollmap_script_t;
typedef struct plugin_scrollmap_script_executor * plugin_scrollmap_script_executor_t;
typedef struct plugin_scrollmap_obj * plugin_scrollmap_obj_t;
typedef struct plugin_scrollmap_obj_it * plugin_scrollmap_obj_it_t;
typedef struct plugin_scrollmap_obj_type_map * plugin_scrollmap_obj_type_map_t;
typedef struct plugin_scrollmap_team * plugin_scrollmap_team_t;
typedef struct plugin_scrollmap_team_it * plugin_scrollmap_team_it_t;
typedef struct plugin_scrollmap_render * plugin_scrollmap_render_t;

typedef const char * (*plugin_scrollmap_obj_name_fun_t)(void * ctx, plugin_scrollmap_obj_t obj);
typedef int (*plugin_scrollmap_obj_on_init_fun_t)(void * ctx, plugin_scrollmap_obj_t obj, const char * obj_type, const char * args);
typedef void (*plugin_scrollmap_obj_on_update_fun_t)(void * ctx, plugin_scrollmap_obj_t obj);
typedef void (*plugin_scrollmap_obj_on_event_fun_t)(void * ctx, plugin_scrollmap_obj_t obj, const char * event);
typedef void (*plugin_scrollmap_obj_on_destory_fun_t)(void * ctx, plugin_scrollmap_obj_t obj);
    
#ifdef __cplusplus
}
#endif

#endif
