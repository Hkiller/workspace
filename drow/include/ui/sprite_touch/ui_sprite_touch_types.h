#ifndef UI_SPRITE_TOUCH_TYPES_H
#define UI_SPRITE_TOUCH_TYPES_H
#include "cpe/tl/tl_types.h"
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_types.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "protocol/ui/sprite_touch/ui_sprite_touch_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_SPRITE_TOUCH_MAX_FINGER_COUNT (3)

typedef struct ui_sprite_touch_mgr * ui_sprite_touch_mgr_t;
typedef struct ui_sprite_touch_env * ui_sprite_touch_env_t;
typedef struct ui_sprite_touch_trace * ui_sprite_touch_trace_t;
typedef struct ui_sprite_touch_point * ui_sprite_touch_point_t;

typedef struct ui_sprite_touch_touchable * ui_sprite_touch_touchable_t;
typedef struct ui_sprite_touch_box * ui_sprite_touch_box_t;

typedef struct ui_sprite_touch_move * ui_sprite_touch_move_t;
typedef struct ui_sprite_touch_click * ui_sprite_touch_click_t;
typedef struct ui_sprite_touch_scale * ui_sprite_touch_scale_t;

#ifdef __cplusplus
}
#endif

#endif
