#ifndef UI_SPRITE_CAMERA_ACTION_TOUCH_H
#define UI_SPRITE_CAMERA_ACTION_TOUCH_H
#include "ui_sprite_camera_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CAMERA_TOUCH_NAME;

ui_sprite_camera_touch_t ui_sprite_camera_touch_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_camera_touch_free(ui_sprite_camera_touch_t touch);

int ui_sprite_camera_touch_set_decorator(ui_sprite_camera_touch_t touch, const char * decorator);

#ifdef __cplusplus
}
#endif

#endif
