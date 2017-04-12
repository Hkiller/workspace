#ifndef UI_SPRITE_CAMERA_MOVE_H
#define UI_SPRITE_CAMERA_MOVE_H
#include "ui_sprite_camera_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CAMERA_MOVE_NAME;

ui_sprite_camera_move_t ui_sprite_camera_move_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_camera_move_free(ui_sprite_camera_move_t move);

int ui_sprite_camera_move_set_decorator(ui_sprite_camera_move_t move, const char * decorator);

#ifdef __cplusplus
}
#endif

#endif
