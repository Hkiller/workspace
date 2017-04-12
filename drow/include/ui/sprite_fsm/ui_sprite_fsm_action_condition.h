#ifndef UI_SPRITE_FSM_ACTION_CONDITION_H
#define UI_SPRITE_FSM_ACTION_CONDITION_H
#include "ui_sprite_fsm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_FSM_ACTION_FSM_NAME;

ui_sprite_fsm_ins_t ui_sprite_fsm_action_fsm_create(ui_sprite_fsm_state_t fsm_state, const char * name);

#ifdef __cplusplus
}
#endif

#endif
