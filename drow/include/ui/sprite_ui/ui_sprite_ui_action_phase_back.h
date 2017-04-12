#ifndef UI_SPRITE_UI_ACTION_PHASE_BACK_H
#define UI_SPRITE_UI_ACTION_PHASE_BACK_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_PHASE_BACK_TYPE_NAME;

ui_sprite_ui_action_phase_back_t ui_sprite_ui_action_phase_back_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_phase_back_free(ui_sprite_ui_action_phase_back_t action_phase_back);

const char * ui_sprite_ui_action_phase_back_phase(ui_sprite_ui_action_phase_back_t action_phase_back);
void ui_sprite_ui_action_phase_back_set_phase(ui_sprite_ui_action_phase_back_t action_phase_back, const char * phase);

const char * ui_sprite_ui_action_phase_back_load(ui_sprite_ui_action_phase_back_t action_phase_back);
void ui_sprite_ui_action_phase_back_set_load(ui_sprite_ui_action_phase_back_t action_phase_back, const char * phase);
    
#ifdef __cplusplus
}
#endif

#endif
