#ifndef UI_SPRITE_TOUCH_MOVE_H
#define UI_SPRITE_TOUCH_MOVE_H
#include "ui_sprite_touch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_TOUCH_MOVE_NAME;

ui_sprite_touch_move_t ui_sprite_touch_move_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_touch_move_free(ui_sprite_touch_move_t move);

uint8_t ui_sprite_touch_move_finger_count(ui_sprite_touch_move_t move);
int ui_sprite_touch_move_set_finger_count(ui_sprite_touch_move_t move, uint8_t finger_count);

uint8_t ui_sprite_touch_move_is_capture(ui_sprite_touch_move_t move);
int ui_sprite_touch_move_set_is_capture(ui_sprite_touch_move_t move, uint8_t is_capture);

uint8_t ui_sprite_touch_move_is_grab(ui_sprite_touch_move_t move);
int ui_sprite_touch_move_set_is_grab(ui_sprite_touch_move_t move, uint8_t is_grab);

float ui_sprite_touch_move_z(ui_sprite_touch_move_t move);
int ui_sprite_touch_move_set_z(ui_sprite_touch_move_t move, float z);
    
uint16_t ui_sprite_touch_move_threshold(ui_sprite_touch_move_t move);
void ui_sprite_touch_move_set_threshold(ui_sprite_touch_move_t move, uint16_t threshold);

const char * ui_sprite_touch_move_on_begin(ui_sprite_touch_move_t move);
int ui_sprite_touch_move_set_on_begin(ui_sprite_touch_move_t move, const char * on_begin);

const char * ui_sprite_touch_move_on_move(ui_sprite_touch_move_t move);
int ui_sprite_touch_move_set_on_move(ui_sprite_touch_move_t move, const char * on_move);

const char * ui_sprite_touch_move_on_end(ui_sprite_touch_move_t move);
int ui_sprite_touch_move_set_on_end(ui_sprite_touch_move_t move, const char * on_end);

const char * ui_sprite_touch_move_on_cancel(ui_sprite_touch_move_t move);
int ui_sprite_touch_move_set_on_cancel(ui_sprite_touch_move_t move, const char * on_cancel);

float ui_sprite_touch_move_stick_duration(ui_sprite_touch_move_t move);
void ui_sprite_touch_move_set_stick_duration(ui_sprite_touch_move_t move, float stick_duration);

#ifdef __cplusplus
}
#endif

#endif
