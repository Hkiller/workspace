#ifndef UI_SPRITE_UI_ACTION_NAVIGATION_H
#define UI_SPRITE_UI_ACTION_NAVIGATION_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_NAVIGATION_NAME;

ui_sprite_ui_action_navigation_t ui_sprite_ui_action_navigation_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_navigation_free(ui_sprite_ui_action_navigation_t navigation);

int ui_sprite_ui_action_navigation_set_event(
    ui_sprite_ui_action_navigation_t navigation, const char * event , const char * condition);

void ui_sprite_ui_action_navigation_set_navigation(
    ui_sprite_ui_action_navigation_t action_navigation, plugin_ui_navigation_t navigation);
    
#ifdef __cplusplus
}
#endif

#endif
