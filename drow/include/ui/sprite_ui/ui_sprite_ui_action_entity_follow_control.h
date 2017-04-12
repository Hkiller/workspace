#ifndef UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_H
#define UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_ENTITY_FOLLOW_CONTROL_NAME;
    
ui_sprite_ui_action_entity_follow_control_t ui_sprite_ui_action_entity_follow_control_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_entity_follow_control_free(ui_sprite_ui_action_entity_follow_control_t action_entity_follow_control);

#ifdef __cplusplus
}
#endif

#endif
