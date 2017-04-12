#ifndef UI_SPRITE_UI_ACTION_GUARD_PACKAGE_H
#define UI_SPRITE_UI_ACTION_GUARD_PACKAGE_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_GUARD_PACKAGE_NAME;

ui_sprite_ui_action_guard_package_t ui_sprite_ui_action_guard_package_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_guard_package_free(ui_sprite_ui_action_guard_package_t guard_package);
    
#ifdef __cplusplus
}
#endif

#endif
