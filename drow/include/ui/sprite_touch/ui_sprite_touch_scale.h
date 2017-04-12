#ifndef UI_SPRITE_TOUCH_SCALE_H
#define UI_SPRITE_TOUCH_SCALE_H
#include "ui_sprite_touch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_TOUCH_SCALE_NAME;

ui_sprite_touch_scale_t ui_sprite_touch_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_touch_scale_free(ui_sprite_touch_scale_t scale);

uint8_t ui_sprite_touch_scale_finger_count(ui_sprite_touch_scale_t scale);
int ui_sprite_touch_scale_set_finger_count(ui_sprite_touch_scale_t scale, uint8_t finger_count);

uint8_t ui_sprite_touch_scale_is_capture(ui_sprite_touch_scale_t scale);
int ui_sprite_touch_scale_set_is_capture(ui_sprite_touch_scale_t scale, uint8_t is_capture);

uint8_t ui_sprite_touch_scale_is_grab(ui_sprite_touch_scale_t scale);
int ui_sprite_touch_scale_set_is_grab(ui_sprite_touch_scale_t scale, uint8_t is_grab);

float ui_sprite_touch_scale_z(ui_sprite_touch_scale_t scale);
int ui_sprite_touch_scale_set_z(ui_sprite_touch_scale_t scale, float z);

uint16_t ui_sprite_touch_scale_threshold(ui_sprite_touch_scale_t scale);
void ui_sprite_touch_scale_set_threshold(ui_sprite_touch_scale_t scale, uint16_t threshold);

const char * ui_sprite_touch_scale_on_begin(ui_sprite_touch_scale_t scale);
int ui_sprite_touch_scale_set_on_begin(ui_sprite_touch_scale_t scale, const char * on_begin);

const char * ui_sprite_touch_scale_on_scale(ui_sprite_touch_scale_t scale);
int ui_sprite_touch_scale_set_on_scale(ui_sprite_touch_scale_t scale, const char * on_scale);

const char * ui_sprite_touch_scale_on_end(ui_sprite_touch_scale_t scale);
int ui_sprite_touch_scale_set_on_end(ui_sprite_touch_scale_t scale, const char * on_end);

const char * ui_sprite_touch_scale_on_cancel(ui_sprite_touch_scale_t scale);
int ui_sprite_touch_scale_set_on_cancel(ui_sprite_touch_scale_t scale, const char * on_cancel);

#ifdef __cplusplus
}
#endif

#endif
