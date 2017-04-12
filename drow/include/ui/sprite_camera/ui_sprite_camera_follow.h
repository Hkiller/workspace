#ifndef UI_SPRITE_CAMERA_FOLLOW_H
#define UI_SPRITE_CAMERA_FOLLOW_H
#include "ui_sprite_camera_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CAMERA_FOLLOW_NAME;

ui_sprite_camera_follow_t ui_sprite_camera_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_camera_follow_free(ui_sprite_camera_follow_t follow);

int ui_sprite_camera_follow_set_decorator(ui_sprite_camera_follow_t follow, const char * decorator);

#ifdef __cplusplus
}
#endif

#endif
