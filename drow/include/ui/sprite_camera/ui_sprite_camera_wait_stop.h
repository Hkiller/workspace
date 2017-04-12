#ifndef UI_SPRITE_CAMERA_WAIT_STOP_H
#define UI_SPRITE_CAMERA_WAIT_STOP_H
#include "ui_sprite_camera_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CAMERA_WAIT_STOP_NAME;

ui_sprite_camera_wait_stop_t ui_sprite_camera_wait_stop_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_camera_wait_stop_free(ui_sprite_camera_wait_stop_t wait_stop);

#ifdef __cplusplus
}
#endif

#endif
