#ifndef UI_SPRITE_UI_ACTION_SHOW_POPUP_H
#define UI_SPRITE_UI_ACTION_SHOW_POPUP_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_SHOW_POPUP_NAME;

ui_sprite_ui_action_show_popup_t ui_sprite_ui_action_show_popup_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_show_popup_free(ui_sprite_ui_action_show_popup_t show_popup);

int ui_sprite_ui_action_show_popup_set_popup_name(ui_sprite_ui_action_show_popup_t show_popup, const char * popup_name);
int ui_sprite_ui_action_show_popup_set_before_popup_name(ui_sprite_ui_action_show_popup_t show_popup, const char * before_popup_name);    

#ifdef __cplusplus
}
#endif

#endif
