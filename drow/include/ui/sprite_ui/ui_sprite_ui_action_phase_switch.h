#ifndef UI_SPRITE_UI_ACTION_PHASE_SWITCH_H
#define UI_SPRITE_UI_ACTION_PHASE_SWITCH_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_PHASE_SWITCH_NAME;

ui_sprite_ui_action_phase_switch_t ui_sprite_ui_action_phase_switch_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_phase_switch_free(ui_sprite_ui_action_phase_switch_t action_phase_switch);

const char * ui_sprite_ui_action_phase_switch_to(ui_sprite_ui_action_phase_switch_t action_phase_switch);
void ui_sprite_ui_action_phase_switch_set_to(ui_sprite_ui_action_phase_switch_t action_phase_switch, const char * to);

const char * ui_sprite_ui_action_phase_switch_loading(ui_sprite_ui_action_phase_switch_t action_phase_switch);
void ui_sprite_ui_action_phase_switch_set_loading(ui_sprite_ui_action_phase_switch_t action_phase_switch, const char * loading);
    
#ifdef __cplusplus
}
#endif

#endif
