#ifndef UI_SPRITE_SCROLLMAP_SET_SPEED_H
#define UI_SPRITE_SCROLLMAP_SET_SPEED_H
#include "ui_sprite_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SCROLLMAP_SET_SPEED_NAME;

ui_sprite_scrollmap_set_speed_t ui_sprite_scrollmap_set_speed_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_scrollmap_set_speed_free(ui_sprite_scrollmap_set_speed_t send_evt);

float ui_sprite_scrollmap_set_speed_to_speed(ui_sprite_scrollmap_set_speed_t set_speed);
void ui_sprite_scrollmap_set_speed_set_to_speed(ui_sprite_scrollmap_set_speed_t set_speed, float to_speed);

float ui_sprite_scrollmap_set_speed_acceleration(ui_sprite_scrollmap_set_speed_t set_speed);
void ui_sprite_scrollmap_set_speed_set_acceleration(ui_sprite_scrollmap_set_speed_t set_speed, float acceleration);

#ifdef __cplusplus
}
#endif

#endif
