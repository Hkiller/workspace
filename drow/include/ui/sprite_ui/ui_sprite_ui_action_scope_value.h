#ifndef UI_SPRITE_UI_ACTION_SCOPE_VALUE_H
#define UI_SPRITE_UI_ACTION_SCOPE_VALUE_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_SCOPE_VALUE_NAME;

ui_sprite_ui_action_scope_value_t ui_sprite_ui_action_scope_value_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_scope_value_free(ui_sprite_ui_action_scope_value_t scope_value);

#ifdef __cplusplus
}
#endif

#endif
