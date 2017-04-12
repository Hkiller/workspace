#ifndef UI_SPRITE_CAMERA_SHAKE_H
#define UI_SPRITE_CAMERA_SHAKE_H
#include "ui_sprite_camera_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CAMERA_SHAKE_NAME;

ui_sprite_camera_shake_t ui_sprite_camera_shake_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_camera_shake_free(ui_sprite_camera_shake_t shake);

#ifdef __cplusplus
}
#endif

#endif
