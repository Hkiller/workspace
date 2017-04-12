#ifndef UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_H
#define UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_H
#include "ui_sprite_moving_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_NAME;

ui_sprite_moving_move_set_to_begin_t ui_sprite_moving_move_set_to_begin_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_moving_move_set_to_begin_free(ui_sprite_moving_move_set_to_begin_t send_evt);

const char * ui_sprite_moving_move_set_to_begin_res(ui_sprite_moving_move_set_to_begin_t move_set_to_begin);
int ui_sprite_moving_move_set_to_begin_set_res(ui_sprite_moving_move_set_to_begin_t move_set_to_begin, const char * res);

const char * ui_sprite_moving_move_set_to_begin_node_name(ui_sprite_moving_move_set_to_begin_t move_set_to_begin);
int ui_sprite_moving_move_set_to_begin_set_node_name(ui_sprite_moving_move_set_to_begin_t move_set_to_begin, const char * node_name);

ui_sprite_moving_policy_t ui_sprite_moving_move_set_to_begin_move_policy(ui_sprite_moving_move_set_to_begin_t move_set_to_begin);
void ui_sprite_moving_move_set_to_begin_set_move_policy(ui_sprite_moving_move_set_to_begin_t move_set_to_begin, ui_sprite_moving_policy_t policy);
    
#ifdef __cplusplus
}
#endif

#endif
