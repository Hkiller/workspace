#ifndef UI_SPRITE_FSM_ACTION_FSM_H
#define UI_SPRITE_FSM_ACTION_FSM_H
#include "ui_sprite_fsm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_FSM_ACTION_FSM_NAME;

ui_sprite_fsm_action_fsm_t ui_sprite_fsm_action_fsm_create(ui_sprite_fsm_state_t fsm_state, const char * name);

dr_data_t ui_sprite_fsm_action_fsm_data(ui_sprite_fsm_action_fsm_t action_fsm);
int ui_sprite_fsm_action_fsm_set_data(ui_sprite_fsm_action_fsm_t action_fsm, dr_data_t data);
int ui_sprite_fsm_action_fsm_init_data(ui_sprite_fsm_action_fsm_t action_fsm, LPDRMETA data_meta, size_t data_capacity);

const char * ui_sprite_fsm_action_fsm_load_from(ui_sprite_fsm_action_fsm_t action_fsm);
int ui_sprite_fsm_action_fsm_set_load_from(ui_sprite_fsm_action_fsm_t action_fsm, const char * load_from);
    
#ifdef __cplusplus
}
#endif

#endif
