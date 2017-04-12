#ifndef UI_SPRITE_SCROLLMAP_CANCEL_LOOP_H
#define UI_SPRITE_SCROLLMAP_CANCEL_LOOP_H
#include "ui_sprite_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SCROLLMAP_CANCEL_LOOP_NAME;

ui_sprite_scrollmap_cancel_loop_t ui_sprite_scrollmap_cancel_loop_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_scrollmap_cancel_loop_free(ui_sprite_scrollmap_cancel_loop_t send_evt);

float ui_sprite_scrollmap_cancel_loop_to_speed(ui_sprite_scrollmap_cancel_loop_t cancel_loop);
void ui_sprite_scrollmap_cancel_loop_set_to_speed(ui_sprite_scrollmap_cancel_loop_t cancel_loop, float to_speed);

float ui_sprite_scrollmap_cancel_loop_acceleration(ui_sprite_scrollmap_cancel_loop_t cancel_loop);
void ui_sprite_scrollmap_cancel_loop_set_acceleration(ui_sprite_scrollmap_cancel_loop_t cancel_loop, float acceleration);

#ifdef __cplusplus
}
#endif

#endif
