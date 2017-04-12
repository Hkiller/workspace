#ifndef UI_SPRITE_2D_WAIT_MOVE_DISTANCE_H
#define UI_SPRITE_2D_WAIT_MOVE_DISTANCE_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_WAIT_MOVE_DISTANCE_NAME;

ui_sprite_2d_wait_move_distance_t ui_sprite_2d_wait_move_distance_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_wait_move_distance_free(ui_sprite_2d_wait_move_distance_t move);

int ui_sprite_2d_wait_move_distance_set_distance(ui_sprite_2d_wait_move_distance_t wait_move_distance, const char * distaicne);
const char * ui_sprite_2d_wait_move_distance_distance(ui_sprite_2d_wait_move_distance_t wait_move_distance);

#ifdef __cplusplus
}
#endif

#endif
