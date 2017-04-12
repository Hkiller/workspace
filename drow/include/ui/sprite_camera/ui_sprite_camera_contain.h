#ifndef UI_SPRITE_CAMERA_CONTAIN_H
#define UI_SPRITE_CAMERA_CONTAIN_H
#include "ui_sprite_camera_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CAMERA_CONTAIN_NAME;

ui_sprite_camera_contain_t ui_sprite_camera_contain_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_camera_contain_free(ui_sprite_camera_contain_t contain);

int ui_sprite_camera_contain_set_decorator(ui_sprite_camera_contain_t contain, const char * decorator);

#ifdef __cplusplus
}
#endif

#endif
