#ifndef UI_SPRITE_2D_TRACK_FLIP_H
#define UI_SPRITE_2D_TRACK_FLIP_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_TRACK_FLIP_NAME;

ui_sprite_2d_track_flip_t ui_sprite_2d_track_flip_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_track_flip_free(ui_sprite_2d_track_flip_t track_flip);

uint8_t ui_sprite_2d_track_flip_process_x(ui_sprite_2d_track_flip_t track_flip);
void ui_sprite_2d_track_flip_set_process_x(ui_sprite_2d_track_flip_t track_flip, uint8_t process_x);

uint8_t ui_sprite_2d_track_flip_process_y(ui_sprite_2d_track_flip_t track_flip);
void ui_sprite_2d_track_flip_set_process_y(ui_sprite_2d_track_flip_t track_flip, uint8_t process_y);

#ifdef __cplusplus
}
#endif

#endif
