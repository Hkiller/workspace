#ifndef UI_SPRITE_TRI_ACTION_SEND_EVENT_H
#define UI_SPRITE_TRI_ACTION_SEND_EVENT_H
#include "ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_TRI_ACTION_SEND_EVENT;

ui_sprite_tri_action_send_event_t ui_sprite_tri_action_send_event_create(ui_sprite_tri_rule_t rule);
void ui_sprite_tri_action_send_event_free(ui_sprite_tri_action_send_event_t send_event);

#ifdef __cplusplus
}
#endif

#endif
