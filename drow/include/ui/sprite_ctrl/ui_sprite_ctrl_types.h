#ifndef UI_SPRITE_CONTROL_TYPES_H
#define UI_SPRITE_CONTROL_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_types.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_ctrl_module * ui_sprite_ctrl_module_t;

/*弹射控件 */
typedef struct ui_sprite_ctrl_circle * ui_sprite_ctrl_circle_t;

/*转盘控件 */
typedef struct ui_sprite_ctrl_turntable * ui_sprite_ctrl_turntable_t;
typedef struct ui_sprite_ctrl_turntable_member * ui_sprite_ctrl_turntable_member_t;
typedef struct ui_sprite_ctrl_turntable_join * ui_sprite_ctrl_turntable_join_t;
typedef struct ui_sprite_ctrl_turntable_touch * ui_sprite_ctrl_turntable_touch_t;
typedef struct ui_sprite_ctrl_turntable_active * ui_sprite_ctrl_turntable_active_t;

/*运动轨迹控件 */
typedef struct ui_sprite_ctrl_track_mgr * ui_sprite_ctrl_track_mgr_t;
typedef struct ui_sprite_ctrl_track * ui_sprite_ctrl_track_t;
typedef struct ui_sprite_ctrl_track_meta * ui_sprite_ctrl_track_meta_t;
typedef struct ui_sprite_ctrl_track_follow * ui_sprite_ctrl_track_follow_t;
typedef struct ui_sprite_ctrl_track_manip * ui_sprite_ctrl_track_manip_t;

#ifdef __cplusplus
}
#endif

#endif


