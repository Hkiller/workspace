#ifndef UI_SPRITE_CTRL_TURNTABLE_TOUCH_H
#define UI_SPRITE_CTRL_TURNTABLE_TOUCH_H
#include "gd/app/app_types.h"
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CTRL_TURNTABLE_TOUCH_NAME;

ui_sprite_ctrl_turntable_touch_t ui_sprite_ctrl_turntable_touch_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ctrl_turntable_touch_free(ui_sprite_ctrl_turntable_touch_t ctrl);

int ui_sprite_ctrl_turntable_touch_set_decorator(ui_sprite_ctrl_turntable_touch_t touch, const char * decorator);

#ifdef __cplusplus
}
#endif

#endif
