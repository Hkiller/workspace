#ifndef UI_SPRITE_UI_ACTION_SEND_EVENT_H
#define UI_SPRITE_UI_ACTION_SEND_EVENT_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_SEND_EVENT_NAME;

ui_sprite_ui_action_send_event_t ui_sprite_ui_action_send_event_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_send_event_free(ui_sprite_ui_action_send_event_t send_event);

int ui_sprite_ui_action_send_event_set_page_name(ui_sprite_ui_action_send_event_t send_event, const char * page_name);
int ui_sprite_ui_action_send_event_set_on_enter(ui_sprite_ui_action_send_event_t send_event, const char * on_enter);
int ui_sprite_ui_action_send_event_set_on_exit(ui_sprite_ui_action_send_event_t send_event, const char * on_exit);
    
#ifdef __cplusplus
}
#endif

#endif
