#ifndef UI_SPRITE_MOVING_TYPES_H
#define UI_SPRITE_MOVING_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "plugin/moving/plugin_moving_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_moving_module * ui_sprite_moving_module_t;
typedef struct ui_sprite_moving_env * ui_sprite_moving_env_t;
typedef struct ui_sprite_moving_obj * ui_sprite_moving_obj_t;
    
typedef struct ui_sprite_moving_move_by_plan * ui_sprite_moving_move_by_plan_t;
typedef struct ui_sprite_moving_move_set_to_begin * ui_sprite_moving_move_set_to_begin_t;
typedef struct ui_sprite_moving_move_suspend * ui_sprite_moving_move_suspend_t;

typedef enum ui_sprite_moving_policy {
    ui_sprite_moving_policy_begin_at_cur_pos = 1,
    ui_sprite_moving_policy_end_at_cur_pos = 2,
    ui_sprite_moving_policy_no_adj = 3,
} ui_sprite_moving_policy_t;
    
#ifdef __cplusplus
}
#endif

#endif
