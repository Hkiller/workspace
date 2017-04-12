#ifndef UI_SPRITE_MOVING_MOVE_BY_PLAN_H
#define UI_SPRITE_MOVING_MOVE_BY_PLAN_H
#include "ui_sprite_moving_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_MOVING_MOVE_BY_PLAN_NAME;

ui_sprite_moving_move_by_plan_t ui_sprite_moving_move_by_plan_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_moving_move_by_plan_free(ui_sprite_moving_move_by_plan_t send_evt);

const char * ui_sprite_moving_move_by_plan_res(ui_sprite_moving_move_by_plan_t move_by_plan);
int ui_sprite_moving_move_by_plan_set_res(ui_sprite_moving_move_by_plan_t move_by_plan, const char * res);

const char * ui_sprite_moving_move_by_plan_node_name(ui_sprite_moving_move_by_plan_t move_by_plan);
int ui_sprite_moving_move_by_plan_set_node_name(ui_sprite_moving_move_by_plan_t move_by_plan, const char * node_name);

const char * ui_sprite_moving_move_by_plan_move_policy(ui_sprite_moving_move_by_plan_t move_by_plan);
int ui_sprite_moving_move_by_plan_set_move_policy(ui_sprite_moving_move_by_plan_t move_by_plan, const char * move_policy);

const char * ui_sprite_moving_move_by_plan_loop_count(ui_sprite_moving_move_by_plan_t move_by_plan);
int ui_sprite_moving_move_by_plan_set_loop_count(ui_sprite_moving_move_by_plan_t move_by_plan, const char * loop_count);

#ifdef __cplusplus
}
#endif

#endif
