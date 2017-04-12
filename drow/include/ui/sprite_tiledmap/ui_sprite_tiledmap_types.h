#ifndef UI_SPRITE_TILEDMAP_TYPES_H
#define UI_SPRITE_TILEDMAP_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "plugin/tiledmap/plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_tiledmap_module * ui_sprite_tiledmap_module_t;
typedef struct ui_sprite_tiledmap_env * ui_sprite_tiledmap_env_t;
typedef struct ui_sprite_tiledmap_layer * ui_sprite_tiledmap_layer_t;
typedef struct ui_sprite_tiledmap_layer_it * ui_sprite_tiledmap_layer_it_t;    
typedef struct ui_sprite_tiledmap_tile * ui_sprite_tiledmap_tile_t;
typedef struct ui_sprite_tiledmap_tile_it * ui_sprite_tiledmap_tile_it_t;

typedef struct ui_sprite_tiledmap_action_layer_follow * ui_sprite_tiledmap_action_layer_follow_t;
    
#ifdef __cplusplus
}
#endif

#endif
