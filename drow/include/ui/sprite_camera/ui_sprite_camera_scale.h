#ifndef UI_SPRITE_CAMERA_SCALE_H
#define UI_SPRITE_CAMERA_SCALE_H
#include "ui_sprite_camera_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CAMERA_SCALE_NAME;

ui_sprite_camera_scale_t ui_sprite_camera_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_camera_scale_free(ui_sprite_camera_scale_t scale);

int ui_sprite_camera_scale_set_decorator(ui_sprite_camera_scale_t scale, const char * decorator);

#ifdef __cplusplus
}
#endif

#endif
