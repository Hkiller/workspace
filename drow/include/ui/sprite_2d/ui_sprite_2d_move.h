#ifndef UI_SPRITE_2D_MOVE_H
#define UI_SPRITE_2D_MOVE_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_MOVE_NAME;

ui_sprite_2d_move_t ui_sprite_2d_move_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_move_free(ui_sprite_2d_move_t move);

int ui_sprite_2d_move_set_max_speed(ui_sprite_2d_move_t move, const char * max_speed);
const char * ui_sprite_2d_move_max_speed(ui_sprite_2d_move_t move);

int ui_sprite_2d_move_set_take_time(ui_sprite_2d_move_t move, const char * take_time);
const char * ui_sprite_2d_move_take_time(ui_sprite_2d_move_t move);

#ifdef __cplusplus
}
#endif

#endif
