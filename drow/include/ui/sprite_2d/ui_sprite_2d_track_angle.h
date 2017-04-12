#ifndef UI_SPRITE_2D_TRACK_ANGLE_H
#define UI_SPRITE_2D_TRACK_ANGLE_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_TRACK_ANGLE_NAME;

ui_sprite_2d_track_angle_t ui_sprite_2d_track_angle_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_track_angle_free(ui_sprite_2d_track_angle_t track_angle);

#ifdef __cplusplus
}
#endif

#endif
