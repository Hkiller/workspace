#ifndef UI_SPRITE_BASIC_WAIT_EVENT_H
#define UI_SPRITE_BASIC_WAIT_EVENT_H
#include "ui_sprite_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BASIC_WAIT_EVENT_NAME;

ui_sprite_basic_wait_event_t ui_sprite_basic_wait_event_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_basic_wait_event_free(ui_sprite_basic_wait_event_t wait_evt);

const char * ui_sprite_basic_wait_event_event(ui_sprite_basic_wait_event_t wait_evt);
int ui_sprite_basic_wait_event_set_event(ui_sprite_basic_wait_event_t wait_evt, const char * evt);

const char * ui_sprite_basic_wait_event_condition(ui_sprite_basic_wait_event_t wait_evt);
int ui_sprite_basic_wait_event_set_condition(ui_sprite_basic_wait_event_t wait_evt, const char * evt);

#ifdef __cplusplus
}
#endif

#endif
