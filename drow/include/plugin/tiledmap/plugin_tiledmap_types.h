#ifndef DROW_PLUGIN_TILEDMAP_TYPES_H
#define DROW_PLUGIN_TILEDMAP_TYPES_H
#include "cpe/pal/pal_types.h"
#include "render/model/ui_model_types.h"
#include "render/cache/ui_cache_types.h"
#include "plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum plugin_tiledmap_fill_base_policy {
    plugin_tiledmap_base_policy_left_top = 1,
    plugin_tiledmap_base_policy_left_bottom,
    plugin_tiledmap_base_policy_right_top,
    plugin_tiledmap_base_policy_right_bottom,
} plugin_tiledmap_fill_base_policy_t;

typedef enum plugin_tiledmap_fill_repeat_policy {
    plugin_tiledmap_repeat_policy_none = 1,
    plugin_tiledmap_repeat_policy_x,
    plugin_tiledmap_repeat_policy_y,
    plugin_tiledmap_repeat_policy_xy,
} plugin_tiledmap_fill_repeat_policy_t;
    
typedef struct plugin_tiledmap_module * plugin_tiledmap_module_t;
typedef struct plugin_tiledmap_data_scene * plugin_tiledmap_data_scene_t;
typedef struct plugin_tiledmap_data_layer * plugin_tiledmap_data_layer_t;
typedef struct plugin_tiledmap_data_layer_it * plugin_tiledmap_data_layer_it_t;
typedef struct plugin_tiledmap_data_tile * plugin_tiledmap_data_tile_t;
typedef struct plugin_tiledmap_data_tile_it * plugin_tiledmap_data_tile_it_t;
typedef struct plugin_tiledmap_env * plugin_tiledmap_env_t;
typedef struct plugin_tiledmap_layer * plugin_tiledmap_layer_t;
typedef struct plugin_tiledmap_layer_it * plugin_tiledmap_layer_it_t;    
typedef struct plugin_tiledmap_tile * plugin_tiledmap_tile_t;
typedef struct plugin_tiledmap_tile_it * plugin_tiledmap_tile_it_t;
typedef struct plugin_tiledmap_render_layer * plugin_tiledmap_render_layer_t;
typedef struct plugin_tiledmap_render_obj * plugin_tiledmap_render_obj_t;    
    
#ifdef __cplusplus
}
#endif

#endif
