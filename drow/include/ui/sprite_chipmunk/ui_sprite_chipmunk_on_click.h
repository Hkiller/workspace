#ifndef UI_SPRITE_CHIPMUNK_ON_CLICK_H
#define UI_SPRITE_CHIPMUNK_ON_CLICK_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_ON_CLICK_NAME;

ui_sprite_chipmunk_on_click_t ui_sprite_chipmunk_on_click_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_chipmunk_on_click_free(ui_sprite_chipmunk_on_click_t click);

uint8_t ui_sprite_chipmunk_on_click_finger_count(ui_sprite_chipmunk_on_click_t click);
int ui_sprite_chipmunk_on_click_set_finger_count(ui_sprite_chipmunk_on_click_t click, uint8_t finger_count);

uint8_t ui_sprite_chipmunk_on_click_is_grab(ui_sprite_chipmunk_on_click_t click);
int ui_sprite_chipmunk_on_click_set_is_grab(ui_sprite_chipmunk_on_click_t click, uint8_t is_grab);

float ui_sprite_chipmunk_on_click_z(ui_sprite_chipmunk_on_click_t click);
int ui_sprite_chipmunk_on_click_set_z(ui_sprite_chipmunk_on_click_t click, float z);
    
const char * ui_sprite_chipmunk_on_click_on_click_down(ui_sprite_chipmunk_on_click_t click);
int ui_sprite_chipmunk_on_click_set_on_click_down(ui_sprite_chipmunk_on_click_t click, const char * on_click_down);

const char * ui_sprite_chipmunk_on_click_on_click_up(ui_sprite_chipmunk_on_click_t click);
int ui_sprite_chipmunk_on_click_set_on_click_up(ui_sprite_chipmunk_on_click_t click, const char * on_click_up);

#ifdef __cplusplus
}
#endif

#endif
