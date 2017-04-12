#ifndef UI_SPRITE_2D_WAIT_STOP_H
#define UI_SPRITE_2D_WAIT_STOP_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_2D_WAIT_STOP_NAME;

ui_sprite_2d_wait_stop_t ui_sprite_2d_wait_stop_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_2d_wait_stop_free(ui_sprite_2d_wait_stop_t wait_stop);

float ui_sprite_2d_wait_stop_threshold(ui_sprite_2d_wait_stop_t wait_stop);
void ui_sprite_2d_wait_stop_set_threshold(ui_sprite_2d_wait_stop_t wait_stop, float threshold);
    
#ifdef __cplusplus
}
#endif

#endif
