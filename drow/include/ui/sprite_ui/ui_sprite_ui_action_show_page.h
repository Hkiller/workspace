#ifndef UI_SPRITE_UI_ACTION_SHOW_PAGE_H
#define UI_SPRITE_UI_ACTION_SHOW_PAGE_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_SHOW_PAGE_NAME;

ui_sprite_ui_action_show_page_t ui_sprite_ui_action_show_page_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_show_page_free(ui_sprite_ui_action_show_page_t show_page);

int ui_sprite_ui_action_show_page_set_page_name(ui_sprite_ui_action_show_page_t show_page, const char * page_name);
int ui_sprite_ui_action_show_page_set_before_page_name(ui_sprite_ui_action_show_page_t show_page, const char * before_page_name);    

#ifdef __cplusplus
}
#endif

#endif
