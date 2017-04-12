#ifndef UI_SPRITE_BASIC_SEND_EVENT_H
#define UI_SPRITE_BASIC_SEND_EVENT_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_SEND_EVENT_NAME;

ui_sprite_basic_send_event_t ui_sprite_basic_send_event_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_send_event_free(ui_sprite_basic_send_event_t send_evt);

const char * ui_sprite_basic_send_event_on_enter(ui_sprite_basic_send_event_t send_evt);
int ui_sprite_basic_send_event_set_on_enter(ui_sprite_basic_send_event_t send_evt, const char * evt);

const char * ui_sprite_basic_send_event_on_exist(ui_sprite_basic_send_event_t send_evt);
int ui_sprite_basic_send_event_set_on_exit(ui_sprite_basic_send_event_t send_evt, const char * evt);

#ifdef __cplusplus
}
#endif

#endif
