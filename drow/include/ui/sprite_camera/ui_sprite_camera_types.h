#ifndef UI_SPRITE_CAMERA_TYPES_H
#define UI_SPRITE_CAMERA_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_camera_module * ui_sprite_camera_module_t;

/*camera*/
typedef struct ui_sprite_camera_env * ui_sprite_camera_env_t;

/*actions*/
typedef struct ui_sprite_camera_touch * ui_sprite_camera_touch_t;
typedef struct ui_sprite_camera_move * ui_sprite_camera_move_t;
typedef struct ui_sprite_camera_follow * ui_sprite_camera_follow_t;
typedef struct ui_sprite_camera_contain * ui_sprite_camera_contain_t;
typedef struct ui_sprite_camera_scale * ui_sprite_camera_scale_t;
typedef struct ui_sprite_camera_shake * ui_sprite_camera_shake_t;
typedef struct ui_sprite_camera_wait_stop * ui_sprite_camera_wait_stop_t;
typedef struct ui_sprite_camera_trace_in_line * ui_sprite_camera_trace_in_line_t;
    
#ifdef __cplusplus
}
#endif

#endif


