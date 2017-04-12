#ifndef UI_SPRITE_SCROLLMAP_TYPES_H
#define UI_SPRITE_SCROLLMAP_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "plugin/scrollmap/plugin_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_scrollmap_runtime_size_policy {
    ui_sprite_scrollmap_runtime_size_no_adj = 0
    , ui_sprite_scrollmap_runtime_size_fix_x
    , ui_sprite_scrollmap_runtime_size_fix_y
    , ui_sprite_scrollmap_runtime_size_resize    
} ui_sprite_scrollmap_runtime_size_policy_t;
    
typedef struct ui_sprite_scrollmap_module * ui_sprite_scrollmap_module_t;
typedef struct ui_sprite_scrollmap_env * ui_sprite_scrollmap_env_t;
typedef struct ui_sprite_scrollmap_obj * ui_sprite_scrollmap_obj_t;

typedef struct ui_sprite_scrollmap_set_speed * ui_sprite_scrollmap_set_speed_t;
typedef struct ui_sprite_scrollmap_cancel_loop * ui_sprite_scrollmap_cancel_loop_t;
typedef struct ui_sprite_scrollmap_suspend * ui_sprite_scrollmap_suspend_t;
typedef struct ui_sprite_scrollmap_gen_team * ui_sprite_scrollmap_gen_team_t;
typedef struct ui_sprite_scrollmap_obj_move_suspend * ui_sprite_scrollmap_obj_move_suspend_t;
    
#ifdef __cplusplus
}
#endif

#endif
